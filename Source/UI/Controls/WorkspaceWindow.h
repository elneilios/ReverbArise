#pragma once
#include "SurfaceLifecycle.h"
#include "PresentationSurface.h"
#include "FocusViewport.h"

namespace m3::ui::controls
{
class WorkspaceWindow final:public juce::DocumentWindow
{
public:
    WorkspaceWindow(const juce::String& title,juce::Component& content,int width,int height,std::function<void()> closed,int scale=0)
        :DocumentWindow(title,m3::ui::background,DocumentWindow::closeButton),onClose(std::move(closed)),logicalWidth(width),logicalHeight(height),scalable(scale!=0)
    {
        setUsingNativeTitleBar(true);setResizable(false,false);
        if(scalable)
        {
            setLookAndFeel(&comfortableTheme);
            surface.contentRoot().addAndMakeVisible(content);content.setBounds(0,0,width,height);
            viewport.setName("auxiliaryPresentationViewport");viewport.setFocusRevealEnabled(true);viewport.setViewedComponent(&surface,false);
            setContentNonOwned(&viewport,false);setPresentationScale(scale);
        }
        else{setContentNonOwned(&content,false);setSize(width,height);}
    }
    ~WorkspaceWindow()override{setLookAndFeel(nullptr);}
    void setReturnFocus(SurfaceFocusReturn value){returnFocus=std::move(value);}
    bool keyPressed(const juce::KeyPress& key)override
    {if(escape.keyPressed(key,juce::Component::getCurrentlyFocusedComponent()))return true;return DocumentWindow::keyPressed(key);}
    void closeButtonPressed()override
    {
        if(!isVisible())return;
        auto focus=returnFocus;focus.useCurrentInput();setVisible(false);
        const auto closed=onClose;if(closed)closed();focus.post();
    }
    void setPresentationScale(int scale)
    {
        if(!scalable)return;
        const auto* display=juce::Desktop::getInstance().getDisplays().getDisplayForRect(getScreenBounds());
        if(!display)return;
        const auto work=display->userBounds.toNearestInt();juce::BorderSize<int> frame;
        if(const auto* peer=getPeer())if(const auto measured=peer->getFrameSizeIfPresent())frame=*measured;
        if(scale==lastScale && work==lastWork && frame==lastFrame)return;
        lastScale=scale;lastWork=work;lastFrame=frame;
        const auto width=(logicalWidth*scale+99)/100,height=(logicalHeight*scale+99)/100;
        surface.present(logicalWidth,logicalHeight,scale);surface.setSize(width,height);
        const auto border=getContentComponentBorder();
        const juce::Rectangle<int> available{0,0,std::max(1,work.getWidth()-frame.getLeftAndRight()-border.getLeftAndRight()),std::max(1,work.getHeight()-frame.getTopAndBottom()-border.getTopAndBottom())};
        const auto scroll=viewport.getScrollBarThickness();
        setSize(std::min(width+(height>available.getHeight()?scroll:0),available.getWidth())+border.getLeftAndRight(),std::min(height+(width>available.getWidth()?scroll:0),available.getHeight())+border.getTopAndBottom());
        if(isVisible())setTopLeftPosition(juce::jlimit(work.getX()+frame.getLeft(),std::max(work.getX()+frame.getLeft(),work.getRight()-getWidth()-frame.getRight()),getX()),
            juce::jlimit(work.getY()+frame.getTop(),std::max(work.getY()+frame.getTop(),work.getBottom()-getHeight()-frame.getBottom()),getY()));
    }
private:
    SurfaceFocusReturn returnFocus;
    SurfaceEscape escape{*this,[this]{closeButtonPressed();}};
    std::function<void()> onClose;
    int logicalWidth,logicalHeight,lastScale=0;bool scalable;
    juce::Rectangle<int> lastWork;juce::BorderSize<int> lastFrame;
    m3::ui::controls::PresentationSurface surface;
    m3::ui::controls::FocusViewport viewport;
    m3::ui::WorkspaceTheme comfortableTheme;
};
}
