#pragma once
#include <Arduino.h>

static const char ARCADE_HTML[] PROGMEM = R"HTML(
<!doctype html><html lang="tr"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,minimum-scale=1,user-scalable=no,viewport-fit=cover"><meta name="theme-color" content="#080a12"><title>LED Arcade</title><style>
:root{--accent:#6d7cff;--dark:#0a1020;--soft:#a6b6ff;--glow:rgba(109,124,255,.45);--bg:#080a12;--panel:#141827;--text:#fff;--muted:#aab2cb}*{box-sizing:border-box;-webkit-tap-highlight-color:transparent}html,body{margin:0;width:100%;min-height:100%;background:var(--bg);color:var(--text);font-family:system-ui,-apple-system,sans-serif;-webkit-user-select:none;user-select:none;-webkit-touch-callout:none}body{overflow-x:hidden;overflow-y:auto;overscroll-behavior-y:contain}html.game-active,body.game-active{overflow:hidden!important;overscroll-behavior:none!important}body.game-active{position:fixed;inset:0;width:100%;height:var(--viewport-height,100dvh);touch-action:none}html.stack-active,body.stack-active{position:fixed!important;inset:0;width:100%;height:var(--viewport-height,100dvh);overflow:hidden!important;overscroll-behavior:none!important;touch-action:none!important}button{font:inherit;color:inherit}.app{width:min(100%,600px);min-height:100vh;min-height:100dvh;margin:auto;padding:16px max(16px,env(safe-area-inset-right)) calc(16px + env(safe-area-inset-bottom)) max(16px,env(safe-area-inset-left));display:flex;flex-direction:column;background:radial-gradient(circle at top,var(--dark),var(--bg) 58%)}.top{display:flex;justify-content:space-between;align-items:center;gap:10px}.brand{font-weight:1000;letter-spacing:.13em}.lang{display:flex;gap:4px}.lang button{border:1px solid #3c435b;background:#111522;border-radius:9px;padding:5px 8px;font-size:11px}.lang .on{background:var(--accent);border-color:var(--accent)}.identity{display:flex;align-items:center;gap:8px;font-size:12px;font-weight:900}.dot{width:16px;height:16px;border-radius:50%;background:var(--accent);box-shadow:0 0 18px var(--glow)}.status{font-size:10px;color:var(--muted)}h1{font-size:31px;margin:24px 0 7px}p{color:var(--muted);line-height:1.35}.grid{display:grid;gap:12px}.card,.btn{border:1px solid #303854;background:linear-gradient(160deg,#1a2033,#101420);border-radius:18px;padding:16px;box-shadow:0 12px 30px #0007;text-align:left}.card strong{font-size:21px;display:block;margin-top:8px}.card small{color:var(--muted)}.preview{width:100%;height:112px;border-radius:12px;background:#04050a;image-rendering:pixelated}.badge{display:inline-block;margin-top:8px;padding:4px 8px;border-radius:999px;background:#25304d;font-size:10px;font-weight:900}.btn{width:100%;text-align:center;font-weight:900;margin-top:10px}.primary{background:linear-gradient(135deg,var(--accent),var(--soft));border:0;color:#fff}.danger{border-color:#7f3347;color:#ffb8c7}.hidden{display:none!important}.players{display:grid;gap:8px}.player{display:flex;justify-content:space-between;background:#121725;border-radius:12px;padding:10px 12px}.session{display:grid;grid-template-columns:1fr 1fr;gap:8px;margin-top:10px}.hint{padding:13px;border-left:4px solid var(--accent);background:#111626;border-radius:10px;color:#dbe2ff}.full{position:fixed;inset:0;width:100vw;height:var(--viewport-height,100dvh);z-index:30;background:var(--dark);touch-action:none;display:flex;align-items:stretch;justify-content:stretch;overflow:hidden;padding:env(safe-area-inset-top) env(safe-area-inset-right) env(safe-area-inset-bottom) env(safe-area-inset-left)}.tapSurface{position:absolute;inset:0;background:radial-gradient(circle,var(--accent),var(--dark) 72%);touch-action:none}.tapSurface.flash{filter:brightness(1.65)}.tronSurface{position:absolute;inset:env(safe-area-inset-top) env(safe-area-inset-right) env(safe-area-inset-bottom) env(safe-area-inset-left);display:grid;grid-template-columns:minmax(0,1fr) minmax(0,1fr);touch-action:none;overflow:hidden}.turn{display:flex;min-width:0;min-height:0;width:100%;height:100%;align-items:center;justify-content:center;font-size:clamp(54px,20vmin,120px);line-height:1;font-weight:1000;background:var(--dark);border:0;padding:0;overflow:hidden;touch-action:none}.turn:first-child{border-right:1px solid var(--soft)}.turn.flash{background:var(--accent)}.announce{font-size:min(18vw,86px);font-weight:1000;text-align:center;color:var(--accent);text-shadow:0 0 25px var(--glow)}.result{text-align:center;margin:auto;width:100%;padding:20px 0}.result h1{font-size:clamp(28px,8vw,38px)}.footer{margin-top:auto;text-align:center;color:#737d9d;font-size:10px;padding-top:12px}
body.menu-active{overflow:hidden}.menu-active .app{height:100vh;height:100dvh;min-height:0;overflow:hidden;padding-top:10px;padding-bottom:10px}.menu-active .top{flex:0 0 auto}.menu-active .identity,.menu-active .status{display:none}.menu-active #games,.menu-active #platform{flex:1;min-height:0;display:flex;flex-direction:column}.menu-active h1{font-size:clamp(20px,6vw,28px);margin:10px 0 8px}.menu-active .grid{flex:1;min-height:0;grid-template-columns:repeat(2,minmax(0,1fr));gap:8px}.gameCard{min-width:0;min-height:0;border:1px solid #303854;background:linear-gradient(160deg,#1a2033,#101420);border-radius:14px;padding:9px;box-shadow:0 8px 20px #0007;display:flex;flex-direction:column;overflow:hidden}.gameCard .preview{height:auto;aspect-ratio:8/3;flex:0 0 auto;border-radius:8px}.gameCard strong{font-size:clamp(14px,4.2vw,18px);line-height:1.05;margin:7px 0 3px}.gameCard small{display:block;color:var(--muted);font-size:clamp(9px,2.8vw,12px);line-height:1.15;min-height:2.3em;overflow:hidden}.gameMeta{display:flex;align-items:center;justify-content:space-between;gap:5px;margin-top:auto;padding-top:6px}.gameMeta .badge{margin:0;padding:3px 6px;font-size:8px}.playBtn{border:0;border-radius:8px;background:linear-gradient(135deg,var(--accent),var(--soft));color:#fff;font-weight:1000;font-size:11px;padding:7px 9px;touch-action:manipulation}.menu-active .footer{display:none}.menu-active #platform .card{padding:10px;min-height:0;overflow:hidden}.menuTitle{display:flex;align-items:center;justify-content:space-between;gap:10px}.menuTitle h1{margin-right:auto}.menuBack{margin:0 0 12px;background:linear-gradient(160deg,#1a2033,#101420);border-color:#303854;color:#dfe5ff;touch-action:manipulation}.menu-active #platform .preview{height:auto;aspect-ratio:8/3}.menu-active #platform .card strong{font-size:clamp(14px,4vw,18px);margin-top:6px}.menu-active #platform .card small{font-size:10px}
@media (max-height:480px){.menu-active .gameCard small{display:none}.menu-active .gameCard strong{margin-bottom:4px}.menu-active h1{margin:5px 0 5px}.menu-active .top{padding:0}.menu-active .gameMeta{padding-top:3px}}
@media (orientation:landscape) and (max-height:560px){.app{width:min(100%,900px);padding-top:10px;padding-bottom:12px}.top{position:sticky;top:0;z-index:5;padding:4px 0;background:linear-gradient(var(--dark),transparent)}h1{font-size:24px;margin:12px 0 6px}.preview{height:76px}.grid{grid-template-columns:1fr 1fr}.card{padding:11px;border-radius:14px}.card strong{font-size:17px}.hint{padding:9px}.player{padding:7px 10px}.btn{padding:10px;margin-top:7px}.footer{padding-top:8px}.result{margin:0 auto;padding:10px 0 20px}.result h1{margin-top:8px}}

/* v11: bounded arcade UI. Every normal state fits one viewport; gameplay remains full-screen. */
body:not(.game-active){overflow:hidden;overscroll-behavior:none}.app{height:var(--viewport-height,100dvh);min-height:0;overflow:hidden}.top{flex:0 0 auto}.view-shell{min-height:0;flex:1;overflow:hidden}.menu-active #games,.menu-active #platform{height:100%;min-height:0}.menu-active .grid{grid-template-rows:repeat(2,minmax(0,1fr))}.gameCard{cursor:pointer;touch-action:manipulation;position:relative}.gameCard:active{transform:scale(.985);filter:brightness(1.08)}.gameCard[aria-disabled="true"]{opacity:.42;cursor:default}.card.selected{border-color:var(--accent);box-shadow:0 0 0 2px var(--accent),0 8px 20px #0007}.playBtn{pointer-events:none}.comingPreview{display:grid;place-items:center;background:repeating-linear-gradient(135deg,#080a12,#080a12 8px,#101528 8px,#101528 16px);color:#68708c;font-weight:1000;letter-spacing:.12em;font-size:clamp(11px,3vw,16px)}
#lobby{height:100%;min-height:0;display:grid;grid-template-rows:auto auto 1fr auto;gap:8px;overflow:hidden}#lobby.hidden{display:none!important}.lobbyHead{min-height:0}.lobbyHead h1{margin:8px 0 4px;font-size:clamp(20px,6vw,30px)}#instructions{font-size:clamp(11px,3vw,14px);padding:9px 10px}#roleText{margin:0;font-size:12px}.players{min-height:0;overflow:hidden;display:grid;grid-template-columns:repeat(2,minmax(0,1fr));grid-auto-rows:minmax(0,1fr);gap:6px}.player{min-height:0;padding:7px 8px;align-items:center;font-size:clamp(10px,2.8vw,13px);overflow:hidden}.player span,.player strong{white-space:nowrap;overflow:hidden;text-overflow:ellipsis}.lobbyActions{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:7px}.lobbyActions .btn{margin:0;padding:10px 6px;min-height:44px;font-size:clamp(11px,3vw,14px)}
body[data-view="result"],body[data-view="boss_result"]{overflow:hidden}.result{height:100%;min-height:0;display:flex;flex-direction:column;justify-content:center;overflow:hidden;padding:8px}.result .btn{margin-top:7px;padding:10px}.result .session{margin-top:7px}.result h1{margin:4px 0}.result p{margin:4px 0 8px}.result.input-locked .btn{pointer-events:none;opacity:.35}.result.input-locked::after{content:"";position:absolute;inset:0;z-index:2}
@media (orientation:landscape) and (max-height:560px){#lobby{grid-template-columns:minmax(0,1.5fr) minmax(180px,.7fr);grid-template-rows:auto 1fr;column-gap:10px}.lobbyHead{grid-column:1}.players{grid-column:1;grid-row:2;grid-template-columns:repeat(2,minmax(0,1fr))}.lobbyActions{grid-column:2;grid-row:1/3;grid-template-columns:1fr;align-content:center}.lobbyActions .btn{min-height:38px;padding:7px}.menu-active .grid{grid-template-columns:repeat(4,minmax(0,1fr));grid-template-rows:1fr}.menu-active .gameCard strong{font-size:clamp(11px,2.2vw,16px)}.menu-active .gameCard small{display:none}.gameMeta{justify-content:center}.menu-active h1{font-size:18px;margin:4px 0}}
@media (max-width:360px),(max-height:600px){.menu-active .gameCard{padding:6px}.menu-active .gameCard strong{font-size:12px}.menu-active .gameCard small{display:none}.gameMeta .badge{font-size:7px}.playBtn{display:none}.menu-active h1{font-size:18px;margin:4px 0}.top{min-height:28px}.brand{font-size:13px}.lang button{padding:3px 6px}.lobbyActions .btn{min-height:40px;padding:7px}}

/* v12: portrait-first boot + Pixel Raider controller */
.raiderSurface{position:absolute;inset:env(safe-area-inset-top) env(safe-area-inset-right) env(safe-area-inset-bottom) env(safe-area-inset-left);display:grid;grid-template-rows:1fr 1fr;touch-action:none;overflow:hidden}.raiderMove{display:flex;align-items:center;justify-content:center;border:0;background:var(--dark);color:var(--soft);font-size:clamp(58px,22vmin,132px);font-weight:1000;touch-action:none}.raiderMove:first-child{border-bottom:1px solid var(--soft)}.raiderMove.flash{background:var(--accent);color:#fff}.clashSurface{position:absolute;inset:env(safe-area-inset-top) env(safe-area-inset-right) env(safe-area-inset-bottom) env(safe-area-inset-left);display:grid;grid-template-columns:1fr 1fr;grid-template-rows:30% 40% 30%;grid-template-areas:"up up" "left right" "down down";touch-action:none;overflow:hidden}.clashMove{display:flex;align-items:center;justify-content:center;border:0;background:var(--dark);color:var(--soft);font-size:clamp(48px,18vmin,118px);font-weight:1000;touch-action:none}.clashMove.flash{background:var(--accent);color:#fff}#clashUp{grid-area:up;border-bottom:1px solid var(--soft)}#clashLeft{grid-area:left;border-right:1px solid var(--soft)}#clashRight{grid-area:right}#clashDown{grid-area:down;border-top:1px solid var(--soft)}.gestureSurface{touch-action:none;overscroll-behavior:none}.gestureSurface.dragging{filter:brightness(1.08)}.stackSurface{position:absolute;inset:env(safe-area-inset-top) env(safe-area-inset-right) env(safe-area-inset-bottom) env(safe-area-inset-left);background:radial-gradient(circle at 50% 45%,var(--accent),var(--dark) 72%);touch-action:none;overscroll-behavior:none;overflow:hidden;contain:layout paint}.stackTop{position:absolute;z-index:10;top:10px;left:10px;right:10px;display:grid;grid-template-columns:auto auto 1fr;align-items:start;gap:8px;pointer-events:none}.stackPauseBtn,.stackHelpBtn{pointer-events:auto;border:1px solid #ffffff35;background:#090d18cc;border-radius:12px;padding:9px 11px;min-height:38px;font-size:11px;font-weight:1000;letter-spacing:.04em;touch-action:manipulation}.stackPauseBtn{min-width:86px}.stackHelpBtn{width:40px;padding:8px;font-size:18px;line-height:1}.stackPauseBtn:active,.stackHelpBtn:active{background:var(--accent)}.stackHud{padding-top:8px;text-align:center;font-size:11px;font-weight:900;letter-spacing:.06em;color:#fffc;pointer-events:none}.stackNext{position:absolute;z-index:1;inset:0;display:grid;place-items:center;pointer-events:none;opacity:.28;filter:saturate(.9);mix-blend-mode:screen}.stackNext span{position:absolute;z-index:2;left:50%;top:50%;transform:translate(-50%,-50%);font-size:clamp(28px,10vw,64px);font-weight:1000;letter-spacing:.22em;color:#fff;opacity:.18;white-space:nowrap;text-shadow:0 0 24px #000}.stackNext canvas{width:min(72vw,320px);height:auto;aspect-ratio:1;image-rendering:pixelated;opacity:.78}.stackTutorial{position:absolute;z-index:6;left:14px;right:14px;bottom:max(14px,env(safe-area-inset-bottom));min-height:min(43vh,330px);padding:14px 16px 16px;border:1px solid #ffffff2b;border-radius:20px;background:linear-gradient(160deg,#090d18ed,#11192be8);box-shadow:0 18px 42px #000a,0 0 30px var(--glow);backdrop-filter:blur(10px);pointer-events:none}.stackTutorialHead{display:grid;grid-template-columns:1fr auto auto;align-items:center;gap:10px}.stackTutorialHead>strong{font-size:12px;letter-spacing:.16em}.stackTutorialProgress{display:flex;gap:5px}.stackTutorialDot{width:7px;height:7px;border-radius:50%;background:#ffffff25;transition:.18s}.stackTutorialDot.done{background:var(--accent);box-shadow:0 0 10px var(--glow)}.stackTutorialDot.active{background:#fff;transform:scale(1.25)}.stackTutorialClose{pointer-events:auto;width:32px;height:32px;padding:0;border:1px solid #ffffff28;border-radius:50%;background:#ffffff0d;color:#fff;font-size:20px;line-height:1;touch-action:manipulation}.stackTutorialStep{display:none;min-height:210px;flex-direction:column;align-items:center;justify-content:center;gap:12px;text-align:center}.stackTutorialStep.active{display:flex}.stackTutorialTitle{font-size:clamp(13px,3.8vmin,18px);letter-spacing:.14em;font-weight:1000;color:#fff}.stackTutorialHint{font-size:clamp(9px,2.5vmin,12px);letter-spacing:.08em;font-weight:800;color:#ffffff85}.stackTutorialPrompt{display:block;text-align:center;font-size:9px;letter-spacing:.12em;color:#ffffff65}.stackTapStage{position:relative;width:82px;height:82px}.stackTapRing,.stackTapDot,.stackRotateArc{position:absolute;left:50%;top:50%;transform:translate(-50%,-50%);border-radius:50%}.stackTapRing{width:52px;height:52px;border:2px solid #ffffff55;animation:stackTutorialTapPulse 1.8s ease-out infinite}.stackTapDot{width:18px;height:18px;background:#ffffffd8;box-shadow:0 0 18px var(--glow);animation:stackTutorialTapDot 1.8s ease-in-out infinite}.stackRotateArc{width:70px;height:70px;border:2px solid transparent;border-top-color:var(--soft);border-right-color:var(--soft);animation:stackTutorialRotate 1.8s ease-in-out infinite}.stackFlickStage{position:relative;width:min(70vw,300px);height:86px}.stackFlickBaseline{position:absolute;left:12%;right:12%;top:50%;height:1px;background:linear-gradient(90deg,transparent,#ffffff1c 20%,#ffffff38 50%,#ffffff1c 80%,transparent)}.stackFlickTrail{position:absolute;top:50%;width:82px;height:3px;border-radius:999px;opacity:0}.stackFlickTrail.right{left:50%;background:linear-gradient(90deg,#ffffff75,transparent);transform-origin:left;animation:stackTrailRight 3.2s ease-out infinite}.stackFlickTrail.left{right:50%;background:linear-gradient(270deg,#ffffff75,transparent);transform-origin:right;animation:stackTrailLeft 3.2s ease-out infinite}.stackFinger{position:absolute;left:50%;top:50%;width:20px;height:28px;margin:-14px -10px;border:2px solid #ffffffd8;border-radius:11px 11px 13px 13px;background:#ffffff12;box-shadow:0 0 18px var(--glow);opacity:0}.stackFinger:before{content:"";position:absolute;left:5px;top:3px;width:6px;height:8px;border-radius:50%;background:#ffffffb5}.stackFinger.right{animation:stackFlickRight 3.2s cubic-bezier(.2,.8,.25,1) infinite}.stackFinger.left{animation:stackFlickLeft 3.2s cubic-bezier(.2,.8,.25,1) infinite}.stackDropStage{position:relative;width:90px;height:128px}.stackDropRail{position:absolute;left:50%;top:12%;bottom:15%;width:2px;transform:translateX(-50%);background:linear-gradient(180deg,#ffffff65,transparent);opacity:.5}.stackDropFinger{position:absolute;left:50%;top:16%;width:20px;height:28px;margin:-14px -10px;border:2px solid #ffffffd8;border-radius:11px 11px 13px 13px;background:#ffffff12;box-shadow:0 0 18px var(--glow);animation:stackFlickDown 2.2s cubic-bezier(.2,.8,.25,1) infinite}.stackDropFinger:before{content:"";position:absolute;left:5px;top:3px;width:6px;height:8px;border-radius:50%;background:#ffffffb5}.stackDropChevron{position:absolute;left:50%;bottom:5%;width:12px;height:12px;border-right:2px solid #ffffffaa;border-bottom:2px solid #ffffffaa;transform:translateX(-50%) rotate(45deg)}.stackClearCallout{position:absolute;z-index:7;left:50%;top:50%;transform:translate(-50%,-50%);max-width:90vw;font-size:clamp(24px,8vmin,52px);font-weight:1000;line-height:1.12;letter-spacing:.1em;text-align:center;white-space:pre-line;color:#fff;text-shadow:0 0 28px var(--glow),0 0 10px #fff;pointer-events:none;animation:stackClearPop .36s ease-out both}.stackClearCallout.levelBreak{z-index:12;min-width:min(86vw,380px);padding:22px 18px;border:1px solid #ffffff3d;border-radius:22px;background:#070b15e8;box-shadow:0 20px 48px #000c,0 0 35px var(--glow);backdrop-filter:blur(10px);animation:stackLevelScanPulse .7s ease-in-out infinite}.stackClearCallout.perfectClear{z-index:13;min-width:min(90vw,410px);padding:24px 18px;border:2px solid #ffd54a;border-radius:24px;background:#09070eea;box-shadow:0 22px 52px #000d,0 0 48px #ffd54aaa;color:#fff4a8;animation:stackPerfectClearCall 2.2s ease-out both}@keyframes stackLevelScanPulse{0%,100%{transform:translate(-50%,-50%) scale(.98);box-shadow:0 20px 48px #000c,0 0 24px var(--glow)}50%{transform:translate(-50%,-50%) scale(1.02);box-shadow:0 20px 48px #000c,0 0 42px var(--glow)}}@keyframes stackPerfectClearCall{0%{transform:translate(-50%,-50%) scale(.55);opacity:0}18%{transform:translate(-50%,-50%) scale(1.08);opacity:1}78%{transform:translate(-50%,-50%) scale(1);opacity:1}100%{transform:translate(-50%,-50%) scale(.94);opacity:0}}.stackPausedOverlay{position:absolute;z-index:8;inset:0;display:flex;flex-direction:column;align-items:center;justify-content:center;gap:10px;background:#050811d6;border:0;color:#fff;pointer-events:auto;touch-action:manipulation}.stackPausedOverlay:active{background:#0b1120e8}.stackPausedOverlay span{font-size:clamp(72px,24vmin,140px);font-weight:1000;color:var(--accent);text-shadow:0 0 28px var(--glow)}.stackPausedOverlay strong{font-size:clamp(18px,6vmin,32px);letter-spacing:.13em}@keyframes stackTutorialTapPulse{0%,20%{transform:translate(-50%,-50%) scale(.55);opacity:0}38%{opacity:.8}70%,100%{transform:translate(-50%,-50%) scale(1.25);opacity:0}}@keyframes stackTutorialTapDot{0%,18%,100%{transform:translate(-50%,-50%) scale(.72);opacity:.42}34%,58%{transform:translate(-50%,-50%) scale(1);opacity:1}}@keyframes stackTutorialRotate{0%,18%{transform:translate(-50%,-50%) rotate(-80deg);opacity:0}34%{opacity:.8}72%,100%{transform:translate(-50%,-50%) rotate(190deg);opacity:0}}@keyframes stackFlickRight{0%,8%{transform:translateX(0);opacity:0}11%{opacity:1}21%{transform:translateX(94px);opacity:1}24%,100%{transform:translateX(104px);opacity:0}}@keyframes stackTrailRight{0%,10%{transform:scaleX(0);opacity:0}13%{opacity:.75}22%{transform:scaleX(1);opacity:.5}25%,100%{opacity:0}}@keyframes stackFlickLeft{0%,48%{transform:translateX(0);opacity:0}51%{opacity:1}61%{transform:translateX(-94px);opacity:1}64%,100%{transform:translateX(-104px);opacity:0}}@keyframes stackTrailLeft{0%,50%{transform:scaleX(0);opacity:0}53%{opacity:.75}62%{transform:scaleX(1);opacity:.5}65%,100%{opacity:0}}@keyframes stackFlickDown{0%,12%{top:16%;opacity:0}16%{opacity:1}35%{top:78%;opacity:1}40%,100%{top:84%;opacity:0}}@keyframes stackClearPop{0%{transform:translate(-50%,-50%) scale(.55);opacity:0}45%{transform:translate(-50%,-50%) scale(1.08);opacity:1}100%{transform:translate(-50%,-50%) scale(1);opacity:.9}}
/* Portrait is the safe default. Landscape layout is enabled by measured viewport class, not stale orientation media queries. */
body:not(.viewport-landscape) .menu-active .grid{grid-template-columns:repeat(2,minmax(0,1fr));grid-template-rows:repeat(2,minmax(0,1fr))}
body.viewport-landscape .menu-active .grid{grid-template-columns:repeat(4,minmax(0,1fr));grid-template-rows:1fr}
body.viewport-landscape #lobby{grid-template-columns:minmax(0,1.5fr) minmax(180px,.7fr);grid-template-rows:auto 1fr;column-gap:10px}
body.viewport-landscape #lobby .lobbyHead{grid-column:1}
body.viewport-landscape #lobby .players{grid-column:1;grid-row:2;grid-template-columns:repeat(2,minmax(0,1fr))}
body.viewport-landscape #lobby .lobbyActions{grid-column:2;grid-row:1/3;grid-template-columns:1fr;align-content:center}
body:not(.viewport-landscape) #games[data-arena="matrix_8x32"] .grid{grid-template-columns:repeat(2,minmax(0,1fr));grid-template-rows:repeat(3,minmax(0,1fr))}
body:not(.viewport-landscape) #games[data-arena="strip_1d"] .grid{grid-template-columns:repeat(2,minmax(0,1fr));grid-template-rows:1fr}
body.viewport-landscape #games[data-arena="matrix_8x32"] .grid{grid-template-columns:repeat(6,minmax(0,1fr));grid-template-rows:1fr}
body:not(.viewport-landscape) #games[data-arena="screen_arcade"] .grid{grid-template-columns:repeat(2,minmax(0,1fr));grid-template-rows:1fr}
body.viewport-landscape #games[data-arena="matrix_8x32"] .grid{grid-template-columns:repeat(5,minmax(0,1fr));grid-template-rows:1fr}
body.viewport-landscape #games[data-arena="strip_1d"] .grid{grid-template-columns:repeat(2,minmax(0,1fr));grid-template-rows:1fr}
body.viewport-landscape #games[data-arena="screen_arcade"] .grid{grid-template-columns:repeat(2,minmax(0,360px));grid-template-rows:1fr;justify-content:center}
.tapClashSurface{position:absolute;inset:env(safe-area-inset-top) env(safe-area-inset-right) env(safe-area-inset-bottom) env(safe-area-inset-left);display:grid;grid-template-rows:auto minmax(0,1fr);gap:10px;padding:12px;background:radial-gradient(circle at 50% 35%,#18213d,var(--dark) 68%);touch-action:none;overflow:hidden}.tcHud{display:grid;grid-template-columns:auto minmax(0,1fr);align-items:center;gap:10px}.tcTimer{font-size:clamp(24px,8vw,42px);font-weight:1000;font-variant-numeric:tabular-nums}.tcScores{display:flex;justify-content:flex-end;gap:6px;min-width:0}.tcScore{min-width:0;padding:7px 9px;border:1px solid #ffffff22;border-radius:12px;background:#070a12bb;font-size:clamp(10px,3vw,14px);font-weight:900;white-space:nowrap}.tcGrid{width:min(92vw,calc(var(--viewport-height,100dvh) - 105px),520px);height:min(92vw,calc(var(--viewport-height,100dvh) - 105px),520px);margin:auto;display:grid;grid-template-columns:repeat(3,minmax(0,1fr));grid-template-rows:repeat(3,minmax(0,1fr));gap:9px}.tcCell{border:1px solid #39415c;border-radius:18px;background:linear-gradient(145deg,#151b2b,#0b0f1a);box-shadow:inset 0 0 0 1px #ffffff08;touch-action:none}.tcCell.target{border-color:#fff7b0;background:radial-gradient(circle,#fff 0 18%,#ffe95b 20% 47%,#b47a00 68%,#211500 100%);box-shadow:0 0 34px #ffe95bbb,inset 0 0 22px #fff;animation:tcPulse .48s ease-in-out infinite alternate}.tcCell.flash{filter:brightness(1.5)}.tcCell:disabled{opacity:.72}.tcLock{position:absolute;inset:0;z-index:5;display:grid;place-items:center;background:#05070dcc;font-size:clamp(32px,12vw,72px);font-weight:1000;color:#ffbdca;text-align:center;pointer-events:none}@keyframes tcPulse{from{transform:scale(.96)}to{transform:scale(1)}}body.viewport-landscape .tapClashSurface{grid-template-columns:minmax(150px,.55fr) minmax(0,1fr);grid-template-rows:1fr;align-items:center}.viewport-landscape .tcHud{display:flex;flex-direction:column;align-items:stretch}.viewport-landscape .tcScores{flex-direction:column}.viewport-landscape .tcGrid{width:min(72vh,560px);height:min(72vh,560px)}
.brainDuelSurface{position:absolute;inset:env(safe-area-inset-top) env(safe-area-inset-right) env(safe-area-inset-bottom) env(safe-area-inset-left);display:grid;grid-template-rows:auto auto minmax(0,1fr) auto;gap:10px;padding:12px;background:radial-gradient(circle at 50% 25%,#25183d,var(--dark) 70%);touch-action:none;overflow:hidden}.bdHud{display:grid;grid-template-columns:auto 1fr auto;align-items:center;gap:8px}.bdProgress,.bdDifficulty,.bdTimer{padding:7px 9px;border-radius:10px;background:#080b14cc;border:1px solid #ffffff22;font-weight:1000;font-size:clamp(10px,3vw,14px)}.bdDifficulty{text-align:center;white-space:normal}.bdTimer{font-size:clamp(20px,7vw,34px);font-variant-numeric:tabular-nums;text-align:right}.bdQuestion{display:grid;place-items:center;min-height:72px;padding:10px 14px;border-radius:16px;background:#080b14bb;border:1px solid #ffffff22;text-align:center;font-size:clamp(24px,8vw,48px);font-weight:1000;line-height:1.08}.bdAnswers{min-height:0;display:grid;grid-template-columns:repeat(2,minmax(0,1fr));grid-template-rows:repeat(2,minmax(0,1fr));gap:10px}.bdAnswer{min-width:0;min-height:0;border:1px solid #3b4260;border-radius:18px;background:linear-gradient(145deg,#1b2032,#0b0f1a);font-size:clamp(21px,7vw,42px);font-weight:1000;padding:10px;touch-action:none;overflow-wrap:anywhere}.bdAnswer.flash{filter:brightness(1.35)}.bdAnswer.selected{border-color:var(--soft);box-shadow:0 0 0 2px var(--accent)}.bdAnswer.correct{border-color:#73ffb4;background:linear-gradient(145deg,#167044,#092818);box-shadow:0 0 24px #32e88788}.bdAnswer.wrong{border-color:#ff7795;background:linear-gradient(145deg,#72263b,#2c0a14)}.bdAnswer:disabled{opacity:.76}.bdStatus{min-height:32px;text-align:center;font-size:clamp(14px,4vw,21px);font-weight:1000;color:var(--soft)}body.viewport-landscape .brainDuelSurface{grid-template-columns:minmax(220px,.8fr) minmax(0,1.2fr);grid-template-rows:auto 1fr auto}.viewport-landscape .bdHud{grid-column:1}.viewport-landscape .bdQuestion{grid-column:1;grid-row:2}.viewport-landscape .bdAnswers{grid-column:2;grid-row:1/4}.viewport-landscape .bdStatus{grid-column:1;grid-row:3}
</style></head><body class="menu-active" data-view="platform"><main class="app"><div class="top"><div class="brand">LED ARCADE</div><div class="identity"><span id="dot" class="dot hidden"></span><span id="identity">GUEST</span></div><div class="lang"><button id="trBtn">TR</button><button id="enBtn">EN</button></div><div id="status" class="status">…</div></div>
<section id="platform"><h1 data-i="choosePlatform"></h1><div class="grid"><button id="oneDBtn" class="card"><canvas id="p1" class="preview" width="64" height="24"></canvas><strong>1D ARENA</strong><small data-i="oneDDesc"></small></button><button id="matrixBtn" class="card"><canvas id="pm" class="preview" width="64" height="24"></canvas><strong>8×32 ARENA</strong><small data-i="matrixDesc"></small></button><button id="screenBtn" class="card"><canvas id="ps" class="preview" width="64" height="24"></canvas><strong>SCREEN ARCADE</strong><small data-i="screenDesc"></small></button></div></section>
<section id="games" class="hidden"><div class="menuTitle"><h1 data-i="chooseGame"></h1></div><button id="platformsBtn" class="btn menuBack" type="button" data-i="backPlatforms"></button><div class="grid"><button id="rallyCard" class="gameCard hidden" type="button"><canvas id="rallyPreview" class="preview" width="64" height="24"></canvas><strong>REFLEX RALLY</strong><small data-i="rallyDesc"></small><div class="gameMeta"><span class="badge">MVP</span><span class="playBtn" data-i="play"></span></div></button><button id="pushCard" class="gameCard hidden" type="button"><canvas id="pushPreview" class="preview" width="64" height="24"></canvas><strong>POWER PUSH</strong><small data-i="pushDesc"></small><div class="gameMeta"><span class="badge">MVP</span><span class="playBtn" data-i="play"></span></div></button><button id="derbyCard" class="gameCard" type="button"><canvas id="derbyPreview" class="preview" width="64" height="24"></canvas><strong>PIXEL DERBY</strong><small data-i="derbyDesc"></small><div class="gameMeta"><span class="badge">PLAYABLE</span><span class="playBtn" data-i="play"></span></div></button><button id="tronCard" class="gameCard" type="button"><canvas id="tronPreview" class="preview" width="64" height="24"></canvas><strong>TRON ARENA</strong><small data-i="tronDesc"></small><div class="gameMeta"><span class="badge">BETA</span><span class="playBtn" data-i="play"></span></div></button><button id="raiderCard" class="gameCard" type="button"><canvas id="raiderPreview" class="preview" width="64" height="24"></canvas><strong>PIXEL RAIDER</strong><small data-i="raiderDesc"></small><div class="gameMeta"><span class="badge">BETA</span><span class="playBtn" data-i="play"></span></div></button><button id="clashCard" class="gameCard" type="button"><canvas id="clashPreview" class="preview" width="64" height="24"></canvas><strong>COLOR CLASH</strong><small data-i="clashDesc"></small><div class="gameMeta"><span class="badge">BETA</span><span class="playBtn" data-i="play"></span></div></button><button id="pongCard" class="gameCard" type="button"><canvas id="pongPreview" class="preview" width="64" height="24"></canvas><strong>PIXEL PONG</strong><small data-i="pongDesc"></small><div class="gameMeta"><span class="badge">MVP</span><span class="playBtn" data-i="play"></span></div></button><button id="tapClashCard" class="gameCard hidden" type="button"><canvas id="tapClashPreview" class="preview" width="64" height="24"></canvas><strong>TAP CLASH</strong><small data-i="tapClashDesc"></small><div class="gameMeta"><span class="badge">MVP</span><span class="playBtn" data-i="play"></span></div></button><button id="brainDuelCard" class="gameCard hidden" type="button"><canvas id="brainDuelPreview" class="preview" width="64" height="24"></canvas><strong>BRAIN DUEL</strong><small data-i="brainDuelDesc"></small><div class="gameMeta"><span class="badge">MVP</span><span class="playBtn" data-i="play"></span></div></button></div></section>
<section id="lobby" class="hidden"><div class="lobbyHead"><h1 id="gameTitle"></h1><div id="instructions" class="hint"></div><p id="roleText"></p></div><div id="players" class="players"></div><div class="lobbyActions"><button id="readyBtn" class="btn primary" data-i="ready"></button><button id="waitBtn" class="btn" data-i="break"></button><button id="gamesBtn" class="btn" data-i="backGames"></button></div></section>
<section id="announce" class="hidden result"><div id="announceText" class="announce">BOSS</div></section><section id="countdown" class="hidden result"><div id="countText" class="announce">3</div></section>
<section id="race" class="hidden full"><div id="tapSurface" class="tapSurface"></div><div id="tronSurface" class="tronSurface hidden"><button id="leftTurn" class="turn">↶</button><button id="rightTurn" class="turn">↷</button></div><div id="raiderSurface" class="raiderSurface hidden"><button id="moveUp" class="raiderMove">↑</button><button id="moveDown" class="raiderMove">↓</button></div><div id="clashSurface" class="clashSurface hidden"><button id="clashUp" class="clashMove">↑</button><button id="clashLeft" class="clashMove">←</button><button id="clashRight" class="clashMove">→</button><button id="clashDown" class="clashMove">↓</button></div><div id="pongSurface" class="raiderSurface hidden"><button id="pongUp" class="raiderMove">↑</button><button id="pongDown" class="raiderMove">↓</button></div><div id="tapClashSurface" class="tapClashSurface hidden"><div class="tcHud"><div id="tcTimer" class="tcTimer">30.0</div><div id="tcScores" class="tcScores"></div></div><div id="tcGrid" class="tcGrid"><button class="tcCell" data-cell="0"></button><button class="tcCell" data-cell="1"></button><button class="tcCell" data-cell="2"></button><button class="tcCell" data-cell="3"></button><button class="tcCell" data-cell="4"></button><button class="tcCell" data-cell="5"></button><button class="tcCell" data-cell="6"></button><button class="tcCell" data-cell="7"></button><button class="tcCell" data-cell="8"></button></div><div id="tcLock" class="tcLock hidden"></div></div><div id="brainDuelSurface" class="brainDuelSurface hidden"><div class="bdHud"><div id="bdProgress" class="bdProgress">1/10</div><div id="bdDifficulty" class="bdDifficulty"></div><div id="bdTimer" class="bdTimer">8.0</div></div><div id="bdQuestion" class="bdQuestion"></div><div id="bdAnswers" class="bdAnswers"><button id="bdAnswer0" class="bdAnswer" data-answer="0"></button><button id="bdAnswer1" class="bdAnswer" data-answer="1"></button><button id="bdAnswer2" class="bdAnswer" data-answer="2"></button><button id="bdAnswer3" class="bdAnswer" data-answer="3"></button></div><div id="bdStatus" class="bdStatus"></div></div></section>
<section id="boss" class="hidden full"><div id="bossSurface" class="tapSurface"></div></section>
<section id="result" class="hidden result"><h1 id="winnerText"></h1><p id="resultText"></p><button id="rematchBtn" class="btn primary" data-i="continue"></button><button id="resultGamesBtn" class="btn" data-i="backGames"></button><button id="resultBreakBtn" class="btn" data-i="break"></button></section>
<section id="boss_result" class="hidden result"><h1 id="bossResultText"></h1><p id="bossResultScore"></p><button id="bossContinueBtn" class="btn primary" data-i="continue"></button></section>
<div id="joinBox" class="hidden"><button id="joinBtn" class="btn primary" data-i="join"></button></div><div class="footer">10.10.10.10 · play · oyna</div></main><script>
document.querySelector('#games .grid').insertAdjacentHTML('beforeend','<button id="stackCard" class="gameCard" type="button"><canvas id="stackPreview" class="preview" width="64" height="24"></canvas><strong>STACK SHIFT</strong><small data-i="stackDesc"></small><div class="gameMeta"><span class="badge">MVP</span><span class="playBtn" data-i="play"></span></div></button>');
document.querySelector('#race').insertAdjacentHTML('beforeend','<div id="stackSurface" class="stackSurface hidden"><div class="stackTop"><button id="stackPauseBtn" class="stackPauseBtn" type="button" data-gesture-ignore>Ⅱ PAUSE</button><button id="stackHelpBtn" class="stackHelpBtn" type="button" data-gesture-ignore aria-label="Help">?</button><div id="stackHud" class="stackHud"></div></div><div class="stackNext"><span id="stackNextLabel">NEXT</span><canvas id="stackNextPreview" width="160" height="160"></canvas></div><div id="stackGuide" class="stackTutorial hidden"><div class="stackTutorialHead"><strong id="stackTutorialHeading">CONTROLS</strong><div class="stackTutorialProgress"><span class="stackTutorialDot" data-stack-step="rotate"></span><span class="stackTutorialDot" data-stack-step="move"></span><span class="stackTutorialDot" data-stack-step="drop"></span></div><button id="stackTutorialClose" class="stackTutorialClose" type="button" data-gesture-ignore aria-label="Close">×</button></div><div id="stackTutorialRotate" class="stackTutorialStep"><div class="stackTapStage"><span class="stackTapRing"></span><span class="stackTapDot"></span><span class="stackRotateArc"></span></div><strong id="stackTapText" class="stackTutorialTitle">TAP · ROTATE</strong></div><div id="stackTutorialMove" class="stackTutorialStep"><div class="stackFlickStage"><span class="stackFlickBaseline"></span><span class="stackFlickTrail right"></span><span class="stackFlickTrail left"></span><span class="stackFinger right"></span><span class="stackFinger left"></span></div><strong id="stackSwipeText" class="stackTutorialTitle">SWIPE LEFT / RIGHT</strong></div><div id="stackTutorialDrop" class="stackTutorialStep"><div class="stackDropStage"><span class="stackDropRail"></span><span class="stackDropFinger"></span><span class="stackDropChevron"></span></div><strong id="stackDropText" class="stackTutorialTitle">SWIPE DOWN · DROP</strong><small id="stackHoldText" class="stackTutorialHint">HOLD · FAST FALL</small></div><small id="stackTutorialPrompt" class="stackTutorialPrompt">TRY THE GESTURE TO CONTINUE</small></div><div id="stackClearCallout" class="stackClearCallout hidden"></div><button id="stackPauseOverlay" class="stackPausedOverlay hidden" type="button" data-gesture-ignore><span>▶</span><strong id="stackPauseText">RESUME</strong></button></div>');
const $=id=>document.getElementById(id),views=["platform","games","lobby","announce","countdown","race","result","boss","boss_result"];
const palettes=[['#247cff','#06152f','#8ac0ff'],['#ff2a92','#30071f','#ff9dcc'],['#24c970','#062a18','#8cf0b9'],['#ffd52f','#332800','#fff09c'],['#27d5e8','#052a2f','#9af4ff'],['#ff8731','#321707','#ffc090'],['#a665ff','#1f0a36','#d5b2ff'],['#f2f4ff','#171922','#ffffff']];
const colorNames={tr:['MAVİ','PEMBE','YEŞİL','SARI','TURKUAZ','TURUNCU','MOR','BEYAZ'],en:['BLUE','PINK','GREEN','YELLOW','CYAN','ORANGE','PURPLE','WHITE']};
const T={tr:{choosePlatform:'Platform seç',soon:'Yakında',oneDDesc:'1–2 oyuncu · refleks ve güç oyunları',matrixDesc:'Yarış ve arena oyunları',screenDesc:'2–3 oyuncu · ekran ve kontrol aynı cihazda',chooseGame:'Oyun seç',play:'OYNA',derbyDesc:'2–8 oyuncu · yarış ve King Boss',tronDesc:'2–4 oyuncu · dön, iz bırak, hayatta kal',raiderDesc:'1 oyuncu · kaç, ateş et, güçlen',clashDesc:'2–4 oyuncu · alanı kendi rengine boya',pongDesc:'1–2 oyuncu · paddle’ı yönet, ilk 5 sayıyı al',rallyDesc:'1–2 oyuncu · top bölgenize gelince vur',pushDesc:'1–2 oyuncu · hızlı dokun, çizgiyi it',tapClashDesc:'3 oyuncu · hedefe ilk dokunan puanı alır',brainDuelDesc:'2 oyuncu · matematik ve kelime düellosu',rallyHelp:'Top kendi renk bölgenize girince dokun. Kaçırırsanız rakip kazanır.',pushHelp:'Hızlı dokunarak sınırı rakibin tarafına it. Süre sonunda üstün taraf kazanır.',tapClashHelp:'Parlayan kareye ilk dokunan puan alır. Yanlış kare 0,65 saniye kilitler.',brainDuelHelp:'10 soruda doğru cevabı seç. Sorular kolay, orta ve zor arasında değişir.',ready:'HAZIRIM',start:'OYUNU BAŞLAT',break:'MOLA',returnGame:'OYUNA DÖN',leave:'OYUNDAN AYRIL',backGames:'OYUNLARA DÖN',backPlatforms:'PLATFORMLARA DÖN',continue:'DEVAM ET',join:'OYUNA KATIL',guest:'İZLEYİCİ',connected:'Bağlandı',reconnecting:'Yeniden bağlanıyor…',derbyHelp:'Ekranın herhangi bir yerine dokun. İlk bitiren kazanır.',tronHelp:'Sol yarı sola, sağ yarı sağa döndürür. Duvara veya izlere çarpma.',raiderHelp:'Üst yarı yukarı, alt yarı aşağı. Engellerden kaç; ateş otomatik.',clashHelp:'Yön seç ve hareket et. Süre bitince en çok alanı boyayan kazanır.',pongHelp:'Üst yarı paddle’ı yukarı, alt yarı aşağı taşır. İlk 5 sayıya ulaşan kazanır.',readyState:'HAZIR',waiting:'BEKLİYOR',onBreak:'MOLADA',disconnected:'KOPTU',won:'KAZANDIN!',winner:'KAZANAN',draw:'BERABERE',bossWon:'KING BOSS KAZANDI',teamWon:'TAKIM BOSS’U YENDİ',cpu:'CPU',you:'SEN',locked:'KİLİTLENDİN',score:'SKOR',answered:'CEVABIN ALINDI',correct:'DOĞRU!',wrong:'YANLIŞ',question:'SORU',easy:'KOLAY',normal:'ORTA',challenge:'ZOR',math:'MATEMATİK',word:'KELİME'},en:{choosePlatform:'Choose platform',soon:'Coming soon',oneDDesc:'1–2 players · reflex and power games',matrixDesc:'Racing and arena games',screenDesc:'2–3 players · display and controls on each device',chooseGame:'Choose game',play:'PLAY',derbyDesc:'2–8 players · race and King Boss',tronDesc:'2–4 players · turn, trail, survive',raiderDesc:'1 player · dodge, shoot, power up',clashDesc:'2–4 players · paint the largest area',pongDesc:'1–2 players · move the paddle, first to 5 wins',rallyDesc:'1–2 players · return the ball on time',pushDesc:'1–2 players · tap fast and push the line',tapClashDesc:'3 players · first tap on the target scores',brainDuelDesc:'2 players · math and word duel',rallyHelp:'Tap when the ball enters your colored zone. Missing gives the point away.',pushHelp:'Tap rapidly to push the boundary into the opponent side. Lead when time expires.',tapClashHelp:'Tap the glowing cell first. A wrong cell locks you for 0.65 seconds.',brainDuelHelp:'Choose the correct answer across 10 mixed easy, normal and challenge questions.',ready:'READY',start:'START GAME',break:'BREAK',returnGame:'RETURN TO GAME',leave:'LEAVE GAME',backGames:'BACK TO GAMES',backPlatforms:'BACK TO PLATFORMS',continue:'CONTINUE',join:'JOIN GAME',guest:'SPECTATOR',connected:'Connected',reconnecting:'Reconnecting…',derbyHelp:'Tap anywhere. First racer to the finish wins.',tronHelp:'Left half turns left, right half turns right. Avoid walls and trails.',raiderHelp:'Top half moves up, bottom half moves down. Shooting is automatic.',clashHelp:'Choose a direction and keep moving. Paint the largest area before time runs out.',pongHelp:'Top half moves the paddle up, bottom half moves it down. First to 5 points wins.',readyState:'READY',waiting:'WAITING',onBreak:'ON BREAK',disconnected:'OFFLINE',won:'YOU WON!',winner:'WINNER',draw:'DRAW',bossWon:'KING BOSS WON',teamWon:'TEAM DEFEATED THE BOSS',cpu:'CPU',you:'YOU',locked:'LOCKED',score:'SCORE',answered:'ANSWER LOCKED',correct:'CORRECT!',wrong:'WRONG',question:'QUESTION',easy:'EASY',normal:'NORMAL',challenge:'CHALLENGE',math:'MATH',word:'WORD'}};
Object.assign(T.tr,{stackDesc:'1 oyuncu · blokları döndür, satırları temizle',stackHelp:'Dokun: döndür. Sağa/sola kaydır: hareket. Aşağı kaydır: anında bırak. Basılı tut: hızlı indir; bırakınca normal hıza dön.',pause:'DURAKLAT',resume:'DEVAM',next:'SIRADAKİ',tutorial:'HAREKETLER',tutorialPrompt:'HAREKETİ YAPARAK DEVAM ET',closeTutorial:'KAPAT',tapRotate:'DOKUN · DÖNDÜR',swipeMove:'KISA KAYDIR · SAĞ / SOL',swipeDrop:'KISA KAYDIR · AŞAĞI BIRAK',holdFast:'BASILI TUT · HIZLI İNDİR, BIRAK · NORMAL',single:'TEKLİ',double:'İKİLİ',triple:'ÜÇLÜ',tetris:'TETRIS!',perfectClear:'KUSURSUZ TEMİZLİK!',emptyRows:'BOŞ SATIR',paused:'DURAKLATILDI'});
Object.assign(T.en,{stackDesc:'1 player · rotate blocks and clear lines',stackHelp:'Tap to rotate. Swipe left/right to move. Swipe down to hard drop. Hold to fall fast; release to return to normal speed.',pause:'PAUSE',resume:'RESUME',next:'NEXT',tutorial:'CONTROLS',tutorialPrompt:'PERFORM THE GESTURE TO CONTINUE',closeTutorial:'CLOSE',tapRotate:'TAP · ROTATE',swipeMove:'QUICK SWIPE · LEFT / RIGHT',swipeDrop:'QUICK SWIPE · DOWN TO DROP',holdFast:'HOLD · FAST FALL, RELEASE · NORMAL',single:'SINGLE',double:'DOUBLE',triple:'TRIPLE',tetris:'TETRIS!',perfectClear:'PERFECT CLEAR!',emptyRows:'EMPTY ROWS',paused:'PAUSED'});
let lang=localStorage.getItem('ledArcadeLang')||((navigator.language||'en').toLowerCase().startsWith('tr')?'tr':'en');let ws,state=null,me=-1,ready=false,rt,ht,lastInput=0,currentView='platform',gameActive=false,lastStage='',lastGame='',pointerDown=false,resultLockedUntil=0,resultNeedsRelease=false,resultUnlockTimer=0,audioCtx=null,audioMaster=null,audioReady=false,lastCountdownSound=-1,lastTcTargetSound=0,lastTcEventSound=0,lastBrainQuestionSound=0,lastBrainEventSound=0,stackTutorialActive=false,stackTutorialProgress={rotate:false,move:false,drop:false},cid=localStorage.getItem('ledArcadeCid');if(!cid){cid=crypto.randomUUID?crypto.randomUUID():Date.now()+'-'+Math.random();localStorage.setItem('ledArcadeCid',cid)}
function tr(k){return T[lang][k]||k}function applyLang(){document.documentElement.lang=lang;document.querySelectorAll('[data-i]').forEach(e=>e.textContent=tr(e.dataset.i));$('trBtn').classList.toggle('on',lang==='tr');$('enBtn').classList.toggle('on',lang==='en');render(state)}
function theme(slot){const p=slot>=0?palettes[slot]:['#6d7cff','#0a1020','#a6b6ff'];document.documentElement.style.setProperty('--accent',p[0]);document.documentElement.style.setProperty('--dark',p[1]);document.documentElement.style.setProperty('--soft',p[2]);document.documentElement.style.setProperty('--glow',p[0]+'88')}
function ensureAudio(){const C=window.AudioContext||window.webkitAudioContext;if(!C)return false;try{if(!audioCtx){try{audioCtx=new C({latencyHint:'interactive'})}catch(_){audioCtx=new C()}audioMaster=audioCtx.createGain();audioMaster.gain.value=.9;audioMaster.connect(audioCtx.destination)}return true}catch(_){audioCtx=null;audioMaster=null;return false}}function primeAudio(){if(!audioCtx)return;try{const b=audioCtx.createBuffer(1,1,22050),s=audioCtx.createBufferSource();s.buffer=b;s.connect(audioMaster||audioCtx.destination);s.start(0)}catch(_){}}function unlockAudio(){if(!ensureAudio())return Promise.resolve(false);primeAudio();let resumed;try{resumed=audioCtx.state==='suspended'?audioCtx.resume():Promise.resolve()}catch(err){resumed=Promise.reject(err)}return Promise.resolve(resumed).then(()=>{primeAudio();audioReady=audioCtx.state==='running';return audioReady}).catch(()=>false)}function tone(freq,duration=.08,delay=0,gain=.11,type='square'){if(!ensureAudio())return;try{if(audioCtx.state==='suspended')audioCtx.resume().catch(()=>{});const start=audioCtx.currentTime+delay,o=audioCtx.createOscillator(),v=audioCtx.createGain();o.type=type;o.frequency.setValueAtTime(freq,start);v.gain.setValueAtTime(.0001,start);v.gain.exponentialRampToValueAtTime(gain,start+.008);v.gain.exponentialRampToValueAtTime(.0001,start+duration);o.connect(v);v.connect(audioMaster||audioCtx.destination);o.start(start);o.stop(start+duration+.03)}catch(_){}}function audioGesture(test=false){const wasReady=audioReady&&audioCtx?.state==='running';const p=unlockAudio();if(test){if(audioCtx)tone(520,.07,0,.13);p.then(ok=>{if(ok&&!wasReady)tone(760,.08,.04,.12,'sine')})}return p}function playStateAudio(s,p,enteringResult){const screenGame=s.game==='tap_clash'||s.game==='brain_duel';if(!screenGame){lastCountdownSound=-1;lastTcTargetSound=0;lastTcEventSound=0;lastBrainQuestionSound=0;lastBrainEventSound=0;return}if(s.stage==='countdown'){if(s.countdown!==lastCountdownSound){lastCountdownSound=s.countdown;if(s.countdown>0)tone(420+s.countdown*45,.09);else{tone(720,.1);tone(980,.12,.09)}}}else lastCountdownSound=-1;if(s.game==='tap_clash'){if(s.stage==='race'&&s.tapClashTarget>=0&&s.tapClashTargetId!==lastTcTargetSound){lastTcTargetSound=s.tapClashTargetId;tone(640,.055,0,.028,'sine')}if(s.tapClashEventId&&s.tapClashEventId!==lastTcEventSound){lastTcEventSound=s.tapClashEventId;if(s.tapClashEventType===2){if(s.tapClashEventSlot===me){tone(780,.08);tone(1080,.1,.07)}else tone(250,.055,0,.025)}else if(s.tapClashEventType===3&&s.tapClashEventSlot===me){tone(120,.16,0,.05,'sawtooth')}}}if(s.game==='brain_duel'){if(s.stage==='race'&&s.brainQuestionId&&s.brainQuestionId!==lastBrainQuestionSound){lastBrainQuestionSound=s.brainQuestionId;tone(560,.07);tone(720,.07,.08)}if(s.brainEventId&&s.brainEventId!==lastBrainEventSound){lastBrainEventSound=s.brainEventId;if(s.brainEventType===2&&s.brainEventSlot===me){if(s.brainEventCorrect){tone(760,.08);tone(1040,.12,.08)}else tone(150,.18,0,.055,'sawtooth')}else if(s.brainEventType===3){tone(450,.06,0,.035,'sine')}}}if(enteringResult){if(s.winner<0){tone(420,.12);tone(420,.12,.14)}else if(s.winner===me){tone(600,.1);tone(800,.1,.1);tone(1080,.18,.2)}else{tone(330,.12);tone(220,.2,.12)}}}
let viewportSamples=0,viewportTimer;function syncViewport(){const vv=window.visualViewport;const w=Math.round(vv?.width||window.innerWidth),h=Math.round(vv?.height||window.innerHeight);document.documentElement.style.setProperty('--viewport-height',`${h}px`);viewportSamples++;clearTimeout(viewportTimer);viewportTimer=setTimeout(()=>{const v=window.visualViewport;const sw=Math.round(v?.width||window.innerWidth),sh=Math.round(v?.height||window.innerHeight);document.body.classList.toggle('viewport-landscape',sw>sh*1.12);if(gameActive)window.scrollTo(0,0)},viewportSamples<2?220:60);if(gameActive)window.scrollTo(0,0)}function show(v){const changed=v!==currentView;currentView=v;gameActive=v==='race'||v==='boss';const menuActive=v==='platform'||v==='games';views.forEach(x=>$(x).classList.toggle('hidden',x!==v));document.documentElement.classList.toggle('game-active',gameActive);document.body.classList.toggle('game-active',gameActive);document.body.classList.toggle('menu-active',menuActive);document.body.dataset.view=v;if(changed){if(gameActive){window.scrollTo(0,0);syncViewport()}else if(!menuActive)requestAnimationFrame(()=>window.scrollTo(0,0))}}function send(m){if(ws?.readyState===1)ws.send(m)}function mine(s){return s?.players?.find(p=>p.slot===me)}
function connect(){clearTimeout(rt);ws=new WebSocket("ws://10.10.10.10:81/");ws.onopen=()=>{$('status').textContent=tr('connected');send('HELLO|'+cid);clearInterval(ht);ht=setInterval(()=>send('PING'),5000)};ws.onclose=()=>{clearInterval(ht);$('status').textContent=tr('reconnecting');rt=setTimeout(connect,2000)};ws.onerror=()=>ws.close();ws.onmessage=e=>{try{render(JSON.parse(e.data))}catch(_){}}}
const STACK_PREVIEW_SHAPES=[[[0,1],[1,1],[2,1],[3,1]],[[1,0],[2,0],[1,1],[2,1]],[[1,0],[0,1],[1,1],[2,1]],[[1,0],[2,0],[0,1],[1,1]],[[0,0],[1,0],[1,1],[2,1]],[[0,0],[0,1],[1,1],[2,1]],[[2,0],[0,1],[1,1],[2,1]]];
const STACK_PREVIEW_COLORS=['#27d5e8','#ffd52f','#a665ff','#24c970','#ff405c','#247cff','#ff8731'];
function drawStackNext(piece){const cv=$('stackNextPreview'),c=cv.getContext('2d');c.clearRect(0,0,cv.width,cv.height);const shape=STACK_PREVIEW_SHAPES[piece]||STACK_PREVIEW_SHAPES[0],xs=shape.map(v=>v[0]),ys=shape.map(v=>v[1]),cols=Math.max(...xs)-Math.min(...xs)+1,rows=Math.max(...ys)-Math.min(...ys)+1,cell=Math.floor(Math.min((cv.width-24)/cols,(cv.height-24)/rows)),w=cols*cell,h=rows*cell,ox=Math.floor((cv.width-w)/2)-Math.min(...xs)*cell,oy=Math.floor((cv.height-h)/2)-Math.min(...ys)*cell;c.fillStyle=STACK_PREVIEW_COLORS[piece]||STACK_PREVIEW_COLORS[0];shape.forEach(([x,y])=>c.fillRect(ox+x*cell+2,oy+y*cell+2,cell-4,cell-4))}
const STACK_TUTORIAL_KEY='ledArcadeStackTutorialV2';
const STACK_TUTORIAL_STEPS=['rotate','move','drop'];
function stackTutorialWasCompleted(){try{return localStorage.getItem(STACK_TUTORIAL_KEY)==='1'}catch(_){return false}}
function rememberStackTutorial(){try{localStorage.setItem(STACK_TUTORIAL_KEY,'1')}catch(_){}}
function currentStackTutorialStep(){return STACK_TUTORIAL_STEPS.find(step=>!stackTutorialProgress[step])||''}
function renderStackTutorial(){const guide=$('stackGuide');if(!guide)return;guide.classList.toggle('hidden',!stackTutorialActive);const current=currentStackTutorialStep();[['rotate','stackTutorialRotate'],['move','stackTutorialMove'],['drop','stackTutorialDrop']].forEach(([step,id])=>$(id).classList.toggle('active',stackTutorialActive&&step===current));document.querySelectorAll('.stackTutorialDot').forEach(dot=>{const step=dot.dataset.stackStep;dot.classList.toggle('done',!!stackTutorialProgress[step]);dot.classList.toggle('active',stackTutorialActive&&step===current)})}
function openStackTutorial(force=false){if(!force&&stackTutorialWasCompleted())return;stackTutorialActive=true;stackTutorialProgress={rotate:false,move:false,drop:false};renderStackTutorial()}
function closeStackTutorial(remember=true){stackTutorialActive=false;if(remember)rememberStackTutorial();renderStackTutorial()}
function noteStackTutorial(action){if(!stackTutorialActive||!Object.prototype.hasOwnProperty.call(stackTutorialProgress,action))return;stackTutorialProgress[action]=true;if(STACK_TUTORIAL_STEPS.every(step=>stackTutorialProgress[step]))closeStackTutorial(true);else renderStackTutorial()}
function stackInput(message,action){if(input(message))noteStackTutorial(action)}
function renderStackController(s){
  const ghostCount=Number.isInteger(s.stackGhostPieces)?s.stackGhostPieces:0;
  $('stackHud').textContent=`${lang==='tr'?'SKOR':'SCORE'} ${s.stackScore} · ${lang==='tr'?'SATIR':'LINES'} ${s.stackLines} · LV ${s.stackLevel} · ${lang==='tr'?'GÖLGE':'GHOST'} ${ghostCount}`;
  $('stackPauseBtn').textContent=s.stackPaused?`▶ ${tr('resume')}`:`Ⅱ ${tr('pause')}`;
  $('stackPauseBtn').setAttribute('aria-pressed',s.stackPaused?'true':'false');
  $('stackHelpBtn').setAttribute('aria-label',tr('tutorial'));
  $('stackHelpBtn').setAttribute('title',tr('tutorial'));
  $('stackNextLabel').textContent=tr('next');
  $('stackTutorialHeading').textContent=tr('tutorial');
  $('stackTutorialPrompt').textContent=tr('tutorialPrompt');
  $('stackTutorialClose').setAttribute('aria-label',tr('closeTutorial'));
  $('stackTutorialClose').setAttribute('title',tr('closeTutorial'));
  $('stackTapText').textContent=tr('tapRotate');
  $('stackSwipeText').textContent=tr('swipeMove');
  $('stackDropText').textContent=tr('swipeDrop');
  $('stackHoldText').textContent=tr('holdFast');
  $('stackPauseText').textContent=tr('resume');
  $('stackPauseOverlay').classList.toggle('hidden',!s.stackPaused);

  const clearCount=s.stackClearActive?s.stackClearCount:0;
  const clearName=clearCount>=4?tr('tetris'):clearCount===3?tr('triple'):clearCount===2?tr('double'):clearCount===1?tr('single'):'';
  const ghostAward=s.stackClearActive&&s.stackGhostAward>0?`
${lang==='tr'?'GÖLGE':'GHOST'} +${s.stackGhostAward}`:'';
  const scannedRows=Number.isInteger(s.stackLevelScannedRows)?s.stackLevelScannedRows:0;
  const scannedBonus=Number.isInteger(s.stackLevelScannedBonus)?s.stackLevelScannedBonus:0;
  const totalRows=Number.isInteger(s.stackLevelEmptyRows)?s.stackLevelEmptyRows:0;
  const levelCall=s.stackLevelBreak?(s.stackLevelIntro?`↑
${lang==='tr'?'SEVİYE':'LEVEL'} ${s.stackLevel}`:`${lang==='tr'?'SEVİYE':'LEVEL'} ${s.stackLevel}
${tr('emptyRows')} ${scannedRows}/${totalRows}
+${scannedBonus}`):'';
  const perfectGhost=s.stackPerfectClearGhostAward>0?`
${lang==='tr'?'GÖLGE':'GHOST'} +${s.stackPerfectClearGhostAward}`:'';
  const perfectCall=s.stackPerfectClear?`${tr('perfectClear')}
+${s.stackPerfectClearBonus}${perfectGhost}`:'';
  const clearText=perfectCall||levelCall||(clearName+ghostAward);
  $('stackClearCallout').textContent=clearText;
  $('stackClearCallout').classList.toggle('levelBreak',!!s.stackLevelBreak);
  $('stackClearCallout').classList.toggle('perfectClear',!!s.stackPerfectClear);
  $('stackClearCallout').classList.toggle('hidden',!clearText);
  drawStackNext(Number.isInteger(s.stackNextPiece)?s.stackNextPiece:0);
  renderStackTutorial();
}

const GESTURE_MOVE_THRESHOLD_PX=10;
const GESTURE_SWIPE_THRESHOLD_PX=34;
const GESTURE_SWIPE_MAX_MS=420;
const GESTURE_AXIS_RATIO=1.35;
const GESTURE_HOLD_MS=420;
const PONG_DRAG_STEP_PX=26;
const PONG_DRAG_SEND_MS=75;
const STACK_DRAG_STEP_PX=24;
const STACK_SOFT_DROP_SEND_MS=90;

function bindGestureSurface(el,handlers={}){
  if(!el)return;

  el.classList.add('gestureSurface');

  let pointerId=null;
  let startX=0;
  let startY=0;
  let lastX=0;
  let lastY=0;
  let startedAt=0;
  let moved=false;
  let holdActive=false;
  let holdTimer=0;

  const clearHold=()=>{
    if(holdTimer){
      clearTimeout(holdTimer);
      holdTimer=0;
    }
  };

  el.addEventListener('pointerdown',e=>{
    if(pointerId!==null||e.target.closest?.('[data-gesture-ignore]'))return;

    e.preventDefault();

    pointerId=e.pointerId;
    startX=lastX=e.clientX;
    startY=lastY=e.clientY;
    startedAt=performance.now();
    moved=false;
    holdActive=false;

    el.setPointerCapture?.(pointerId);

    if(handlers.holdStart){
      const holdDelay=handlers.holdMs??GESTURE_HOLD_MS;
      holdTimer=setTimeout(()=>{
        if(pointerId!==null&&!moved){
          holdActive=true;
          handlers.holdStart();
        }
      },holdDelay);
    }
  },{passive:false});

  el.addEventListener('pointermove',e=>{
    if(e.pointerId!==pointerId)return;

    e.preventDefault();

    const dx=e.clientX-startX;
    const dy=e.clientY-startY;
    const stepX=e.clientX-lastX;
    const stepY=e.clientY-lastY;

    if(!moved&&Math.hypot(dx,dy)>=GESTURE_MOVE_THRESHOLD_PX){
      moved=true;
      clearHold();
      el.classList.add('dragging');
      handlers.dragStart?.({
        x:e.clientX,
        y:e.clientY,
        duration:performance.now()-startedAt
      });
    }

    if(moved){
      handlers.drag?.({
        dx,
        dy,
        stepX,
        stepY,
        x:e.clientX,
        y:e.clientY,
        duration:performance.now()-startedAt
      });
    }

    lastX=e.clientX;
    lastY=e.clientY;
  },{passive:false});

  const finish=(e,cancelled)=>{
    if(e.pointerId!==pointerId)return;

    e.preventDefault();
    clearHold();

    const dx=e.clientX-startX;
    const dy=e.clientY-startY;
    const duration=performance.now()-startedAt;

    if(!cancelled){
      if(moved){
        const ax=Math.abs(dx);
        const ay=Math.abs(dy);

        if(
          duration<=GESTURE_SWIPE_MAX_MS &&
          Math.max(ax,ay)>=GESTURE_SWIPE_THRESHOLD_PX
        ){
          if(ax>ay*GESTURE_AXIS_RATIO){
            handlers.swipe?.(dx<0?'left':'right');
          }else if(ay>ax*GESTURE_AXIS_RATIO){
            handlers.swipe?.(dy<0?'up':'down');
          }
        }

        handlers.dragEnd?.({
          dx,
          dy,
          duration
        });
      }else if(!holdActive){
        const rect=el.getBoundingClientRect();

        handlers.tap?.({
          xRatio:(e.clientX-rect.left)/Math.max(1,rect.width),
          yRatio:(e.clientY-rect.top)/Math.max(1,rect.height)
        });
      }
    }else{
      handlers.cancel?.();
    }

    if(holdActive){
      handlers.holdEnd?.();
    }

    el.classList.remove('dragging');

    try{
      el.releasePointerCapture?.(pointerId);
    }catch(_){}

    pointerId=null;
    moved=false;
    holdActive=false;
  };

  el.addEventListener(
    'pointerup',
    e=>finish(e,false),
    {passive:false}
  );

  el.addEventListener(
    'pointercancel',
    e=>finish(e,true),
    {passive:false}
  );
}

function directionalInput(direction,map){
  const entry=map[direction];

  if(entry){
    input(entry[0],$(entry[1]));
  }
}

function bindGameGestures(){
  bindGestureSurface(
    $('tronSurface'),
    {
      tap:({xRatio})=>directionalInput(
        xRatio<.5?'left':'right',
        {
          left:['TURN_LEFT','leftTurn'],
          right:['TURN_RIGHT','rightTurn']
        }
      ),
      swipe:direction=>directionalInput(
        direction,
        {
          left:['TURN_LEFT','leftTurn'],
          right:['TURN_RIGHT','rightTurn']
        }
      )
    }
  );

  bindGestureSurface(
    $('raiderSurface'),
    {
      tap:({yRatio})=>directionalInput(
        yRatio<.5?'up':'down',
        {
          up:['MOVE_UP','moveUp'],
          down:['MOVE_DOWN','moveDown']
        }
      ),
      swipe:direction=>directionalInput(
        direction,
        {
          up:['MOVE_UP','moveUp'],
          down:['MOVE_DOWN','moveDown']
        }
      )
    }
  );

  bindGestureSurface(
    $('clashSurface'),
    {
      tap:({xRatio,yRatio})=>{
        const direction=
          yRatio<.3?'up':
          yRatio>.7?'down':
          xRatio<.5?'left':
          'right';

        directionalInput(
          direction,
          {
            up:['CLASH_UP','clashUp'],
            right:['CLASH_RIGHT','clashRight'],
            down:['CLASH_DOWN','clashDown'],
            left:['CLASH_LEFT','clashLeft']
          }
        );
      },
      swipe:direction=>directionalInput(
        direction,
        {
          up:['CLASH_UP','clashUp'],
          right:['CLASH_RIGHT','clashRight'],
          down:['CLASH_DOWN','clashDown'],
          left:['CLASH_LEFT','clashLeft']
        }
      )
    }
  );

  let pongAccumY=0;
  let pongLastSent=0;

  bindGestureSurface(
    $('pongSurface'),
    {
      tap:({yRatio})=>directionalInput(
        yRatio<.5?'up':'down',
        {
          up:['PONG_UP','pongUp'],
          down:['PONG_DOWN','pongDown']
        }
      ),

      dragStart:()=>{
        pongAccumY=0;
        pongLastSent=0;
      },

      drag:({stepY})=>{
        pongAccumY+=stepY;

        const now=performance.now();

        if(
          now-pongLastSent<PONG_DRAG_SEND_MS ||
          Math.abs(pongAccumY)<PONG_DRAG_STEP_PX
        ){
          return;
        }

        const direction=pongAccumY<0?'up':'down';

        pongAccumY+=
          direction==='up'
            ? PONG_DRAG_STEP_PX
            : -PONG_DRAG_STEP_PX;

        pongLastSent=now;

        directionalInput(
          direction,
          {
            up:['PONG_UP','pongUp'],
            down:['PONG_DOWN','pongDown']
          }
        );
      },

      dragEnd:()=>{
        pongAccumY=0;
      },

      cancel:()=>{
        pongAccumY=0;
      }
    }
  );

  let stackAccumY=0;
  let stackLastSent=0;
  let stackHoldTimer=0;
  const stopStackHold=()=>{
    if(stackHoldTimer){
      clearInterval(stackHoldTimer);
      stackHoldTimer=0;
    }
    $('stackSurface').classList.remove('softDropping');
  };

  bindGestureSurface(
    $('stackSurface'),
    {
      holdMs:300,
      tap:()=>stackInput('STACK_ROTATE','rotate'),
      swipe:direction=>{
        if(direction==='left')stackInput('STACK_LEFT','move');
        else if(direction==='right')stackInput('STACK_RIGHT','move');
        else if(direction==='down')stackInput('STACK_HARD_DROP','drop');
      },
      holdStart:()=>{
        input('STACK_SOFT_DROP');
        stopStackHold();
        $('stackSurface').classList.add('softDropping');
        stackHoldTimer=setInterval(()=>input('STACK_SOFT_DROP'),70);
      },
      holdEnd:stopStackHold,
      dragStart:()=>{
        stopStackHold();
        stackAccumY=0;
        stackLastSent=0;
      },
      drag:({stepY,duration})=>{
        if(duration<=GESTURE_SWIPE_MAX_MS||stepY<=0)return;
        stackAccumY+=stepY;
        const now=performance.now();
        if(now-stackLastSent<STACK_SOFT_DROP_SEND_MS||stackAccumY<STACK_DRAG_STEP_PX)return;
        stackAccumY-=STACK_DRAG_STEP_PX;
        stackLastSent=now;
        input('STACK_SOFT_DROP');
      },
      dragEnd:()=>{
        stopStackHold();
        stackAccumY=0;
      },
      cancel:()=>{
        stopStackHold();
        stackAccumY=0;
      }
    }
  );
}

function resultActionButtons(){return ['rematchBtn','resultGamesBtn','resultBreakBtn','bossContinueBtn'].map($).filter(Boolean)}function updateResultGuard(){const locked=performance.now()<resultLockedUntil||resultNeedsRelease;['result','boss_result'].forEach(id=>$(id).classList.toggle('input-locked',locked));resultActionButtons().forEach(b=>b.disabled=locked);if(!locked)clearTimeout(resultUnlockTimer)}function armResultGuard(){resultLockedUntil=performance.now()+1000;resultNeedsRelease=pointerDown;clearTimeout(resultUnlockTimer);resultUnlockTimer=setTimeout(updateResultGuard,1020);updateResultGuard()}function canUseResultAction(){updateResultGuard();return performance.now()>=resultLockedUntil&&!resultNeedsRelease}function render(s){if(!s)return;const enteringResult=(s.stage==='result'||s.stage==='boss_result')&&s.stage!==lastStage;state=s;me=s.you;const p=mine(s);theme(p?p.slot:-1);$('dot').classList.toggle('hidden',!p);$('identity').textContent=p?`${colorNames[lang][p.slot]} · ${p.slot+1}`:tr('guest');$('joinBox').classList.toggle('hidden',!!p);$('oneDBtn').classList.toggle('selected',s.arena==='strip_1d');$('matrixBtn').classList.toggle('selected',s.arena==='matrix_8x32');$('screenBtn').classList.toggle('selected',s.arena==='screen_arcade');show(s.stage);if(s.stage==='games'){const strip=s.arena==='strip_1d',screen=s.arena==='screen_arcade';$('games').dataset.arena=s.arena;['rallyCard','pushCard'].forEach(id=>$(id).classList.toggle('hidden',!strip));['derbyCard','tronCard','raiderCard','clashCard','pongCard'].forEach(id=>$(id).classList.toggle('hidden',strip||screen));['tapClashCard','brainDuelCard'].forEach(id=>$(id).classList.toggle('hidden',!screen));drawPreviews()}if(s.stage==='lobby')renderLobby(s,p);if(s.stage==='announce')$('announceText').textContent=s.announcePhase%2===0?'BOSS':(s.bossSlot>=0?colorNames[lang][s.bossSlot]:'BOSS');if(s.stage==='countdown')$('countText').textContent=s.countdown||'GO';if(s.stage==='race'){const tron=s.game==='tron_arena',raider=s.game==='pixel_raider',clash=s.game==='color_clash',pong=s.game==='pixel_pong',tapClash=s.game==='tap_clash',brain=s.game==='brain_duel';$('tapSurface').classList.toggle('hidden',tron||raider||clash||pong||tapClash||brain);$('tronSurface').classList.toggle('hidden',!tron);$('raiderSurface').classList.toggle('hidden',!raider);$('clashSurface').classList.toggle('hidden',!clash);$('pongSurface').classList.toggle('hidden',!pong);$('tapClashSurface').classList.toggle('hidden',!tapClash);$('brainDuelSurface').classList.toggle('hidden',!brain);if(tapClash)renderTapClash(s,p);if(brain)renderBrainDuel(s,p)}if(s.stage==='result')renderResult(s,p);if(s.stage==='boss_result')renderBossResult(s,p);if(enteringResult)armResultGuard();playStateAudio(s,p,enteringResult);lastStage=s.stage}
function renderLobby(s,p){$('gameTitle').textContent=s.game==='tron_arena'?'TRON ARENA':s.game==='pixel_raider'?'PIXEL RAIDER':s.game==='color_clash'?'COLOR CLASH':s.game==='pixel_pong'?'PIXEL PONG':s.game==='reflex_rally'?'REFLEX RALLY':s.game==='power_push'?'POWER PUSH':s.game==='tap_clash'?'TAP CLASH':s.game==='brain_duel'?'BRAIN DUEL':'PIXEL DERBY';$('instructions').textContent=s.game==='tron_arena'?tr('tronHelp'):s.game==='pixel_raider'?tr('raiderHelp'):s.game==='color_clash'?tr('clashHelp'):s.game==='pixel_pong'?tr('pongHelp'):s.game==='reflex_rally'?tr('rallyHelp'):s.game==='power_push'?tr('pushHelp'):s.game==='tap_clash'?tr('tapClashHelp'):s.game==='brain_duel'?tr('brainDuelHelp'):tr('derbyHelp');$('roleText').textContent=p?`${colorNames[lang][p.slot]} · ${p.waiting?tr('onBreak'):''}`:tr('guest');ready=!!p?.ready;$('readyBtn').classList.toggle('hidden',!p||p.waiting);$('readyBtn').textContent=ready?tr('readyState')+' ✓':tr('ready');$('waitBtn').textContent=p?.waiting?tr('returnGame'):tr('break');$('players').innerHTML=s.players.map(x=>`<div class="player"><span style="color:${palettes[x.slot][0]}">${colorNames[lang][x.slot]}${x.slot===me?' · '+tr('you'):''}</span><strong>${x.isCpu?tr('cpu'):x.waiting?tr('onBreak'):x.connected?(x.ready?tr('readyState'):tr('waiting')):tr('disconnected')}</strong></div>`).join('');$('waitBtn').textContent=p?.waiting?tr('returnGame'):tr('break')}
function renderTapClash(s,p){$('tcTimer').textContent=(Math.max(0,s.tapClashRemainingMs||0)/1000).toFixed(1);$('tcScores').innerHTML=(s.players||[]).filter(x=>x.connected&&!x.waiting).map(x=>`<div class="tcScore" style="color:${palettes[x.slot][0]}">${colorNames[lang][x.slot]} ${x.score}</div>`).join('');const locked=!!p&&(p.tapClashLockedMs||0)>0,canTap=!!p&&!p.waiting&&!locked&&(s.tapClashTarget??-1)>=0;document.querySelectorAll('.tcCell').forEach(b=>{const cell=Number(b.dataset.cell);b.classList.toggle('target',cell===s.tapClashTarget);b.disabled=!canTap});$('tcLock').classList.toggle('hidden',!locked);$('tcLock').textContent=locked?`${tr('locked')} · ${((p.tapClashLockedMs||0)/1000).toFixed(1)}`:''}
function renderBrainDuel(s,p){$('bdProgress').textContent=`${tr('question')} ${s.brainQuestionNumber}/${s.brainTotalQuestions}`;const bdScores=(s.players||[]).filter(x=>x.connected&&!x.waiting).map(x=>`${colorNames[lang][x.slot]} ${x.score}`).join(' / ');$('bdDifficulty').textContent=`${s.brainKind===0?tr('math'):tr('word')} · ${[tr('easy'),tr('normal'),tr('challenge')][s.brainDifficulty]||tr('normal')} · ${bdScores}`;$('bdTimer').textContent=(Math.max(0,s.brainRemainingMs||0)/1000).toFixed(1);$('bdQuestion').textContent=lang==='tr'?s.brainPromptTr:s.brainPromptEn;const opts=lang==='tr'?s.brainOptionsTr:s.brainOptionsEn;document.querySelectorAll('.bdAnswer').forEach((b,i)=>{b.textContent=opts?.[i]||'';b.classList.toggle('selected',p?.brainSelectedAnswer===i);b.classList.toggle('correct',s.brainPhase===2&&s.brainCorrectIndex===i);b.classList.toggle('wrong',s.brainPhase===2&&p?.brainSelectedAnswer===i&&!p?.brainAnswerCorrect);b.disabled=!p||p.waiting||p.brainAnswered||s.brainPhase!==1});$('bdStatus').textContent=!p?'':s.brainPhase===2?(p.brainAnswered?(p.brainAnswerCorrect?tr('correct'):tr('wrong')):''):p.brainAnswered?tr('answered'):''}
function renderResult(s,p){if(s.game==='pixel_raider'){$('winnerText').textContent=s.raiderNewRecord?(lang==='tr'?'YENİ REKOR!':'NEW RECORD!'):(lang==='tr'?'OYUN BİTTİ':'GAME OVER');$('resultText').textContent=`${lang==='tr'?'SKOR':'SCORE'} ${s.raiderScore} · ${lang==='tr'?'MESAFE':'DISTANCE'} ${s.raiderDistance} · ${lang==='tr'?'REKOR':'BEST'} ${s.raiderBestScore}`;}else if(s.game==='pixel_pong'){if(s.winner<0)$('winnerText').textContent=tr('draw');else $('winnerText').textContent=s.winner===me?tr('won'):`${tr('winner')}: ${colorNames[lang][s.winner]}`;$('resultText').textContent=`${colorNames[lang][s.pongLeftSlot]||'LEFT'} ${s.pongLeftScore} · ${s.pongRightScore} ${colorNames[lang][s.pongRightSlot]||'RIGHT'}`;}else if(s.game==='tap_clash'){if(s.winner<0)$('winnerText').textContent=tr('draw');else $('winnerText').textContent=s.winner===me?tr('won'):`${tr('winner')}: ${colorNames[lang][s.winner]}`;$('resultText').textContent=(s.players||[]).filter(x=>x.connected&&!x.waiting).sort((a,b)=>b.score-a.score).map(x=>`${colorNames[lang][x.slot]} ${x.score}`).join(' · ');}else if(s.game==='brain_duel'){if(s.winner<0)$('winnerText').textContent=tr('draw');else $('winnerText').textContent=s.winner===me?tr('won'):`${tr('winner')}: ${colorNames[lang][s.winner]}`;$('resultText').textContent=(s.players||[]).filter(x=>x.connected&&!x.waiting).sort((a,b)=>b.score-a.score).map(x=>`${colorNames[lang][x.slot]} ${x.score}`).join(' · ');}else if(s.game==='color_clash'){if(s.winner<0)$('winnerText').textContent=tr('draw');else $('winnerText').textContent=s.winner===me?tr('won'):`${tr('winner')}: ${colorNames[lang][s.winner]}`;const total=(s.clashCounts||[]).reduce((a,b)=>a+b,0)||1;$('resultText').textContent=(s.players||[]).filter(x=>(s.clashCounts?.[x.slot]||0)>0).map(x=>`${colorNames[lang][x.slot]} ${Math.round((s.clashCounts[x.slot]||0)*100/total)}%`).join(' · ');}else{if(s.winner<0)$('winnerText').textContent=tr('draw');else $('winnerText').textContent=s.winner===me?tr('won'):`${tr('winner')}: ${colorNames[lang][s.winner]}`;$('resultText').textContent=p?`${colorNames[lang][p.slot]} · ${p.points} pts${p.waiting?' · '+tr('onBreak'):''}`:'';}$('resultBreakBtn').textContent=p?.waiting?tr('returnGame'):tr('break');$('rematchBtn').classList.toggle('hidden',!!p?.waiting)}
function renderBossResult(s,p){$('bossResultText').textContent=s.bossDefeated?tr('teamWon'):tr('bossWon');$('bossResultScore').textContent=p&&p.slot!==s.bossSlot?`${p.bossDamage} damage`:''}
function input(msg,el){unlockAudio();const n=Date.now();if(n-lastInput<35)return;lastInput=n;send(msg);if(el){el.classList.add('flash');setTimeout(()=>el.classList.remove('flash'),70)}navigator.vibrate?.(7)}
function drawPreviews(){drawRally($('rallyPreview'));drawPush($('pushPreview'));drawDerby($('derbyPreview'));drawTron($('tronPreview'));drawRaider($('raiderPreview'));drawClash($('clashPreview'));drawPong($('pongPreview'));drawTapClash($('tapClashPreview'));drawBrainDuel($('brainDuelPreview'));drawPlatform($('pm'));drawOneD($('p1'));drawScreen($('ps'))}function px(c,x,y,w,h,col){c.fillStyle=col;c.fillRect(x,y,w,h)}function prep(canvas){const c=canvas.getContext('2d');c.imageSmoothingEnabled=false;c.fillStyle='#03040a';c.fillRect(0,0,64,24);return c}function drawRally(cv){const c=prep(cv);px(c,2,10,14,4,palettes[0][0]);px(c,48,10,14,4,palettes[1][0]);px(c,31,9,3,6,'#fff')}function drawPush(cv){const c=prep(cv);px(c,2,10,29,4,palettes[0][0]);px(c,34,10,28,4,palettes[1][0]);px(c,31,7,3,10,'#fff')}function drawDerby(cv){const c=prep(cv);[5,12,19].forEach((y,i)=>{px(c,2,y,35,1,'#24263a');px(c,10+i*7,y-1,3,3,palettes[i][0]);px(c,59,y-2,1,5,'#fff')});px(c,42,3,2,18,'#6f4a08')}function drawRaider(cv){const c=prep(cv);for(let x=0;x<64;x+=5){px(c,x,0,3,5,'#16314f');px(c,x,19,3,5,'#16314f')}px(c,8,11,5,3,'#32d7ff');px(c,16,12,24,1,'#ff9d00');px(c,44,9,3,3,'#ff3b5c');px(c,53,15,3,3,'#ffe94a');px(c,58,5,5,7,'#244c70')}function drawClash(cv){const c=prep(cv);for(let y=0;y<24;y+=3)for(let x=0;x<64;x+=3){const owner=(x<31?0:1);px(c,x,y,3,3,palettes[owner][0]+'88')}px(c,18,8,4,4,'#fff');px(c,45,14,4,4,'#fff')}function drawPong(cv){const c=prep(cv);px(c,3,6,2,12,palettes[0][0]);px(c,59,8,2,12,palettes[1][0]);for(let y=2;y<24;y+=5)px(c,31,y,1,2,'#24263a');px(c,37,11,3,3,'#fff');px(c,34,11,2,3,'#35394d')}function drawTron(cv){const c=prep(cv);for(let x=0;x<64;x+=4)px(c,x,0,1,24,'#0d1121');for(let y=0;y<24;y+=4)px(c,0,y,64,1,'#0d1121');px(c,5,5,24,2,palettes[0][0]);px(c,27,5,2,11,palettes[0][0]);px(c,58,18,2,2,palettes[1][0]);px(c,38,18,22,2,palettes[1][0]);px(c,38,9,2,11,palettes[1][0])}function drawPlatform(cv){const c=prep(cv);for(let x=0;x<64;x+=3)for(let y=0;y<24;y+=3)px(c,x,y,1,1,(x+y)%2?'#6d7cff':'#a665ff')}function drawOneD(cv){const c=prep(cv);px(c,3,11,58,2,'#23263a');px(c,17,9,4,6,'#25d878');px(c,44,9,4,6,'#ff2a92')}function drawScreen(cv){const c=prep(cv);[4,25,46].forEach((x,i)=>{px(c,x,4,14,16,'#151b2c');px(c,x+2,6,10,10,palettes[i][0]+'88');px(c,x+6,18,2,1,'#fff')})}function drawTapClash(cv){const c=prep(cv);for(let y=3;y<21;y+=6)for(let x=15;x<51;x+=6)px(c,x,y,5,5,'#151b2c');px(c,33,9,5,5,'#fff');px(c,6,10,5,5,palettes[0][0]);px(c,53,10,5,5,palettes[1][0])}function drawBrainDuel(cv){const c=prep(cv);px(c,4,3,56,6,'#202844');px(c,9,12,20,4,palettes[0][0]);px(c,35,12,20,4,palettes[1][0]);px(c,9,18,20,3,'#27304d');px(c,35,18,20,3,'#27304d');px(c,30,4,4,4,'#fff')}
const renderScreenArcade=render;
render=s=>{if(!s)return;const enteringStack=s.stage==='race'&&s.game==='stack_shift'&&(lastStage!=='race'||lastGame!=='stack_shift');renderScreenArcade(s);const stack=s.stage==='race'&&s.game==='stack_shift';document.documentElement.classList.toggle('stack-active',stack);document.body.classList.toggle('stack-active',stack);$('stackCard').classList.toggle('hidden',s.arena!=='matrix_8x32');if(s.stage==='race'){if(stack)$('tapSurface').classList.add('hidden');$('stackSurface').classList.toggle('hidden',!stack);if(stack){if(enteringStack)openStackTutorial(false);renderStackController(s)}}lastGame=s.game||''};
const renderLobbyScreenArcade=renderLobby;
renderLobby=(s,p)=>{renderLobbyScreenArcade(s,p);if(s.game==='stack_shift'){$('gameTitle').textContent='STACK SHIFT';$('instructions').textContent=tr('stackHelp')}};
const renderResultScreenArcade=renderResult;
renderResult=(s,p)=>{renderResultScreenArcade(s,p);if(s.game==='stack_shift'){$('winnerText').textContent=s.stackNewRecord?(lang==='tr'?'YENİ REKOR!':'NEW RECORD!'):(lang==='tr'?'OYUN BİTTİ':'GAME OVER');$('resultText').textContent=`${lang==='tr'?'SKOR':'SCORE'} ${s.stackScore} · ${lang==='tr'?'SATIR':'LINES'} ${s.stackLines} · LV ${s.stackLevel} · ${lang==='tr'?'REKOR':'BEST'} ${s.stackBestScore}`}};
input=(msg,el)=>{unlockAudio();if(state?.game==='stack_shift'&&state?.stackPaused&&msg.startsWith('STACK_')&&msg!=='STACK_PAUSE')return false;const n=Date.now();if(n-lastInput<35)return false;lastInput=n;send(msg);if(el){el.classList.add('flash');setTimeout(()=>el.classList.remove('flash'),70)}navigator.vibrate?.(7);return true};
const drawScreenArcadePreviews=drawPreviews;
drawPreviews=()=>{drawScreenArcadePreviews();drawStack($('stackPreview'))};
const stackSurfaceEl=$('stackSurface'),blockStackOverscroll=e=>{if(document.body.classList.contains('stack-active'))e.preventDefault()};['touchstart','touchmove','touchend'].forEach(name=>stackSurfaceEl.addEventListener(name,blockStackOverscroll,{capture:true,passive:false}));
['leftTurn','rightTurn','moveUp','moveDown','pongUp','pongDown','clashUp','clashRight','clashDown','clashLeft'].forEach(id=>$(id).style.pointerEvents='none');
bindGameGestures();
window.addEventListener('pointerdown',()=>{pointerDown=true;if(!audioReady||audioCtx?.state!=='running')audioGesture(false)},{capture:true,passive:true});window.addEventListener('touchend',()=>{if(!audioReady||audioCtx?.state!=='running')audioGesture(false)},{capture:true,passive:true});window.addEventListener('pointerup',()=>{pointerDown=false;if(resultNeedsRelease){resultNeedsRelease=false;updateResultGuard()}},{capture:true,passive:true});window.addEventListener('pointercancel',()=>{pointerDown=false;if(resultNeedsRelease){resultNeedsRelease=false;updateResultGuard()}},{capture:true,passive:true});['gesturestart','gesturechange','gestureend'].forEach(n=>document.addEventListener(n,e=>e.preventDefault(),{passive:false}));document.addEventListener('touchmove',e=>{if(gameActive||e.touches.length>1)e.preventDefault()},{passive:false});document.addEventListener('dblclick',e=>e.preventDefault(),{passive:false});document.addEventListener('wheel',e=>{if(e.ctrlKey)e.preventDefault()},{passive:false});document.addEventListener('contextmenu',e=>e.preventDefault());document.addEventListener('dragstart',e=>e.preventDefault());
$('trBtn').onclick=()=>{lang='tr';localStorage.setItem('ledArcadeLang',lang);applyLang()};$('enBtn').onclick=()=>{lang='en';localStorage.setItem('ledArcadeLang',lang);applyLang()};$('oneDBtn').addEventListener('click',()=>send('SELECT_PLATFORM|strip_1d'));$('matrixBtn').addEventListener('click',()=>send('SELECT_PLATFORM|matrix_8x32'));$('screenBtn').addEventListener('click',()=>send('SELECT_PLATFORM|screen_arcade'));$('rallyCard').addEventListener('click',()=>send('SELECT_GAME|reflex_rally'));$('pushCard').addEventListener('click',()=>send('SELECT_GAME|power_push'));$('derbyCard').addEventListener('click',()=>send('SELECT_GAME|pixel_derby'));$('tronCard').addEventListener('click',()=>send('SELECT_GAME|tron_arena'));$('raiderCard').addEventListener('click',()=>send('SELECT_GAME|pixel_raider'));$('clashCard').addEventListener('click',()=>send('SELECT_GAME|color_clash'));$('pongCard').addEventListener('click',()=>send('SELECT_GAME|pixel_pong'));$('tapClashCard').addEventListener('click',()=>send('SELECT_GAME|tap_clash'));$('brainDuelCard').addEventListener('click',()=>send('SELECT_GAME|brain_duel'));$('platformsBtn').addEventListener('click',()=>send('BACK_TO_PLATFORMS'));function backToGames(e){e?.preventDefault();send('BACK_TO_GAMES')}$('gamesBtn').addEventListener('click',backToGames);$('resultGamesBtn').onpointerdown=e=>guardedResultAction(e,()=>send('BACK_TO_GAMES'));$('readyBtn').onpointerdown=e=>{e.preventDefault();audioGesture(true);send('READY|'+(ready?0:1))};function guardedResultAction(e,action){e.preventDefault();if(!canUseResultAction())return;action()}$('rematchBtn').onpointerdown=$('bossContinueBtn').onpointerdown=e=>guardedResultAction(e,()=>send('REMATCH'));function wait(){const p=mine(state);if(p)send(p.waiting?'WAIT|0':'WAIT|1')}$('waitBtn').onpointerdown=e=>{e.preventDefault();wait()};$('resultBreakBtn').onpointerdown=e=>guardedResultAction(e,wait);$('joinBtn').onpointerdown=e=>{e.preventDefault();audioGesture(true);send('HELLO|'+cid)};$('tapSurface').addEventListener('pointerdown',e=>{e.preventDefault();input('TAP',$('tapSurface'))},{passive:false});$('bossSurface').addEventListener('pointerdown',e=>{e.preventDefault();input('TAP',$('bossSurface'))},{passive:false});$('leftTurn').addEventListener('pointerdown',e=>{e.preventDefault();input('TURN_LEFT',$('leftTurn'))},{passive:false});$('rightTurn').addEventListener('pointerdown',e=>{e.preventDefault();input('TURN_RIGHT',$('rightTurn'))},{passive:false});$('moveUp').addEventListener('pointerdown',e=>{e.preventDefault();input('MOVE_UP',$('moveUp'))},{passive:false});$('moveDown').addEventListener('pointerdown',e=>{e.preventDefault();input('MOVE_DOWN',$('moveDown'))},{passive:false});$('pongUp').addEventListener('pointerdown',e=>{e.preventDefault();input('PONG_UP',$('pongUp'))},{passive:false});$('pongDown').addEventListener('pointerdown',e=>{e.preventDefault();input('PONG_DOWN',$('pongDown'))},{passive:false});$('clashUp').addEventListener('pointerdown',e=>{e.preventDefault();input('CLASH_UP',$('clashUp'))},{passive:false});$('clashRight').addEventListener('pointerdown',e=>{e.preventDefault();input('CLASH_RIGHT',$('clashRight'))},{passive:false});$('clashDown').addEventListener('pointerdown',e=>{e.preventDefault();input('CLASH_DOWN',$('clashDown'))},{passive:false});$('clashLeft').addEventListener('pointerdown',e=>{e.preventDefault();input('CLASH_LEFT',$('clashLeft'))},{passive:false});document.querySelectorAll('.tcCell').forEach(b=>b.addEventListener('pointerdown',e=>{e.preventDefault();audioGesture(false);const p=mine(state);if(!p||p.waiting||(p.tapClashLockedMs||0)>0||(state?.tapClashTarget??-1)<0)return;input('TAP_CLASH|'+state.tapClashTargetId+'|'+b.dataset.cell,b)},{passive:false}));document.querySelectorAll('.bdAnswer').forEach(b=>b.addEventListener('pointerdown',e=>{e.preventDefault();audioGesture(false);const p=mine(state);if(!p||p.waiting||p.brainAnswered||state?.brainPhase!==1)return;input('BRAIN_ANSWER|'+state.brainQuestionId+'|'+b.dataset.answer,b)},{passive:false}));window.addEventListener('resize',syncViewport,{passive:true});window.addEventListener('orientationchange',()=>setTimeout(syncViewport,80),{passive:true});window.visualViewport?.addEventListener('resize',syncViewport,{passive:true});document.body.classList.remove('viewport-landscape');syncViewport();applyLang();drawPreviews();connect();
$('stackCard').addEventListener('click',()=>send('SELECT_GAME|stack_shift'));
function toggleStackPause(e){e.preventDefault();e.stopPropagation();input('STACK_PAUSE')}function showStackTutorial(e){e.preventDefault();e.stopPropagation();openStackTutorial(true)}function dismissStackTutorial(e){e.preventDefault();e.stopPropagation();closeStackTutorial(true)}
$('stackPauseBtn').addEventListener('pointerdown',toggleStackPause,{passive:false});$('stackPauseOverlay').addEventListener('pointerdown',toggleStackPause,{passive:false});$('stackHelpBtn').addEventListener('pointerdown',showStackTutorial,{passive:false});$('stackTutorialClose').addEventListener('pointerdown',dismissStackTutorial,{passive:false});
</script></body></html>
)HTML";
