#pragma once
#include <UI/Controls/ModeIcon.h>

inline void modeIconCases()
{
    using namespace m3::ui::controls;
    ModeIconCache cache;REQUIRE(!cache.drawable(ModeIcon::none));
    const ModeIcon roles[]{ModeIcon::learn,ModeIcon::perform,ModeIcon::exportAudio};
    const char* names[]{"learn_svg","perform_svg","export_svg"};
    for(size_t i=0;i<3;++i)
    {
        int length=0;const auto* bytes=PreverbUIResources::getNamedResource(names[i],length);REQUIRE(bytes && length>0);
        const auto xml=juce::parseXML(juce::String::fromUTF8(bytes,length));REQUIRE(xml);
        REQUIRE(xml->getStringAttribute("viewBox")=="0 0 16 16");
        REQUIRE(xml->getDoubleAttribute("stroke-width")==1.6);
        int circles=0;for(const auto* child:xml->getChildIterator())if(child->hasTagName("circle"))
        {++circles;REQUIRE(child->getDoubleAttribute("r")==1.35);}
        REQUIRE(circles==(i==1?3:0));
        for(const auto selected:{false,true})
        {
            const auto* drawable=cache.drawable(roles[i],selected);REQUIRE(drawable);
            const auto bounds=drawable->getDrawableBounds();
            REQUIRE(std::isfinite(bounds.getX()) && std::isfinite(bounds.getY()) && bounds.getWidth()>0 && bounds.getHeight()>0);
            REQUIRE(juce::Rectangle<float>(0,0,16,16).contains(bounds));
            REQUIRE(drawable==cache.drawable(roles[i],selected));
        }
        for(int size:{14,16,20,24,32})
        {
            juce::Image inactive(juce::Image::ARGB,size+8,size+8,true),active(inactive.createCopy()),disabled(inactive.createCopy());
            const juce::Rectangle<float> box(4,4,static_cast<float>(size),static_cast<float>(size));
            {
                juce::Graphics normalGraphics(inactive),activeGraphics(active),disabledGraphics(disabled);
                cache.draw(normalGraphics,roles[i],box,false,1);cache.draw(activeGraphics,roles[i],box,true,1);cache.draw(disabledGraphics,roles[i],box,false,.4f);
            } // Flush native image contexts before reading their pixels.
            int ink=0,maxAlpha=0;
            for(int y=0;y<size+8;++y)for(int x=0;x<size+8;++x)
            {
                const auto a=inactive.getPixelAt(x,y),b=active.getPixelAt(x,y),d=disabled.getPixelAt(x,y);
                maxAlpha=std::max(maxAlpha,static_cast<int>(a.getAlpha()));
                REQUIRE(a.getAlpha()==b.getAlpha());
                if(!box.contains(static_cast<float>(x),static_cast<float>(y)))REQUIRE(a.isTransparent());
                // Fractional 14 px strokes need not contain a fully opaque pixel.
                if(a.getAlpha()>180){++ink;REQUIRE(a.getRed()>240 && b.getRed()<20);REQUIRE(d.getAlpha()<110);}
            }
            if(ink==0)throw std::runtime_error("Missing icon ink: role="+std::to_string(i)+" size="+std::to_string(size)+" maxAlpha="+std::to_string(maxAlpha));
            if(i==1 && size>=16)for(const auto point:{juce::Point<float>(2.4f,12.8f),{9.3f,3},{13.6f,10.4f}})
            {
                const auto x=juce::roundToInt(4+point.x*size/16-.5f),y=juce::roundToInt(4+point.y*size/16-.5f);
                // At nominal size the 1.1-unit hole spans several partially
                // covered pixels; higher scale provides a clear centre sample.
                REQUIRE(inactive.getPixelAt(x,y).getAlpha()<(size<=20?220:128));
            }
        }
    }
}
