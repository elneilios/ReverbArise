#pragma once
#include <UI/PresentationFit.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace m3::ui::controls
{
// One vector/component transform for user magnification. Native peer/host DPI
// remains owned by JUCE. The screen owner keeps bindings and logical layout.
class PresentationSurface final:public juce::Component
{
    class Content final:public juce::Component
    {
    public:std::function<void(juce::Graphics&)> paintContent;
        void paint(juce::Graphics& g)override{if(paintContent)paintContent(g);}
    } content;
public:
    PresentationSurface(){setName("presentationSurface");content.setName("presentationContent");addAndMakeVisible(content);}
    juce::Component& contentRoot()noexcept{return content;}
    const juce::Component& contentRoot()const noexcept{return content;}
    void setPainter(std::function<void(juce::Graphics&)> paint){content.paintContent=std::move(paint);}
    void present(PresentationPreferences value)
    {present(value.width,value.height,value.scalePercent);}
    void present(int width,int height,int percent)
    {
        const auto scale=percent/100.f;
        content.setTransform(juce::AffineTransform::scale(scale));content.setBounds(0,0,width,height);
    }
};
}
