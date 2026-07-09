#pragma once
#include <Arduino.h>

const char CONTROLLER_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="tr">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<title>LED Arcade Color Rally</title>
<style>
html,body{margin:0;height:100%;background:#080b12;color:white;font-family:system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif;user-select:none;-webkit-user-select:none;touch-action:manipulation;overscroll-behavior:none}
body{display:flex;align-items:center;justify-content:center;padding:14px;box-sizing:border-box}
.card{width:min(94vw,520px);background:#151b2a;border:1px solid #2b3448;border-radius:28px;padding:18px;text-align:center;box-sizing:border-box}
h1{margin:0 0 6px;font-size:26px}.role{font-size:27px;font-weight:1000;margin:6px 0}
.blueText{color:#1f8ed7}.pinkText{color:#d72a78}.gray{color:#9ca8bd}
.score{display:grid;grid-template-columns:1fr 1fr;gap:10px;margin:12px 0}
.box{background:#0f1420;border:1px solid #273044;border-radius:18px;padding:10px}
.label{font-size:12px;color:#9ca8bd;font-weight:800}.num{font-size:30px;font-weight:1000;line-height:1.05}
.hint{min-height:62px;margin:12px 0;color:#ffe066;font-size:19px;font-weight:950;line-height:1.25}
.target{font-size:16px;color:#d7dde8;font-weight:800;min-height:24px}
.buttons{display:grid;grid-template-rows:1fr 1fr 1fr;gap:10px;margin-top:12px}
button{width:100%;height:24vh;min-height:82px;max-height:128px;border:0;border-radius:22px;color:white;font-size:32px;font-weight:1000;letter-spacing:.05em;box-shadow:0 12px 30px rgba(0,0,0,.32)}
button:active{transform:scale(.985);filter:brightness(1.25)}
.red{background:linear-gradient(180deg,#f04b43,#7d1714)}
.green{background:linear-gradient(180deg,#24c66a,#126436)}
.blueBtn{background:linear-gradient(180deg,#2687ff,#123a77)}
.disabled{filter:grayscale(.72);opacity:.55}
.systemButtons{display:grid;grid-template-columns:1fr 1fr;gap:10px;margin-top:12px}
.smallBtn{height:56px;min-height:56px;max-height:56px;border-radius:16px;font-size:17px;letter-spacing:0}
.pauseBtn{background:linear-gradient(180deg,#6b7280,#303642)}
.muteBtn{background:linear-gradient(180deg,#7c5cff,#37246e)}
.status{margin-top:12px;color:#9ca8bd;font-weight:800;min-height:22px}
.tiny{margin-top:6px;color:#667085;font-size:12px;line-height:1.35}
</style>
</head>
<body>
<main class="card">
  <h1>Color Rally</h1>
  <div id="role" class="role gray">BAĞLANIYOR</div>
  <div class="score">
    <div class="box"><div class="label">MAVİ</div><div id="blueStats" class="num blueText">5 HP</div><div id="blueWho" class="label">CPU</div></div>
    <div class="box"><div class="label">PEMBE</div><div id="pinkStats" class="num pinkText">5 HP</div><div id="pinkWho" class="label">CPU</div></div>
  </div>
  <div id="target" class="target">Gelen renge bas.</div>
  <div id="hint" class="hint">Top senin alana gelince aynı renge bas.</div>
  <div class="buttons">
    <button id="redBtn" class="red">KIRMIZI</button>
    <button id="greenBtn" class="green">YEŞİL</button>
    <button id="blueBtn" class="blueBtn">MAVİ</button>
  </div>
  <div class="systemButtons">
    <button id="pauseBtn" class="smallBtn pauseBtn">OYUNU DURDUR</button>
    <button id="muteBtn" class="smallBtn muteBtn">SESSİZ</button>
  </div>
  <div id="status" class="status">Hazır</div>
  <div class="tiny">SSID: LED-Arcade · IP: 10.10.10.1</div>
</main>
<script>
function getCid(){let cid=localStorage.getItem("ledArcadeColorRallyCid");if(!cid){cid="c"+Math.random().toString(16).slice(2)+Date.now().toString(16);localStorage.setItem("ledArcadeColorRallyCid",cid)}return cid}
const cid=getCid();
let role="none",targetSide="none",ballColor="red",inZone=false,mode="playing",lastPress=0,gamePaused=false,audioMuted=false;
const roleEl=document.getElementById("role"),targetEl=document.getElementById("target"),hintEl=document.getElementById("hint"),statusEl=document.getElementById("status"),blueStatsEl=document.getElementById("blueStats"),pinkStatsEl=document.getElementById("pinkStats"),blueWhoEl=document.getElementById("blueWho"),pinkWhoEl=document.getElementById("pinkWho"),redBtn=document.getElementById("redBtn"),greenBtn=document.getElementById("greenBtn"),blueBtn=document.getElementById("blueBtn"),pauseBtn=document.getElementById("pauseBtn"),muteBtn=document.getElementById("muteBtn");
function colorTr(c){if(c==="red")return"KIRMIZI";if(c==="green")return"YEŞİL";if(c==="blue")return"MAVİ";return""}
function roleText(r){if(r==="blue")return"MAVİ OYUNCU";if(r==="pink")return"PEMBE OYUNCU";return"İZLEYİCİ"}
function paint(){roleEl.className="role gray";if(role==="blue")roleEl.className="role blueText";if(role==="pink")roleEl.className="role pinkText";roleEl.textContent=roleText(role);pauseBtn.textContent=gamePaused?"OYUNU SÜRDÜR":"OYUNU DURDUR";muteBtn.textContent=audioMuted?"SESİ AÇ":"SESSİZ";redBtn.classList.remove("disabled");greenBtn.classList.remove("disabled");blueBtn.classList.remove("disabled");if(role==="none"||mode==="point"||gamePaused){redBtn.classList.add("disabled");greenBtn.classList.add("disabled");blueBtn.classList.add("disabled")}targetEl.textContent="GELEN RENK: "+colorTr(ballColor);if(gamePaused){hintEl.textContent="OYUN DURDU. Devam etmek için OYUNU SÜRDÜR’e bas."}else if(role==="none"){hintEl.textContent="İzleyicisin. Slot boşalırsa otomatik oyuna alınırsın."}else if(mode==="point"){hintEl.textContent="Hasar! Yeni rally başlıyor."}else if(targetSide===role&&inZone){hintEl.textContent="ŞİMDİ BAS: "+colorTr(ballColor)}else if(targetSide===role){hintEl.textContent="Hazırlan. Top sana geliyor: "+colorTr(ballColor)}else{hintEl.textContent="Bekle. Rakip savunuyor."}}
async function join(){try{const r=await fetch("/join?cid="+encodeURIComponent(cid)+"&t="+Date.now(),{cache:"no-store"});const d=await r.json();role=d.role||"none";statusEl.textContent=d.message||"Bağlandı";paint()}catch(e){statusEl.textContent="Bağlantı yok"}}
async function state(){try{const r=await fetch("/state?cid="+encodeURIComponent(cid)+"&t="+Date.now(),{cache:"no-store"});const d=await r.json();role=d.role||"none";targetSide=d.targetSide||"none";ballColor=d.ballColor||"red";inZone=!!d.inZone;mode=d.mode;gamePaused=!!d.gamePaused;audioMuted=!!d.audioMuted;blueStatsEl.textContent=d.blueHp+" HP · x"+d.blueCombo;pinkStatsEl.textContent=d.pinkHp+" HP · x"+d.pinkCombo;blueWhoEl.textContent=d.blueHuman?"PLAYER":"CPU";pinkWhoEl.textContent=d.pinkHuman?"PLAYER":"CPU";paint()}catch(e){statusEl.textContent="State alınamadı"}}
async function fire(color){if(gamePaused){statusEl.textContent="Oyun durdu";return}if(role==="none"){statusEl.textContent="İzleyici oynayamaz";return}const now=performance.now();if(now-lastPress<80)return;lastPress=now;try{const t0=performance.now();const r=await fetch("/press?cid="+encodeURIComponent(cid)+"&color="+encodeURIComponent(color)+"&t="+Date.now(),{cache:"no-store"});const d=await r.json();const ms=Math.round(performance.now()-t0);statusEl.textContent=(d.ok?"HIT":d.message)+" · "+ms+" ms"}catch(e){statusEl.textContent="Basış gitmedi"}}
async function setGamePaused(next){try{const r=await fetch("/game_pause?paused="+(next?1:0)+"&cid="+encodeURIComponent(cid)+"&t="+Date.now(),{cache:"no-store"});const d=await r.json();gamePaused=!!d.gamePaused;if(typeof d.audioMuted==="boolean")audioMuted=d.audioMuted;statusEl.textContent=gamePaused?"Oyun durdu":"Oyun sürdü";paint()}catch(e){statusEl.textContent="Pause gönderilemedi"}}
async function setMuted(next){try{const r=await fetch("/mute?muted="+(next?1:0)+"&cid="+encodeURIComponent(cid)+"&t="+Date.now(),{cache:"no-store"});const d=await r.json();audioMuted=!!d.audioMuted;statusEl.textContent=audioMuted?"Ses kapalı":"Ses açık";paint()}catch(e){statusEl.textContent="Mute gönderilemedi"}}
redBtn.addEventListener("pointerdown",ev=>{ev.preventDefault();fire("red")},{passive:false});
greenBtn.addEventListener("pointerdown",ev=>{ev.preventDefault();fire("green")},{passive:false});
blueBtn.addEventListener("pointerdown",ev=>{ev.preventDefault();fire("blue")},{passive:false});
pauseBtn.addEventListener("pointerdown",ev=>{ev.preventDefault();setGamePaused(!gamePaused)},{passive:false});
muteBtn.addEventListener("pointerdown",ev=>{ev.preventDefault();setMuted(!audioMuted)},{passive:false});
join();setInterval(state,180);
</script>
</body>
</html>
)HTML";
