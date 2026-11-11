#pragma once
#include "CapturedActionButton.h"
namespace m3::ui::controls
{
// Generic native-file gesture surface. Bindings own file/revision/session safety;
// this control never opens, writes or retains a filesystem path.
class FileDragTile final:public CapturedActionButton
{
public:
    FileDragTile():CapturedActionButton("Drag WAV to Playlist")
    {
        setTooltip("Drag the ready WAV to your Playlist. Click or press Enter to Save WAV.");setHelpText(getTooltip());
        onClick=[this]{if(!dragged && onSave)onSave();};
    }
    void setStatusText(const juce::String& value){if(status!=value){status=value;setDescription(value);setTooltip(value+" | Drag when ready; click or press Enter to Save WAV.");setHelpText(getTooltip());repaint();}}
    void setDragFeedbackActive(bool active)
    {
        if(dragFeedbackActive==active)return;
        dragFeedbackActive=active;
        repaint();
    }
    bool hasSolidBorder() const
    {
        return hasCapturedInvocation() ? dragFeedbackActive || isFocusVisible() : hasKeyboardFocus(true);
    }
    std::function<void()> onDrag,onSave;
    std::function<void(PressId)> onGuardedDrag;
    void mouseDown(const juce::MouseEvent& e)override
    {
        if(!hasCapturedInvocation())dragged=false;
        CapturedActionButton::mouseDown(e);
    }
    bool keyPressed(const juce::KeyPress& key)override
    {
        if(!hasCapturedInvocation())dragged=false;
        return CapturedActionButton::keyPressed(key);
    }
    void mouseDrag(const juce::MouseEvent& e)override
    {
        CapturedActionButton::mouseDrag(e);
        if(hasCapturedInvocation())
        {
            const auto press=pointerPress();
            if(isEnabled() && ownsPress(e) && e.mods.isLeftButtonDown() && press!=draggedPress
               && e.getDistanceFromDragStart()>=4)
            {
                draggedPress=press;
                suppressPointerInvocation();
                if(onGuardedDrag)onGuardedDrag(press);
            }
            return;
        }
        if(isEnabled() && e.mods.isLeftButtonDown() && !dragged && e.getDistanceFromDragStart()>=4){dragged=true;if(onDrag)onDrag();}
    }
    void paintButton(juce::Graphics& g,bool over,bool down)override
    {
        const auto alpha=isEnabled()?1.f:.35f;const auto box=focusInkBounds();
        g.setColour(background);g.fillRoundedRectangle(box,radius);
        g.setColour((over || down?hover:aqua).withMultipliedAlpha(alpha));
        juce::Path border,dashes;border.addRoundedRectangle(box,radius);const float lengths[]{6,4};
        if(hasSolidBorder())g.strokePath(border,juce::PathStrokeType(controlGeometry::focusStroke));
        else {juce::PathStrokeType(1.5f).createDashedStroke(dashes,border,lengths,2);g.fillPath(dashes);}
        const auto c=box.getCentre();const auto top=box.getY()+std::max(14.f,(box.getHeight()-150)*.5f);
        g.setColour(m3::ui::text.withMultipliedAlpha(alpha));juce::Path file;file.startNewSubPath(c.x-25,top);file.lineTo(c.x+8,top);file.lineTo(c.x+25,top+17);
        file.lineTo(c.x+25,top+67);file.lineTo(c.x-25,top+67);file.closeSubPath();file.startNewSubPath(c.x+8,top);file.lineTo(c.x+8,top+17);file.lineTo(c.x+25,top+17);
        g.strokePath(file,juce::PathStrokeType(1.7f));
        for(int n=0;n<7;++n){const auto h=5.f+static_cast<float>((n*7)%13);g.drawLine(c.x-16+n*5.f,top+34-h*.5f,c.x-16+n*5.f,top+34+h*.5f,1.5f);}
        g.setColour(aqua.withMultipliedAlpha(alpha));g.fillRect(c.x-24,top+48,48.f,18.f);g.setColour(background.withMultipliedAlpha(alpha));g.setFont(interFont(bodySize,InterFace::semibold));g.drawText("WAV",juce::Rectangle<float>(c.x-24,top+48,48,18),juce::Justification::centred);
        g.setColour(m3::ui::text.withMultipliedAlpha(alpha));g.setFont(componentTextFont(*this,TextRole::sectionHeading));g.drawFittedText(getButtonText(),juce::Rectangle<int>(8,juce::roundToInt(top+83),getWidth()-16,30),juce::Justification::centred,1,.75f);
        g.setColour(status.isEmpty()?secondary.withMultipliedAlpha(alpha):textStyle(TextRole::helper).ink);g.setFont(status.isEmpty()?interFont(bodySize,InterFace::italic):componentTextFont(*this,TextRole::helper));g.drawText(status.isEmpty()?juce::String("Song-aligned ")+juce::String::charToString(0x2022)+" preserves timing":status,juce::Rectangle<float>(8,top+120,getWidth()-16.f,24),juce::Justification::centred);
    }
private:
    PressId draggedPress=0;
    bool dragFeedbackActive=false;
    bool dragged=false;juce::String status;
};
}
