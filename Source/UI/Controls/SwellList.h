#pragma once
#include "SwellListOrder.h"
#include <UI/WorkspaceTheme.h>
#include <set>
#include <utility>

namespace m3::ui::controls
{
struct SwellListRow
{
    juce::String id,name,irName,status;
    int64_t arrival=0;
    bool enabled=true,muted=false,solo=false,frozen=false,error=false;
};

// A specialized virtualized presentation control. Bindings own model/session
// authority; callbacks contain identities, never indices into another snapshot.
// Standards revision 2: scoped list interaction approval, existing row appearance.
class SwellList final:public juce::ListBox,private juce::ListBoxModel
{
public:
    SwellList():juce::ListBox("Swells",nullptr)
    {
        setModel(this);setName("swellList");setMultipleSelectionEnabled(true);
        setRowHeight(64);setColour(juce::ListBox::backgroundColourId,background);
    }
    ~SwellList()override{setModel(nullptr);}
    std::function<void(std::vector<juce::String>,juce::String)> onSelection;
    std::function<void(juce::String,uint64_t)> onReveal;
    std::function<void(juce::String)> onToggleEnabled;
    std::function<void(SwellListRow,const juce::MouseEvent&)> onMenu;

    const SwellListRow* rowAt(int row)const noexcept
    {return juce::isPositiveAndBelow(row,static_cast<int>(rows.size()))?&rows[static_cast<size_t>(row)]:nullptr;}
    int rowFor(const juce::String& id)const noexcept
    {for(size_t i=0;i<rows.size();++i)if(rows[i].id==id)return static_cast<int>(i);return -1;}
    const juce::String& primaryId()const noexcept{return primary;}
    static juce::Rectangle<int> menuAnchor(const juce::MouseEvent& event)
    {
        // Keep fractional coordinates through nested user/host transforms and
        // round once at the native popup boundary.
        const auto p=event.eventComponent->localPointToGlobal(event.position).roundToInt();
        return {p.x,p.y,1,1};
    }

    void display(std::vector<SwellListRow> next,const std::vector<juce::String>& selection,
                 const juce::String& nextPrimary,uint64_t sessionEpoch=0)
    {
        const juce::ScopedValueSetter<bool> guard(updating,true);
        auto* viewport=getViewport();const int oldY=viewport->getViewPositionY(),height=viewport->getViewHeight(),rh=getRowHeight();
        const bool reset=epoch!=sessionEpoch;epoch=sessionEpoch;
        std::vector<std::pair<juce::String,int>> anchors;
        if(!reset)for(int i=oldY/rh;i<static_cast<int>(rows.size()) && i*rh<oldY+height;++i)
            anchors.emplace_back(rows[static_cast<size_t>(i)].id,i*rh-oldY);
        const auto oldPrimaryRow=rowFor(primary);const auto* oldPrimary=rowAt(oldPrimaryRow);
        const bool wasVisible=!reset && oldPrimary && oldPrimaryRow*rh<oldY+height && (oldPrimaryRow+1)*rh>oldY;
        const auto oldArrival=oldPrimary?oldPrimary->arrival:int64_t{};
        const bool primaryChanged=reset || primary!=nextPrimary;
        const auto order=chronologicalSwellOrder(std::span<const SwellListRow>(next));
        std::vector<SwellListRow> sorted;sorted.reserve(next.size());
        for(auto index:order)sorted.push_back(std::move(next[index]));
        bool changed=reset || sorted.size()!=rows.size();
        if(!changed)for(size_t i=0;i<rows.size();++i)if(sorted[i].id!=rows[i].id){changed=true;break;}
        rows=std::move(sorted);primary=nextPrimary;
        if(changed)updateContent();
        juce::SparseSet<int> selectedRows;
        const std::set<juce::String> selectedIds(selection.begin(),selection.end());
        for(size_t i=0;i<rows.size();++i)
            if(selectedIds.contains(rows[i].id))
                selectedRows.addRange({static_cast<int>(i),static_cast<int>(i+1)});
        const int primaryRow=rowFor(primary);
        if(changed || selectedRows!=getSelectedRows() || getLastRowSelected()!=primaryRow)
        {
            // JUCE's last-selected row is also its keyboard/shift-selection
            // anchor. Reinstall it explicitly without forwarding model edits.
            setSelectedRows(selectedRows,juce::dontSendNotification);
            if(primaryRow>=0 && selectedRows.contains(primaryRow))
            {deselectRow(primaryRow);selectRow(primaryRow,true,false);}
        }
        if(reset)viewport->setViewPosition(0,0);
        else if(changed)
        {
            int anchoredY=oldY;
            for(const auto& [id,offset]:anchors)if(const int row=rowFor(id);row>=0)
            {anchoredY=row*rh-offset;break;}
            viewport->setViewPosition(viewport->getViewPositionX(),std::max(0,anchoredY));
        }
        const auto* current=rowAt(primaryRow);
        if(current && (primaryChanged || (wasVisible && oldArrival!=current->arrival)))
            scrollToEnsureRowIsOnscreen(primaryRow);
        repaint();
    }

private:
    // JUCE recycles row components during refresh. Pin a press before selection
    // callbacks can refresh/reorder, including a press held across a timer tick.
    class RowSurface final:public juce::Component
    {
    public:
        explicit RowSurface(SwellList& list):owner(list){setAccessible(false);}
        void display(juce::String id,uint64_t token){identity=std::move(id);shownEpoch=token;}
        void mouseDown(const juce::MouseEvent& e)override
        {
            pressedIdentity=identity;pressedEpoch=shownEpoch;
            const auto mode=owner.getViewport()->getScrollOnDragMode();
            const bool scrolling=mode==juce::Viewport::ScrollOnDragMode::all
                || (mode==juce::Viewport::ScrollOnDragMode::nonHover && !e.source.canHover());
            selectOnUp=!owner.getRowSelectedOnMouseDown() || owner.isRowSelected(owner.rowFor(identity)) || scrolling;
            if(!selectOnUp)invoke(e,false);
        }
        void mouseUp(const juce::MouseEvent& e)override
        {
            const auto pending=std::exchange(selectOnUp,false);
            if(pending && !e.mouseWasDraggedSinceMouseDown() && !owner.getViewport()->isCurrentlyScrollingOnDrag())invoke(e,true);
        }
    private:
        SwellList& owner;juce::String identity,pressedIdentity;
        uint64_t shownEpoch=0,pressedEpoch=0;bool selectOnUp=false;
        void invoke(const juce::MouseEvent& e,bool up)
        {
            if(owner.updating || !owner.isEnabled() || pressedEpoch!=owner.epoch)return;
            const int row=owner.rowFor(pressedIdentity);if(row<0)return;
            const auto id=pressedIdentity;const auto token=pressedEpoch;
            const juce::Component::SafePointer<SwellList> safe(&owner);
            const juce::Component::SafePointer<juce::Component> source(e.eventComponent);
            {const juce::ScopedValueSetter<bool> guard(owner.pointerSelection,true);
                owner.selectRowsBasedOnModifierKeys(row,e.mods,up);}
            if(safe && source && safe->epoch==token)safe->listBoxItemClicked(safe->rowFor(id),e);
        }
    };
    juce::Component* refreshComponentForRow(int row,bool,juce::Component* existing)override
    {
        auto* surface=dynamic_cast<RowSurface*>(existing);
        if(!surface){delete existing;surface=new RowSurface(*this);}
        surface->display(rowAt(row)?rowAt(row)->id:juce::String{},epoch);return surface;
    }
    std::vector<SwellListRow> rows;
    juce::String primary;
    bool updating=false,pointerSelection=false;
    uint64_t epoch=0;
    int getNumRows()override{return static_cast<int>(rows.size());}
    juce::String getNameForRow(int row)override
    {const auto* w=rowAt(row);return w?w->name:juce::String{};}
    void selectedRowsChanged(int row)override
    {
        if(updating)return;
        std::vector<juce::String> ids;
        for(int i=0;i<getNumRows();++i)if(isRowSelected(i))ids.push_back(rows[static_cast<size_t>(i)].id);
        const auto* w=rowAt(row);primary=w && isRowSelected(row)?w->id:ids.empty()?juce::String{}:ids.front();
        const auto id=primary;const auto token=epoch;const bool reveal=!pointerSelection;
        const juce::Component::SafePointer<SwellList> safe(this);
        if(onSelection)onSelection(std::move(ids),id);
        if(safe && reveal && safe->epoch==token && safe->primary==id && id.isNotEmpty() && safe->onReveal)safe->onReveal(id,token);
    }
    void listBoxItemClicked(int row,const juce::MouseEvent& event)override
    {
        const auto* found=rowAt(row);if(!found || updating)return;const auto w=*found;
        if(event.mods.isPopupMenu() || (event.eventComponent && event.x>=event.eventComponent->getWidth()-26))
        {if(onMenu)onMenu(w,event);return;}
        if(event.x<28){if(onToggleEnabled)onToggleEnabled(w.id);return;}
        if(primary.isNotEmpty() && onReveal)onReveal(primary,epoch);
    }
    void paintListBoxItem(int row,juce::Graphics& g,int width,int height,bool isSelected)override
    {
        const auto* found=rowAt(row);if(!found)return;const auto& w=*found;
        if(isSelected){g.setColour(aqua.withAlpha(.06f));g.fillRoundedRectangle(juce::Rectangle<float>(1,1,static_cast<float>(width-2),static_cast<float>(height-2)),4);g.setColour(aqua);g.drawRoundedRectangle(juce::Rectangle<float>(1,1,static_cast<float>(width-2),static_cast<float>(height-2)),4,1);}
        g.setColour(m3::ui::text);g.setFont(fontFor(*this,bodySize));g.drawText(w.name,32,2,width-62,20,juce::Justification::left);
        g.drawRect(8,8,15,15);if(w.enabled){g.setColour(aqua);g.fillRect(11,11,9,9);}
        g.setFont(fontFor(*this,smallSize));g.drawText(w.irName.isNotEmpty()?w.irName:"No IR",32,23,width-62,18,juce::Justification::left);
        g.setFont(fontFor(*this,captionSize));g.setColour(w.error?warning:secondary);
        g.drawText(w.status+(w.muted?" | mute":"")+(!w.enabled?" | off":"")+(w.solo?" | solo":"")+(w.frozen?" | frozen":""),32,43,width-62,height-43,juce::Justification::left);
        g.setColour(secondary);
        for(int n=0;n<3;++n)g.fillEllipse(juce::Rectangle<float>(3,3).withCentre({static_cast<float>(width-13),static_cast<float>(height/2-7+n*7)}));
    }
};
}
