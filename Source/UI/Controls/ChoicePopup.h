#pragma once
#include "PopupInteraction.h"
#include "SelectorPresentation.h"
#include "FocusVisibility.h"
#include <memory>

namespace m3::ui::controls
{
enum class PopupDismissal { commit,cancel,outside,tabForward,tabBackward,ownerGone };

// One-level, opt-in popup presentation. Rows may be navigated while unavailable;
// the immutable snapshot and original binding still decide activation authority.
class ChoicePopup final:public juce::Component,private juce::Timer
{
    class Row final:public juce::Component
    {
    public:
        Row(ChoicePopup& p,int i):owner(p),index(i)
        {setInterceptsMouseClicks(false,false);setEnabled(item().eligible);setTitle(item().label);setDescription(item().reason);}
        const PopupChoice& item()const{return owner.interaction.items()[static_cast<size_t>(index)];}
        juce::TextLayout textLayout(int width)const
        {
            juce::AttributedString textBlock;
            textBlock.setWordWrap(juce::AttributedString::byWord);
            const auto ink = isDestructive() ? danger : text;
            textBlock.append(item().label,owner.rowFont,item().eligible?ink:ink.withMultipliedAlpha(.55f));
            if(item().reason.isNotEmpty())textBlock.append("\n"+item().reason,owner.reasonFont,textStyle(TextRole::helper).ink);
            juce::TextLayout result;result.createLayout(textBlock,static_cast<float>(juce::jmax(1,width)));return result;
        }
        int heightFor(int width)const
        {return juce::jmax(owner.unit(controlGeometry::height),juce::roundToInt(std::ceil(textLayout(width-2*owner.unit(SelectorPresentation::rowPadding)-checkColumnWidth()).getHeight()))+2*owner.unit(SelectorPresentation::rowPadding));}
        void paint(juce::Graphics& g)override
        {
            const auto active=owner.interaction.inspected()==index;
            if(active)
            {
                const bool held = owner.interaction.heldIndex() == index && item().eligible;
                g.setColour(inspectedFill(held));
                g.fillRect(getLocalBounds());
                g.setColour(isDestructive() ? danger : aqua);
                g.fillRect(getLocalBounds().removeFromLeft(owner.unit(controlGeometry::focusStroke)));
            }
            auto area=getLocalBounds().reduced(owner.unit(SelectorPresentation::rowPadding));
            const auto check=area.removeFromLeft(checkColumnWidth());
            if(item().hasSelectionMarker())
            {
                const auto centre=check.toFloat().getCentre();juce::Path tick;
                tick.startNewSubPath(centre.x-owner.unit(5),centre.y);tick.lineTo(centre.x-owner.unit(1),centre.y+owner.unit(4));tick.lineTo(centre.x+owner.unit(6),centre.y-owner.unit(5));
                g.setColour(text);g.strokePath(tick,juce::PathStrokeType(rule*2*owner.magnification));
            }
            textLayout(area.getWidth()).draw(g,area.toFloat());
        }
        std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler()override
        {
            class Handler final:public juce::AccessibilityHandler
            {
            public:
                explicit Handler(Row& row):juce::AccessibilityHandler(row,juce::AccessibilityRole::menuItem,
                    juce::AccessibilityActions{}
                        .addAction(juce::AccessibilityActionType::focus,[&row]{row.owner.inspect(row.index,true);})
                        .addAction(juce::AccessibilityActionType::press,[&row]{row.owner.visibility->accessibilityInput();row.owner.activate(row.index);})),target(row){}
                juce::AccessibleState getCurrentState()const override
                {
                    auto state=juce::AccessibleState{}.withFocusable();
                    if(target.item().representsChoice())state=state.withSelectable();
                    if(target.item().hasSelectionMarker())state=state.withSelected();
                    if(target.owner.interaction.keyboardIndex()==target.index && target.owner.hasKeyboardFocus(true))state=state.withFocused();
                    return state;
                }
                juce::String getHelp()const override{return target.item().reason;}
            private:Row& target;
            };
            return std::make_unique<Handler>(*this);
        }
    private:
        bool isDestructive() const noexcept { return item().role == PopupRowRole::destructiveAction; }
        int checkColumnWidth() const noexcept
        {
            return item().representsChoice() ? owner.unit(SelectorPresentation::checkWidth) : 0;
        }
        juce::Colour inspectedFill(bool held) const
        {
            if (isDestructive())
            {
                // Popup-only blends for the scoped native review.
                constexpr float destructiveHoverMix = .14f, destructiveHeldMix = .24f;
                return background.interpolatedWith(danger, held ? destructiveHeldMix : destructiveHoverMix);
            }
            return held ? SelectorPresentation::held() : SelectorPresentation::raised();
        }
        ChoicePopup& owner;
        int index;
    };
public:
    ChoicePopup(juce::Component& source,std::vector<PopupChoice> items,std::function<void(int,PopupDismissal)> result)
        :origin(&source),interaction(std::move(items)),completion(std::move(result)),
         rowFont(SelectorPresentation::valueFont(source)),reasonFont(componentTextFont(source,TextRole::helper))
    {
        magnification=source.getWidth()>0?static_cast<float>(source.getScreenBounds().getWidth())/source.getWidth():1.f;
        rowFont=rowFont.withHeight(rowFont.getHeight()*magnification);reasonFont=reasonFont.withHeight(reasonFont.getHeight()*magnification);
        setName("selectorChoicePopup");setTitle(source.getTitle());setWantsKeyboardFocus(true);setOpaque(true);
        setLookAndFeel(&theme);
        addAndMakeVisible(viewport);viewport.setViewedComponent(&content,false);viewport.setScrollBarsShown(true,false);
        for(size_t i=0;i<interaction.items().size();++i){auto row=std::make_unique<Row>(*this,static_cast<int>(i));content.addAndMakeVisible(*row);rows.push_back(std::move(row));}
        addMouseListener(this,true);
    }
    ~ChoicePopup()override{stopTimer();exitModalState();removeMouseListener(this);setLookAndFeel(nullptr);}
    const PopupInteraction& state()const noexcept{return interaction;}
    juce::Component& rowComponent(int index){return *rows.at(static_cast<size_t>(index));}
    static juce::Rectangle<int> placedBounds(juce::Rectangle<int> anchor,juce::Rectangle<int> work,int width,int height)
    {
        width=juce::jlimit(1,juce::jmax(1,work.getWidth()),width);height=juce::jlimit(1,juce::jmax(1,work.getHeight()),height);
        const auto below=anchor.getBottom()+spacing::tight;
        const auto y=below+height<=work.getBottom()?below:anchor.getY()-spacing::tight-height;
        return {juce::jlimit(work.getX(),work.getRight()-width,anchor.getX()),juce::jlimit(work.getY(),work.getBottom()-height,y),width,height};
    }
    void show(juce::Rectangle<int> anchor,bool keyboard)
    {
        auto* display=juce::Desktop::getInstance().getDisplays().getDisplayForRect(anchor);
        const auto work=display?display->userArea:anchor.expanded(SelectorPresentation::popupMaximumWidth);
        const auto width=juce::jmin(work.getWidth(),juce::jlimit(unit(SelectorPresentation::popupMinimumWidth),unit(SelectorPresentation::popupMaximumWidth),anchor.getWidth()));
        int height=2*unit(SelectorPresentation::popupPadding);
        for(auto& row:rows)height+=row->heightFor(width-2*unit(SelectorPresentation::popupPadding)-viewport.getScrollBarThickness());
        setBounds(placedBounds(anchor,work,width,height));
        addToDesktop(juce::ComponentPeer::windowIsTemporary);setVisible(true);enterModalState(true);startTimerHz(20);
        if(keyboard && !rows.empty())
        {
            int index=0;for(size_t i=0;i<interaction.items().size();++i)if(interaction.items()[i].hasSelectionMarker())index=static_cast<int>(i);
            inspect(index,false);
        }
    }
    void resized()override
    {
        viewport.setBounds(getLocalBounds().reduced(unit(SelectorPresentation::popupPadding)));
        const auto width=juce::jmax(1,viewport.getWidth()-viewport.getScrollBarThickness());int y=0;
        for(auto& row:rows){const auto height=row->heightFor(width);row->setBounds(0,y,width,height);y+=height;}
        content.setSize(width,y);
    }
    void paint(juce::Graphics& g)override
    {g.fillAll(background);g.setColour(outline);g.drawRect(getLocalBounds(),rule);}
    void inspect(int index,bool accessibility=false)
    {
        if(!interaction.valid(index) || interaction.done())return;
        if(accessibility)visibility->accessibilityInput();else visibility->keyboardInput();
        if(origin!=nullptr)origin->repaint();
        interaction.keyboardInspect(index);grabKeyboardFocus();
        const auto bounds=rows[static_cast<size_t>(index)]->getBounds();
        if(bounds.getY()<viewport.getViewPositionY())viewport.setViewPosition(0,bounds.getY());
        else if(bounds.getBottom()>viewport.getViewPositionY()+viewport.getViewHeight())viewport.setViewPosition(0,bounds.getBottom()-viewport.getViewHeight());
        repaintRows();
        if(auto* handler=rows[static_cast<size_t>(index)]->getAccessibilityHandler())handler->grabFocus();
    }
    void activate(int index)
    {
        if(origin==nullptr || !origin->isEnabled()){dismiss(PopupDismissal::ownerGone);return;}
        const auto result=interaction.complete(index);if(result!=0)finish(result,PopupDismissal::commit);
    }
    void dismiss(PopupDismissal why=PopupDismissal::cancel){if(interaction.done())return;interaction.cancel();finish(0,why);}
    bool showFocusOnReturn()const noexcept{return dismissalFocusVisible;}
    bool keyPressed(const juce::KeyPress& key)override
    {
        visibility->keyboardInput();const auto code=key.getKeyCode();
        if(origin!=nullptr)origin->repaint();
        if(code==juce::KeyPress::escapeKey){dismiss();return true;}
        if(code==juce::KeyPress::tabKey){dismiss(key.getModifiers().isShiftDown()?PopupDismissal::tabBackward:PopupDismissal::tabForward);return true;}
        if(code==juce::KeyPress::returnKey || code==juce::KeyPress::spaceKey){activate(interaction.keyboardIndex());return true;}
        if(rows.empty())return true;
        const auto count=static_cast<int>(rows.size());const auto current=interaction.keyboardIndex();
        if(code==juce::KeyPress::homeKey){inspect(0);return true;}
        if(code==juce::KeyPress::endKey){inspect(count-1);return true;}
        if(code==juce::KeyPress::downKey || code==juce::KeyPress::upKey)
        {inspect(current<0?(code==juce::KeyPress::downKey?0:count-1):(current+(code==juce::KeyPress::downKey?1:-1)+count)%count);return true;}
        return false;
    }
    void inputAttemptWhenModal()override{visibility->pointerInput();dismiss(PopupDismissal::outside);}
    void focusLost(FocusChangeType) override
    {
        if (interaction.done())
            return;

        if (hasKeyboardFocus(true) || !isCurrentlyModal(false))
        {
            interaction.navigationLost();
            repaintRows();
            return;
        }

        // A native host control can take focus without a JUCE modal input attempt.
        // Cancel now and close after the native focus transfer has finished.
        interaction.cancel();
        const juce::Component::SafePointer<ChoicePopup> safe(this);
        juce::MessageManager::callAsync([safe]
        {
            if (safe != nullptr)
                safe->finish(0, PopupDismissal::outside);
        });
    }
    void mouseMove(const juce::MouseEvent& event)override{pointAt(event);}
    void mouseEnter(const juce::MouseEvent& event)override{pointAt(event);}
    void mouseExit(const juce::MouseEvent&)override{interaction.pointerExit();repaintRows();}
    void mouseDown(const juce::MouseEvent& event)override
    {visibility->pointerInput();if(origin!=nullptr)origin->repaint();const auto index=indexAt(event);interaction.pointerEnter(index);interaction.pointerDown(index);repaintRows();}
    void mouseUp(const juce::MouseEvent& event)override
    {const auto index=indexAt(event);const bool held=index>=0 && interaction.heldIndex()==index;interaction.pointerUp();repaintRows();if(held)activate(index);}
    void mouseDrag(const juce::MouseEvent& event)override{pointAt(event);}
private:
    int unit(float value)const noexcept{return juce::jmax(1,juce::roundToInt(value*magnification));}
    int indexAt(const juce::MouseEvent& event)
    {
        const auto point=event.getEventRelativeTo(&content).position;
        if(!viewport.getLocalBounds().contains(event.getEventRelativeTo(&viewport).position.toInt()))return -1;
        for(size_t i=0;i<rows.size();++i)if(rows[i]->getBounds().contains(point.toInt()))return static_cast<int>(i);
        return -1;
    }
    void pointAt(const juce::MouseEvent& event){interaction.pointerEnter(indexAt(event));repaintRows();}
    void repaintRows(){for(auto& row:rows)row->repaint();}
    void timerCallback()override
    {if(origin==nullptr || !origin->isShowing() || !origin->isEnabled())dismiss(PopupDismissal::ownerGone);else if(!juce::Process::isForegroundProcess())dismiss(PopupDismissal::outside);}
    void finish(int id,PopupDismissal why)
    {
        dismissalFocusVisible=visibility->isVisible();
        stopTimer();exitModalState();setVisible(false);
        if(origin!=nullptr)origin->repaint();
        auto callback=std::move(completion);
        if(callback)juce::MessageManager::callAsync([callback=std::move(callback),id,why]{callback(id,why);});
    }
    WorkspaceTheme theme;
    juce::Component::SafePointer<juce::Component> origin;
    PopupInteraction interaction;
    std::function<void(int,PopupDismissal)> completion;
    juce::Font rowFont,reasonFont;
    juce::Viewport viewport;
    juce::Component content;
    std::vector<std::unique_ptr<Row>> rows;
    juce::SharedResourcePointer<FocusVisibility> visibility;
    bool dismissalFocusVisible=false;
    float magnification=1.f;
};
}
