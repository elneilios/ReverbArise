#pragma once
#include <UI/ConvolutionIrBinding.h>
#include <UI/Controls/PresentationSurface.h>
#include <UI/Controls/SegmentedSelector.h>

namespace convolutionIrCases
{
using namespace m3;
using namespace m3::ui;
using namespace m3::ui::controls;
template<class F>void settle(F condition,const std::source_location line=std::source_location::current())
{
    const auto deadline=juce::Time::getMillisecondCounter()+30000;
    while(!condition())
    {if(juce::Time::getMillisecondCounter()>deadline)throw std::runtime_error("IR binding timeout at "+std::to_string(line.line()));juce::Thread::sleep(2);}
}
struct Fixture
{
    TestDirectory root;CaptureSession capture{root.file};workflow::State flow;
    PreviewContext context;std::unique_ptr<SwellEngine> engine;juce::String a,b;juce::File first,second;
    Fixture()
    {
        std::vector<float> dry(10000,.1f);REQUIRE(capture.commit(0,1000,dry.data(),5000));
        context={capture.snapshot(),{true,0,0,1000,120,4,4},0};
        engine=std::make_unique<SwellEngine>(root.file,[this]{return context;});engine->setAutomaticProcessing(false);
        settle([&]{return engine->savedSession().defaultIR.present();});
        first=root.file.getChildFile("A short personal response.wav");second=root.file.getChildFile("B different personal response.wav");
        writeTestWav(first,1,1000,std::vector<float>(101,.003f));writeTestWav(second,1,1000,std::vector<float>(151,.004f));
        REQUIRE(engine->assignDefaultIR(first));settle([&]{return !engine->snapshot().defaultIRPending;});
        REQUIRE(flow.requestMode(workflow::Mode::live,{true,true,true,false}));engine->useWorkflow(flow,[]{return uint64_t{1};});
        REQUIRE(engine->addTarget(1500));a=engine->savedSession().primary;
        REQUIRE(engine->addTarget(3000));b=engine->savedSession().primary;
        ready();
    }
    void ready(){settle([&]{const auto state=engine->savedSession();return find(state,a)->phase==Phase::ready && find(state,b)->phase==Phase::ready;});}
    void select(const juce::String& id){REQUIRE(engine->selectTargets({id},id));}
};
inline void choose(ConvolutionIrEditor& editor,int id)
{editor.selector.beforeInvocation();editor.selector.setSelectedId(id,juce::sendNotificationSync);}
inline void buttonSurfaceCases()
{
    ActionButton reference("");reference.setSize(responsiveGeometry::controlHeight,responsiveGeometry::controlHeight);
    for(auto role:{ActionIcon::openFile,ActionIcon::settings})for(bool enabled:{true,false})
    for(const auto state:std::array<std::pair<bool,bool>,3>{{{false,false},{true,false},{true,true}}})
    {
        IconActionButton icon(role);icon.setSize(reference.getWidth(),reference.getHeight());icon.setEnabled(enabled);reference.setEnabled(enabled);
        const auto render=[&](auto& button)
        {juce::Image image(juce::Image::RGB,button.getWidth(),button.getHeight(),true);juce::Graphics g(image);g.fillAll(background);button.paintButton(g,state.first,state.second);return image;};
        const auto actual=render(icon),expected=render(reference);
        const auto contentBounds=commandBodyBounds(icon.getLocalBounds()).reduced(static_cast<int>(controlGeometry::focusOutset));
        for(int y=0;y<actual.getHeight();++y)for(int x=0;x<actual.getWidth();++x)
        {
            // State colours are sampled on straight edges, away from curved
            // corner antialiasing and the intentionally different glyphs.
            if((x==actual.getWidth()/2 && (y==0 || y==actual.getHeight()-1))
                || (y==actual.getHeight()/2 && (x==0 || x==actual.getWidth()-1)))
            {
                if(actual.getPixelAt(x,y)!=expected.getPixelAt(x,y))
                    throw std::runtime_error("Button surface mismatch role="+std::to_string(static_cast<int>(role))+" enabled="+std::to_string(enabled)+" hover="+std::to_string(state.first)+" down="+std::to_string(state.second)+" at "+std::to_string(x)+","+std::to_string(y)+" actual="+actual.getPixelAt(x,y).toDisplayString(true).toStdString()+" expected="+expected.getPixelAt(x,y).toDisplayString(true).toStdString());
            }
            else if(contentBounds.contains(x,y))
            {
                const auto pixel=actual.getPixelAt(x,y);
                const auto accent=pixel.getGreen()>pixel.getRed()+40 && pixel.getBlue()>pixel.getRed()+40;
                // The owner retained neutral glyph ink; body feedback is separate.
                REQUIRE(!accent);
            }
        }
    }
}
inline void componentCases()
{
    WorkspaceTheme theme;theme.setResponsiveViewport(900,600);
    ConvolutionIrEditor editor;editor.setLookAndFeel(&theme);editor.setSize(259,editor.preferredHeight());
    ConvolutionIrEditor::View view;
    view.choices={{"a","One response","Real description"},{"b",juce::String::repeatedString("Long imported name ",8),"Other description"}};
    view.accepted="a";view.editable=true;view.status="Accepted: One response";
    int changes=0,imports=0;juce::String requested;
    editor.onChoice=[&](const auto& key){++changes;requested=key;};editor.onImport=[&]{++imports;};
    editor.display(view);editor.display(view);REQUIRE(changes==0 && editor.selector.getSelectedId()==1);
    REQUIRE(editor.getLookAndFeel().getLabelFont(editor.status).getHeight()==theme.resolvedHeight(textStyle(TextRole::helper).height));
    choose(editor,2);REQUIRE(changes==1 && requested=="b" && editor.selector.getSelectedId()==1);
    // The control keeps the original offered identity even if a refresh reorders entries.
    editor.selector.beforeInvocation();std::reverse(view.choices.begin(),view.choices.end());editor.display(view);
    editor.selector.setSelectedId(1,juce::sendNotificationSync);REQUIRE(changes==2 && requested=="a");
    REQUIRE(editor.selector.getSelectedId()==2);
    editor.selector.keyPressed(juce::KeyPress(juce::KeyPress::upKey));
    juce::MessageManager::getInstance()->runDispatchLoopUntil(30);REQUIRE(changes==3 && requested=="b");
    const auto count=changes;
    const auto point=editor.selector.getLocalBounds().getCentre().toFloat();const auto time=juce::Time::getCurrentTime();
    juce::MouseEvent event(juce::Desktop::getInstance().getMainMouseSource(),point,{},0,0,0,0,0,&editor.selector,&editor.selector,time,point,time,0,false);
    juce::MouseWheelDetails wheel{};wheel.deltaY=1;editor.selector.mouseWheelMove(event,wheel);
    juce::MessageManager::getInstance()->runDispatchLoopUntil(30);REQUIRE(changes==count);
    editor.fileButton.keyPressed(juce::KeyPress(juce::KeyPress::returnKey));
    juce::MessageManager::getInstance()->runDispatchLoopUntil(30);REQUIRE(imports==1);
    controlReview::pointerClick(editor.fileButton);REQUIRE(imports==2);
    view.editable=false;view.reason="Unfreeze this swell before changing its IR.";view.accepted={};view.status="Unavailable";
    editor.display(view);REQUIRE(!editor.selector.isEnabled() && !editor.fileButton.isEnabled() && editor.selector.getSelectedId()==0);
    REQUIRE(editor.selector.getTooltip().contains("Unfreeze") && editor.fileButton.getDescription().contains("Unfreeze"));
    choose(editor,1);editor.fileButton.keyPressed(juce::KeyPress(juce::KeyPress::spaceKey));
    juce::MessageManager::getInstance()->runDispatchLoopUntil(30);REQUIRE(changes==count && imports==2);
    view.editable=true;view.accepted="b";view.status="Requested: New response | Accepted: Old response";
    editor.display(view);editor.setSize(editor.getWidth(),editor.preferredHeight());
    REQUIRE(editor.status.getDescription()==view.status);
    REQUIRE(editor.status.getY()==editor.selector.getBottom()+spacing::tight);
    view.status={};editor.display(view);
    REQUIRE(!editor.status.isVisible() && editor.getNumChildComponents()==4);
    view.status="Couldn't load IR. Choose another; previous default kept.";view.error=true;view.editable=false;view.reason="The WAV could not be decoded";
    editor.display(view);
    REQUIRE(editor.status.isVisible() && editor.status.isEnabled() && !editor.selector.isEnabled());
    REQUIRE(editor.status.findColour(juce::Label::textColourId)==danger && editor.status.getDescription().contains(view.reason));
    REQUIRE(editor.status.getHeight()<=2*controlGeometry::feedbackHeight);
    const auto narrowHeight=editor.preferredHeight(160);editor.setSize(160,narrowHeight);
    REQUIRE(editor.status.getHeight()==2*controlGeometry::feedbackHeight && editor.preferredHeight()==narrowHeight);
    view.status={};editor.display(view);editor.setSize(160,editor.preferredHeight(160));
    REQUIRE(editor.getHeight()==editor.fileButton.getBottom() && !editor.status.isVisible());
    for(int width:{259,276,304})
    {
        editor.setSize(width,editor.preferredHeight(width));
        for(auto* child:editor.getChildren())REQUIRE(editor.getLocalBounds().contains(child->getBounds()));
        REQUIRE(!editor.selector.getBounds().intersects(editor.fileButton.getBounds()));
        const auto body=commandBodyBounds(editor.fileButton.getLocalBounds()).translated(editor.fileButton.getX(),editor.fileButton.getY());
        REQUIRE(body.getY()==editor.selector.getY() && body.getBottom()==editor.selector.getBottom());
        REQUIRE(body.getHeight()==responsiveGeometry::controlHeight && body.getWidth()==body.getHeight());
    }
    editor.setLookAndFeel(nullptr);
}
inline void bindingCases()
{
    Fixture fixture;auto& engine=*fixture.engine;ConvolutionIrEditor defaults,selected;
    ConvolutionIrBinding defaultBinding(defaults,engine),selectedBinding(selected,engine);
    const auto refresh=[&]{const auto state=engine.snapshot();defaultBinding.displayDefault(state);selectedBinding.displaySelected(state);};
    defaultBinding.onRefresh=refresh;selectedBinding.onRefresh=refresh;refresh();
    auto unavailable=engine.snapshot();unavailable.session.defaultIR={};defaultBinding.displayDefault(unavailable);
    REQUIRE(defaults.selector.getSelectedId()==0 && defaults.status.getText()=="Default IR unavailable");refresh();
    const auto saved=engine.savedSession();const auto jobs=engine.snapshot().renderCount;
    for(int i=0;i<4;++i)refresh();REQUIRE(engine.snapshot().renderCount==jobs && sameSavedSwell(saved.swells.front(),engine.savedSession().swells.front()));
    auto completion=defaultBinding.beginFileImport();completion(fixture.second);
    settle([&]{return !engine.snapshot().defaultIRPending;});
    const auto accepted=engine.savedSession();REQUIRE(accepted.defaultIR!=saved.defaultIR);
    REQUIRE(sameSavedSwell(*find(saved,fixture.a),*find(accepted,fixture.a)) && sameSavedSwell(*find(saved,fixture.b),*find(accepted,fixture.b)));
    refresh();fixture.select(fixture.a);refresh();
    const auto captured=selectedBinding.beginFileImport();fixture.select(fixture.b);refresh();captured(fixture.second);
    try{settle([&]{return find(engine.savedSession(),fixture.a)->ir.content==accepted.defaultIR.content;});}
    catch(...){const auto state=engine.snapshot();std::cerr<<"Pinned import: "<<state.message<<" | A: "<<find(state.session,fixture.a)->status<<"\n";throw;}
    fixture.ready();
    REQUIRE(find(engine.savedSession(),fixture.b)->ir==saved.defaultIR && engine.savedSession().defaultIR==accepted.defaultIR);
    // Catalogue and retained-managed choices use the same pinned command route.
    fixture.select(fixture.a);refresh();const auto bundled=engine.irChoices().front().content;
    choose(selected,1);settle([&]{return find(engine.savedSession(),fixture.a)->ir.content==bundled;});fixture.ready();
    REQUIRE(engine.savedSession().defaultIR==accepted.defaultIR && find(engine.savedSession(),fixture.b)->ir==saved.defaultIR);
    refresh();int managedChoice=0;const auto entries=engine.irChoices();
    for(size_t i=0;i<entries.size();++i)if(entries[i].content==accepted.defaultIR.content)managedChoice=static_cast<int>(i+1);
    REQUIRE(managedChoice>0);choose(selected,managedChoice);
    settle([&]{return find(engine.savedSession(),fixture.a)->ir==accepted.defaultIR;});fixture.ready();
    // Cancel and failed decode cannot replace the accepted default or admitted history.
    refresh();const auto beforeCancel=engine.savedSession();defaultBinding.beginFileImport()({});
    REQUIRE(engine.savedSession().defaultIR==beforeCancel.defaultIR && !engine.snapshot().defaultIRPending);
    defaultBinding.beginFileImport()(fixture.root.file.getChildFile("missing.wav"));
    settle([&]{return !engine.snapshot().defaultIRPending;});REQUIRE(engine.savedSession().defaultIR==beforeCancel.defaultIR);
    REQUIRE(engine.savedSession().importedIRs.size()==beforeCancel.importedIRs.size());
    refresh();const auto older=defaultBinding.beginFileImport();const auto newer=defaultBinding.beginFileImport();
    newer(fixture.first);settle([&]{return !engine.snapshot().defaultIRPending;});older(fixture.second);
    REQUIRE(engine.savedSession().defaultIR.content==saved.defaultIR.content && !engine.snapshot().defaultIRPending);
    // A newer menu choice also invalidates an older file-dialog completion.
    refresh();const auto obsoleteFile=defaultBinding.beginFileImport();choose(defaults,1);
    settle([&]{return !engine.snapshot().defaultIRPending;});const auto afterMenu=engine.savedSession().defaultIR;
    obsoleteFile(fixture.second);REQUIRE(engine.savedSession().defaultIR==afterMenu && !engine.snapshot().defaultIRPending);
    // Capture pass locking is enforced by the engine, even for captured continuations.
    REQUIRE(fixture.flow.requestMode(workflow::Mode::capture,{true,true,true,false}));refresh();
    const auto passFile=defaultBinding.beginFileImport();const auto pass=fixture.flow.begin(true);REQUIRE(pass!=0);
    passFile(fixture.second);REQUIRE(!engine.snapshot().defaultIRPending && engine.savedSession().defaultIR==afterMenu);
    refresh();REQUIRE(!defaults.selector.isEnabled());
    REQUIRE(fixture.flow.stop(pass) && fixture.flow.validating(pass) && fixture.flow.complete(pass,true,true,false));
    REQUIRE(fixture.flow.requestMode(workflow::Mode::live,{true,true,true,false}));
    fixture.select(fixture.a);refresh();const auto staleInput=selectedBinding.beginFileImport();
    const auto input=find(engine.savedSession(),fixture.a)->inputGeneration;
    REQUIRE(engine.assignIR(fixture.a,input,fixture.first,false,engine.assignmentEpoch()));
    settle([&]{return find(engine.savedSession(),fixture.a)->ir.content==saved.defaultIR.content;});fixture.ready();
    const auto currentInput=find(engine.savedSession(),fixture.a)->inputGeneration;
    staleInput(fixture.second);REQUIRE(find(engine.savedSession(),fixture.a)->inputGeneration==currentInput && currentInput!=input);
    REQUIRE(find(engine.savedSession(),fixture.a)->ir.content==saved.defaultIR.content);
    refresh();const auto frozenFile=selectedBinding.beginFileImport();REQUIRE(engine.setFrozen(fixture.a,true));
    frozenFile(fixture.second);REQUIRE(find(engine.savedSession(),fixture.a)->ir.content==saved.defaultIR.content && find(engine.savedSession(),fixture.a)->inputGeneration==currentInput);
    refresh();REQUIRE(!selected.selector.isEnabled() && selected.fileButton.getTooltip().contains("Unfreeze"));
    REQUIRE(engine.setFrozen(fixture.a,false));refresh();const auto modeFile=selectedBinding.beginFileImport();
    REQUIRE(fixture.flow.requestMode(workflow::Mode::finalise,{true,true,true,false}));modeFile(fixture.second);
    REQUIRE(find(engine.savedSession(),fixture.a)->ir.content==saved.defaultIR.content);
    REQUIRE(fixture.flow.requestMode(workflow::Mode::live,{true,true,true,false}));refresh();
    const auto deletedFile=selectedBinding.beginFileImport();REQUIRE(engine.removeTarget(fixture.a,engine.assignmentEpoch()));deletedFile(fixture.second);
    REQUIRE(!find(engine.savedSession(),fixture.a) && find(engine.savedSession(),fixture.b)->ir==saved.defaultIR);
    auto empty=engine.snapshot();empty.session.primary={};selectedBinding.displaySelected(empty);
    REQUIRE(!selected.selector.isEnabled() && selected.selector.getSelectedId()==0 && selected.status.getText()=="No swell selected");
    fixture.select(fixture.b);refresh();const auto oldSession=selectedBinding.beginFileImport();const auto oldDefault=defaultBinding.beginFileImport();
    const auto restore=engine.savedSession();engine.restore(restore);oldSession(fixture.second);oldDefault(fixture.second);
    REQUIRE(find(engine.savedSession(),fixture.b)->ir==find(restore,fixture.b)->ir && engine.savedSession().defaultIR==restore.defaultIR);
    // A dialog completion after its binding dies is harmless even while the editor survives.
    std::function<void(const juce::File&)> destroyed;
    {ConvolutionIrEditor control;auto binding=std::make_unique<ConvolutionIrBinding>(control,engine);binding->displayDefault(engine.snapshot());destroyed=binding->beginFileImport();binding.reset();destroyed(fixture.second);}
    REQUIRE(engine.savedSession().defaultIR==restore.defaultIR);
    // Reopen retained choices through the binding with both external originals
    // gone. The saved assignment/cache identities survive; instances stay isolated.
    REQUIRE(fixture.first.deleteFile() && fixture.second.deleteFile());
    juce::String encoded,error;Session reopened;REQUIRE(encode(engine.savedSession(),encoded,error));REQUIRE(decode(encoded,reopened,error));
    REQUIRE(reopened.importedIRs.size()==2 && find(reopened,fixture.b)->ir==find(restore,fixture.b)->ir);
    SwellEngine copy(fixture.root.file,[&]{return fixture.context;});copy.restore(reopened);
    ConvolutionIrEditor restoredControl;ConvolutionIrBinding restoredBinding(restoredControl,copy);
    restoredBinding.displayDefault(copy.snapshot());const auto retained=copy.irChoices();int retainedId=0;
    for(size_t i=0;i<retained.size();++i)if(retained[i].content==accepted.defaultIR.content)retainedId=static_cast<int>(i+1);
    REQUIRE(retainedId>0);choose(restoredControl,retainedId);settle([&]{return !copy.snapshot().defaultIRPending;});
    REQUIRE(copy.savedSession().defaultIR==accepted.defaultIR && copy.savedSession().importedIRs.size()==2);
    REQUIRE(engine.savedSession().defaultIR==restore.defaultIR && find(copy.savedSession(),fixture.b)->render==find(restore,fixture.b)->render);
}

class Panel final:public juce::Component,private juce::Timer
{
public:
    explicit Panel(int initialState=1):learnBinding(learn,*fixture.engine),performBinding(perform,*fixture.engine)
    {
        state=initialState;
        learn.setName("reviewLearnIr");perform.setName("reviewPerformIr");
        setName("sharedIrFamilyReview");addAndMakeVisible(surface);
        for(auto* child:std::array<juce::Component*,11>{&learn,&perform,&title,&instructions,&learnTitle,&performTitle,&learnScope,&performScope,&states,&sizes,&feedback})surface.contentRoot().addAndMakeVisible(child);
        title.setText("Shared IR editor - family review",juce::dontSendNotification);applyTextRole(title,TextRole::sectionHeading);
        instructions.setText("Choose/import an IR in either context. Tab through controls; hover for full help. Test states and sizes below.",juce::dontSendNotification);
        learnTitle.setText("Learn / Sound",juce::dontSendNotification);performTitle.setText("Perform / Sound",juce::dontSendNotification);
        learnScope.setName("learnSoundScope");performScope.setName("performSoundScope");
        learnScope.setText("Future swells only",juce::dontSendNotification);performScope.setText("Selected swell only",juce::dontSendNotification);
        for(auto* label:{&learnScope,&performScope}){applyTextRole(*label,TextRole::helper);label->setTooltip(label->getText());label->setDescription(label->getText());}
        for(auto* label:{&learnTitle,&performTitle})applyTextRole(*label,TextRole::controlLabel);
        for(auto* label:std::array<juce::Label*,2>{&instructions,&feedback})applyTextRole(*label,TextRole::helper);
        states.display({{1,"Ready","Edit both independent assignments"},{2,"Frozen","Selected swell is frozen"},{3,"Empty","No selected swell"},{4,"Busy","Pending requests and long names"},{5,"Failed","Previous response retained"},{6,"Locked","Workflow unavailable"}},1);
        states.onRequest=[this](int id){state=id;refresh();};
        sizes.display({{1,"900 / 100%","Compact baseline"},{2,"1100 / 125%","Fitting magnification"},{3,"1400 / 100%","Larger workspace"}},1);
        sizes.onRequest=[this](int id){profile=id;resized();};
        setSize(900,600);startTimerHz(8);refresh();
    }
    void resized()override
    {
        const int width=profile==1?900:profile==2?1100:1400,height=profile==3?900:600,percent=profile==2?125:100;
        const auto scale=percent/100.f;setSize(juce::roundToInt(width*scale),juce::roundToInt(height*scale));
        surface.setBounds(getLocalBounds());surface.present(width,height,percent);
        if(auto* theme=dynamic_cast<WorkspaceTheme*>(&getLookAndFeel()))theme->setResponsiveViewport(width,height);
        auto area=juce::Rectangle<int>(width,height).reduced(responsiveGeometry::outerMargin);
        title.setBounds(area.removeFromTop(controlGeometry::headingHeight));area.removeFromTop(spacing::related);
        instructions.setBounds(area.removeFromTop(controlGeometry::labelHeight));area.removeFromTop(spacing::section);
        auto row=area.removeFromTop(responsiveGeometry::soundModuleHeight+controlGeometry::labelHeight);
        auto left=row.removeFromLeft(259);row.removeFromLeft(spacing::section);auto right=row.removeFromLeft(259);
        learnTitle.setBounds(left.removeFromTop(controlGeometry::labelHeight));performTitle.setBounds(right.removeFromTop(controlGeometry::labelHeight));
        learnScope.setBounds(left.removeFromTop(controlGeometry::feedbackHeight));performScope.setBounds(right.removeFromTop(controlGeometry::feedbackHeight));
        left.removeFromTop(responsiveGeometry::contentPadding);right.removeFromTop(responsiveGeometry::contentPadding);
        learn.setBounds(left.removeFromTop(learn.preferredHeight(left.getWidth())));perform.setBounds(right.removeFromTop(perform.preferredHeight(right.getWidth())));
        area.removeFromTop(spacing::section);states.setBounds(area.removeFromTop(responsiveGeometry::controlHeight));area.removeFromTop(spacing::related);
        sizes.setBounds(area.removeFromTop(responsiveGeometry::controlHeight));area.removeFromTop(spacing::related);
        feedback.setBounds(area.removeFromTop(controlGeometry::labelHeight));
    }
    void paint(juce::Graphics& graphics)override{graphics.fillAll(background);}
    void lookAndFeelChanged()override{resized();}
private:
    void timerCallback()override{refresh();}
    void refresh()
    {
        auto shown=fixture.engine->snapshot();
        if(state==2)find(shown.session,shown.session.primary)->frozen=true;
        if(state==3)shown.session.primary={};
        if(state==4)
        {
            shown.defaultIRPending=true;shown.requestedDefaultIRName="A very long newly imported convolution response.wav";
            shown.retainedIRName="Accepted response";shown.requestedIRName="Another very long response still preparing.wav";
        }
        if(state==5){shown.defaultIRError="Could not decode imported response; choose another.";auto* swell=find(shown.session,shown.session.primary);swell->phase=Phase::error;swell->status="Import failed; previous IR retained";}
        if(state==6)shown.workflowState.phase=workflow::Phase::recording;
        learnBinding.displayDefault(shown);performBinding.displaySelected(shown);
        resized();
        // Non-ready review fixtures describe states only; do not let their
        // synthetic snapshot submit commands against the live backing model.
        if(state==4 || state==5){learn.selector.setEnabled(false);learn.fileButton.setEnabled(false);perform.selector.setEnabled(false);perform.fileButton.setEnabled(false);}
        states.display({{1,"Ready","Edit both independent assignments"},{2,"Frozen","Selected swell is frozen"},{3,"Empty","No selected swell"},{4,"Busy","Pending requests and long names"},{5,"Failed","Previous response retained"},{6,"Locked","Workflow unavailable"}},state);
        sizes.display({{1,"900 / 100%","Compact baseline"},{2,"1100 / 125%","Fitting magnification"},{3,"1400 / 100%","Larger workspace"}},profile);
        juce::String help;auto* hovered=juce::Desktop::getInstance().getMainMouseSource().getComponentUnderMouse();
        for(auto* item=hovered;item && item!=this;item=item->getParentComponent())if(auto* tooltip=dynamic_cast<juce::TooltipClient*>(item)){help=tooltip->getTooltip();if(help.isNotEmpty())break;}
        if(help.isEmpty() && (state==4 || state==5))help="Read-only status specimen. Return to Ready to edit.";
        feedback.setText(help.replace("\n"," | "),juce::dontSendNotification);feedback.setTooltip(help);
    }
    Fixture fixture;PresentationSurface surface;ConvolutionIrEditor learn,perform;
    ConvolutionIrBinding learnBinding,performBinding;
    juce::Label title,instructions,learnTitle,performTitle,learnScope,performScope;EllipsisLabel feedback;SegmentedSelector states,sizes;
    int state=1,profile=1;
};
inline juce::Component* findReviewComponent(juce::Component& root,const juce::String& name)
{
    if(root.getName()==name)return &root;
    for(auto* child:root.getChildren())if(auto* match=findReviewComponent(*child,name))return match;
    return nullptr;
}
inline void hostOwnershipCases()
{
    Panel host;
    for(const auto* name:{"reviewLearnIr","reviewPerformIr"})
    {
        auto* field=dynamic_cast<ConvolutionIrEditor*>(findReviewComponent(host,name));REQUIRE(field);
        REQUIRE(!field->selector.getTooltip().contains("swells only") && !field->fileButton.getTooltip().contains("swell only"));
    }
    for(const auto* name:{"learnSoundScope","performSoundScope"})
    {
        auto* explanation=findReviewComponent(host,name);REQUIRE(explanation && explanation->isVisible());
        for(auto* parent=explanation->getParentComponent();parent;parent=parent->getParentComponent())
            REQUIRE(dynamic_cast<ConvolutionIrEditor*>(parent)==nullptr);
    }
}
inline void cases(){buttonSurfaceCases();componentCases();bindingCases();hostOwnershipCases();}
}
