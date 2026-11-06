#pragma once
#include <Storage/AssetInventory.h>
#include <UI/Controls/ActionButton.h>
#include <UI/Controls/EllipsisLabel.h>
#include <UI/UiGeometry.h>

namespace m3::ui
{
// Settings composition staged for native review. No asset mutation controls.
class StorageInventoryPanel final:public FlatSurface,private juce::Timer
{
public:
    using Provider=std::function<storage::Request()>;
    explicit StorageInventoryPanel(Provider source):provider(std::move(source))
    {
        setName("storageInventory");heading.setText("Storage",juce::dontSendNotification);applyTextRole(heading,TextRole::sectionHeading);addAndMakeVisible(heading);
        const juce::StringArray captions{"Location","Storage","Files","Capture history","Duplicate bytes","Scan coverage"};
        for(size_t i=0;i<labels.size();++i){labels[i].setText(captions[static_cast<int>(i)],juce::dontSendNotification);applyTextRole(labels[i],TextRole::controlLabel);addAndMakeVisible(labels[i]);
            values[i].setName("storageValue"+juce::String(static_cast<int>(i)));values[i].setTitle(captions[static_cast<int>(i)]);applyTextRole(values[i],TextRole::helper);addAndMakeVisible(values[i]);}
        applyTextRole(details,TextRole::helper,juce::Justification::topLeft);details.setName("storageCategories");addAndMakeVisible(details);
        applyTextRole(help,TextRole::helper,juce::Justification::topLeft);help.setName("storageProtection");
        help.setText("All audio and history remain protected.\nOlder FLPs and external Playlist files cannot be fully enumerated.\nNo cleanup is available.",juce::dontSendNotification);help.setDescription(help.getText());addAndMakeVisible(help);
        inspectButton.setName("inspectStorage");inspectButton.setTooltip("Read file counts, sizes and duplicate identities for this selected scope. No files are changed.");inspectButton.onClick=[this]{begin();};addAndMakeVisible(inspectButton);
        cancelButton.setName("cancelStorageScan");cancelButton.onClick=[this]{service.cancel();setValue(5,"Cancelled; inspect again");inspectButton.setEnabled(true);cancelButton.setEnabled(false);};addAndMakeVisible(cancelButton);cancelButton.setEnabled(false);
        setValue(0,"This instance — inspect to show its path");setValue(5,"Not scanned");startTimerHz(10);
    }
    ~StorageInventoryPanel()override{stopTimer();}
    void bind(Provider next){service.cancel();provider=std::move(next);shown.reset();details.setText({},juce::dontSendNotification);for(size_t i=0;i<values.size();++i)setValue(i,{});setValue(5,"Scope changed; inspect again");}
    void begin()
    {
        auto request=provider();juce::StringArray roots;for(const auto& root:request.roots)roots.add(root.getFullPathName());setValue(0,roots.joinIntoString("; "));
        shown.reset();for(size_t i=1;i<values.size();++i)setValue(i,{});service.request(std::move(request));setValue(5,"Inspecting selected roots...");inspectButton.setEnabled(false);cancelButton.setEnabled(true);
    }
    void poll()
    {
        if(service.busy())return;const auto report=service.snapshot();if(!report || report==shown)return;shown=report;inspectButton.setEnabled(true);cancelButton.setEnabled(false);
        setValue(1,juce::String(report->inventoryComplete?"":"At least ")+juce::File::descriptionOfSizeInBytes(static_cast<juce::int64>(report->physicalBytes))+" logical / "+juce::File::descriptionOfSizeInBytes(static_cast<juce::int64>(report->allocatedBytes))+" allocated");
        setValue(2,juce::String(report->files.size())+" paths / "+juce::String(report->physicalFiles)+" physical files");
        setValue(3,juce::String(report->observedCaptureSnapshots)+" observed snapshots; older FLPs unknown");
        setValue(4,juce::File::descriptionOfSizeInBytes(static_cast<juce::int64>(report->exactDuplicateBytes))+" byte-identical; "+juce::String(report->decodedEquivalentFiles)+" decoded-equivalent audio files");
        const auto status=report->inventoryComplete?"Inspected":"Incomplete";
        setValue(5,juce::String(status)+" at "+report->scannedAt.formatted("%H:%M:%S")+"; "+juce::String(report->missingReferences)+" unverified observed references");
        juce::String summary;for(const auto& [name,category]:report->categories)summary+=name+": "+juce::String(category.files)+" files - "+juce::File::descriptionOfSizeInBytes(static_cast<juce::int64>(category.bytes))+"\n";

        details.setText(summary,juce::dontSendNotification);details.setDescription(summary);
        juce::String uncertainty=help.getText()+"\nDecoded comparison skipped for "+juce::String(report->decodedSkipped)+" files.";for(const auto& issue:report->issues)uncertainty+="\n"+issue;for(const auto& issue:report->catalog.uncertainty)uncertainty+="\n"+issue;values[5].setDescription(uncertainty);values[5].setTooltip(uncertainty);
    }
    void resized()override
    {
        auto area=getLocalBounds().reduced(spacing::section);heading.setBounds(area.removeFromTop(controlGeometry::headingHeight));area.removeFromTop(spacing::related);
        for(size_t i=0;i<labels.size();++i){auto row=area.removeFromTop(controlGeometry::labelHeight);labels[i].setBounds(row.removeFromLeft(row.getWidth()/3));values[i].setBounds(row);area.removeFromTop(spacing::tight);}
        area.removeFromTop(spacing::related);auto buttons=area.removeFromTop(controlGeometry::height);inspectButton.setBounds(buttons.removeFromLeft(buttons.getWidth()/2-spacing::related));buttons.removeFromLeft(spacing::related);cancelButton.setBounds(buttons);
        area.removeFromTop(spacing::related);help.setBounds(area.removeFromBottom(3*learnTypography::secondaryLineHeight));area.removeFromBottom(spacing::related);details.setBounds(area);
    }
    const storage::Report* displayedReport()const{return shown.get();}
    bool scanning()const{return service.busy();}
private:
    void timerCallback()override{poll();}
    void setValue(size_t index,const juce::String& text){values[index].setText(text,juce::dontSendNotification);values[index].setDescription(text);values[index].setTooltip(text);}
    Provider provider;storage::InventoryService service;std::shared_ptr<const storage::Report> shown;
    juce::Label heading,details,help;std::array<juce::Label,6> labels;std::array<controls::EllipsisLabel,6> values;
    controls::ActionButton inspectButton{"Inspect storage",controls::CommandIcon::none,true},cancelButton{"Cancel scan"};
};
}
