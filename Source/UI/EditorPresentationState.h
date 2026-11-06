#pragma once
#include "PresentationFit.h"

namespace m3::ui
{
// Per-editor state/policy, independent of JUCE components and musical state.
// The GUI owner applies effective() to its content root and invokes persistence
// on its message/lifecycle path. Merely observing a new display never saves.
class EditorPresentationState
{
public:
    EditorPresentationState(PresentationStorage& storage,PresentationFitContext display)
        :preferences(storage),context(display),current(fitPresentation(preferences.preferences(),context)){}
    const PresentationPreferences& desired()const noexcept{return preferences.preferences();}
    const FittedPresentation& effective()const noexcept{return current;}
    const LoadedPresentation& initialRead()const noexcept{return preferences.initialRead();}
    bool retryInitialRead(){if(!preferences.retryInitialRead())return false;resolve();return true;}
    bool hasPendingSave()const noexcept{return preferences.hasPendingSave();}
    const juce::Result& lastSave()const noexcept{return preferences.lastSave();}
    bool canSelectSize(int width,int height)const noexcept
    {return presentationFits({width,height,current.effective.scalePercent},context);}
    bool canSelectScale(int percent)const noexcept
    {auto candidate=current.effective;candidate.scalePercent=percent;return presentationFits(candidate,context);}
    bool selectSize(int width,int height,double now)
    {
        if(!canSelectSize(width,height) || !preferences.setSize(width,height,now))return false;
        resolve();return true;
    }
    bool selectScale(int percent,double now)
    {
        if(!canSelectScale(percent) || !preferences.setScale(percent,now))return false;
        resolve();return true;
    }
    bool manualResize(PresentationExtent contentPresentation,double now)
    {
        if(contentPresentation.width<0 || contentPresentation.height<0)return false;
        // This input excludes external chrome and already uses host logical
        // units. Reverse user magnification once; do not apply host DPI again.
        const auto scale=current.effective.scalePercent;
        const auto width=static_cast<juce::int64>(contentPresentation.width)*100/scale;
        const auto height=static_cast<juce::int64>(contentPresentation.height)*100/scale;
        if(width>2000 || height>1400)return false;
        return selectSize(static_cast<int>(width),static_cast<int>(height),now);
    }
    bool displayChanged(PresentationFitContext display)
    {
        const auto previous=current;context=display;resolve();
        return previous.effective!=current.effective || previous.supported!=current.supported || previous.recovered!=current.recovered;
    }
    bool persistIfDue(double now,bool force=false){return preferences.saveIfDue(now,force);}
private:
    void resolve(){current=fitPresentation(desired(),context);}
    PresentationPreferenceSession preferences;PresentationFitContext context;FittedPresentation current;
    JUCE_DECLARE_NON_COPYABLE(EditorPresentationState)
};
}
