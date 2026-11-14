#pragma once

// Extraction contract: use the approved native faces/metrics, not system fonts.
// This independent table deliberately names the old role values for regression.
static void uiStandardsCases()
{
    using namespace m3::ui;
    struct Baseline {TextRole role;float height;InterFace face;juce::Colour ink;int line;};
    const std::array<Baseline,9> baseline{{
        {TextRole::sectionHeading,21,InterFace::semibold,text,30},
        {TextRole::controlLabel,14,InterFace::medium,text,24},
        {TextRole::helper,14,InterFace::regular,learnTypography::descriptionColour,20},
        {TextRole::ruler,14,InterFace::medium,text,20},
        {TextRole::emptyHeading,24,InterFace::semibold,text,32},
        {TextRole::emptyBody,14,InterFace::regular,learnTypography::descriptionColour,40},
        {TextRole::tagline,21,InterFace::italic,text,46},
        {TextRole::headerBody,14,InterFace::regular,headerBodyInk,32},
        {TextRole::controlValue,13,InterFace::regular,text,28}
    }};
    for(const auto& expected:baseline)
    {
        const auto role=textStyle(expected.role);const auto font=textFont(expected.role);
        REQUIRE(font.getTypefacePtr()==interTypeface(expected.face) && font.getHeight()==expected.height);
        REQUIRE(role.ink==expected.ink && role.lineHeight==expected.line);
        REQUIRE(font.getAscent()+font.getDescent()<=role.lineHeight+.01f);
        const auto oldFont=interFont(expected.height,expected.face);
        for(const auto copy:{"Source Capture","Automatic Swells","Marker Sensitivity","Agjp 0123456789 %"})
            REQUIRE(std::abs(juce::GlyphArrangement::getStringWidth(font,copy)-juce::GlyphArrangement::getStringWidth(oldFont,copy))<.01f);
        juce::Label label;applyTextRole(label,expected.role);
        REQUIRE(label.getFont().getTypefacePtr()==font.getTypefacePtr() && label.findColour(juce::Label::textColourId)==expected.ink);
        // Custom Graphics and labels must resolve the same face and ink.
        juce::Image actual(juce::Image::ARGB,320,role.lineHeight,true),old(juce::Image::ARGB,320,role.lineHeight,true);
        {juce::Graphics g(actual);applyTextRole(g,expected.role);g.drawText("Agjp 0123456789 %",actual.getBounds(),juce::Justification::centredLeft);}
        {juce::Graphics g(old);g.setFont(oldFont);g.setColour(expected.ink);g.drawText("Agjp 0123456789 %",old.getBounds(),juce::Justification::centredLeft);}
        for(int y=0;y<actual.getHeight();++y)for(int x=0;x<actual.getWidth();++x)REQUIRE(actual.getPixelAt(x,y)==old.getPixelAt(x,y));
    }
    REQUIRE(headerBodyInk!=learnTypography::descriptionColour && headerBodyInk!=secondary);
    juce::Component peer;peer.setBounds(80,80,500,240);peer.addToDesktop(0);peer.setVisible(true);peer.toFront(true);
    controls::IrSelector picker;peer.addAndMakeVisible(picker);picker.setBounds(24,24,260,44);
    picker.addItem("One",1);picker.addItem("Two",2);picker.addItem("Three",3);picker.setSelectedId(2,juce::dontSendNotification);
    unsigned invocations=0,changes=0,epoch=1,invoked=0;int accepted=2;
    picker.beforeInvocation=[&]{++invocations;invoked=epoch;};
    picker.onChange=[&]{++changes;if(invoked==epoch)accepted=picker.getSelectedId();else picker.setSelectedId(accepted,juce::dontSendNotification);};
    wheelOver(picker,1.f);wheelOver(picker,-1.f);juce::MessageManager::getInstance()->runDispatchLoopUntil(30);
    REQUIRE(picker.getSelectedId()==2 && invocations==0 && changes==0);
    REQUIRE(picker.keyPressed(juce::KeyPress(juce::KeyPress::downKey)));
    REQUIRE(picker.activePopup());picker.activePopup()->keyPressed(juce::KeyPress(juce::KeyPress::endKey));
    REQUIRE(changes==0 && picker.getSelectedId()==2);++epoch;
    picker.activePopup()->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));
    juce::MessageManager::getInstance()->runDispatchLoopUntil(30);
    REQUIRE(invocations==1 && changes==1 && accepted==2 && picker.getSelectedId()==2);
    REQUIRE(picker.keyPressed(juce::KeyPress(juce::KeyPress::downKey)));REQUIRE(picker.activePopup());
    picker.activePopup()->keyPressed(juce::KeyPress(juce::KeyPress::endKey));
    picker.activePopup()->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));juce::MessageManager::getInstance()->runDispatchLoopUntil(30);
    REQUIRE(accepted==3 && changes==2 && invocations==2);
    picker.setSelectedId(1,juce::dontSendNotification);juce::MessageManager::getInstance()->runDispatchLoopUntil(30);REQUIRE(changes==2);
}

static void uiStandardsLayoutCases(PreverbAudioProcessorEditor& editor,const juce::String& state)
{
    using namespace m3::ui;
    auto* learn=dynamic_cast<LearnWorkspace*>(named(editor,"learnWorkspace"));REQUIRE(learn);
    auto* heading=dynamic_cast<juce::Label*>(named(editor,"learnCaptureHeading"));
    auto* helper=dynamic_cast<juce::Label*>(named(editor,"learnAutomaticHelp"));REQUIRE(heading && helper);
    const auto old=editor.getBounds();
    for(const auto size:{juce::Point<int>{1100,850},juce::Point<int>{1199,900},juce::Point<int>{1200,900},
        juce::Point<int>{1239,900},juce::Point<int>{1240,900},juce::Point<int>{1400,900},juce::Point<int>{1400,1000},juce::Point<int>{1600,1000}})
    {
        editor.setSize(size.x,size.y);editor.refresh();
        const auto chart=learn->timelineBounds();REQUIRE(chart.getWidth()>0 && chart.getHeight()>0);
        for(auto* child:learn->getChildren())if(child->isVisible() && !learn->getLocalBounds().contains(child->getBounds()))
            throw std::runtime_error(("Learn control outside bounds: "+child->getName()+" at "+child->getBounds().toString()
                +" in "+learn->getLocalBounds().toString()).toStdString());
        REQUIRE(juce::GlyphArrangement::getStringWidth(heading->getFont(),heading->getText())<=heading->getWidth());
        REQUIRE(helper->getText().isEmpty()?helper->getHeight()==0:helper->getHeight()>=textStyle(TextRole::helper).lineHeight);
        for(const auto* name:{"learnIRLabel","learnSensitivityLabel"})
        {
            auto* label=dynamic_cast<juce::Label*>(named(editor,name));REQUIRE(label);
            if(label->isVisible())REQUIRE(juce::GlyphArrangement::getStringWidth(label->getFont(),label->getText())<=label->getWidth());
        }
        for(const auto* name:{"recaptureExisting","learnAutomaticSwells"})
        {
            auto* toggle=dynamic_cast<controls::Switch*>(named(*learn,name));REQUIRE(toggle);
            REQUIRE(toggle->getLocalBounds().toFloat().contains(toggle->focusInkBounds()));
            REQUIRE(juce::GlyphArrangement::getStringWidth(componentTextFont(*toggle,TextRole::controlLabel),toggle->getButtonText())
                    <= toggle->labelBounds().getWidth());
        }
        if(learn->showsEmptyPrompt())for(const auto& region:learn->emptyStateRegions())REQUIRE(chart.contains(region));
        for(float scale:{1.f,1.25f,1.5f,2.f})
        {
            const auto image=learn->createComponentSnapshot(learn->getLocalBounds(),true,scale);
            REQUIRE(image.isValid() && image.getWidth()==juce::roundToInt(learn->getWidth()*scale));
        }
        snapshotEditor(editor,"standards-"+state+"-"+juce::String(size.x)+"x"+juce::String(size.y));
    }
    editor.setSize(old.getWidth(),old.getHeight());editor.refresh();
}
