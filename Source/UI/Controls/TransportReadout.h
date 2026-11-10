#pragma once
#include <UI/WorkspaceTheme.h>
#include <UI/TransportPresentation.h>

namespace m3::ui::controls
{
// Owner-approved issue #68 extraction of the existing footer's 15-logical-pixel
// Inter Regular. Slot geometry is independent of the current transport values.
class TransportReadout final:public juce::Component,public juce::SettableTooltipClient
{
public:
    static constexpr float footerFontHeight=15;
    struct Geometry
    {
        juce::Rectangle<int> group;
        std::array<juce::Rectangle<int>,4> fields,values,labels;
        std::array<juce::Rectangle<int>,3> separators;
        juce::Rectangle<int> bpmUnit;
        bool operator==(const Geometry&)const=default;
    };
    TransportReadout(){setName("hostInfo");display({});}
    static juce::Font readoutFont(){return interFont(footerFontHeight);}
    const TransportPresentation& presentation()const{return current;}
    const Geometry& geometry()const{return layout;}
    void display(const TransportPresentation& next)
    {
        if(current==next && getTitle().isNotEmpty())return;
        current=next;setTitle(current.description());setDescription(current.help());setTooltip(current.help());repaint();
    }
    static int textWidth(const juce::String& value)
    {return static_cast<int>(std::ceil(juce::GlyphArrangement::getStringWidth(readoutFont(),value)));}
    bool overflows(size_t field)const{return textWidth(current.values[field])>layout.values[field].getWidth();}
    void resized()override
    {
        // Worst-case digit metrics, including sign/decimal, are independent of live text.
        int digit=0;for(char c='0';c<='9';++c)digit=std::max(digit,textWidth(juce::String::charToString(c)));
        const auto unavailable=textWidth("unavailable");
        const std::array<int,4> budgets{
            std::max(unavailable,6*digit+textWidth("-")),
            std::max(unavailable,6*digit+textWidth(".")),
            std::max(unavailable,6*digit+textWidth(".")),
            std::max(unavailable,6*digit+textWidth(" / "))};
        const std::array<juce::String,4> names{"Bar","Beat","",""};
        const auto unit=textWidth("BPM"),separatorWidth=textWidth("|");
        const auto gap=spacing::tight;
        int total=3*(separatorWidth+2*spacing::related)+unit+gap;
        for(size_t i=0;i<4;++i)total+=budgets[i]+(names[i].isEmpty()?0:textWidth(names[i])+gap);
        // Supported editor widths fit the full budget. Narrow containers clip safely;
        // they never trigger value-dependent layout, scaling or neighbouring overlap.
        layout={};layout.group={(getWidth()-total)/2,0,total,getHeight()};auto area=layout.group;
        for(size_t i=0;i<4;++i)
        {
            auto field=area.removeFromLeft(budgets[i]+(names[i].isEmpty()?0:textWidth(names[i])+gap)+(i==2?unit+gap:0));
            layout.fields[i]=field;
            if(names[i].isNotEmpty()){layout.labels[i]=field.removeFromLeft(textWidth(names[i]));field.removeFromLeft(gap);}
            layout.values[i]=field.removeFromLeft(budgets[i]);
            if(i==2){field.removeFromLeft(gap);layout.bpmUnit=field;}
            if(i<3){area.removeFromLeft(spacing::related);layout.separators[i]=area.removeFromLeft(separatorWidth);area.removeFromLeft(spacing::related);}
        }
    }
    void paint(juce::Graphics& g)override
    {
        g.setFont(readoutFont());g.setColour(text);
        const std::array<juce::String,4> names{"Bar","Beat","",""};
        for(size_t i=0;i<4;++i)
        {
            g.drawText(names[i],layout.labels[i],juce::Justification::centredLeft,true);
            g.drawText(current.values[i],layout.values[i],juce::Justification::centredRight,true);
        }
        g.drawText("BPM",layout.bpmUnit,juce::Justification::centredLeft,true);
        for(const auto& separator:layout.separators)g.drawText("|",separator,juce::Justification::centred,true);
    }
    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler()override
    {return std::make_unique<juce::AccessibilityHandler>(*this,juce::AccessibilityRole::staticText);}
private:
    TransportPresentation current;
    Geometry layout;
};
}
