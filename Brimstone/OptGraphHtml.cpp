// OptGraphHtml.cpp
//
// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
#include "stdafx.h"
#include "OptGraphHtml.h"

// A dependency-free canvas convergence chart: one colored line per pyramid
// level (matching the legacy graph -- L0/finest=red, L1=green, L2=blue,
// L3/coarsest=magenta), plotting -log10(F) against the global iteration index,
// with auto-scaled axes, grid, title and legend. addPoint()/resetChart() are
// installed on window for the host to call.
const wchar_t* g_optGraphHtml = LR"HTML(<!doctype html>
<html><head><meta charset="utf-8"><style>
  html,body{margin:0;height:100%;background:#0b0b0f;overflow:hidden;
    font:12px 'Segoe UI',Tahoma,sans-serif;color:#cccccc}
  #wrap{position:absolute;left:0;top:0;right:0;bottom:0}
  canvas{display:block}
</style></head><body>
<div id="wrap"><canvas id="cv"></canvas></div>
<script>
(function(){
  var COLORS=['#ff4040','#40ff40','#4080ff','#ff40ff'];
  var NAMES =['L0 (finest)','L1','L2','L3 (coarsest)'];
  var PAD={l:54,r:12,t:24,b:28};
  var series=[[],[],[],[]];
  var dirty=true;
  var cv=document.getElementById('cv'), ctx=cv.getContext('2d');
  var W=0,H=0,DPR=window.devicePixelRatio||1;

  function resize(){
    var wrap=document.getElementById('wrap');
    W=wrap.clientWidth; H=wrap.clientHeight;
    cv.width=W*DPR; cv.height=H*DPR; cv.style.width=W+'px'; cv.style.height=H+'px';
    ctx.setTransform(DPR,0,0,DPR,0,0); dirty=true;
  }
  window.addEventListener('resize',resize);

  function bounds(){
    var x0=Infinity,x1=-Infinity,y0=Infinity,y1=-Infinity,any=false,i,j,s,p;
    for(i=0;i<4;i++){s=series[i];for(j=0;j<s.length;j++){any=true;p=s[j];
      if(p.x<x0)x0=p.x; if(p.x>x1)x1=p.x; if(p.y<y0)y0=p.y; if(p.y>y1)y1=p.y;}}
    if(!any){x0=0;x1=1;y0=0;y1=1;}
    if(x1<=x0)x1=x0+1; if(y1<=y0)y1=y0+1;
    var yp=(y1-y0)*0.08; y0-=yp; y1+=yp;
    return {x0:x0,x1:x1,y0:y0,y1:y1};
  }
  function nice(range,ticks){
    var raw=range/ticks, mag=Math.pow(10,Math.floor(Math.log(raw)/Math.LN10)), n=raw/mag;
    return (n<1.5?1:n<3?2:n<7?5:10)*mag;
  }
  function draw(){
    dirty=false; ctx.clearRect(0,0,W,H);
    var b=bounds();
    var pw=W-PAD.l-PAD.r, ph=H-PAD.t-PAD.b;
    function px(x){return PAD.l+(x-b.x0)/(b.x1-b.x0)*pw;}
    function py(y){return H-PAD.b-(y-b.y0)/(b.y1-b.y0)*ph;}
    var y,x,X,Y,i,j,s;
    ctx.lineWidth=1; ctx.strokeStyle='#23232b'; ctx.fillStyle='#888888';
    ctx.textAlign='right'; ctx.textBaseline='middle';
    var ys=nice(b.y1-b.y0,5);
    for(y=Math.ceil(b.y0/ys)*ys; y<=b.y1; y+=ys){
      Y=py(y); ctx.beginPath();ctx.moveTo(PAD.l,Y);ctx.lineTo(W-PAD.r,Y);ctx.stroke();
      ctx.fillText(y.toFixed(2),PAD.l-6,Y);
    }
    ctx.textAlign='center'; ctx.textBaseline='top';
    var xs=nice(b.x1-b.x0,6);
    for(x=Math.ceil(b.x0/xs)*xs; x<=b.x1; x+=xs){
      X=px(x); ctx.beginPath();ctx.moveTo(X,PAD.t);ctx.lineTo(X,H-PAD.b);ctx.stroke();
      ctx.fillText(String(Math.round(x)),X,H-PAD.b+5);
    }
    ctx.lineWidth=1.5;
    for(i=0;i<4;i++){s=series[i]; if(s.length<1)continue;
      ctx.strokeStyle=COLORS[i]; ctx.beginPath();
      for(j=0;j<s.length;j++){X=px(s[j].x);Y=py(s[j].y); if(j)ctx.lineTo(X,Y);else ctx.moveTo(X,Y);}
      ctx.stroke();
      X=px(s[s.length-1].x); Y=py(s[s.length-1].y);
      ctx.fillStyle=COLORS[i]; ctx.beginPath(); ctx.arc(X,Y,2.5,0,6.2832); ctx.fill();
    }
    ctx.fillStyle='#dddddd'; ctx.textAlign='left'; ctx.textBaseline='top';
    ctx.fillText('Convergence:  -log10(F)  vs  iteration',PAD.l,5);
    ctx.textAlign='right';
    for(i=0;i<4;i++){ if(series[i].length<1)continue;
      ctx.fillStyle=COLORS[i]; ctx.fillText(NAMES[i],W-PAD.r,5+i*14);
    }
  }
  function loop(){ if(dirty)draw(); window.requestAnimationFrame(loop); }

  window.addPoint=function(level,x,y){
    if(level>=0&&level<4&&isFinite(y)){ series[level].push({x:x,y:y}); dirty=true; }
  };
  window.resetChart=function(){ for(var i=0;i<4;i++)series[i].length=0; dirty=true; };

  resize(); loop();
})();
</script></body></html>
)HTML";
