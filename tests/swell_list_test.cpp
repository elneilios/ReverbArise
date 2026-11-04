#include "TestSupport.h"
#include <UI/Controls/SwellList.h>
#include <juce_gui_extra/juce_gui_extra.h>

using namespace m3::ui::controls;
static std::vector<SwellListRow> fixture(int count=50)
{
    std::vector<SwellListRow> rows;
    for(int i=count-1;i>=0;--i)
    {SwellListRow row;row.id="id-"+juce::String(i).paddedLeft('0',4);row.arrival=i*48000;row.name="Custom "+juce::String(count-i);row.irName="Preverb Hall";row.status="Ready";rows.push_back(row);}
    return rows;
}
static void click(juce::Component& root,juce::Point<int> point,juce::ModifierKeys mods={})
{
    auto* target=root.getComponentAt(point);REQUIRE(target);const auto p=target->getLocalPoint(&root,point).toFloat();
    const auto time=juce::Time::getCurrentTime();
    const auto event=[&](juce::ModifierKeys keys){return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),p,keys,0,0,0,0,0,target,target,time,p,time,1,false);};
    target->mouseDown(event(mods));target->mouseUp(event({}));
}
static void cases()
{
    m3::ui::WorkspaceTheme theme; // Same embedded fonts and row theme as production.
    SwellList list;list.setLookAndFeel(&theme);list.setBounds(0,0,260,300);list.setVisible(true);
    auto rows=fixture();const auto first=juce::String("id-0000");int notifications=0;
    std::vector<juce::String> selection;juce::String primary,toggled,menuId;
    list.onSelection=[&](auto ids,auto id){++notifications;selection=std::move(ids);primary=id;};
    list.onToggleEnabled=[&](auto id){toggled=id;};
    list.onMenu=[&](auto row,const auto& e){menuId=row.id;REQUIRE(SwellList::menuAnchor(e).getPosition()==e.getScreenPosition());};
    list.display(rows,{first},first);REQUIRE(notifications==0 && list.rowAt(0)->id==first);
    REQUIRE(list.getModel()->getNameForRow(0)=="Custom 50");
    click(list,{12,18});REQUIRE(toggled==first);
    click(list,{80,84});REQUIRE(primary=="id-0001");
    click(list,{80,148},juce::ModifierKeys::ctrlModifier);REQUIRE(selection.size()==2 && primary=="id-0002");
    click(list,{list.getWidth()-12,148});REQUIRE(menuId=="id-0002");
    list.display(rows,{"id-0001","id-0002"},"id-0002");
    const auto before=notifications;
    rows.back().arrival=10000000; // Original row zero now becomes the last row.
    list.display(rows,{"id-0001","id-0002"},"id-0002");
    REQUIRE(notifications==before && list.isRowSelected(list.rowFor("id-0001")) && list.primaryId()=="id-0002");
    REQUIRE(list.getLastRowSelected()==list.rowFor("id-0002"));
    list.keyPressed(juce::KeyPress(juce::KeyPress::downKey));REQUIRE(primary=="id-0003" && selection.size()==1);
    list.display(rows,{first},first);
    auto* viewport=list.getViewport();viewport->setViewPosition(0,15*64+17);
    const auto oldY=viewport->getViewPositionY();const auto anchor=list.rowAt(oldY/64)->id;
    const int offset=list.getRowPosition(list.rowFor(anchor),true).getY();
    SwellListRow earlier;earlier.id="earlier";earlier.arrival=-1;earlier.name="ZZZ";rows.push_back(earlier);
    const auto refreshNotifications=notifications;
    list.display(rows,{first},first);
    REQUIRE(viewport->getViewPositionY()==oldY+64);
    REQUIRE(list.getRowPosition(list.rowFor(anchor),true).getY()==offset);
    REQUIRE(notifications==refreshNotifications); // Offscreen primary stays offscreen.
    rows.front().name="Renamed";list.display(rows,{first},first);
    REQUIRE(viewport->getViewPositionY()==oldY+64);
    list.display(rows,{first},first);REQUIRE(viewport->getViewPositionY()==oldY+64);
    const auto neighbor=list.rowAt(list.rowFor(anchor)+1)->id;
    const int neighborOffset=list.getRowPosition(list.rowFor(neighbor),true).getY();
    std::erase_if(rows,[&](const auto& row){return row.id==anchor;});list.display(rows,{first},first);
    REQUIRE(list.getRowPosition(list.rowFor(neighbor),true).getY()==neighborOffset);
    // External deliberate selection is revealed once; later user scrolling wins.
    list.display(rows,{neighbor},neighbor);REQUIRE(list.getRowPosition(list.rowFor(neighbor),true).intersects(list.getLocalBounds()));
    for(auto& row:rows)if(row.id==neighbor)row.arrival=-2;
    list.display(rows,{neighbor},neighbor);REQUIRE(list.getRowPosition(list.rowFor(neighbor),true).getY()>=0);
    REQUIRE(list.primaryId()==neighbor && notifications==refreshNotifications);
    viewport->setViewPosition(0,1000);list.display(rows,{neighbor},neighbor);
    REQUIRE(viewport->getViewPositionY()==1000);
    list.display(rows,{neighbor},neighbor,2);REQUIRE(list.getRowPosition(list.rowFor(neighbor),true).getY()>=0);
    list.display({}, {},{},2);REQUIRE(list.getModel()->getNumRows()==0 && viewport->getViewPositionY()==0);
    const auto many=fixture(500);list.display(many,{first},first,2);
    REQUIRE(list.getModel()->getNumRows()==500 && list.getNumChildComponents()<30);
    for(float scale:{1.f,1.25f,1.5f,2.f})REQUIRE(list.createComponentSnapshot(list.getLocalBounds(),true,scale).isValid());
    // A selected row activates on release. Recycle that visible row between
    // press and release: the action must still address its pressed identity.
    rows=fixture(5);list.display(rows,{first},first,3);viewport->setViewPosition(0,0);
    const auto heldPress=[&]
    {
        const juce::Component::SafePointer<juce::Component> component(list.getComponentAt(juce::Point<int>{12,20}));REQUIRE(component);
        const auto at=juce::Time::getCurrentTime();const juce::Point<float> p{12,20};
        juce::MouseEvent e(juce::Desktop::getInstance().getMainMouseSource(),p,juce::ModifierKeys::leftButtonModifier,0,0,0,0,0,component,component,at,p,at,1,false);
        component->mouseDown(e);return std::make_pair(component,e);
    };
    auto held=heldPress();rows.back().arrival=500000;list.display(rows,{first},first,3);toggled.clear();
    REQUIRE(held.first);held.first->mouseUp(held.second);REQUIRE(toggled==first);
    viewport->setViewPosition(0,0);rows=fixture(5);list.display(rows,{first},first,4);auto heldReset=heldPress();
    list.display(rows,{first},first,5);toggled.clear();const auto staleNotifications=notifications;
    if(heldReset.first)heldReset.first->mouseUp(heldReset.second);REQUIRE(toggled.isEmpty() && notifications==staleNotifications);
    auto heldDeletion=heldPress();std::erase_if(rows,[&](const auto& row){return row.id==first;});list.display(rows,{}, {},5);toggled.clear();
    if(heldDeletion.first)heldDeletion.first->mouseUp(heldDeletion.second);REQUIRE(toggled.isEmpty());
    list.setLookAndFeel(nullptr);
}

// Lightweight native review, using the exact reusable list without a processor.
class Review final:public juce::Component
{
public:
    Review()
    {
        setLookAndFeel(&theme);addAndMakeVisible(list);addAndMakeVisible(info);
        info.setFont(m3::ui::textFont(m3::ui::TextRole::helper));info.setJustificationType(juce::Justification::topLeft);
        selected={"id-0009"};primary=selected.front();rows=fixture(40);
        for(size_t i=0;i<buttons.size();++i)
        {addAndMakeVisible(buttons[i]);buttons[i].setButtonText(juce::StringArray{"Add earlier target","Move selected earlier / later","Rename selected","Minimum / larger size"}[static_cast<int>(i)]);}
        buttons[0].onClick=[this]{SwellListRow row;row.id="added-"+juce::String(++added);row.arrival=-added*48000;row.name="Earlier custom "+juce::String(added);row.irName="Preverb Hall";row.status="Ready";rows.push_back(row);refresh();};
        buttons[1].onClick=[this]{for(auto& row:rows)if(row.id==primary)row.arrival=row.arrival<0?40*48000:-10*48000;refresh();};
        buttons[2].onClick=[this]{for(auto& row:rows)if(row.id==primary)row.name=row.name=="AAA arbitrary"?"ZZZ arbitrary":"AAA arbitrary";refresh();};
        buttons[3].onClick=[this]{if(auto* top=getTopLevelComponent())top->setSize(top->getWidth()<1200?1400:1100,top->getWidth()<1200?900:850);};
        list.onSelection=[this](auto ids,auto id){selected=std::move(ids);primary=id;refresh();};
        list.onToggleEnabled=[this](auto id){for(auto& row:rows)if(row.id==id)row.enabled=!row.enabled;refresh();};
        list.onMenu=[this](auto row,const auto& e)
        {
            juce::PopupMenu menu;menu.addItem(1,"Rename selected");menu.addItem(2,"Delete this swell");
            menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(e.eventComponent).withTargetScreenArea(SwellList::menuAnchor(e)),
                [safe=juce::Component::SafePointer<Review>(this),id=row.id](int result)
                {if(!safe)return;if(result==1)for(auto& w:safe->rows)if(w.id==id)w.name="Menu rename";
                    if(result==2){std::erase_if(safe->rows,[&](const auto& w){return w.id==id;});std::erase(safe->selected,id);if(safe->primary==id)safe->primary={};}safe->refresh();});
        };
        setSize(1100,850);refresh();
    }
    ~Review()override{setLookAndFeel(nullptr);}
    void paint(juce::Graphics& g)override{g.fillAll(m3::ui::background);}
    void resized()override
    {
        auto area=getLocalBounds().reduced(24);list.setBounds(area.removeFromLeft(260));area.removeFromLeft(24);
        for(auto& b:buttons){b.setBounds(area.removeFromTop(36));area.removeFromTop(12);}info.setBounds(area);
    }
private:
    m3::ui::WorkspaceTheme theme;SwellList list;juce::Label info;std::array<juce::TextButton,4> buttons;
    std::vector<SwellListRow> rows;std::vector<juce::String> selected;juce::String primary;int added=0;
    void refresh()
    {
        list.display(rows,selected,primary);
        const auto row=std::find_if(rows.begin(),rows.end(),[&](const auto& w){return w.id==primary;});
        info.setText("Issue #4 - native Swells list interaction review\n\nSelect rows with mouse, Ctrl/Shift or keyboard. Checkbox and menu actions follow IDs. Scroll away from the selection, then insert an earlier target. Move a visible selected target across the list, and rename it without changing its order.\n\nPrimary: "+primary+"\nSelected: "+juce::String(static_cast<int>(selected.size()))+"\nAbsolute target sample: "+(row==rows.end()?juce::String("none"):juce::String(row->arrival))+"\n\nUses existing Perform row appearance. No audio, source capture or project state is touched.",juce::dontSendNotification);
    }
};
class ReviewWindow final:public juce::DocumentWindow
{
public:
    bool closed=false;
    ReviewWindow():DocumentWindow("Issue #4 - Swells list review",m3::ui::background,allButtons)
    {
        setUsingNativeTitleBar(true);setContentOwned(new Review,true);setResizable(true,false);centreWithSize(1100,850);setVisible(true);
        const auto output=juce::SystemStats::getEnvironmentVariable("PREVERB_SWELL_LIST_OUTPUT",{});
        if(output.isNotEmpty())
        {
            juce::File directory(output);REQUIRE(directory.createDirectory().wasOk());
            for(const auto size:{juce::Point<int>{1100,850},juce::Point<int>{1400,900}})
            {
                setSize(size.x,size.y);
                const auto image=getContentComponent()->createComponentSnapshot(getContentComponent()->getLocalBounds());
                auto stream=directory.getChildFile("list-review-"+juce::String(size.x)+"x"+juce::String(size.y)+".png").createOutputStream();
                REQUIRE(stream);juce::PNGImageFormat png;REQUIRE(png.writeImageToStream(image,*stream));
            }
            centreWithSize(1100,850);
        }
    }
    void closeButtonPressed()override{closed=true;setVisible(false);juce::MessageManager::getInstance()->stopDispatchLoop();}
};
int main(int argc,char** argv)
{
    juce::ScopedJuceInitialiser_GUI gui;
    return runTest([&]{cases();if(argc>1 && juce::String(argv[1])=="--review")
        {ReviewWindow window;juce::MessageManager::getInstance()->runDispatchLoop();}});
}
