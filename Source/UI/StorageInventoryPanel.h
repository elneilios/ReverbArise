#pragma once
#include <Storage/AssetInventory.h>
#include <UI/Controls/ActionButton.h>
#include <UI/Controls/EllipsisLabel.h>
#include <UI/UiGeometry.h>

namespace m3::ui
{
// Read-only Settings composition. No asset mutation controls.
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
        cancelButton.setName("cancelStorageScan");cancelButton.setTooltip("Stop the current storage scan. Reopening Storage starts a fresh scan.");
        cancelButton.onClick=[this]{service.cancel();setValue(5,shown?"Refresh stopped; showing scan from "+shown->scannedAt.formatted("%H:%M:%S"):"Scan stopped. Reopen Storage to scan again.");cancelButton.setVisible(false);resized();};addChildComponent(cancelButton);
        setValue(0,"This instance");setValue(5,"Storage is scanned when this page opens");startTimerHz(10);
    }
    ~StorageInventoryPanel()override{stopTimer();}
    void bind(Provider next){service.cancel();provider=std::move(next);clearResults();pageOpen=isShowing();if(pageOpen)begin();}
    void visibilityChanged()override{updateVisibility();}
    void parentHierarchyChanged()override{updateVisibility();}
    void poll()
    {
        updateVisibility();if(!pageOpen || service.busy())return;const auto report=service.snapshot();if(!report || report==shown)return;shown=report;cancelButton.setVisible(false);resized();
        setValue(1,juce::String(report->inventoryComplete?"":"At least ")+juce::File::descriptionOfSizeInBytes(static_cast<juce::int64>(report->physicalBytes))+" logical / "+juce::File::descriptionOfSizeInBytes(static_cast<juce::int64>(report->allocatedBytes))+" allocated");
        setValue(2,juce::String(report->files.size())+" paths / "+juce::String(report->physicalFiles)+" physical files");
        setValue(3,juce::String(report->observedCaptureSnapshots)+" observed snapshots; older FLPs unknown");
        setValue(4,juce::File::descriptionOfSizeInBytes(static_cast<juce::int64>(report->exactDuplicateBytes))+" byte-identical; "+juce::String(report->decodedEquivalentFiles)+" decoded-equivalent audio files");
        const auto status=report->inventoryComplete?"Scanned":"Incomplete";
        setValue(5,juce::String(status)+" at "+report->scannedAt.formatted("%H:%M:%S")+"; "+juce::String(report->missingReferences)+" unverified observed references");
        juce::String summary;for(const auto& [name,category]:report->categories)summary+=name+": "+juce::String(category.files)+" files - "+juce::File::descriptionOfSizeInBytes(static_cast<juce::int64>(category.bytes))+"\n";
        details.setText(summary,juce::dontSendNotification);details.setDescription(summary);
        juce::String uncertainty=help.getText()+"\nDecoded comparison skipped for "+juce::String(report->decodedSkipped)+" files.";for(const auto& issue:report->issues)uncertainty+="\n"+issue;for(const auto& issue:report->catalog.uncertainty)uncertainty+="\n"+issue;values[5].setDescription(uncertainty);values[5].setTooltip(uncertainty);
    }
    int preferredHeight(int width)const
    {
        return spacing::section+controlGeometry::headingHeight+spacing::related
            +static_cast<int>(labels.size())*(controlGeometry::labelHeight+spacing::tight)
            +(cancelButton.isVisible()?spacing::related+controlGeometry::height:0)
            +spacing::related+textHeight(details,width-2*spacing::section)+spacing::related+textHeight(help,width-2*spacing::section);
    }
    void resized()override
    {
        auto area=getLocalBounds().reduced(spacing::section,0);area.removeFromTop(spacing::section);heading.setBounds(area.removeFromTop(controlGeometry::headingHeight));area.removeFromTop(spacing::related);
        for(size_t i=0;i<labels.size();++i){auto row=area.removeFromTop(controlGeometry::labelHeight);labels[i].setBounds(row.removeFromLeft(row.getWidth()/3));values[i].setBounds(row);area.removeFromTop(spacing::tight);}
        if(cancelButton.isVisible()){area.removeFromTop(spacing::related);auto buttons=area.removeFromTop(controlGeometry::height);cancelButton.setBounds(buttons.removeFromLeft(buttons.getWidth()/2));}
        area.removeFromTop(spacing::related);details.setBounds(area.removeFromTop(textHeight(details,area.getWidth())));area.removeFromTop(spacing::related);help.setBounds(area.removeFromTop(textHeight(help,area.getWidth())));
    }
    const storage::Report* displayedReport()const{return shown.get();}
    bool scanning()const{return service.busy();}
    void refresh(){begin();}
private:
    static int textHeight(const juce::Label& label,int width)
    {juce::AttributedString text(label.getText().trimEnd());text.setFont(label.getFont());juce::TextLayout layout;layout.createLayout(text,static_cast<float>(std::max(1,width-label.getBorderSize().getLeftAndRight())));return static_cast<int>(std::ceil(layout.getHeight()))+label.getBorderSize().getTopAndBottom();}
    void clearResults(){shown.reset();details.setText({},juce::dontSendNotification);for(size_t i=0;i<values.size();++i)setValue(i,{});cancelButton.setVisible(false);}
    void updateVisibility()
    {
        const bool open=isShowing();if(open==pageOpen)return;pageOpen=open;
        if(open)begin();else{service.cancel();setValue(5,shown?"Last scan at "+shown->scannedAt.formatted("%H:%M:%S"):"Storage is scanned when this page opens");}
    }
    void begin()
    {
        auto request=provider();if(!shown)clearResults();juce::StringArray roots;for(const auto& root:request.roots)roots.add(root.getFullPathName());setValue(0,roots.isEmpty()?juce::String("Unavailable"):roots.joinIntoString("; "));
        service.request(std::move(request));setValue(5,shown?"Refreshing; showing scan from "+shown->scannedAt.formatted("%H:%M:%S"):"Scanning this instance's storage...");cancelButton.setVisible(true);resized();
    }
    void timerCallback()override{poll();}
    void setValue(size_t index,const juce::String& content){values[index].setText(content,juce::dontSendNotification);values[index].setDescription(content);values[index].setTooltip(content);}
    Provider provider;storage::InventoryService service;std::shared_ptr<const storage::Report> shown;
    bool pageOpen=false;
    juce::Label heading,details,help;std::array<juce::Label,6> labels;std::array<controls::EllipsisLabel,6> values;
    controls::ActionButton cancelButton{"Stop scan"};
};
}
