#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
namespace m3::ui
{
// State-independent region geometry. Content changes never move a band/divider.
struct WorkspaceLayout
{
    juce::Rectangle<int> header,globals,workspace,swells,editor,inspector,audition,overview,info;
    explicit WorkspaceLayout(juce::Rectangle<int> bounds)
    {
        auto a=bounds.reduced(bounds.getWidth()<1200?16:20);
        header=a.removeFromTop(70);globals=a.removeFromTop(66);
        info=a.removeFromBottom(44);overview=a.removeFromBottom(142);audition=a.removeFromBottom(82);
        workspace=a;swells=a.removeFromLeft(bounds.getWidth()<1200?180:210);a.removeFromLeft(20);
        inspector=a.removeFromRight(bounds.getWidth()<1200?320:340);a.removeFromRight(20);editor=a;
    }
};
}
