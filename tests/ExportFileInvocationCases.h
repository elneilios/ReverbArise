#pragma once
#include "TestSupport.h"
#include <UI/ExportFileInvocations.h>

namespace exportFileInvocationReview
{
using m3::ui::ExportFileInvocations;
using m3::ui::controls::FileDragTile;
using m3::ui::controls::CapturedActionButton;
using m3::ui::controls::WavDragResult;

inline void cases()
{
    CapturedActionButton save("Save WAV");
    FileDragTile tile;
    juce::Component host;
    host.setSize(390, 300);
    host.addAndMakeVisible(save);
    host.addAndMakeVisible(tile);
    save.setBounds(10, 8, 140, 36);
    tile.setBounds(10, 52, 360, 236);
    host.addToDesktop(juce::ComponentPeer::windowIsTemporary);
    host.setVisible(true);
    host.setAlwaysOnTop(true);

    ExportFileInvocations::Scope current{3, 7, {.path = "exports/first.wav"}};
    ExportFileInvocations::SaveResult completeSave;
    juce::StringArray deliveries, handedOff;
    int choosers = 0;
    juce::TemporaryFile readyWav(".wav");
    REQUIRE(readyWav.getFile().replaceWithText("Already prepared by the worker"));
    auto readyPath = readyWav.getFile();
    auto result = WavDragResult::dropped;
    juce::String status;
    auto binding = std::make_unique<ExportFileInvocations>(save, tile,
        [&](const auto& scope, const juce::File& destination)
        {
            REQUIRE(destination != juce::File{});
            deliveries.add("Save " + scope.file.path);
        },
        [&](const auto& scope) { return scope == current ? readyPath : juce::File{}; },
        [&](const juce::File& path, const auto& canStart, const auto& canContinue)
        {
            REQUIRE(canStart() && canContinue() && tile.hasSolidBorder());
            handedOff.add(path.getFullPathName());
            return result;
        },
        [&](const auto& scope, auto complete)
        {
            REQUIRE(scope == current);
            ++choosers;
            completeSave = std::move(complete);
        });
    binding->setCurrent(current);
    bool deviceDown = true;
    binding->pointerStillDown = [&](auto) { return deviceDown; };
    binding->onStatus = [&](const auto& message) { status = message; };

    auto stamp = juce::Time::currentTimeMillis();
    const auto nextPress = [&] { return juce::Time(++stamp); };
    const auto mouse = [](juce::Component& target, juce::Time down, int dx, juce::ModifierKeys mods)
    {
        const auto start = target.getLocalBounds().getCentre().toFloat();
        return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(), start.translated(float(dx), 0),
            mods, 0, 0, 0, 0, 0, &target, &target, down, start, down, 1, dx != 0);
    };
    const auto primary = juce::ModifierKeys::leftButtonModifier;
    const auto right = juce::ModifierKeys::rightButtonModifier;
    const auto pump = [] { juce::MessageManager::getInstance()->runDispatchLoopUntil(20); };
    const auto destination = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("export-invocation-test.wav");

    auto press = nextPress();
    tile.mouseDown(mouse(tile, press, 0, right));
    tile.mouseDrag(mouse(tile, press, 12, right));
    tile.mouseUp(mouse(tile, press, 12, {}));
    REQUIRE(deliveries.isEmpty() && choosers == 0);

    press = nextPress();
    tile.mouseDown(mouse(tile, press, 0, primary));
    tile.mouseDrag(mouse(tile, nextPress(), 12, primary));
    tile.mouseDrag(mouse(tile, press, 3, primary));
    REQUIRE(deliveries.isEmpty());
    current.file.path = "exports/second.wav";
    binding->setCurrent(current);
    tile.mouseDrag(mouse(tile, press, 12, primary));
    tile.mouseUp(mouse(tile, press, 12, {}));
    REQUIRE(deliveries.isEmpty() && choosers == 0);

    press = nextPress();
    save.mouseDown(mouse(save, press, 0, primary));
    ++current.pass;
    binding->setCurrent(current);
    save.mouseUp(mouse(save, press, 0, {}));
    REQUIRE(choosers == 0);
    save.keyPressed(juce::KeyPress(juce::KeyPress::returnKey));
    ++current.session;
    binding->setCurrent(current);
    pump();
    REQUIRE(choosers == 0);

    save.keyPressed(juce::KeyPress(juce::KeyPress::returnKey));
    pump();
    REQUIRE(choosers == 1 && binding->isChoosing());
    completeSave({});
    completeSave(destination); // A repeated native completion cannot revive cancellation.
    REQUIRE(!binding->isChoosing() && deliveries.isEmpty());
    save.keyPressed(juce::KeyPress(juce::KeyPress::returnKey));
    pump();
    ++current.pass;
    binding->setCurrent(current);
    completeSave(destination);
    REQUIRE(!binding->isChoosing() && deliveries.isEmpty());

    press = nextPress();
    tile.mouseDown(mouse(tile, press, 0, primary));
    tile.mouseDrag(mouse(tile, press, 6, primary));
    tile.mouseDrag(mouse(tile, press, 12, primary));
    REQUIRE(handedOff.isEmpty());
    pump();
    tile.mouseUp(mouse(tile, press, 12, {}));
    REQUIRE(handedOff.size() == 1 && handedOff[0] == readyPath.getFullPathName() && choosers == 2);
    REQUIRE(!tile.hasSolidBorder() && status.contains("song origin"));

    press = nextPress();
    tile.mouseDown(mouse(tile, press, 0, primary));
    tile.mouseDrag(mouse(tile, press, 6, primary));
    tile.mouseUp(mouse(tile, press, 6, {}));
    press = nextPress();
    tile.mouseDown(mouse(tile, press, 0, primary));
    pump();
    REQUIRE(handedOff.size() == 1);
    tile.mouseDrag(mouse(tile, press, 6, primary));
    result = WavDragResult::cancelled;
    pump();
    tile.mouseUp(mouse(tile, press, 6, {}));
    REQUIRE(handedOff.size() == 2 && !tile.hasSolidBorder() && status.contains("cancelled"));

    press = nextPress();
    tile.mouseDown(mouse(tile, press, 0, primary));
    tile.mouseDrag(mouse(tile, press, 6, primary));
    result = WavDragResult::unavailable;
    pump();
    tile.mouseUp(mouse(tile, press, 6, {}));
    REQUIRE(handedOff.size() == 3 && !tile.hasSolidBorder() && status.contains("Use Save WAV"));

    const auto previousPath = readyPath;
    readyPath = readyPath.getSiblingFile("p8-missing-" + juce::Uuid().toString() + ".wav");
    press = nextPress();
    tile.mouseDown(mouse(tile, press, 0, primary));
    tile.mouseDrag(mouse(tile, press, 6, primary));
    pump();
    tile.mouseUp(mouse(tile, press, 6, {}));
    REQUIRE(handedOff.size() == 3 && !tile.hasSolidBorder() && status.contains("unavailable"));
    readyPath = previousPath;

    auto* accessible = tile.getAccessibilityHandler();
    REQUIRE(accessible && accessible->getActions().invoke(juce::AccessibilityActionType::press));
    pump();
    REQUIRE(choosers == 3);
    completeSave(destination);
    completeSave(destination);
    REQUIRE(deliveries.size() == 1 && deliveries[0] == "Save exports/second.wav");

    press = nextPress();
    tile.mouseDown(mouse(tile, press, 0, primary));
    tile.mouseDrag(mouse(tile, press, 6, primary));
    deviceDown = false;
    pump();
    tile.mouseUp(mouse(tile, press, 6, {}));
    REQUIRE(handedOff.size() == 3);
    tile.keyPressed(juce::KeyPress(juce::KeyPress::returnKey));
    pump();
    REQUIRE(choosers == 4 && binding->isChoosing());
    binding.reset();
    completeSave(destination);
    REQUIRE(deliveries.size() == 1);

    // Drive the peer so JUCE records actual source buttons and down timestamps.
    // The binding uses its production pointer check for these events.
    int nativeSaves = 0, nativeHandoffs = 0;
    ExportFileInvocations nativeBinding(save, tile,
        [&](const auto&, const auto&) { ++nativeSaves; },
        [&](const auto&) { return readyPath; },
        [&](const auto& path, const auto& canStart, const auto& canContinue)
        {
            REQUIRE(path == readyPath && canStart() && canContinue() && tile.hasSolidBorder());
            ++nativeHandoffs;
            return WavDragResult::dropped;
        },
        [&](const auto&, auto complete) { complete(destination); });
    nativeBinding.setCurrent(current);
    auto* peer = host.getPeer();
    REQUIRE(peer);
    const auto send = [&](juce::Point<int> point, juce::ModifierKeys mods)
    {
        peer->handleMouseEvent(juce::MouseInputSource::InputSourceType::mouse, point.toFloat(),
            mods, 0.0f, 0.0f, ++stamp);
    };
    const auto savePoint = save.getBounds().getCentre();
    send(savePoint, primary);
    send(savePoint, {});
    REQUIRE(nativeSaves == 1);

    const auto tilePoint = tile.getBounds().getCentre();
    const auto farPoint = tilePoint.translated(12, 0);
    send(tilePoint, {});
    const auto source = juce::Desktop::getInstance().getMainMouseSource();
    REQUIRE(source.getComponentUnderMouse() == &tile);
    send(tilePoint, primary);
    REQUIRE(source.isDragging() && source.getLastMouseDownTime() == juce::Time(stamp));
    send(farPoint, primary);
    pump();
    send(farPoint, {});
    REQUIRE(nativeHandoffs == 1 && nativeSaves == 1 && !tile.hasSolidBorder());

    send(tilePoint, primary);
    send(farPoint, primary);
    send(farPoint, {});
    // Drain the cancelled drag before the next gesture. Keep the peer's
    // synthetic Save press and release together so native pointer events
    // cannot interrupt the simulated press while the message loop runs.
    pump();
    REQUIRE(nativeHandoffs == 1 && nativeSaves == 1 && !tile.hasSolidBorder());
    send(savePoint, {});
    send(savePoint, primary);
    send(savePoint, {});
    pump();
    REQUIRE(nativeHandoffs == 1 && nativeSaves == 2);

    send(tilePoint, {});
    send(tilePoint, primary);
    send(farPoint, primary);
    nativeBinding.setCurrent(std::nullopt);
    nativeBinding.setCurrent(current);
    pump();
    send(farPoint, {});
    REQUIRE(nativeHandoffs == 1 && nativeSaves == 2 && !tile.hasSolidBorder());
    host.removeFromDesktop();
}
}
