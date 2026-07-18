// DvhViewHtml.cpp
//
// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
#include "stdafx.h"
#include "DvhViewHtml.h"

const wchar_t* g_dvhViewHtml = LR"HTML(<!doctype html>
<html><head><meta charset="utf-8"><style>
  :root{--bg:#0b0b0f;--panel:#15151c;--line:#2a2a34;--txt:#cfcfd6;--dim:#8a8a95}
  html,body{margin:0;height:100%;background:var(--bg);color:var(--txt);
    font:12px 'Segoe UI',Tahoma,sans-serif;overflow:hidden}
  #root{position:absolute;inset:0;display:flex;flex-direction:column}
  #chartWrap{flex:1 1 55%;position:relative;min-height:80px}
  canvas{display:block}
  #lists{flex:1 1 45%;display:flex;gap:6px;padding:6px;box-sizing:border-box;min-height:120px}
  .list{flex:1 1 0;background:var(--panel);border:1px solid var(--line);border-radius:4px;
    display:flex;flex-direction:column;min-width:0}
  .list h3{margin:0;padding:4px 8px;font-size:11px;font-weight:600;letter-spacing:.04em;
    color:var(--dim);border-bottom:1px solid var(--line);text-transform:uppercase}
  .rows{flex:1 1 0;overflow-y:auto;overflow-x:hidden}
  .cols{display:flex;padding:2px 6px;font-size:10px;color:var(--dim);gap:4px}
  .cols span{flex:1 1 0;min-width:0}
  .cName{flex:2 1 0 !important}
  .row{display:flex;align-items:center;gap:4px;padding:3px 6px;cursor:grab;
    border-bottom:1px solid #1e1e26}
  .row:hover{background:#1c1c24}
  .row.drag{opacity:.4}
  .row .sw{width:12px;height:12px;border-radius:2px;flex:0 0 auto;cursor:pointer;border:1px solid #000}
  .row .nm{flex:2 1 0;min-width:0;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
  .row input{flex:1 1 0;min-width:0;width:100%;box-sizing:border-box;background:#0e0e14;
    color:var(--txt);border:1px solid #26262f;border-radius:2px;font:11px monospace;padding:1px 3px}
  .row input.w{color:#ffd479}
  .list.over{outline:1px dashed #4080ff;outline-offset:-2px}
  .hidecol{display:none}
</style></head><body>
<div id="root">
  <div id="chartWrap"><canvas id="cv"></canvas></div>
  <div id="lists">
    <div class="list" data-type="1"><h3>Target</h3>
      <div class="cols"><span class="cName">Name</span><span>Min</span><span>Max</span><span>Weight</span></div>
      <div class="rows"></div></div>
    <div class="list" data-type="2"><h3>OAR</h3>
      <div class="cols"><span class="cName">Name</span><span>Min</span><span>Max</span><span>Weight</span></div>
      <div class="rows"></div></div>
    <div class="list" data-type="0"><h3>None</h3>
      <div class="cols"><span class="cName">Name</span><span>Min</span><span>Max</span><span>Weight</span></div>
      <div class="rows"></div></div>
  </div>
</div>
)HTML" LR"HTML(
<script>
(function(){
  function post(s){ if(window.chrome&&window.chrome.webview) window.chrome.webview.postMessage(s); }

  // ---------- structure editor ----------
  var structs={};       // id -> {id,name,color,type,min,max,weight,priority}
  var dragId=null;

  function fmt(v){ return (v==null||isNaN(v))?'':(+v).toFixed(2); }

  function makeRow(s){
    var row=document.createElement('div'); row.className='row'; row.draggable=true; row.dataset.id=s.id;
    var sw=document.createElement('div'); sw.className='sw'; sw.style.background=s.color;
    sw.title='color'; sw.onclick=function(e){e.stopPropagation(); pickColor(s.id, s.color);};
    var nm=document.createElement('div'); nm.className='nm'; nm.textContent=s.name; nm.title=s.name;
    var mn=document.createElement('input'); mn.value=fmt(s.min); mn.title='min dose';
    var mx=document.createElement('input'); mx.value=fmt(s.max); mx.title='max dose';
    var wt=document.createElement('input'); wt.className='w'; wt.value=fmt(s.weight); wt.title='weight';
    mn.onchange=function(){ commitInterval(s.id, mn.value, mx.value); };
    mx.onchange=function(){ commitInterval(s.id, mn.value, mx.value); };
    wt.onchange=function(){ post('weight|'+s.id+'|'+(parseFloat(wt.value)||0)); };
    // None list has no numeric fields
    if(s.type===0){ mn.classList.add('hidecol'); mx.classList.add('hidecol'); wt.classList.add('hidecol'); }
    row.appendChild(sw); row.appendChild(nm); row.appendChild(mn); row.appendChild(mx); row.appendChild(wt);
    row.addEventListener('dragstart',function(e){ dragId=s.id; row.classList.add('drag'); e.dataTransfer.effectAllowed='move'; });
    row.addEventListener('dragend',function(){ dragId=null; row.classList.add; row.classList.remove('drag'); });
    return row;
  }

  function commitInterval(id,mn,mx){
    var a=parseFloat(mn), b=parseFloat(mx);
    if(isFinite(a)&&isFinite(b)) post('interval|'+id+'|'+a+'|'+b);
  }
  function pickColor(id,cur){
    var inp=document.createElement('input'); inp.type='color'; inp.value=cur||'#808080';
    inp.style.position='fixed'; inp.style.left='-100px'; document.body.appendChild(inp);
    inp.addEventListener('input',function(){ post('color|'+id+'|'+inp.value.replace('#','')); });
    inp.addEventListener('change',function(){ document.body.removeChild(inp); });
    inp.click();
  }

  function rebuild(){
    document.querySelectorAll('.list').forEach(function(list){
      var t=+list.dataset.type, rows=list.querySelector('.rows'); rows.innerHTML='';
      Object.values(structs).filter(function(s){return s.type===t;})
        .sort(function(a,b){return a.priority-b.priority;})
        .forEach(function(s){ rows.appendChild(makeRow(s)); });
    });
  }

  // drag & drop between/within lists
  function insertBefore(rows,y){
    var els=[].slice.call(rows.querySelectorAll('.row:not(.drag)'));
    for(var i=0;i<els.length;i++){ var r=els[i].getBoundingClientRect();
      if(y < r.top + r.height/2) return els[i]; }
    return null;
  }
  document.querySelectorAll('.list').forEach(function(list){
    var rows=list.querySelector('.rows');
    list.addEventListener('dragover',function(e){ e.preventDefault(); list.classList.add('over');
      var after=insertBefore(rows,e.clientY); var drag=document.querySelector('.row.drag');
      if(drag){ if(after) rows.insertBefore(drag,after); else rows.appendChild(drag); } });
    list.addEventListener('dragleave',function(){ list.classList.remove('over'); });
    list.addEventListener('drop',function(e){ e.preventDefault(); list.classList.remove('over');
      if(dragId==null) return;
      var newType=+list.dataset.type;
      if(structs[dragId] && structs[dragId].type!==newType){ post('type|'+dragId+'|'+newType); }
      // full display order across all lists -> priorities
      var order=[];
      document.querySelectorAll('.list .rows .row').forEach(function(r){ order.push(r.dataset.id); });
      post('order|'+order.join(','));
    });
  });

  window.setStructures=function(arr){
    structs={}; (arr||[]).forEach(function(s){ structs[s.id]=s; }); rebuild();
    draw();
  };

  // ---------- DVH chart ----------
  var curves=[];  // {id,color,target,pts:[[dose,vol],...]}
  var cv=document.getElementById('cv'), ctx=cv.getContext('2d');
  var W=0,H=0,DPR=window.devicePixelRatio||1;
  var PAD={l:40,r:10,t:10,b:22};
  function resize(){ var w=document.getElementById('chartWrap');
    W=w.clientWidth; H=w.clientHeight; cv.width=W*DPR; cv.height=H*DPR;
    cv.style.width=W+'px'; cv.style.height=H+'px'; ctx.setTransform(DPR,0,0,DPR,0,0); draw(); }
  window.addEventListener('resize',resize);

  function draw(){
    ctx.clearRect(0,0,W,H);
    var xmax=1; for(var i=0;i<curves.length;i++){ var p=curves[i].pts; for(var j=0;j<p.length;j++) if(p[j][0]>xmax) xmax=p[j][0]; }
    xmax=Math.ceil(xmax/10)*10 || 100;
    var pw=W-PAD.l-PAD.r, ph=H-PAD.t-PAD.b;
    function px(x){return PAD.l+x/xmax*pw;}
    function py(v){return H-PAD.b-v/100*ph;}
    ctx.strokeStyle='#23232b'; ctx.fillStyle='#888'; ctx.lineWidth=1;
    ctx.textAlign='right'; ctx.textBaseline='middle';
    for(var v=0; v<=100; v+=25){ var Y=py(v); ctx.beginPath();ctx.moveTo(PAD.l,Y);ctx.lineTo(W-PAD.r,Y);ctx.stroke(); ctx.fillText(v,PAD.l-4,Y); }
    ctx.textAlign='center'; ctx.textBaseline='top';
    var xs=Math.max(10,Math.round(xmax/5/10)*10);
    for(var x=0;x<=xmax;x+=xs){ var X=px(x); ctx.beginPath();ctx.moveTo(X,PAD.t);ctx.lineTo(X,H-PAD.b);ctx.stroke(); ctx.fillText(x,X,H-PAD.b+4); }
    for(var i=0;i<curves.length;i++){ var c=curves[i], p=c.pts; if(!p||p.length<1) continue;
      ctx.strokeStyle=c.color; ctx.lineWidth=c.target?1:1.7; ctx.setLineDash(c.target?[4,3]:[]);
      ctx.beginPath();
      for(var j=0;j<p.length;j++){ var X=px(p[j][0]),Y=py(p[j][1]); if(j)ctx.lineTo(X,Y);else ctx.moveTo(X,Y); }
      ctx.stroke();
    }
    ctx.setLineDash([]);
    ctx.fillStyle='#bbb'; ctx.textAlign='left'; ctx.textBaseline='top';
    ctx.fillText('DVH:  volume %  vs  dose',PAD.l,PAD.t);
  }
  window.setDvh=function(arr){ curves=arr||[]; draw(); };

  resize();
})();
</script></body></html>
)HTML";
