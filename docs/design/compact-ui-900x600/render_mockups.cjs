const { chromium } = require('C:/Users/neila/.cache/codex-runtimes/codex-primary-runtime/dependencies/node/node_modules/playwright');
const fs = require('node:fs/promises');
const path = require('node:path');
const { pathToFileURL } = require('node:url');
(async()=>{
  const root = __dirname;
  await fs.mkdir(path.join(root,'mockups'),{recursive:true});
  const browser = await chromium.launch({channel:'chrome',headless:true});
  const page = await browser.newPage({viewport:{width:1920,height:1080},deviceScaleFactor:1});
  const cases = [
    ['a','learn','Envelope'], ['a','perform','Timing'], ['a','perform','Envelope'], ['a','perform','Sound'], ['a','export','Envelope'],
    ['b','learn','Envelope'], ['b','perform','Envelope'], ['b','export','Envelope']
  ];
  const report=[];
  for(const [variant,mode,section] of cases){
    await page.goto(pathToFileURL(path.join(root,'index-v1.html')).href+`?capture=1&variant=${variant}&mode=${mode}&section=${section}`);
    await page.evaluate(()=>document.fonts.ready);
    await page.locator('img').evaluateAll(imgs=>Promise.all(imgs.map(i=>i.decode().catch(()=>{}))));
    const plugin=page.locator('.plugin');
    const name=`${variant}-${mode}${mode==='perform'&&variant==='a'?'-'+section.toLowerCase():''}`;
    const box=await plugin.boundingBox();
    if(box.width!==900||box.height!==600)throw Error(name+' has wrong canvas');
    const overflow=await page.evaluate(()=>Array.from(document.querySelectorAll('.plugin .region')).filter(e=>{const b=e.getBoundingClientRect();return b.left<0||b.top<0||b.right>900.1||b.bottom>600.1}).map(e=>e.className));
    const broken=await page.locator('img').evaluateAll(imgs=>imgs.filter(i=>!i.complete||!i.naturalWidth).map(i=>i.src));
    if(overflow.length||broken.length)throw Error(JSON.stringify({name,overflow,broken}));
    let controlClipping=[];
    if(variant==='a'&&mode==='perform'){
      controlClipping=await page.locator('.section-content').evaluate(p=>{
        const b=p.getBoundingClientRect();
        return Array.from(p.querySelectorAll('.cell,.caption,.entry,.select')).filter(e=>{
          const r=e.getBoundingClientRect();
          return r.left<b.left-.1||r.right>b.right+.1||r.top<b.top-.1||r.bottom>b.bottom+.1||e.classList.contains('caption')&&e.scrollWidth>e.clientWidth;
        }).map(e=>e.textContent);
      });
      if(controlClipping.length)throw Error(JSON.stringify({name,controlClipping}));
    }
    await plugin.screenshot({path:path.join(root,'mockups',name+'.png')});
    report.push({name,width:box.width,height:box.height,regionOverflow:overflow,brokenImages:broken,controlClipping});
  }
  await page.goto(pathToFileURL(path.join(root,'index-v1.html')).href+'?capture=1&variant=b&mode=perform');
  await page.evaluate(()=>document.fonts.ready);
  await page.locator('[data-section="Sound"]').click();
  await page.locator('.plugin').screenshot({path:path.join(root,'mockups','b-perform-scrolled.png')});
  if(!await page.getByRole('button',{name:'Reset',exact:true}).isVisible())throw Error('B reset hidden after scroll');
  await page.goto(pathToFileURL(path.join(root,'index-v1.html')).href+'?capture=1&variant=b&mode=learn');
  await page.evaluate(()=>document.fonts.ready);
  await page.locator('[data-learn-section="Capture"]').click();
  await page.locator('.plugin').screenshot({path:path.join(root,'mockups','b-learn-capture.png')});
  await page.goto(pathToFileURL(path.join(root,'index-v1.html')).href+'?variant=a&mode=perform&section=Envelope');
  await page.evaluate(()=>document.fonts.ready);
  await page.screenshot({path:path.join(root,'mockups','review-at-1920x1080.png')});
  await page.locator('#size-toggle').click();
  await page.locator('.plugin').screenshot({path:path.join(root,'mockups','a-perform-expanded-1100x850.png')});
  const tabChecks=[];
  for(const section of ['Timing','Envelope','Sound']){
    await page.locator(`[data-section="${section}"]`).click();
    tabChecks.push({section,controls:await page.locator('.section-content [data-control]').evaluateAll(items=>items.map(e=>e.dataset.control)),count:await page.locator('.section-content [data-control]').count(),resetVisible:await page.getByRole('button',{name:'Reset',exact:true}).isVisible()});
  }
  await fs.writeFile(path.join(root,'mockups','render-checks.json'),JSON.stringify({renderer:'Chromium, deviceScaleFactor 1; browser layout checks only',cases:report,tabChecks},null,2));
  await browser.close();
  console.log(JSON.stringify({rendered:report.length,tabChecks}));
})().catch(e=>{console.error(e);process.exit(1)});
