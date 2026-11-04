#pragma once
#include <UI/WorkspaceTheme.h>
#include <optional>
#include "HeaderInformation.h"

namespace m3::ui::controls
{
enum class NoticeTone { information, warning, error };
struct NoticeMessage
{
    juce::String text,detail;
    NoticeTone tone=NoticeTone::information;
    bool operator==(const NoticeMessage&)const=default;
};

// A presentation surface, not a notification queue or a workflow authority.
class NoticeBar final:public juce::Component,public juce::SettableTooltipClient
{
public:
    static constexpr int preferredHeight=32;
    NoticeBar(){setWantsKeyboardFocus(false);setMouseClickGrabsKeyboardFocus(false);setVisible(false);}
    bool display(std::optional<NoticeMessage> next)
    {
        if(next && next->text.isEmpty())next.reset();
        if(next==message)return false;
        const auto announce=next && (!message || next->text!=message->text || next->tone!=message->tone);
        message=std::move(next);setVisible(message.has_value());
        const auto label=message?message->text:juce::String{};
        const auto detail=message?message->detail:juce::String{};
        const auto prefix=message?(message->tone==NoticeTone::error?"Error: ":message->tone==NoticeTone::warning?"Warning: ":"Information: "):"";
        setTitle(juce::String(prefix)+label);setDescription(detail);setHelpText(detail);setTooltip(label+(detail.isEmpty()?"":"\n"+detail));
        if(announce)if(auto* accessible=getAccessibilityHandler())accessible->notifyAccessibilityEvent(juce::AccessibilityEvent::valueChanged);
        if(announce && isShowing())juce::AccessibilityHandler::postAnnouncement(label,message->tone==NoticeTone::error
            ?juce::AccessibilityHandler::AnnouncementPriority::high:juce::AccessibilityHandler::AnnouncementPriority::medium);
        repaint();return true;
    }
    const std::optional<NoticeMessage>& currentMessage()const noexcept{return message;}
    void setHeaderAppearance(bool enabled){headerAppearance=enabled;repaint();}
    void paint(juce::Graphics& g)override
    {
        if(!message)return;
        const auto ink=message->tone==NoticeTone::error?danger:message->tone==NoticeTone::warning?warning:aqua;
        g.fillAll(background);g.setColour(ink);g.fillRect(0,4,3,getHeight()-8);
        const auto cy=getHeight()*.5f;g.drawEllipse(13,cy-7,14,14,1.25f);
        g.setFont(fontFor(*this,controlSize,juce::Font::bold));g.drawText(message->tone==NoticeTone::information?"i":"!",12,0,16,getHeight(),juce::Justification::centred);
        g.setColour(headerAppearance?headerBodyInk:m3::ui::text);g.setFont(fontFor(*this,bodySize));
        g.drawText(message->text,getLocalBounds().withTrimmedLeft(38).withTrimmedRight(8),juce::Justification::centredLeft);
    }
    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler()override
    {return std::make_unique<juce::AccessibilityHandler>(*this,juce::AccessibilityRole::staticText);}
private:
    std::optional<NoticeMessage> message;
    bool headerAppearance=false;
};
}
