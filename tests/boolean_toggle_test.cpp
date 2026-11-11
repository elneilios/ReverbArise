#include "TestSupport.h"
#include "BooleanToggleReview.h"

using namespace m3::ui;
using namespace m3::ui::controls;

static void paintCases(ToggleForm form)
{
    BooleanToggle control("Enabled", form);
    control.setSize(220, 36);
    const auto body = control.indicatorBounds();
    const auto frame = body.expanded(5);
    REQUIRE(control.getLocalBounds().contains(frame));
    REQUIRE(!frame.intersects(control.labelBounds()));
    REQUIRE(body.getWidth() == (form == ToggleForm::track ? 44 : 18));
    const auto render = [&](CommandState state)
    {
        juce::Image image(juce::Image::ARGB, control.getWidth(), control.getHeight(), true);
        juce::Graphics graphics(image);
        paintToggleIndicator(graphics, body, form, state);
        return image;
    };
    const auto sample = [&](const juce::Image& image) { return image.getPixelAt(body.getCentreX(), body.getY() + 3); };
    for (const bool on : {false, true})
    {
        const auto idle = render({true, false, false, false, on});
        const auto over = render({true, true, false, false, on});
        const auto held = render({true, true, true, false, on});
        const auto focus = render({true, true, true, true, on});
        const auto unavailable = render({false, true, true, true, on});
        REQUIRE(sample(idle) != sample(over));
        REQUIRE(sample(over) != sample(held));
        REQUIRE(sample(held) == sample(focus));
        REQUIRE(sample(idle) != sample(unavailable));
        const auto corner = frame.getTopLeft().translated(2, 1);
        REQUIRE(held.getPixelAt(corner.x, corner.y).getAlpha() == 0);
        REQUIRE(focus.getPixelAt(corner.x, corner.y) == aqua);
        REQUIRE(unavailable.getPixelAt(corner.x, corner.y).getAlpha() == 0);
    }
    REQUIRE(sample(render({false, false, false, false, false})) != sample(render({false, false, false, false, true})));
}

static void inputCases(ToggleForm form)
{
    BooleanToggle control("Sample choice", form);
    control.setSize(240, 36);
    control.setVisible(true);
    juce::Component& mouseTarget = control;
    REQUIRE(control.getWantsKeyboardFocus() && control.getNumChildComponents() == 0);
    int preparations = 0, commits = 0;
    control.beforeInvocation = [&] { ++preparations; };
    control.onClick = [&] { ++commits; };
    const auto event = [&](juce::Point<int> position, juce::ModifierKeys modifiers)
    {
        const auto time = juce::Time::getCurrentTime();
        const auto point = position.toFloat();
        return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(), point, modifiers,
                                0, 0, 0, 0, 0, &control, &control, time, point, time, 1, false);
    };
    const auto indicator = control.indicatorBounds().getCentre();
    const auto rightPress = event(indicator, juce::ModifierKeys::rightButtonModifier);
    const auto rightDrag = event(indicator.translated(2, 0), juce::ModifierKeys::rightButtonModifier);
    mouseTarget.mouseEnter(rightPress);
    control.mouseDown(rightPress);
    mouseTarget.mouseDrag(rightDrag);
    REQUIRE(!control.isDown());
    // Reproduce the held state JUCE's drag handler can set when a real
    // desktop pointer is over the control. Release still needs a valid press.
    control.setState(juce::Button::buttonDown);
    mouseTarget.mouseUp(rightDrag);
    REQUIRE(preparations == 0 && commits == 0 && !control.getToggleState());
    control.setState(juce::Button::buttonNormal);

    for (const auto point : {control.indicatorBounds().getCentre(), control.labelBounds().getCentre()})
    {
        control.mouseDown(event(point, juce::ModifierKeys::leftButtonModifier));
        REQUIRE(control.isDown());
        mouseTarget.mouseUp(event(point, {}));
    }
    REQUIRE(preparations == 2 && commits == 2 && !control.getToggleState());
    control.mouseDown(event(control.labelBounds().getCentre(), juce::ModifierKeys::leftButtonModifier));
    mouseTarget.mouseExit(event({-1, -1}, juce::ModifierKeys::leftButtonModifier));
    mouseTarget.mouseUp(event({-1, -1}, {}));
    REQUIRE(preparations == 3 && commits == 2);

    const auto pump = [] { juce::MessageManager::getInstance()->runDispatchLoopUntil(30); };
    for (const int key : {juce::KeyPress::spaceKey, juce::KeyPress::returnKey})
    {
        REQUIRE(control.keyPressed(juce::KeyPress(key)));
        pump();
    }
    REQUIRE(preparations == 5 && commits == 4 && !control.getToggleState());

    juce::SharedResourcePointer<FocusVisibility> focus;
    focus->pointerInput();
    const auto accessibility = control.createAccessibilityHandler();
    REQUIRE(accessibility->getTitle() == "Sample choice");
    REQUIRE(accessibility->getCurrentState().isCheckable());
    REQUIRE(accessibility->getActions().invoke(juce::AccessibilityActionType::focus) && focus->isVisible());
    REQUIRE(commits == 4);
    REQUIRE(accessibility->getActions().invoke(juce::AccessibilityActionType::toggle));
    pump();
    REQUIRE(preparations == 6 && commits == 5 && accessibility->getCurrentState().isChecked());

    control.setEnabled(false);
    REQUIRE(control.keyPressed(juce::KeyPress(juce::KeyPress::spaceKey)));
    REQUIRE(accessibility->getActions().invoke(juce::AccessibilityActionType::press));
    pump();
    REQUIRE(preparations == 8 && commits == 5 && accessibility->getCurrentState().isChecked());
    control.setToggleState(false, juce::dontSendNotification);
    REQUIRE(commits == 5 && !accessibility->getCurrentState().isChecked());
}

int main(int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI gui;
    return runTest([&]
    {
        if (argc > 1 && juce::String(argv[1]) == "--review")
        {
            toggleReview::run();
            return;
        }
        for (const auto form : {ToggleForm::track, ToggleForm::checkbox})
        {
            paintCases(form);
            inputCases(form);
        }
        toggleReview::Panel preview;
        preview.checkLayout();
        std::cout << "Switch and checkbox paint, activation, accessibility and preview layout passed.\n";
    });
}
