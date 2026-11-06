#pragma once
#include <UI/WorkspaceTheme.h>
#include "ModeIcon.h"
#include <vector>

namespace m3::ui::controls
{
struct SegmentChoice
{
    int id=0;
    juce::String label,help;
    bool available=true;
    juce::String unavailableReason;
    ModeIcon icon=ModeIcon::none;
};

// Model-owned selection: an input emits intent, never an optimistic toggle.
class SegmentedSelector final:public juce::Component,public juce::SettableTooltipClient
{
    class Segment final:public juce::Button
    {
    public:
        Segment(SegmentedSelector& group,size_t position):juce::Button(juce::String{}),owner(group),index(position)
        {setWantsKeyboardFocus(false);setMouseClickGrabsKeyboardFocus(false);onClick=[this]{owner.request(index);};}
        void mouseDown(const juce::MouseEvent& e)override{owner.focusChoice(index);juce::Button::mouseDown(e);}
        void buttonStateChanged()override{owner.repaint();}
        void paintButton(juce::Graphics& g,bool,bool)override
        {
            const auto& choice=owner.choices[index];const auto activeChoice=choice.id==owner.selected;
            const auto alpha=isEnabled()?1.0f:.4f;const auto box=getLocalBounds().toFloat().reduced(.5f);
            const auto path=owner.outlineFor(index,box);
            g.setColour(activeChoice?aqua.withAlpha(alpha):background);g.fillPath(path);
            g.setColour((activeChoice?background:m3::ui::text).withAlpha(alpha));g.setFont(owner.labelFont());
            const auto content=getLocalBounds().toFloat().reduced(5,0);
            if(choice.icon==ModeIcon::none)g.drawText(choice.label,content,juce::Justification::centred);
            else
            {
                const auto labelWidth=juce::GlyphArrangement::getStringWidth(owner.labelFont(),choice.label);
                const auto width=16.0f+8.0f+labelWidth;
                const auto x=content.getCentreX()-width*.5f;
                owner.iconCache.draw(g,choice.icon,{x,content.getCentreY()-8,16,16},activeChoice,alpha);
                g.drawText(choice.label,juce::Rectangle<float>(x+24,content.getY(),labelWidth,content.getHeight()),juce::Justification::centredLeft);
            }
        }
        std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler()override
        {
            class Handler final:public juce::AccessibilityHandler
            {
            public:
                explicit Handler(Segment& value):juce::AccessibilityHandler(value,juce::AccessibilityRole::radioButton,
                    juce::AccessibilityActions().addAction(juce::AccessibilityActionType::press,[&value]{value.owner.request(value.index);})),segment(value){}
                juce::AccessibleState getCurrentState()const override
                {
                    auto state=juce::AccessibilityHandler::getCurrentState().withCheckable().withSelectable();
                    if(segment.owner.choices[segment.index].id==segment.owner.selected)state=state.withChecked().withSelected();
                    return state;
                }
            private:Segment& segment;
            };
            return std::make_unique<Handler>(*this);
        }
    private:
        SegmentedSelector& owner;size_t index;
    };
public:
    SegmentedSelector()
    {setWantsKeyboardFocus(true);setTitle("Mode selection");setFocusContainerType(juce::Component::FocusContainerType::focusContainer);}
    std::function<void(int)> onRequest;
    void setTextHeight(float height){textHeight=height;repaint();}
    void display(std::vector<SegmentChoice> next,int acceptedId)
    {
        bool rebuild=next.size()!=choices.size();
        if(!rebuild)for(size_t i=0;i<next.size();++i)rebuild|=next[i].id!=choices[i].id;
        choices=std::move(next);selected=acceptedId;
        if(rebuild)
        {
            buttons.clear();
            for(size_t i=0;i<choices.size();++i){auto button=std::make_unique<Segment>(*this,i);addAndMakeVisible(*button);buttons.push_back(std::move(button));}
        }
        if(!hasKeyboardFocus(true))for(size_t i=0;i<choices.size();++i)if(choices[i].id==selected)focused=i;
        if(focused>=choices.size())focused=0;
        juce::String description;
        for(size_t i=0;i<choices.size();++i)
        {
            const auto& item=choices[i];auto& button=*buttons[i];
            const auto hint=item.available?item.help:item.unavailableReason;
            button.setButtonText(item.label);button.setTitle(item.label);button.setName("segment-"+juce::String(item.id));
            button.setEnabled(item.available);button.setTooltip(hint);button.setDescription(hint);button.setHelpText(hint);
            if(description.isNotEmpty())description<<". ";description<<item.label<<": "<<hint;
            if(auto* handler=button.getAccessibilityHandler())handler->notifyAccessibilityEvent(juce::AccessibilityEvent::valueChanged);
        }
        setDescription(description);setHelpText("Left/right: inspect choices. Enter/Space: activate an available choice. "+description);
        updateHint();resized();repaint();
    }
    int selectedId()const noexcept{return selected;}
    int focusedId()const noexcept{return choices.empty()?0:choices[focused].id;}
    juce::Button* segment(int id)const noexcept
    {for(size_t i=0;i<choices.size();++i)if(choices[i].id==id)return buttons[i].get();return nullptr;}
    bool keyPressed(const juce::KeyPress& key)override
    {
        if(choices.empty())return false;
        if(key.isKeyCode(juce::KeyPress::leftKey) || key.isKeyCode(juce::KeyPress::rightKey)
            || key.isKeyCode(juce::KeyPress::homeKey) || key.isKeyCode(juce::KeyPress::endKey))
        {
            if(key.isKeyCode(juce::KeyPress::homeKey))focused=0;
            else if(key.isKeyCode(juce::KeyPress::endKey))focused=choices.size()-1;
            else focused=(focused+choices.size()+(key.isKeyCode(juce::KeyPress::leftKey)?-1:1))%choices.size();
            updateHint();repaint();return true;
        }
        if(key.isKeyCode(juce::KeyPress::returnKey) || key.isKeyCode(juce::KeyPress::spaceKey)){request(focused);return true;}
        return false;
    }
    void mouseWheelMove(const juce::MouseEvent&,const juce::MouseWheelDetails&)override{} // Navigation is deliberate, never wheel-selected.
    void focusGained(FocusChangeType)override{updateHint();repaint();}
    void focusLost(FocusChangeType)override{repaint();}
    void resized()override
    {
        if(buttons.empty())return;
        for(size_t i=0;i<buttons.size();++i){const auto a=juce::roundToInt(double(i)*getWidth()/buttons.size());
            const auto b=juce::roundToInt(double(i+1)*getWidth()/buttons.size());buttons[i]->setBounds(a,0,b-a+(i+1<buttons.size()?1:0),getHeight());}
    }
    void paintOverChildren(juce::Graphics& g)override
    {
        // The joined children overlap by one pixel. Paint their borders here,
        // with hover/press last, so a neighbouring selected fill cannot erase
        // the shared edge. Keep child/accessibility order and hit areas stable.
        const auto highlighted=[&](size_t index){const auto& button=*buttons[index];return button.isEnabled()
            && (button.getState()==juce::Button::buttonOver || button.getState()==juce::Button::buttonDown);};
        const auto border=[&](size_t index){const auto& button=*buttons[index];g.setColour((highlighted(index)?aqua:outline).withAlpha(button.isEnabled()?1.0f:.4f));
            g.strokePath(outlineFor(index,controlGeometry::insideStroke(button.getBounds(),rule)),juce::PathStrokeType(rule));};
        for(size_t i=0;i<buttons.size();++i)if(!highlighted(i))border(i);
        for(size_t i=0;i<buttons.size();++i)if(highlighted(i))border(i);
        if(hasKeyboardFocus(true) && focused<buttons.size())
        {g.setColour(choices[focused].id==selected?background:aqua);g.strokePath(outlineFor(focused,buttons[focused]->getBounds().toFloat().reduced(2)),juce::PathStrokeType(2));}
    }
    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler()override
    {return std::make_unique<juce::AccessibilityHandler>(*this,juce::AccessibilityRole::group);}
private:
    juce::Font labelFont()const{return interFont(textHeight,InterFace::semibold);}
    juce::Path outlineFor(size_t index,juce::Rectangle<float> box)const
    {
        juce::Path path;const auto first=index==0,last=index+1==buttons.size();
        path.addRoundedRectangle(box.getX(),box.getY(),box.getWidth(),box.getHeight(),radius,radius,first,last,first,last);
        return path;
    }
    void focusChoice(size_t index){focused=index;updateHint();if(isShowing())grabKeyboardFocus();repaint();}
    void request(size_t index)
    {
        if(!isEnabled() || index>=choices.size())return;
        const auto item=choices[index];if(!item.available || item.id==selected)return;
        const auto callback=onRequest;if(callback)callback(item.id); // No member access after a callback may destroy this component.
    }
    void updateHint(){if(focused<choices.size()){const auto& item=choices[focused];setTooltip(item.label+": "+(item.available?item.help:item.unavailableReason));}}
    std::vector<SegmentChoice> choices;
    ModeIconCache iconCache;
    std::vector<std::unique_ptr<Segment>> buttons;
    int selected=0;size_t focused=0;
    float textHeight=navigationSize;
};
}
