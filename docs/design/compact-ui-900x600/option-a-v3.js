let typography=qs.get('typography')||'responsive';
const renderA2=render;
render=function(){
  renderA2();
  document.querySelector('#typography').value=typography;
  document.querySelector('#typography').disabled=variant!=='a';
  if(variant==='a'){
    document.querySelector('.plugin').classList.toggle('type-calm',typography==='calm');
    if(typography==='calm'){
      document.querySelector('#caption').textContent=document.querySelector('#caption').textContent.replace('A2','A3')+' · CALMER TYPOGRAPHY';
      document.querySelector('#notes').innerHTML='<strong>Option A, calmer typography:</strong> headings 18 px, mode navigation 16 px, body / buttons / tabs 13 px, field captions 12 px. Numeric values remain 12 px. The 900 × 600 geometry, dial sizes and click targets are unchanged; wider gaps between paired controls and smaller text create more visual breathing room. Use the Typography selector to compare with revision 2. Future DSP remains a layout specimen.';
    }
  }
};
document.querySelector('#typography').onchange=e=>{typography=e.target.value;render()};
render();
