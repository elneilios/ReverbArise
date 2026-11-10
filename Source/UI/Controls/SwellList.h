#pragma once
#include "SwellListOrder.h"
#include "ActionButton.h"
#include "BooleanToggle.h"
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
// Row actions follow the same identity and epoch as selection across recycled rows.
class SwellList final:public juce::ListBox,private juce::ListBoxModel,private juce::KeyListener
{
public:
    SwellList():juce::ListBox("Swells",nullptr)
    {
        setModel(this);setName("swellList");setMultipleSelectionEnabled(true);
        setRowHeight(64);setColour(juce::ListBox::backgroundColourId,background);
    }
    ~SwellList()override
    {
        if(inspectorTabTarget)inspectorTabTarget->removeKeyListener(this);
        setModel(nullptr);
    }

    struct MenuRequest
    {
        juce::String id;
        uint64_t epoch;
        juce::Rectangle<int> anchor;
        bool keyboard;
    };
    std::function<void(std::vector<juce::String>,juce::String)> onSelection;
    std::function<void(juce::String,uint64_t)> onReveal;
    std::function<void(juce::String)> onToggleEnabled;
    std::function<void(MenuRequest)> onMenu;

    void setInspectorTabTarget(juce::Component& target)
    {
        if(inspectorTabTarget)inspectorTabTarget->removeKeyListener(this);
        inspectorTabTarget=&target;
        target.addKeyListener(this);
    }

    bool keyPressed(const juce::KeyPress& key)override
    {
        focusVisibility->keyboardInput();
        repaintRows();
        if(key.isKeyCode(juce::KeyPress::tabKey) && !key.getModifiers().isShiftDown())
        {
            if(focusPrimaryAction(false))return true;
            if(inspectorTabTarget){focusInspector();return true;}
        }
        return juce::ListBox::keyPressed(key);
    }

    void focusGained(FocusChangeType cause)override
    {
        juce::ListBox::focusGained(cause);
        if(cause==focusChangedByTabKey)focusVisibility->keyboardInput();
        repaintRows();
    }

    void focusLost(FocusChangeType cause)override
    {
        juce::ListBox::focusLost(cause);
        repaintRows();
    }

    void restoreMenuFocus(const juce::String& id,bool keyboard,bool more=true)
    {
        if(keyboard && focusRowAction(id,more))return;
        grabKeyboardFocus();
    }

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
        const auto focused=focusedAction();
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
        refreshVisibleActions();
        if(focused && !reset && hasKeyboardFocus(false))
            focusRowAction(focused->id,focused->more);
        repaintRows();
    }

private:
    // JUCE recycles row components during refresh. Pin a press before selection
    // callbacks can refresh/reorder, including a press held across a timer tick.
    class RowSurface final:public juce::Component,private juce::KeyListener
    {
    public:
        class EnabledButton final:public BooleanToggle
        {
        public:
            EnabledButton():BooleanToggle("Enabled",ToggleForm::checkbox){}
            void paintButton(juce::Graphics& g,bool over,bool down)override
            {
                paintToggleIndicator(g,indicatorBounds(),ToggleForm::checkbox,
                    {isEnabled(),over,down,isFocusVisible(),getToggleState()});
            }
        };

        class MoreButton final:public CommandButton
        {
        public:
            MoreButton():CommandButton("More actions",true){}
            void invokeCommand()override
            {
                if(!isEnabled())return;
                const juce::Component::SafePointer<MoreButton> safe(this);
                if(beforeInvocation)beforeInvocation();
                if(!safe || !safe->isEnabled())return;
                // Dispatch before this virtual row can be rebound by another event.
                const auto invoke=onClick;
                if(invoke)invoke();
            }
            void mouseDown(const juce::MouseEvent& event)override
            {
                if(!event.mods.isLeftButtonDown())return;
                const juce::ScopedValueSetter guard(pointerAnchor,std::optional{menuAnchor(event)});
                CommandButton::mouseDown(event);
            }
            juce::Rectangle<int> invocationAnchor()const
            {return pointerAnchor.value_or(getScreenBounds());}

            void paintButton(juce::Graphics& g,bool over,bool down)override
            {
                const CommandState state{isEnabled(),over,down,isFocusVisible(),false};
                if(over || down || state.focused)
                    paintCommandSurface(g,getLocalBounds(),CommandVariant::neutral,state);
                g.setColour(commandInk(CommandVariant::neutral,state).glyph);
                const auto centre=getLocalBounds().toFloat().getCentre();
                for(const auto dy:{-5.f,0.f,5.f})
                    g.fillEllipse(juce::Rectangle<float>(3.f,3.f).withCentre({centre.x,centre.y+dy}));
            }

        private:
            std::optional<juce::Rectangle<int>> pointerAnchor;
        };

        explicit RowSurface(SwellList& list):owner(list)
        {
            // The list supplies Tab entry to one row. Other visible rows remain
            // independently focusable through their accessibility actions.
            setFocusContainerType(FocusContainerType::keyboardFocusContainer);
            addAndMakeVisible(enabled);
            addAndMakeVisible(more);
            for(auto* action:{static_cast<juce::Button*>(&enabled),static_cast<juce::Button*>(&more)})
            {
                action->addMouseListener(this,false);
                action->addKeyListener(this);
            }
            enabled.beforeInvocation=[this]{toggleIntent={identity,shownEpoch};};
            more.beforeInvocation=[this]
            {
                menuIntent={identity,shownEpoch,more.invocationAnchor(),owner.focusVisibility->isVisible()};
            };
            enabled.onClick=[this]
            {
                auto intent=std::exchange(toggleIntent,{});
                if(owner.ownsRow(intent.id,intent.epoch) && owner.onToggleEnabled)owner.onToggleEnabled(intent.id);
            };
            more.onClick=[this]{owner.openMenu(std::exchange(menuIntent,{}));};
        }

        void display(const SwellListRow& row,uint64_t token)
        {
            if((identity!=row.id || shownEpoch!=token) && hasKeyboardFocus(true))owner.grabKeyboardFocus();
            identity=row.id;
            shownEpoch=token;
            enabled.setToggleState(row.enabled,juce::dontSendNotification);
            enabled.setButtonText("Enabled for "+row.name);
            enabled.setTooltip(enabled.getButtonText());
            more.setTitle("Actions for "+row.name);
            more.setTooltip(more.getTitle());
        }

        void resized()override
        {
            enabled.setBounds(1,1,30,30);
            more.setBounds(getWidth()-31,(getHeight()-30)/2,30,30);
        }

        std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler()override
        {return std::make_unique<juce::AccessibilityHandler>(*this,juce::AccessibilityRole::group);}

        const juce::String& displayedId()const noexcept{return identity;}
        EnabledButton enabled;
        MoreButton more;
        void mouseDown(const juce::MouseEvent& e)override
        {
            popupPress=e.mods.isPopupMenu();
            if(e.eventComponent!=this && !popupPress)return;
            pressedIdentity=identity;pressedEpoch=shownEpoch;
            if(popupPress){selectOnUp=true;return;}
            const auto mode=owner.getViewport()->getScrollOnDragMode();
            const bool scrolling=mode==juce::Viewport::ScrollOnDragMode::all
                || (mode==juce::Viewport::ScrollOnDragMode::nonHover && !e.source.canHover());
            selectOnUp=!owner.getRowSelectedOnMouseDown() || owner.isRowSelected(owner.rowFor(identity)) || scrolling;
            if(!selectOnUp)invoke(e,false);
        }
        void mouseUp(const juce::MouseEvent& e)override
        {
            if(e.eventComponent!=this && !popupPress)return;
            const auto pending=std::exchange(selectOnUp,false);
            const auto showMenu=std::exchange(popupPress,false);
            if(!pending || e.mouseWasDraggedSinceMouseDown() || owner.getViewport()->isCurrentlyScrollingOnDrag())return;
            if(showMenu)
            {
                owner.focusVisibility->pointerInput();
                owner.openMenu({pressedIdentity,pressedEpoch,menuAnchor(e),false});
            }
            else invoke(e,true);
        }
    private:
        bool keyPressed(const juce::KeyPress& key,juce::Component* source)override
        {
            if(!key.isKeyCode(juce::KeyPress::tabKey))return false;
            owner.focusVisibility->keyboardInput();
            const bool backwards=key.getModifiers().isShiftDown();
            if(source==&enabled)
            {
                if(backwards)owner.grabKeyboardFocus();
                else more.grabKeyboardFocus();
            }
            else if(backwards)enabled.grabKeyboardFocus();
            else owner.focusInspector();
            return true;
        }

        struct RowIntent{juce::String id;uint64_t epoch=0;};
        SwellList& owner;
        juce::String identity,pressedIdentity;
        uint64_t shownEpoch=0,pressedEpoch=0;
        RowIntent toggleIntent;
        MenuRequest menuIntent{};
        bool selectOnUp=false,popupPress=false;
        void invoke(const juce::MouseEvent& e,bool up)
        {
            if(!owner.ownsRow(pressedIdentity,pressedEpoch))return;
            const int row=owner.rowFor(pressedIdentity);
            owner.focusVisibility->pointerInput();
            owner.grabKeyboardFocus();
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
        if(const auto* item=rowAt(row))surface->display(*item,epoch);
        return surface;
    }
    bool keyPressed(const juce::KeyPress& key,juce::Component*)override
    {
        return key.isKeyCode(juce::KeyPress::tabKey) && key.getModifiers().isShiftDown() && focusPrimaryAction(true);
    }

    void repaintRows()
    {
        if(auto* content=getViewport()->getViewedComponent())content->repaint();
    }

    void focusInspector()
    {
        if(inspectorTabTarget && inspectorTabTarget->isShowing() && inspectorTabTarget->isEnabled())
            inspectorTabTarget->grabKeyboardFocus();
        else moveKeyboardFocusToSibling(true);
    }

    RowSurface* surfaceFor(int row)const
    {return dynamic_cast<RowSurface*>(getComponentForRowNumber(row));}

    bool focusRowAction(const juce::String& id,bool more)
    {
        if(auto* surface=surfaceFor(rowFor(id)))
        {
            auto& action=more?static_cast<juce::Button&>(surface->more):surface->enabled;
            const auto* viewport=getViewport();
            const auto* content=viewport->getViewedComponent();
            if(!content || !viewport->getViewArea().contains(content->getLocalArea(&action,action.getLocalBounds())))
                return false;

            focusVisibility->keyboardInput();
            action.grabKeyboardFocus();
            return action.hasKeyboardFocus(false);
        }
        return false;
    }

    bool focusPrimaryAction(bool more)
    {
        const auto row=rowFor(primary);
        if(row<0)return false;
        scrollToEnsureRowIsOnscreen(row);
        return focusRowAction(primary,more);
    }

    struct FocusedAction{juce::String id;bool more;};
    std::optional<FocusedAction> focusedAction()const
    {
        auto* focused=juce::Component::getCurrentlyFocusedComponent();
        if(focused && isParentOf(focused))
            if(auto* surface=dynamic_cast<RowSurface*>(focused->getParentComponent()))
                return FocusedAction{surface->displayedId(),focused==&surface->more};
        return {};
    }

    void refreshVisibleActions()
    {
        const auto* viewport=getViewport();
        const auto height=getRowHeight();
        const auto first=viewport->getViewPositionY()/height;
        const auto end=std::min(getNumRows(),(viewport->getViewPositionY()+viewport->getViewHeight())/height+1);
        for(auto row=first;row<end;++row)
            if(auto* surface=surfaceFor(row))surface->display(*rowAt(row),epoch);
    }

    bool ownsRow(const juce::String& id,uint64_t token)const
    {return !updating && isEnabled() && token==epoch && rowFor(id)>=0;}

    void openMenu(MenuRequest request)
    {
        if(ownsRow(request.id,request.epoch) && onMenu)onMenu(std::move(request));
    }

    juce::SharedResourcePointer<FocusVisibility> focusVisibility;
    juce::Component::SafePointer<juce::Component> inspectorTabTarget;
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
        refreshVisibleActions();
        const auto id=primary;const auto token=epoch;const bool reveal=!pointerSelection;
        const juce::Component::SafePointer<SwellList> safe(this);
        if(onSelection)onSelection(std::move(ids),id);
        if(safe && reveal && safe->epoch==token && safe->primary==id && id.isNotEmpty() && safe->onReveal)safe->onReveal(id,token);
    }
    void listBoxItemClicked(int row,const juce::MouseEvent& event)override
    {
        const auto* found=rowAt(row);if(!found || updating)return;
        if(event.mods.isPopupMenu())
        {
            focusVisibility->pointerInput();
            openMenu({found->id,epoch,menuAnchor(event),false});
            return;
        }
        if(primary.isNotEmpty() && onReveal)onReveal(primary,epoch);
    }
    void paintListBoxItem(int row,juce::Graphics& g,int width,int height,bool isSelected)override
    {
        const auto* found=rowAt(row);if(!found)return;const auto& w=*found;
        if(isSelected){g.setColour(aqua.withAlpha(.06f));g.fillRoundedRectangle(juce::Rectangle<float>(1,1,static_cast<float>(width-2),static_cast<float>(height-2)),4);g.setColour(aqua);g.drawRoundedRectangle(juce::Rectangle<float>(1,1,static_cast<float>(width-2),static_cast<float>(height-2)),4,1);}
        g.setColour(m3::ui::text);g.setFont(fontFor(*this,bodySize));g.drawText(w.name,32,2,width-62,20,juce::Justification::left);
        g.setFont(fontFor(*this,smallSize));g.drawText(w.irName.isNotEmpty()?w.irName:"No IR",32,23,width-62,18,juce::Justification::left);
        g.setFont(fontFor(*this,captionSize));g.setColour(w.error?warning:secondary);
        g.drawText(w.status+(w.muted?" | mute":"")+(!w.enabled?" | off":"")+(w.solo?" | solo":"")+(w.frozen?" | frozen":""),32,43,width-62,height-43,juce::Justification::left);
        if(w.id==primary && hasKeyboardFocus(false) && focusVisibility->isVisible())
            paintCornerBrackets(g,{2,2,width-4,height-4},aqua);
    }
};
}
