#pragma once
void Composition::verifyAuditionStrip()
{
    stopTimer();setScene(0);auto panel=std::make_unique<PreverbAudioProcessorEditor>(*processor);panel->stopAutomaticRefreshForReview();panel->setSize(1100,850);panel->refresh();
    const PerformLayout previous(panel->getLocalBounds());const auto initial=panel->timelineComponent().getBounds();
    REQUIRE(previous.audition.getHeight()==72); // Production profile stays approved until this checkpoint.
    panel->previewSlimAuditionStrip();
    REQUIRE(panel->timelineComponent().getHeight()==initial.getHeight()+20);
    for(int scene:{0,1,2})
    {
        setScene(scene);panel->refresh();
        for(const auto size:std::array<juce::Point<int>,6>{{{1100,850},{1199,850},{1200,850},{1400,900},{1400,1000},{1600,1000}}})
        {
            panel->setSize(size.x,size.y);const PerformLayout bounds(panel->getLocalBounds(),performGeometry::toolbarControlHeight+2*performGeometry::bandInset);
            auto& play=require<juce::Button>(*panel,"audition");auto& stop=require<juce::Button>(*panel,"stopAudition");auto& wet=require<controls::LatchingActionButton>(*panel,"auditionWetOnly");
            auto& status=require<controls::EllipsisLabel>(*panel,"auditionStatus");
            REQUIRE(bounds.audition.getHeight()==52 && bounds.audition.contains(play.getBounds()) && bounds.audition.contains(status.getBounds()));
            REQUIRE(stop.getX()-play.getRight()==wet.getX()-stop.getRight() && wet.getX()-stop.getRight()==8);
            REQUIRE(status.getX()>wet.getRight() && status.getY()==play.getY() && status.getHeight()==play.getHeight());
            REQUIRE(status.getWidth()>=600 && status.getFont().getHeight()==textStyle(TextRole::helper).height);
            REQUIRE(panel->timelineComponent().getBounds()==bounds.detail.withTrimmedTop(performGeometry::columnInset));
            for(const auto scale:{1.f,1.25f,1.5f,2.f})
            {
                const auto frame=panel->createComponentSnapshot(panel->getLocalBounds(),true,scale);REQUIRE(frame.isValid() && frame.getWidth()==juce::roundToInt(size.x*scale));
                if(scale==1.f)for(const auto y:{bounds.work.getBottom(),bounds.audition.getBottom(),bounds.overview.getBottom()})
                    for(int x=bounds.header.getX();x<bounds.header.getRight();++x)REQUIRE(frame.getPixelAt(x,y)==separator);
            }
        }
    }
    // Compare Overview title ink to the actual accepted Audition heading font.
    juce::Label* auditionHeading=nullptr;for(auto* child:panel->getChildren())if(auto* label=dynamic_cast<juce::Label*>(child);label && label->getText()=="Audition")auditionHeading=label;
    REQUIRE(auditionHeading && auditionHeading->getFont().getHeight()==textStyle(TextRole::sectionHeading).height);
    auto& overview=require<m3::SwellOverview>(*panel,"songOverview");const auto actual=overview.createComponentSnapshot(overview.getLocalBounds());
    juce::Image expected(juce::Image::ARGB,overview.getWidth(),controlGeometry::headingHeight,true);
    {juce::Graphics g(expected);g.fillAll(background);g.setColour(text);g.setFont(auditionHeading->getFont());g.drawText("Overview",expected.getBounds(),juce::Justification::centredLeft);}
    for(int y=0;y<expected.getHeight();++y)for(int x=0;x<160;++x)if(actual.getPixelAt(x,y)!=expected.getPixelAt(x,y))
    {
        std::cerr<<"Overview heading mismatch at "<<x<<","<<y<<" actual="<<actual.getPixelAt(x,y).toString()<<" expected="<<expected.getPixelAt(x,y).toString()<<" font="<<auditionHeading->getFont().toString()<<"\n";
        REQUIRE(false);
    }
}
