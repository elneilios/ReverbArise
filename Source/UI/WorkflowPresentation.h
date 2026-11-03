#pragma once
#include <Core/WorkflowState.h>
#include <array>

namespace m3::ui
{
// Display-only vocabulary. workflow::modeName remains the durable state key.
inline const char* modeLabel(workflow::Mode mode) noexcept
{
    switch(mode)
    {
        case workflow::Mode::capture:return "Learn";
        case workflow::Mode::live:return "Perform";
        case workflow::Mode::finalise:return "Export";
    }
    return "Unknown";
}
inline const char* modeSubtitle(workflow::Mode mode) noexcept
{
    switch(mode)
    {
        case workflow::Mode::capture:return "Capture audio and discover musical moments";
        case workflow::Mode::live:return "Shape, edit and play your swells";
        case workflow::Mode::finalise:return "Preview your prepared swells";
    }
    return "Mode unavailable";
}
inline const char* entryExplanation(workflow::EntryBlock block) noexcept
{
    switch(block)
    {
        case workflow::EntryBlock::none:return "";
        case workflow::EntryBlock::recording:return "Learn is recording. Stop the DAW to commit source.";
        case workflow::EntryBlock::settling:return "Source commit/validation is settling.";
        case workflow::EntryBlock::source:return "Learn useful source audio first. Perform also supports verified frozen wet.";
        case workflow::EntryBlock::timing:return "Source needs compatible validated host timing.";
        case workflow::EntryBlock::wet:return "Export needs an enabled, compatible ready swell.";
    }
    return "Mode unavailable";
}
inline const char* workflowStatus(workflow::Snapshot state) noexcept
{
    if(state.mode==workflow::Mode::capture && state.admissionFailed)
        return "Capture could not start - stop and play again";
    switch(state.phase)
    {
        case workflow::Phase::recording:return "Recording - stop the DAW to commit";
        case workflow::Phase::draining:return "Committing captured source";
        case workflow::Phase::validating:return "Validating captured source";
        case workflow::Phase::failed:return "Capture failed - previous source retained";
        case workflow::Phase::idle:break;
    }
    if(state.mode==workflow::Mode::capture)
        return state.recapture?"Recapture armed - replaces encountered audio":"Mapped source protected - play your DAW to learn";
    return state.mode==workflow::Mode::live?"":"DAW wet bypassed - editing locked";
}
struct WorkflowChoice
{
    workflow::Mode mode;
    const char* label;
    const char* subtitle;
    bool available;
    const char* reason;
};
struct WorkflowPresentation
{
    workflow::Mode selected;
    const char* status;
    std::array<WorkflowChoice,3> choices;
};
inline WorkflowPresentation presentWorkflow(workflow::Snapshot state,workflow::Readiness readiness) noexcept
{
    WorkflowPresentation result{state.mode,workflowStatus(state),{}};
    for(size_t i=0;i<result.choices.size();++i)
    {
        const auto mode=static_cast<workflow::Mode>(i);
        const auto block=workflow::entryBlock(state,mode,readiness);
        result.choices[i]={mode,modeLabel(mode),modeSubtitle(mode),block==workflow::EntryBlock::none,entryExplanation(block)};
    }
    return result;
}
}
