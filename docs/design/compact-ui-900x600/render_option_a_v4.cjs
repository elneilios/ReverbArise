const { chromium } = require('C:/Users/neila/.cache/codex-runtimes/codex-primary-runtime/dependencies/node/node_modules/playwright');
const fs = require('node:fs/promises');
const path = require('node:path');
const { pathToFileURL } = require('node:url');
(async()=>{
  const root=__dirname, output=path.join(root,'mockups','v4');
  await fs.mkdir(output,{recursive:true});
  const browser=await chromium.launch({channel:'chrome',headless:true});
  const page=await browser.newPage({viewport:{width:1920,height:1080},deviceScaleFactor:1});
  const errors=[];page.on('pageerror',e=>errors.push(e.message));
  const cases=[
    {name:'a4-learn-capture-generate',mode:'learn',future:'0',learnSection:'Capture & Generate'},
    {name:'a4-learn-sound-current',mode:'learn',future:'0',learnSection:'Sound'},
    {name:'a4-learn-sound-convolution-future',mode:'learn',future:'1',learnSection:'Sound',engine:'convolution'},
    {name:'a4-learn-sound-algorithmic-future',mode:'learn',future:'1',learnSection:'Sound',engine:'algorithmic'},
    {name:'a4-perform-timing',mode:'perform',future:'0',section:'Timing'},
    {name:'a4-perform-envelope',mode:'perform',future:'0',section:'Envelope'},
    {name:'a4-perform-sound-current',mode:'perform',future:'0',section:'Sound'},
    {name:'a4-perform-sound-convolution-future',mode:'perform',future:'1',section:'Sound',engine:'convolution'},
    {name:'a4-perform-sound-algorithmic-future',mode:'perform',future:'1',section:'Sound',engine:'algorithmic'},
    {name:'a4-export-levels',mode:'export',future:'1'}
  ];
  const report=[];
  for(const test of cases){
    const args=new URLSearchParams({capture:'1',variant:'a',typography:'responsive',...test});
    await page.goto(pathToFileURL(path.join(root,'index.html')).href+'?'+args);
    await page.evaluate(()=>document.fonts.ready);
    await page.locator('img').evaluateAll(imgs=>Promise.all(imgs.map(i=>i.decode().catch(()=>{}))));
    const box=await page.locator('.plugin').boundingBox();
    if(box.width!==900||box.height!==600)throw Error(test.name+' wrong dimensions');
    const layout=await page.evaluate(()=>{
      const outside=(e,b)=>{const r=e.getBoundingClientRect();return r.left<b.left-.1||r.top<b.top-.1||r.right>b.right+.1||r.bottom>b.bottom+.1};
      const plugin=document.querySelector('.plugin').getBoundingClientRect();
      const regionOverflow=Array.from(document.querySelectorAll('.plugin .region')).filter(e=>outside(e,plugin)).map(e=>e.className);
      const container=document.querySelector('.section-content')||document.querySelector('.learn-current-body');
      const contentOverflow=container?Array.from(container.querySelectorAll('.cell,.caption,.entry,.select,.character-select,.filter-bypass')).filter(e=>outside(e,container.getBoundingClientRect())).map(e=>e.textContent):[];
      const captionOverflow=Array.from(document.querySelectorAll('.sound-module .caption,.sound-module .engine-caption')).filter(e=>e.scrollWidth>e.clientWidth).map(e=>e.textContent);
      const brokenImages=Array.from(document.images).filter(i=>!i.complete||!i.naturalWidth).map(i=>i.src);
      const reset=document.querySelector('.reset-footer')?.getBoundingClientRect();
      const sound=document.querySelector('.sound-module')?.getBoundingClientRect();
      const actions=document.querySelector('.learn-actions')?.getBoundingClientRect();
      const note=document.querySelector('.learn-sound-note')?.getBoundingClientRect();
      const controls=Array.from(document.querySelectorAll('[data-control],[data-capacity-control],[data-export-control]')).map(e=>e.dataset.control||e.dataset.capacityControl||e.dataset.exportControl);
      const dividerClearances=Array.from(document.querySelectorAll('.list,.inspector,.learn-left,.export-right')).map(e=>{
        const pseudo=getComputedStyle(e,e.matches('.inspector,.export-right')?'::before':'::after');
        return {region:e.className,top:parseFloat(pseudo.top),bottom:parseFloat(pseudo.bottom)};
      });
      if(!dividerClearances.length||dividerClearances.some(d=>!Number.isFinite(d.top)||d.top<8||!Number.isFinite(d.bottom)||d.bottom<8))throw Error('Structural divider clearance failed: '+JSON.stringify(dividerClearances));
      return {regionOverflow,contentOverflow,captionOverflow,brokenImages,controls,dividerClearances,soundBounds:sound?{width:sound.width,height:sound.height,bottom:sound.bottom}:null,overlapsPinned:!!sound&&!!reset&&sound.bottom>reset.top,learnNoteOverlapsActions:!!note&&!!actions&&note.bottom>actions.top};
    });
    if(layout.regionOverflow.length||layout.contentOverflow.length||layout.captionOverflow.length||layout.brokenImages.length||layout.overlapsPinned||layout.learnNoteOverlapsActions||errors.length)throw Error(JSON.stringify({test,layout,errors}));
    await page.locator('.plugin').screenshot({path:path.join(output,test.name+'.png')});
    report.push({name:test.name,width:900,height:600,...layout});
  }
  // The future module must have exactly the same geometry in both contexts.
  const a=report.find(r=>r.name==='a4-learn-sound-convolution-future');
  const b=report.find(r=>r.name==='a4-perform-sound-convolution-future');
  if(a.soundBounds.width!==b.soundBounds.width||a.soundBounds.height!==b.soundBounds.height)throw Error('Shared module geometry diverged: '+JSON.stringify({learn:a.soundBounds,perform:b.soundBounds}));
  await page.goto(pathToFileURL(path.join(root,'index.html')).href+'?typography=responsive&variant=a&mode=learn&future=1&learnSection=Sound');
  await page.locator('[data-engine]').selectOption('algorithmic');
  if(await page.locator('[data-capacity-control="Decay (s)"]').count()!==1)throw Error('Engine disclosure did not update');
  await page.locator('[data-a-learn="Capture & Generate"]').click();
  if(!await page.getByRole('button',{name:'Generate Swells',exact:true}).isVisible())throw Error('Generate disappeared');
  await page.locator('[data-mode="perform"]').click();
  await page.locator('[data-section="Sound"]').click();
  if(!await page.getByRole('button',{name:'Reset',exact:true}).isVisible())throw Error('Reset disappeared');
  await page.goto(pathToFileURL(path.join(root,'index.html')).href+'?typography=responsive&variant=a&mode=perform&future=1&section=Sound&engine=convolution');
  await page.evaluate(()=>document.fonts.ready);
  await page.screenshot({path:path.join(output,'a4-review-1920x1080.png')});
  await page.locator('#size-toggle').click();
  await page.locator('.plugin').screenshot({path:path.join(output,'a4-perform-expanded-1100x850.png')});
  const responsiveChecks=[];
  for(const size of ['900x600','1000x750','1100x850','1400x1000']){
    await page.goto(pathToFileURL(path.join(root,'index.html')).href+'?capture=1&typography=responsive&variant=a&mode=perform&future=1&section=Sound&engine=algorithmic&size='+size);
    await page.evaluate(()=>document.fonts.ready);
    const measured=await page.evaluate(()=>{
      const q=s=>document.querySelector(s),b=s=>q(s).getBoundingClientRect();
      const style=s=>getComputedStyle(q(s));
      const notice=q('.notice'),body=b('.section-content'),sound=b('.sound-module');
      const text=document.createRange();text.selectNodeContents(notice);const lines=Array.from(text.getClientRects());
      const nb=notice.getBoundingClientRect();
      const fullNoticeVisible=notice.scrollWidth<=notice.clientWidth&&notice.scrollHeight<=notice.clientHeight&&lines.every(r=>r.left>=nb.left-.1&&r.right<=nb.right+.1&&r.top>=nb.top-.1&&r.bottom<=nb.bottom+.1);
      return {size:[b('.plugin').width,b('.plugin').height],fonts:{heading:parseFloat(style('.inspector .heading').fontSize),body:parseFloat(style('.help').fontSize),value:parseFloat(style('.sound-module .entry').fontSize)},fullNoticeVisible,noticeText:notice.textContent,logoLeft:b('.wordmark').left,contentLeft:b('.list').left,clearances:{headerToWork:b('.column').top-b('.header').bottom,inspectorLeft:sound.left-b('.inspector').left,inspectorRight:b('.inspector').right-sound.right,tabsToContent:sound.top-b('.inspector .section-tabs').bottom,contentToReset:b('.reset-footer').top-sound.bottom,resetToDivider:b('.audition').top-b('.reset-footer').bottom},moduleContained:sound.left>=body.left&&sound.right<=body.right+.1&&sound.bottom<=body.bottom+.1};
    });
    if(!measured.fullNoticeVisible||measured.logoLeft!==measured.contentLeft||!measured.moduleContained||Object.values(measured.clearances).some(v=>v<7.9))throw Error(JSON.stringify(measured));
    responsiveChecks.push(measured);
    await page.locator('.plugin').screenshot({path:path.join(output,'a4-perform-responsive-'+size+'.png')});
  }
  for(let i=1;i<responsiveChecks.length;i++)for(const role of ['heading','body','value'])if(responsiveChecks[i].fonts[role]<=responsiveChecks[i-1].fonts[role])throw Error('Responsive font did not increase: '+role);
  // Exercise arbitrary resize through the same observer, then preserve size across a tab change.
  await page.locator('.plugin').evaluate(e=>{e.style.width='1050px';e.style.height='800px'});
  await page.waitForFunction(()=>document.querySelector('#dimensions').textContent.startsWith('1050 × 800'));
  await page.locator('[data-section="Envelope"]').click();
  const retained=await page.locator('.plugin').boundingBox();
  if(retained.width!==1050||retained.height!==800)throw Error('Resize was lost on tab change');
  await fs.writeFile(path.join(output,'render-checks-v4.json'),JSON.stringify({renderer:'Installed Chrome; deviceScaleFactor 1; browser mockup checks only',pageErrors:errors,cases:report,sharedModuleGeometryMatches:true,tabAndEngineDisclosureChecks:'passed',responsiveChecks,arbitraryResizeAndTabRetention:'passed'},null,2));
  await browser.close();
  console.log(JSON.stringify({rendered:report.length,sharedModule:a.soundBounds,pageErrors:errors}));
})().catch(e=>{console.error(e);process.exit(1)});
