#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <PreverbUIResources.h>
#include "UiGeometry.h"
#include "ResponsivePresentation.h"
#include <array>

namespace m3::ui
{
inline const juce::Colour background{0xff0f1418}, aqua{0xff00efff}, hover{0xff00c2cc},
    timing{0xffffc700}, text{0xfff8fafc}, secondary{0xffedf1f5}, separator{0xff4a5058}, outline{0xff6b737c},
    danger{0xffff6b5e};
inline constexpr float rule=1, radius=4, headingSize=21, bodySize=14, controlSize=13, smallSize=12, captionSize=11, emptyHeadingSize=24;
inline constexpr float navigationSize=18, modeDescriptionSize=16;
inline const juce::Colour capturedSilence{0xff242a30}, unavailableSource{0xffa54444}, unavailableOverview{0xff773d3d},
    rawWet{0xff63716e}, otherWet{0xff80938f}, warning{0xffe6bb72}, staleWet{0xffdba66f}, previewCursor{0xffffb4e9};

namespace controls
{
enum class CommandVariant { neutral, primary, destructive };
struct CommandState {bool enabled=true,hovered=false,pressed=false,focused=false,latched=false;};
struct CommandInk {juce::Colour fill,border,content,glyph,focus;float stroke;};
// One resolver for command content and surface. Names and labels are metadata.
inline CommandInk commandInk(CommandVariant variant,CommandState state)
{
    const auto alpha=state.enabled?1.f:.35f;
    const auto over=state.enabled && state.hovered;
    const auto down=state.enabled && state.pressed;
    const auto emphasized=variant==CommandVariant::primary || state.latched;
    const auto destructive=variant==CommandVariant::destructive;
    const auto filled=emphasized || (destructive && down);
    const auto border=destructive?danger:down?aqua:over?hover:outline;
    const auto content=filled?background:destructive?danger:text;
    return {(down?(destructive?danger:emphasized?aqua:separator):emphasized?(over?hover:aqua):background).withMultipliedAlpha(alpha),
            border.withMultipliedAlpha(alpha),content.withMultipliedAlpha(alpha),
            content.withMultipliedAlpha(alpha),
            aqua,
            down || (destructive && over)?controlGeometry::focusStroke:rule};
}
// Reserve the bracket stroke and gap inside the component, outside its body.
inline juce::Rectangle<int> commandBodyBounds(juce::Rectangle<int> bounds)
{return bounds.reduced(static_cast<int>(controlGeometry::focusOutset+controlGeometry::focusStroke));}
inline void paintCommandSurface(juce::Graphics& g,juce::Rectangle<int> bounds,CommandVariant variant,CommandState state)
{
    const auto ink=commandInk(variant,state);
    const auto body=commandBodyBounds(bounds);
    g.setColour(ink.fill);g.fillRoundedRectangle(controlGeometry::insideStroke(body,rule),radius);
    g.setColour(ink.border);g.drawRoundedRectangle(controlGeometry::insideStroke(body,ink.stroke),radius,ink.stroke);
    if(state.focused)
    {
        const auto focus=controlGeometry::insideStroke(bounds,controlGeometry::focusStroke);
        const auto leg=std::min(focus.getWidth(),focus.getHeight())*.2f;juce::Path brackets;
        for(const auto corner:std::array<juce::Point<float>,4>{{focus.getTopLeft(),focus.getTopRight(),focus.getBottomLeft(),focus.getBottomRight()}})
        {
            const auto dx=corner.x==focus.getX()?1.f:-1.f,dy=corner.y==focus.getY()?1.f:-1.f;
            brackets.startNewSubPath(corner.x+dx*leg,corner.y);brackets.lineTo(corner);brackets.lineTo(corner.x,corner.y+dy*leg);
        }
        g.setColour(ink.focus);g.strokePath(brackets,juce::PathStrokeType(controlGeometry::focusStroke,juce::PathStrokeType::mitered,juce::PathStrokeType::butt));
    }
}
}

inline const char* resource(const juce::String& filename,int& bytes)
{
    for(int i=0;i<PreverbUIResources::namedResourceListSize;++i)
        if(filename==PreverbUIResources::originalFilenames[i])
            return PreverbUIResources::getNamedResource(PreverbUIResources::namedResourceList[i],bytes);
    bytes=0;return nullptr;
}

// Explicit embedded faces: no installed-family lookup or synthetic weights.
enum class InterFace { regular, semibold, bold, italic, medium };
inline juce::Typeface::Ptr interTypeface(InterFace face)
{
    static const auto faces=[]{
        std::array<juce::Typeface::Ptr,5> result;
        const char* names[]={"Inter-Regular.ttf","Inter-SemiBold.ttf","Inter-Bold.ttf","Inter-Italic.ttf","Inter-Medium.ttf"};
        for(size_t i=0;i<result.size();++i)
        {
            int size=0;const auto* bytes=resource(names[i],size);jassert(bytes && size>0);
            if(bytes)result[i]=juce::Typeface::createSystemTypefaceFor(bytes,static_cast<size_t>(size));
            jassert(result[i]!=nullptr);
        }
        return result;
    }();
    return faces[static_cast<size_t>(face)];
}
inline InterFace interStyle(const juce::String& style)
{
    if(style.containsIgnoreCase("italic"))return InterFace::italic;
    if(style.containsIgnoreCase("semi"))return InterFace::semibold;
    if(style.containsIgnoreCase("medium"))return InterFace::medium;
    return style.containsIgnoreCase("bold")?InterFace::bold:InterFace::regular;
}
inline juce::Font interFont(float height,InterFace face=InterFace::regular)
{return juce::Font(juce::FontOptions(interTypeface(face)).withHeight(height));}
inline juce::Font interFont(float height,int flags)
{return interFont(height,(flags&juce::Font::italic)?InterFace::italic:(flags&juce::Font::bold)?InterFace::bold:InterFace::regular);}

// Production standards revision 1: appearance roles, independent of bindings.
inline const juce::Colour headerBodyInk{0xffc7cdd4};
enum class TextRole { sectionHeading,controlLabel,helper,ruler,emptyHeading,emptyBody,tagline,headerBody,controlValue };
struct TextStyle {float height;InterFace face;juce::Colour ink;int lineHeight;};
inline TextStyle textStyle(TextRole role)
{
    switch(role)
    {
        case TextRole::sectionHeading:return {headingSize,InterFace::semibold,text,controlGeometry::headingHeight};
        case TextRole::controlLabel:return {bodySize,InterFace::medium,text,controlGeometry::labelHeight};
        case TextRole::helper:return {learnTypography::secondarySize,InterFace::regular,learnTypography::descriptionColour,learnTypography::secondaryLineHeight};
        case TextRole::ruler:return {learnTypography::secondarySize,InterFace::medium,text,learnTypography::secondaryLineHeight};
        case TextRole::emptyHeading:return {emptyHeadingSize,InterFace::semibold,text,learnGeometry::emptyHeadingHeight};
        case TextRole::emptyBody:return {bodySize,InterFace::regular,learnTypography::descriptionColour,learnGeometry::emptyTextHeight};
        case TextRole::tagline:return {headingSize,InterFace::italic,text,46};
        case TextRole::headerBody:return {bodySize,InterFace::regular,headerBodyInk,32};
        case TextRole::controlValue:return {controlSize,InterFace::regular,text,controlGeometry::valueHeight};
    }
    jassertfalse;return {bodySize,InterFace::regular,text,learnTypography::secondaryLineHeight};
}
inline juce::Font textFont(TextRole role)
{const auto style=textStyle(role);return interFont(style.height,style.face);}
inline void applyTextRole(juce::Label& label,TextRole role,juce::Justification alignment=juce::Justification::centredLeft)
{label.setFont(textFont(role));label.setColour(juce::Label::textColourId,textStyle(role).ink);label.setJustificationType(alignment);}
inline void applyTextRole(juce::Graphics& graphics,TextRole role)
{graphics.setFont(textFont(role));graphics.setColour(textStyle(role).ink);}

class FlatSurface:public juce::Component
{
public:void paint(juce::Graphics& g)override{g.fillAll(background);}
};
class InvocationComboBox:public juce::ComboBox
{
public:
    std::function<void()> beforeInvocation;
    void showPopup()override{if(beforeInvocation)beforeInvocation();juce::ComboBox::showPopup();}
    bool keyPressed(const juce::KeyPress& key)override{if(!isPopupActive() && beforeInvocation)beforeInvocation();return juce::ComboBox::keyPressed(key);}
};
class FlatViewport:public juce::Viewport
{
public:void paint(juce::Graphics& g)override{g.fillAll(background);}
};
class WorkspaceTheme:public juce::LookAndFeel_V4
{
public:
    void setResponsiveViewport(int width,int height)noexcept{responsive=true;responsiveType=ResponsiveTypography::at(width,height);}
    bool usesResponsiveTypography()const noexcept{return responsive;}
    float resolvedHeight(float height)const noexcept
    {
        if(!responsive)return height;
        if(height==headingSize)return responsiveType.heading;
        if(height==navigationSize)return responsiveType.navigation;
        if(height==bodySize)return responsiveType.body;
        if(height==controlSize || height==smallSize || height==captionSize)return responsiveType.value;
        return height;
    }
    WorkspaceTheme()
    {
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
    bool interAvailable()const{return interTypeface(InterFace::regular)!=nullptr && interTypeface(InterFace::semibold)!=nullptr
        && interTypeface(InterFace::bold)!=nullptr && interTypeface(InterFace::italic)!=nullptr && interTypeface(InterFace::medium)!=nullptr;}
    std::unique_ptr<juce::DropShadower> createDropShadowerForComponent(juce::Component&)override{return {};}
    juce::Font font(float height,int flags=0)const
    {
        return interFont(resolvedHeight(height),flags);
    }
    juce::Font getLabelFont(juce::Label& label)override{return interFont(resolvedHeight(label.getFont().getHeight()),interStyle(label.getFont().getTypefaceStyle()));}
    juce::Font getComboBoxFont(juce::ComboBox&)override{return font(controlSize);}
    juce::Font getPopupMenuFont()override{return font(bodySize);}
    juce::Font getSliderPopupFont(juce::Slider&)override{return font(controlSize);}
    juce::Font getTabButtonFont(juce::TabBarButton&,float)override{return font(bodySize);}
    juce::Font getAlertWindowTitleFont()override{return font(headingSize,juce::Font::bold);}
    juce::Font getAlertWindowMessageFont()override{return font(bodySize);}
    juce::Font getAlertWindowFont()override{return font(bodySize);}
    juce::Typeface::Ptr getTypefaceForFont(const juce::Font& f) override
    {
        return interTypeface(interStyle(f.getTypefaceStyle()));
    }
    juce::Font getTextButtonFont(juce::TextButton&,int) override{return font(responsive?bodySize:controlSize);}
    void drawButtonBackground(juce::Graphics& g,juce::Button& b,const juce::Colour&,bool over,bool down) override
    {controls::paintCommandSurface(g,b.getLocalBounds(),controls::CommandVariant::neutral,{b.isEnabled(),over,down,b.hasKeyboardFocus(true),false});}
    void drawButtonText(juce::Graphics& g,juce::TextButton& b,bool over,bool down) override
    {
        g.setColour(controls::commandInk(controls::CommandVariant::neutral,{b.isEnabled(),over,down,b.hasKeyboardFocus(true),false}).content);
        g.setFont(getTextButtonFont(b,b.getHeight()));
        g.drawText(b.getButtonText(),b.getLocalBounds().reduced(spacing::related,spacing::tight),juce::Justification::centred);
    }
    void drawToggleButton(juce::Graphics& g,juce::ToggleButton& b,bool over,bool) override
    {
        const auto alpha=b.isEnabled()?1.0f:.35f;
        const juce::Rectangle<float> box(2,(b.getHeight()-18)*.5f,18,18);
        g.setColour(b.getToggleState()?aqua.withAlpha(alpha):background);g.fillRoundedRectangle(box,3);
        g.setColour((over?aqua:outline).withAlpha(alpha));g.drawRoundedRectangle(box,3,1);
        if(b.getToggleState()) {g.setColour(background);juce::Path tick;
            tick.startNewSubPath(box.getX()+4,box.getY()+9);tick.lineTo(box.getX()+8,box.getY()+13);tick.lineTo(box.getX()+15,box.getY()+5);g.strokePath(tick,juce::PathStrokeType(2));}
        g.setColour(text.withAlpha(alpha));g.setFont(font(controlSize));
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
private:
    bool responsive=false;
    ResponsiveTypography responsiveType;
};

// Custom Graphics painting does not automatically use the component's font
// policy. Resolve explicitly; never change JUCE's process-wide default theme.
inline juce::Font fontFor(juce::Component& component,float height,int flags=0)
{
    if(const auto* theme=dynamic_cast<WorkspaceTheme*>(&component.getLookAndFeel()))return theme->font(height,flags);
    return interFont(height,flags);
}
inline juce::Font navigationFont()
{return interFont(navigationSize,InterFace::semibold);}

inline juce::Font componentTextFont(juce::Component& component,TextRole role)
{
    const auto style=textStyle(role);
    const auto* theme=dynamic_cast<const WorkspaceTheme*>(&component.getLookAndFeel());
    return interFont(theme?theme->resolvedHeight(style.height):style.height,style.face);
}
inline void applyTextRole(juce::Graphics& graphics,juce::Component& component,TextRole role)
{const auto style=textStyle(role);graphics.setFont(componentTextFont(component,role));graphics.setColour(style.ink);}

class Wordmark final:public juce::Component
{
public:
    Wordmark()
    {int size=0;const auto* bytes=resource("reverbarise-wordmark-dark.svg",size);
        if(bytes)image=juce::Drawable::createFromImageData(bytes,static_cast<size_t>(size));setInterceptsMouseClicks(false,false);}
    bool loaded()const noexcept{return image!=nullptr;}
    void setOpticalAlignment(bool enabled){optical=enabled;repaint();}
    void paint(juce::Graphics& g)override
    {
        if(!image)return;
        if(optical)
        {
            const auto ink=image->getOutlineAsPath().getBounds();
            if(!ink.isEmpty()){image->draw(g,1.0f,juce::RectanglePlacement(juce::RectanglePlacement::xLeft|juce::RectanglePlacement::yMid).getTransformToFit(ink,getLocalBounds().toFloat()));return;}
        }
        image->drawWithin(g,getLocalBounds().toFloat(),juce::RectanglePlacement::centred,1);
    }
private:std::unique_ptr<juce::Drawable> image;bool optical=false;
};
}
