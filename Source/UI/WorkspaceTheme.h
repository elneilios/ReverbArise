#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <PreverbUIResources.h>

namespace m3::ui
{
inline const juce::Colour background{0xff0f1418}, aqua{0xff00efff}, hover{0xff00c2cc},
    timing{0xffffc700}, text{0xfff5f7f9}, secondary{0xffc7cdd4}, separator{0xff4a5058}, outline{0xff6b737c},
    danger{0xffff6b5e};
inline constexpr float rule=1, radius=4;

inline const char* resource(const juce::String& filename,int& bytes)
{
    for(int i=0;i<PreverbUIResources::namedResourceListSize;++i)
        if(filename==PreverbUIResources::originalFilenames[i])
            return PreverbUIResources::getNamedResource(PreverbUIResources::namedResourceList[i],bytes);
    bytes=0;return nullptr;
}

class WorkspaceTheme final:public juce::LookAndFeel_V4
{
public:
    WorkspaceTheme()
    {
        int size=0;const auto* bytes=resource("InterVariable.ttf",size);
        if(bytes)inter=juce::Typeface::createSystemTypefaceFor(bytes,static_cast<size_t>(size));
        for(const int id:std::initializer_list<int>{juce::Label::textColourId,juce::TextEditor::textColourId,
                          juce::ComboBox::textColourId,juce::Slider::textBoxTextColourId})setColour(id,text);
        for(const int id:std::initializer_list<int>{juce::TextEditor::backgroundColourId,juce::ComboBox::backgroundColourId,
                          juce::Slider::textBoxBackgroundColourId,juce::ListBox::backgroundColourId,
                          juce::PopupMenu::backgroundColourId})setColour(id,background);
        for(const int id:std::initializer_list<int>{juce::TextEditor::outlineColourId,juce::ComboBox::outlineColourId,
                          juce::Slider::textBoxOutlineColourId})setColour(id,outline);
        setColour(juce::TextEditor::focusedOutlineColourId,aqua);
        setColour(juce::TextEditor::highlightColourId,aqua.withAlpha(.2f));
        setColour(juce::TextEditor::highlightedTextColourId,text);
        setColour(juce::PopupMenu::textColourId,text);setColour(juce::PopupMenu::highlightedBackgroundColourId,hover);
        setColour(juce::PopupMenu::highlightedTextColourId,background);
        setColour(juce::ScrollBar::thumbColourId,outline);
        setColour(juce::ScrollBar::backgroundColourId,background);
        setColour(juce::Slider::thumbColourId,aqua);
        setColour(juce::Slider::trackColourId,aqua);
        setColour(juce::Slider::backgroundColourId,separator);
        setColour(juce::TextButton::textColourOffId,text);setColour(juce::TextButton::textColourOnId,background);
    }
    bool interAvailable()const noexcept{return inter!=nullptr;}
    juce::Font font(float height,int flags=0)const
    {
        if(inter && (flags&(juce::Font::bold|juce::Font::italic))==0)
            return juce::Font(juce::FontOptions(height).withTypeface(inter));
        return juce::Font(juce::FontOptions("Segoe UI",height,flags));
    }
    juce::Font getLabelFont(juce::Label& label)override{return font(label.getFont().getHeight(),label.getFont().getStyleFlags());}
    juce::Font getComboBoxFont(juce::ComboBox&)override{return font(13);}
    juce::Typeface::Ptr getTypefaceForFont(const juce::Font& f) override
    {
        if(inter && !f.isBold() && !f.isItalic())return inter;
        // Keep real bold/italic faces rather than pretending the regular
        // variable typeface provides unverified style axes on this backend.
        return juce::Typeface::createSystemTypefaceFor(juce::Font(juce::FontOptions("Segoe UI",f.getHeight(),f.getStyleFlags())));
    }
    juce::Font getTextButtonFont(juce::TextButton&,int) override{return font(13);}
    void drawButtonBackground(juce::Graphics& g,juce::Button& b,const juce::Colour&,bool over,bool down) override
    {
        const auto active=b.getName()=="audition" || b.getToggleState();
        const auto bounds=b.getLocalBounds().toFloat().reduced(.5f);
        g.setColour(active?aqua.withMultipliedAlpha(b.isEnabled()?1.0f:.25f):background);
        g.fillRoundedRectangle(bounds,radius);
        const auto destructive=b.getName()=="resetWetShape" || b.getName()=="resetSwellMenu";
        g.setColour(destructive?danger.withMultipliedAlpha(b.isEnabled()?1.0f:.35f)
                                :(over || down)?aqua:outline.withMultipliedAlpha(b.isEnabled()?1.0f:.35f));
        g.drawRoundedRectangle(bounds,radius,rule);
    }
    void drawButtonText(juce::Graphics& g,juce::TextButton& b,bool over,bool down) override
    {
        const auto box=b.getLocalBounds().toFloat();const auto c=box.getCentre();
        g.setColour((b.getName()=="audition"?background:text).withMultipliedAlpha(b.isEnabled()?1.0f:.35f));
        if(b.getName()=="audition")
        {juce::Path p;p.addTriangle(c.x-5,c.y-8,c.x-5,c.y+8,c.x+8,c.y);g.fillPath(p);return;}
        if(b.getName()=="stopAudition"){g.fillRect(juce::Rectangle<float>(12,12).withCentre(c));return;}
        if(b.getName()=="diagnostics")
        {
            g.drawEllipse(juce::Rectangle<float>(14,14).withCentre(c),1.5f);
            g.drawEllipse(juce::Rectangle<float>(5,5).withCentre(c),1.5f);
            for(int n=0;n<8;++n){const auto a=juce::MathConstants<float>::twoPi*n/8;
                g.drawLine(c.x+std::cos(a)*7,c.y+std::sin(a)*7,c.x+std::cos(a)*10,c.y+std::sin(a)*10,2);}
            return;
        }
        if(b.getName()=="resetWetShape" || b.getName()=="resetSwellMenu")
        {
            g.setColour(danger.withMultipliedAlpha(b.isEnabled()?1.0f:.35f));
            g.setFont(getTextButtonFont(b,0));
            g.drawFittedText(b.getButtonText(),b.getLocalBounds(),juce::Justification::centred,1);
            return;
        }
        if(b.getName()=="loadIR")
        {
            // File-open glyph: folder tab plus base with a centred opening.
            g.setColour((over?aqua:text).withMultipliedAlpha(b.isEnabled()?1.0f:.35f));
            const auto left=c.x-8,top=c.y-6,right=c.x+8,bottom=c.y+6;
            juce::Path folder;
            folder.startNewSubPath(left,top);folder.lineTo(left-1+5,top);folder.lineTo(left+7,top+3);
            folder.lineTo(right,top+3);folder.lineTo(right,bottom);folder.lineTo(left,bottom);folder.closeSubPath();
            g.strokePath(folder,juce::PathStrokeType(1.5f));
            return;
        }
        juce::LookAndFeel_V4::drawButtonText(g,b,over,down);
    }
    void drawToggleButton(juce::Graphics& g,juce::ToggleButton& b,bool over,bool) override
    {
        const auto alpha=b.isEnabled()?1.0f:.35f;
        const juce::Rectangle<float> box(2,(b.getHeight()-18)*.5f,18,18);
        g.setColour(b.getToggleState()?aqua.withAlpha(alpha):background);g.fillRoundedRectangle(box,3);
        g.setColour((over?aqua:outline).withAlpha(alpha));g.drawRoundedRectangle(box,3,1);
        if(b.getToggleState()) {g.setColour(background);juce::Path tick;
            tick.startNewSubPath(box.getX()+4,box.getY()+9);tick.lineTo(box.getX()+8,box.getY()+13);tick.lineTo(box.getX()+15,box.getY()+5);g.strokePath(tick,juce::PathStrokeType(2));}
        g.setColour(text.withAlpha(alpha));g.setFont(font(13));
        g.drawText(b.getButtonText(),b.getLocalBounds().withTrimmedLeft(28),juce::Justification::centredLeft);
    }
    void drawRotarySlider(juce::Graphics& g,int x,int y,int width,int height,float value,float start,float end,juce::Slider& s) override
    {
        const auto box=juce::Rectangle<float>(static_cast<float>(x),static_cast<float>(y),static_cast<float>(width),static_cast<float>(height)).reduced(5);
        const auto r=std::max(1.0f,std::min(box.getWidth(),box.getHeight())*.5f);const auto c=box.getCentre();
        juce::Path track,fill;track.addCentredArc(c.x,c.y,r,r,0,start,end,true);
        fill.addCentredArc(c.x,c.y,r,r,0,start,start+value*(end-start),true);
        g.setColour(separator);g.strokePath(track,juce::PathStrokeType(3));
        g.setColour(aqua.withMultipliedAlpha(s.isEnabled()?1.0f:.3f));g.strokePath(fill,juce::PathStrokeType(3));
        const auto angle=start+value*(end-start)-juce::MathConstants<float>::halfPi;
        const juce::Point<float> p{c.x+std::cos(angle)*r,c.y+std::sin(angle)*r};
        g.setColour(text);g.fillEllipse(juce::Rectangle<float>(4,4).withCentre(p));
    }
    void drawLinearSlider(juce::Graphics& g,int x,int y,int width,int height,float pos,float,float,juce::Slider::SliderStyle style,juce::Slider& s) override
    {
        if(style!=juce::Slider::LinearHorizontal){juce::LookAndFeel_V4::drawLinearSlider(g,x,y,width,height,pos,pos,pos,style,s);return;}
        const auto cy=static_cast<float>(y)+height*.5f;
        g.setColour(separator);g.drawLine(static_cast<float>(x),cy,static_cast<float>(x+width),cy,3);
        g.setColour(aqua.withMultipliedAlpha(s.isEnabled()?1.0f:.3f));g.drawLine(static_cast<float>(x),cy,pos,cy,3);
        g.fillEllipse(pos-5,cy-5,10,10);
    }
private:juce::Typeface::Ptr inter;
};

class Wordmark final:public juce::Component
{
public:
    Wordmark()
    {int size=0;const auto* bytes=resource("reverbarise-wordmark-dark.svg",size);
        if(bytes)image=juce::Drawable::createFromImageData(bytes,static_cast<size_t>(size));setInterceptsMouseClicks(false,false);}
    bool loaded()const noexcept{return image!=nullptr;}
    void paint(juce::Graphics& g)override
    {if(image)image->drawWithin(g,getLocalBounds().toFloat(),juce::RectanglePlacement::centred,1);}
private:std::unique_ptr<juce::Drawable> image;
};
}
