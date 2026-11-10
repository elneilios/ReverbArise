#pragma once
#include <Core/ExportState.h>
#include "Controls/FileDragTile.h"
#include "Controls/NativeWavDrag.h"

namespace m3::ui
{
// Message-thread binding. The engine still owns delivery eligibility and files.
class ExportFileInvocations final
{
public:
    struct Scope
    {
        uint64_t session = 0, pass = 0;
        ExportFile file;
        bool operator==(const Scope&) const = default;
    };

    using SaveResult = std::function<void(const juce::File&)>;
    using ChooseSave = std::function<void(const Scope&, SaveResult)>;
    using SaveFile = std::function<void(const Scope&, const juce::File&)>;
    using ReadyFile = std::function<juce::File(const Scope&)>;
    using DragGuard = std::function<bool()>;
    using StartExternalDrag = std::function<controls::WavDragResult(const juce::File&, const DragGuard&, const DragGuard&)>;
    using PressId = controls::FileSaveAction::PressId;

    ExportFileInvocations(controls::FileSaveAction& save, controls::FileDragTile& dragTile,
                          SaveFile saveFile, ReadyFile resolve, StartExternalDrag externalDrag, ChooseSave choose = {})
        : tile(dragTile), saveWav(std::move(saveFile)), readyFile(std::move(resolve)),
          startExternalDrag(std::move(externalDrag)), chooseSave(std::move(choose))
    {
        const auto weak = juce::WeakReference<ExportFileInvocations>(this);
        const auto capture = [weak]() -> controls::FileSaveAction::Save
        {
            if (weak == nullptr || !weak->current) return {};
            return [weak, intent = weak->captureIntent()]
            {
                if (weak != nullptr) weak->openSave(intent);
            };
        };
        save.bindSave(capture);
        tile.bindSave(capture);
        tile.beforeInvocation = [weak]
        {
            if (weak != nullptr)
            {
                weak->dragIntent.reset();
                if (weak->current) weak->dragIntent = weak->captureIntent();
            }
        };
        tile.onGuardedDrag = [weak](PressId press)
        {
            if (weak != nullptr) weak->requestDrag(press);
        };
    }

    void setCurrent(std::optional<Scope> scope)
    {
        if (scope == current) return;
        current = std::move(scope);
        ++scopeVersion;
    }

    bool isChoosing() const noexcept { return choosing; }

    // The test seam can model the device state for synthetic mouse events.
    // The binding always also requires the tile's original press ID.
    std::function<bool(PressId)> pointerStillDown;
    std::function<void(const juce::String&)> onStatus;

private:
    struct Intent { Scope scope; uint64_t version; };

    Intent captureIntent() const { return {*current, scopeVersion}; }
    bool isCurrent(const Intent& intent) const { return intent.version == scopeVersion && current == intent.scope; }
    bool isPressActive(PressId press) const
    { return tile.hasPress(press) && (pointerStillDown ? pointerStillDown(press) : tile.isPressActive(press)); }
    void report(const juce::String& message) { if (onStatus) onStatus(message); }

    void openSave(const Intent& intent)
    {
        if (choosing || dragging || !isCurrent(intent) || readyFile(intent.scope) == juce::File{}) return;
        choosing = true;
        const auto serial = ++saveSerial;
        auto complete = [weak = juce::WeakReference<ExportFileInvocations>(this), intent, serial](const juce::File& destination)
        {
            if (weak == nullptr || !weak->choosing || weak->saveSerial != serial) return;
            weak->choosing = false;
            if (destination != juce::File{} && weak->isCurrent(intent))
                weak->saveWav(intent.scope, destination);
        };
        if (chooseSave) { chooseSave(intent.scope, std::move(complete)); return; }

        const auto name = intent.scope.file.path.fromLastOccurrenceOf("/", false, false);
        const auto destination = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile(name);
        chooser = std::make_unique<juce::FileChooser>("Save song-aligned WAV", destination, "*.wav");
        chooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
                                | juce::FileBrowserComponent::warnAboutOverwriting,
            [complete = std::move(complete)](const auto& dialog) { complete(dialog.getResult()); });
    }

    void requestDrag(PressId press)
    {
        auto intent = std::exchange(dragIntent, {});
        if (choosing || dragging || !intent || !isCurrent(*intent)) return;
        juce::MessageManager::callAsync([weak = juce::WeakReference<ExportFileInvocations>(this), intent = *intent, press]
        {
            if (weak != nullptr) weak->startDrag(intent, press);
        });
    }

    void startDrag(const Intent& intent, PressId press)
    {
        if (choosing || dragging || !isCurrent(intent) || !isPressActive(press)) return;
        const auto file = readyFile(intent.scope);
        if (file == juce::File{} || !file.existsAsFile())
        {
            tile.finishPress(press);
            report("The ready WAV is unavailable. Use Save WAV or prepare the mix again.");
            return;
        }

        dragging = true;
        tile.setDragFeedbackActive(true);
        const auto weak = juce::WeakReference<ExportFileInvocations>(this);
        const auto start = startExternalDrag;
        const auto canContinue = [weak, intent]
        {
            return weak != nullptr && weak->isCurrent(intent) && weak->tile.isShowing();
        };
        const auto result = start(file, [weak, intent, press, canContinue]
        {
            return canContinue() && weak->isPressActive(press) && weak->readyFile(intent.scope) != juce::File{};
        }, canContinue);
        if (weak == nullptr) return;
        weak->dragging = false;
        weak->tile.finishPress(press);
        weak->tile.setDragFeedbackActive(false);
        weak->tile.updateMouseCursor();
        if (!weak->isCurrent(intent)) return;
        using controls::WavDragResult;
        weak->report(result == WavDragResult::dropped ? "Place the WAV at song origin. Its managed file is retained."
             : result == WavDragResult::cancelled ? "Drag cancelled. Use Save WAV, or drag again."
             : "External dragging is unavailable. Use Save WAV.");
    }

    controls::FileDragTile& tile;
    SaveFile saveWav;
    ReadyFile readyFile;
    StartExternalDrag startExternalDrag;
    ChooseSave chooseSave;
    std::unique_ptr<juce::FileChooser> chooser;
    std::optional<Scope> current;
    std::optional<Intent> dragIntent;
    uint64_t scopeVersion = 0, saveSerial = 0;
    bool choosing = false, dragging = false;
    JUCE_DECLARE_WEAK_REFERENCEABLE(ExportFileInvocations)
};
}
