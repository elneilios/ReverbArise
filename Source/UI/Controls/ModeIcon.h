#pragma once
#include <UI/WorkspaceTheme.h>

namespace m3::ui::controls
{
enum class ModeIcon { none, learn, perform, exportAudio };

// Constructed by the control before painting. Parsed originals stay immutable;
// each foreground variant is prepared once, never recoloured during paint.
class ModeIconCache final
{
public:
    ModeIconCache()
    {
        const char* names[]{"learn_svg","perform_svg","export_svg"};
        for(size_t i=0;i<originals.size();++i)
        {
            int size=0;const auto* bytes=PreverbUIResources::getNamedResource(names[i],size);
            originals[i]=juce::Drawable::createFromImageData(bytes,static_cast<size_t>(size));
            jassert(originals[i]);
            if(!originals[i])continue;
            inactive[i]=originals[i]->createCopy();active[i]=originals[i]->createCopy();
            active[i]->replaceColour(text,background);
        }
    }
    const juce::Drawable* drawable(ModeIcon icon,bool selected=false)const noexcept
    {
        const auto index=static_cast<size_t>(icon);
        return index==0 || index>3?nullptr:(selected?active:inactive)[index-1].get();
    }
    void draw(juce::Graphics& g,ModeIcon icon,juce::Rectangle<float> bounds,bool selected,float alpha)const
    {
        if(const auto* image=drawable(icon,selected))
            image->draw(g,alpha,juce::AffineTransform::scale(bounds.getWidth()/16,bounds.getHeight()/16)
                .translated(bounds.getX(),bounds.getY()));
    }
private:
    std::array<std::unique_ptr<juce::Drawable>,3> originals,inactive,active;
};
}
