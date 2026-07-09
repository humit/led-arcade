#pragma once
#include <Arduino.h>

const char CONTROLLER_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="tr">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<title>LED Arcade Motion Demo</title>
<style>
html,body{margin:0;height:100%;background:#080b12;color:#fff;font-family:system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif;user-select:none;-webkit-user-select:none;overscroll-behavior:none}
body{display:flex;align-items:center;justify-content:center;padding:14px;box-sizing:border-box}
.card{width:min(94vw,520px);background:#151b2a;border:1px solid #2b3448;border-radius:28px;padding:20px;text-align:center;box-sizing:border-box}
h1{margin:0 0 6px;font-size:28px}.role{font-size:26px;font-weight:900;margin:8px 0}.blue{color:#38a8ff}.pink{color:#ff4d9d}.gray{color:#9ca8bd}
.gauge{height:30px;background:#0c1019;border:1px solid #2b3448;border-radius:999px;overflow:hidden;margin:20px 0;position:relative}.center{position:absolute;left:50%;top:0;bottom:0;width:2px;background:#fff5}.fill{position:absolute;top:0;bottom:0;background:linear-gradient(90deg,#38a8ff,#fff,#ff4d9d);transition:left .04s linear,width .04s linear}
.value{font-size:52px;font-weight:1000;font-variant-numeric:tabular-nums}.telemetry{display:grid;grid-template-columns:1fr 1fr;gap:10px;margin:14px 0}.box{background:#0f1420;border:1px solid #273044;border-radius:18px;padding:12px}.label{font-size:12px;color:#9ca8bd;font-weight:800}.num{font-size:24px;font-weight:900}
button{width:100%;height:64px;border:0;border-radius:18px;color:#fff;font-size:18px;font-weight:900;margin-top:10px}.primary{background:linear-gradient(180deg,#2687ff,#123a77)}.secondary{background:linear-gradient(180deg,#6b7280,#303642)}
.status{min-height:24px;margin-top:14px;color:#ffe066;font-weight:800}.tiny{margin-top:8px;color:#738096;font-size:12px;line-height:1.45}
</style>
</head>
<body>
<main class="card">
  <h1>Motion Demo</h1>
  <div id="role" class="role gray">BAĞLANIYOR</div>
  <div id="value" class="value">0.00</div>
  <div class="gauge"><div class="center"></div><div id="fill" class="fill"></div></div>
  <div class="telemetry">
    <div class="box"><div class="label">GAMMA</div><div id="gamma" class="num">0.0°</div></div>
    <div class="box"><div class="label">RELATIVE</div><div id="relative" class="num">0.0°</div></div>
  </div>
  <button id="motionBtn" class="primary">HAREKET İZNİ VER</button>
  <button id="calibrateBtn" class="secondary">NÖTRÜ KALİBRE ET</button>
  <div id="status" class="status">Telefonu dik tut ve sağa-sola eğ.</div>
  <div class="tiny">WebSocket: <span id="wsState">bağlanıyor</span> · http://go/ · 10.10.10.10</div>
</main>
<script>
function getCid(){let cid=localStorage.getItem("ledArcadeColorRallyCid");if(!cid){cid="c"+Math.random().toString(16).slice(2)+Date.now().toString(16);localStorage.setItem("ledArcadeColorRallyCid",cid)}return cid}
const cid=getCid();
const roleEl=document.getElementById("role"),valueEl=document.getElementById("value"),gammaEl=document.getElementById("gamma"),relativeEl=document.getElementById("relative"),fillEl=document.getElementById("fill"),statusEl=document.getElementById("status"),wsStateEl=document.getElementById("wsState"),motionBtn=document.getElementById("motionBtn"),calibrateBtn=document.getElementById("calibrateBtn");
let role="none",ws=null,neutralGamma=null,filtered=0,lastSent=99,lastSendAt=0,lastRawGamma=0,motionEnabled=false;
const DEAD_ZONE=4,MAX_TILT=30,FILTER_ALPHA=.25,SEND_INTERVAL=50,KEEPALIVE_INTERVAL=250;
function paintRole(){roleEl.className="role gray";if(role==="blue")roleEl.className="role blue";if(role==="pink")roleEl.className="role pink";roleEl.textContent=role==="blue"?"MAVİ OYUNCU":role==="pink"?"PEMBE OYUNCU":"İZLEYİCİ"}
function connectWs(){ws=new WebSocket("ws://"+location.hostname+":81/");ws.onopen=()=>{wsStateEl.textContent="bağlı";ws.send("HELLO|"+cid)};ws.onclose=()=>{wsStateEl.textContent="koptu";setTimeout(connectWs,800)};ws.onerror=()=>{wsStateEl.textContent="hata"};ws.onmessage=e=>{const m=String(e.data);if(m.startsWith("ROLE|")){role=m.slice(5);paintRole()}}}
async function join(){try{const r=await fetch("/join?cid="+encodeURIComponent(cid)+"&t="+Date.now(),{cache:"no-store"});const d=await r.json();role=d.role||"none";paintRole();if(ws&&ws.readyState===WebSocket.OPEN)ws.send("HELLO|"+cid)}catch(e){statusEl.textContent="Join başarısız"}}
function normalizeTilt(relative){const sign=relative<0?-1:1;const mag=Math.abs(relative);if(mag<=DEAD_ZONE)return 0;return sign*Math.min(1,(mag-DEAD_ZONE)/(MAX_TILT-DEAD_ZONE))}
function render(value,relative){valueEl.textContent=value.toFixed(2);gammaEl.textContent=lastRawGamma.toFixed(1)+"°";relativeEl.textContent=relative.toFixed(1)+"°";const pct=(value+1)*50;fillEl.style.left=Math.min(50,pct)+"%";fillEl.style.width=Math.abs(pct-50)+"%"}
function sendMotion(value,force=false){const now=performance.now();if(!ws||ws.readyState!==WebSocket.OPEN)return;if(!force&&now-lastSendAt<SEND_INTERVAL)return;if(!force&&Math.abs(value-lastSent)<.02&&now-lastSendAt<KEEPALIVE_INTERVAL)return;lastSent=value;lastSendAt=now;ws.send("MOTION|"+value.toFixed(3))}
function onOrientation(ev){if(typeof ev.gamma!=="number")return;lastRawGamma=ev.gamma;if(neutralGamma===null)neutralGamma=lastRawGamma;const relative=lastRawGamma-neutralGamma;const raw=normalizeTilt(relative);filtered=filtered*(1-FILTER_ALPHA)+raw*FILTER_ALPHA;render(filtered,relative);sendMotion(filtered)}
async function enableMotion(){try{if(typeof DeviceOrientationEvent!=="undefined"&&typeof DeviceOrientationEvent.requestPermission==="function"){const result=await DeviceOrientationEvent.requestPermission();if(result!=="granted")throw new Error("permission denied")}window.addEventListener("deviceorientation",onOrientation,true);motionEnabled=true;motionBtn.textContent="HAREKET AKTİF";statusEl.textContent="Telefonu sağa-sola eğerek topu hareket ettir."}catch(e){statusEl.textContent="Hareket izni alınamadı: "+e.message}}
motionBtn.addEventListener("click",enableMotion);calibrateBtn.addEventListener("click",()=>{neutralGamma=lastRawGamma;filtered=0;sendMotion(0,true);statusEl.textContent="Nötr konum kaydedildi"});
document.addEventListener("visibilitychange",()=>{if(document.hidden)sendMotion(0,true)});window.addEventListener("pagehide",()=>sendMotion(0,true));
join();connectWs();setInterval(()=>{fetch("/state?cid="+encodeURIComponent(cid)+"&t="+Date.now(),{cache:"no-store"}).catch(()=>{})},3000);
paintRole();render(0,0);
</script>
</body>
</html>
)HTML";
