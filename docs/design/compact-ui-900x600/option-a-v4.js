let reviewSize=qs.get('size')||'900x600';
let typeResizeObserver;
const renderA3=render;
function responsiveMetrics(width,height){
  const small=Math.max(0,Math.min(1,(width-900)/200,(height-600)/250));
  const large=Math.max(0,Math.min(1,(width-1100)/300,(height-850)/150));
  return {body:12+small+large,label:11+small+large,value:11+small+large,heading:16+2*small+3*large,subheading:14+2*small+2*large,nav:14+2*small+2*large,notice:12+small+large};
}
function layoutResponsive(root,width,height){
  reviewSize=Math.round(width)+'x'+Math.round(height);
  const metrics=responsiveMetrics(width,height);
  Object.entries(metrics).forEach(([role,value])=>root.style.setProperty('--ui-'+role,value+'px'));
  const get=s=>root.querySelector(s),all=s=>root.querySelectorAll(s);
  const overviewHeight=Math.round(Math.min(100,48+(height-600)*.128));
  const workBottom=height-40-overviewHeight-48,workTop=84,workHeight=workBottom-workTop;
  const contentWidth=width-24,centreWidth=width-484;
  const header=get('.header');header.style.width=contentWidth+'px';header.style.height='64px';
  const logo=get('.wordmark');logo.src='assets/wordmark-optical.svg';
  const description=get('.description');description.classList.remove('ell');description.style.left='220px';description.style.width=(contentWidth-378-220-12)+'px';
  const notice=get('.notice');if(notice){notice.classList.remove('ell');notice.style.width=contentWidth+'px';notice.style.fontSize=metrics.notice+'px';notice.style.lineHeight='16px';
    // Bounded two-line fallback for unusually long specimen notices, never ellipsis.
    if(notice.scrollHeight>notice.clientHeight){notice.style.fontSize='11px';notice.style.lineHeight='12px';}}
  get('.help').style.width=contentWidth+'px';
  if(mode==='perform'){
    all('.column').forEach(e=>{e.style.top=workTop+'px';e.style.height=workHeight+'px'});
    get('.detail').style.width=centreWidth+'px';get('.inspector').style.left=(width-288)+'px';
    get('.section-content').style.height=(workHeight-144)+'px';
    const zoom=get('.detail .zoom-row'),snap=get('.snap-row');
    let wide=get('.navigation-wide');
    if(centreWidth>=472&&!wide){wide=document.createElement('div');wide.className='navigation-wide';zoom.before(wide);wide.append(zoom,snap);snap.querySelector('.helper')?.remove();}
    if(centreWidth<472&&wide){wide.before(zoom,snap);wide.remove();}
    const navHeight=centreWidth>=472?38:76;
    const laneHeight=Math.floor((workHeight-28-22-24-navHeight-8)/2);
    all('.lane').forEach(e=>e.style.height=laneHeight+'px');
    const audition=get('.audition');audition.style.top=workBottom+'px';audition.style.width=contentWidth+'px';
    const overview=get('.overview');overview.style.top=(workBottom+48)+'px';overview.style.width=contentWidth+'px';overview.style.height=overviewHeight+'px';
    get('.overview img').style.width=contentWidth+'px';get('.overview img').style.height=(overviewHeight-24)+'px';
    get('.host').style.top=(height-40)+'px';get('.host').style.width=contentWidth+'px';
  }else if(mode==='learn'){
    get('.learn-left').style.top=workTop+'px';get('.learn-left').style.height=(height-workTop-20)+'px';
    get('.learn-right').style.top=workTop+'px';get('.learn-right').style.height=(height-workTop-20)+'px';get('.learn-right').style.width=(width-324)+'px';
    get('.learn-right img').style.height=Math.min(height-250,(width-324)*.54)+'px';
  }else{
    get('.export-left').style.top=workTop+'px';get('.export-left').style.height=(height-workTop-32)+'px';get('.export-left').style.width=(width-342)+'px';
    get('.export-right').style.top=workTop+'px';get('.export-right').style.left=(width-312)+'px';get('.export-right').style.height=(height-workTop-32)+'px';
    get('.export-wave').style.width=(width-342)+'px';get('.export-wave').style.height=(height-282)+'px';
  }
  document.querySelector('#dimensions').textContent=`${Math.round(width)} × ${Math.round(height)} · responsive type`;
}
render=function(){
  typeResizeObserver?.disconnect();
  if(typography==='responsive')expanded=false;
  renderA3();
  document.querySelector('#review-size').value=reviewSize;
  document.querySelector('#review-size').disabled=variant!=='a'||typography!=='responsive';
  if(variant==='a'&&typography==='responsive'){
    const root=document.querySelector('.plugin'),[width,height]=reviewSize.split('x').map(Number);
    root.classList.add('type-responsive');root.style.width=width+'px';root.style.height=height+'px';root.style.minWidth='900px';root.style.minHeight='600px';root.style.maxWidth='1600px';root.style.maxHeight='1200px';root.style.resize='both';
    layoutResponsive(root,width,height);
    typeResizeObserver=new ResizeObserver(items=>{for(const item of items)layoutResponsive(root,item.contentRect.width,item.contentRect.height)});typeResizeObserver.observe(root);
    document.querySelector('#caption').textContent=`A4 · ${mode.toUpperCase()} · RESPONSIVE TYPE / COMFORT PADDING${future&&mode!=='export'?' · FUTURE DSP SPECIMEN':''}`;
    document.querySelector('#notes').innerHTML='<strong>Revision 4:</strong> optically aligned logo; full-width message row; 8 px between header and workspace, inspector content and dividers, and Reset and the lower divider. At 900 × 600: headings 16 px, navigation 14 px, body 12 px, captions / values 11 px. Type rises continuously to 18 / 16 / 13 / 12 at 1100 × 850 and 21 / 18 / 14 / 13 at 1400 × 1000. Dial and click-target sizes are retained. Choose a review size or drag the bottom-right resize grip; original typography comparisons remain available. Browser mockup only.';
  }
};
document.querySelector('#review-size').onchange=e=>{reviewSize=e.target.value;render()};
document.querySelector('#size-toggle').onclick=()=>{if(typography==='responsive'){reviewSize=reviewSize==='900x600'?'1100x850':'900x600';render();}else{expanded=!expanded;render();}};
render();
