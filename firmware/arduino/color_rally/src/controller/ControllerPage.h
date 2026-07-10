#pragma once
#include <Arduino.h>

const char CONTROLLER_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="tr">
<head>
<meta charset="utf-8">
<meta
  name="viewport"
  content="width=device-width,initial-scale=1,viewport-fit=cover"
>
<title>LED Arcade Motion</title>

<style>
html,body{
  margin:0;
  min-height:100%;
  background:#080b12;
  color:#fff;
  font-family:system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif;
  user-select:none;
  -webkit-user-select:none;
}

body{
  min-height:100vh;
  display:grid;
  place-items:center;
  padding:18px;
  box-sizing:border-box;
}

.card{
  width:min(94vw,520px);
  padding:22px;
  box-sizing:border-box;
  text-align:center;
  background:#151b2a;
  border:1px solid #2b3448;
  border-radius:28px;
}

h1{
  margin:0 0 8px;
  font-size:28px;
}

.role{
  margin:8px 0 16px;
  color:#9ca8bd;
  font-size:22px;
  font-weight:900;
}

.role.blue{color:#38a8ff}
.role.pink{color:#ff4d9d}

.force{
  font-size:54px;
  font-weight:1000;
  font-variant-numeric:tabular-nums;
}

.gauge{
  position:relative;
  height:32px;
  margin:18px 0;
  overflow:hidden;
  background:#0c1019;
  border:1px solid #2b3448;
  border-radius:999px;
}

.gauge-center{
  position:absolute;
  left:50%;
  top:0;
  bottom:0;
  width:2px;
  background:#ffffff77;
}

.gauge-fill{
  position:absolute;
  top:0;
  bottom:0;
  left:50%;
  width:0;
  background:linear-gradient(90deg,#38a8ff,#fff,#ff4d9d);
  transition:left .04s linear,width .04s linear;
}

.telemetry{
  display:grid;
  grid-template-columns:repeat(3,1fr);
  gap:8px;
  margin:16px 0;
}

.box{
  padding:12px 6px;
  background:#0d121d;
  border:1px solid #283349;
  border-radius:16px;
}

.label{
  color:#8794aa;
  font-size:11px;
  font-weight:800;
}

.value{
  margin-top:5px;
  font-size:20px;
  font-weight:900;
  font-variant-numeric:tabular-nums;
}

button{
  width:100%;
  min-height:66px;
  margin-top:10px;
  border:0;
  border-radius:18px;
  color:white;
  font-size:17px;
  font-weight:900;
}

button:active{
  transform:scale(.985);
}

.primary{
  background:linear-gradient(180deg,#2687ff,#123a77);
}

.secondary{
  background:linear-gradient(180deg,#6b7280,#303642);
}

button:disabled{
  opacity:.65;
}

.status{
  min-height:58px;
  margin-top:16px;
  color:#ffe066;
  font-weight:800;
  line-height:1.45;
  overflow-wrap:anywhere;
}

.meta{
  margin-top:14px;
  color:#718096;
  font-size:12px;
  line-height:1.55;
  overflow-wrap:anywhere;
}
</style>
</head>

<body>
<main class="card">
  <h1>LED Arcade Motion</h1>

  <div id="role" class="role">BAĞLANIYOR</div>

  <div id="force" class="force">0.00</div>

  <div class="gauge">
    <div class="gauge-center"></div>
    <div id="gaugeFill" class="gauge-fill"></div>
  </div>

  <div class="telemetry">
    <div class="box">
      <div class="label">GAMMA</div>
      <div id="gamma" class="value">—</div>
    </div>

    <div class="box">
      <div class="label">NÖTR</div>
      <div id="neutral" class="value">—</div>
    </div>

    <div class="box">
      <div class="label">FARK</div>
      <div id="relative" class="value">—</div>
    </div>
  </div>

  <button id="permissionBtn" class="primary" type="button">
    ORIENTATION İZNİ İSTE
  </button>

  <button id="calibrateBtn" class="secondary" type="button" disabled>
    NÖTRÜ YENİDEN KALİBRE ET
  </button>

  <div id="status" class="status">
    Önce orientation iznini ver.
  </div>

  <div class="meta">
    WSS: <span id="wsState">bağlanıyor</span><br>
    secure=<span id="secure"></span><br>
    origin=<span id="origin"></span>
  </div>
</main>

<script>
"use strict";

const DEAD_ZONE_DEG = 4;
const MAX_TILT_DEG = 30;
const FILTER_ALPHA = 0.25;

const SEND_INTERVAL_MS = 50;
const KEEPALIVE_INTERVAL_MS = 250;
const MIN_FORCE_CHANGE = 0.02;

const roleEl = document.getElementById("role");
const forceEl = document.getElementById("force");
const gammaEl = document.getElementById("gamma");
const neutralEl = document.getElementById("neutral");
const relativeEl = document.getElementById("relative");
const gaugeFillEl = document.getElementById("gaugeFill");

const permissionBtn = document.getElementById("permissionBtn");
const calibrateBtn = document.getElementById("calibrateBtn");

const statusEl = document.getElementById("status");
const wsStateEl = document.getElementById("wsState");

document.getElementById("secure").textContent =
  String(window.isSecureContext);

document.getElementById("origin").textContent =
  location.origin;

function getCid() {
  const key = "ledArcadeMotionCid";

  let cid = "";

  try {
    cid = localStorage.getItem(key) || "";
  } catch (error) {
  }

  if (!cid) {
    cid =
      "c" +
      Math.random().toString(16).slice(2) +
      Date.now().toString(16);

    try {
      localStorage.setItem(key, cid);
    } catch (error) {
    }
  }

  return cid;
}

const cid = getCid();

let socket = null;
let reconnectTimer = null;

let role = "none";

let orientationActive = false;
let eventCount = 0;

let latestGamma = null;
let neutralGamma = null;

let filteredForce = 0;
let lastSentForce = 999;
let lastSendAt = 0;

function updateRole(nextRole) {
  role = nextRole || "none";

  roleEl.className = "role";

  if (role === "blue") {
    roleEl.classList.add("blue");
    roleEl.textContent = "MAVİ OYUNCU";
    return;
  }

  if (role === "pink") {
    roleEl.classList.add("pink");
    roleEl.textContent = "PEMBE OYUNCU";
    return;
  }

  roleEl.textContent = "İZLEYİCİ";
}

function connectWebSocket() {
  clearTimeout(reconnectTimer);

  const wsUrl =
    "wss://" +
    location.host +
    "/ws";

  wsStateEl.textContent = "bağlanıyor";

  socket = new WebSocket(wsUrl);

  socket.addEventListener("open", function () {
    wsStateEl.textContent = "bağlı";
    socket.send("HELLO|" + cid);
  });

  socket.addEventListener("message", function (event) {
    const message = String(event.data);

    if (message.startsWith("ROLE|")) {
      updateRole(message.slice(5));
      return;
    }

    if (message === "PONG") {
      return;
    }
  });

  socket.addEventListener("close", function () {
    wsStateEl.textContent = "koptu";
    updateRole("none");

    reconnectTimer = setTimeout(
      connectWebSocket,
      1000
    );
  });

  socket.addEventListener("error", function () {
    wsStateEl.textContent = "hata";
  });
}

function clamp(value, min, max) {
  return Math.max(min, Math.min(max, value));
}

function normalizeTilt(relativeDegrees) {
  const magnitude = Math.abs(relativeDegrees);

  if (magnitude <= DEAD_ZONE_DEG) {
    return 0;
  }

  const normalized =
    (magnitude - DEAD_ZONE_DEG) /
    (MAX_TILT_DEG - DEAD_ZONE_DEG);

  return (
    relativeDegrees < 0 ? -1 : 1
  ) * clamp(normalized, 0, 1);
}

function renderForce(force, relative) {
  forceEl.textContent = force.toFixed(2);

  gammaEl.textContent =
    latestGamma === null
      ? "—"
      : latestGamma.toFixed(1) + "°";

  neutralEl.textContent =
    neutralGamma === null
      ? "—"
      : neutralGamma.toFixed(1) + "°";

  relativeEl.textContent =
    relative.toFixed(1) + "°";

  const percent = (force + 1) * 50;

  if (force >= 0) {
    gaugeFillEl.style.left = "50%";
    gaugeFillEl.style.width =
      Math.abs(percent - 50) + "%";
  } else {
    gaugeFillEl.style.left =
      percent + "%";

    gaugeFillEl.style.width =
      Math.abs(percent - 50) + "%";
  }
}

function sendMotion(force, immediate) {
  const now = performance.now();

  if (
    !socket ||
    socket.readyState !== WebSocket.OPEN
  ) {
    return;
  }

  if (
    !immediate &&
    now - lastSendAt < SEND_INTERVAL_MS
  ) {
    return;
  }

  if (
    !immediate &&
    Math.abs(force - lastSentForce) < MIN_FORCE_CHANGE &&
    now - lastSendAt < KEEPALIVE_INTERVAL_MS
  ) {
    return;
  }

  lastSentForce = force;
  lastSendAt = now;

  socket.send(
    "MOTION|" +
    force.toFixed(3)
  );
}

function calibrateNeutral() {
  if (latestGamma === null) {
    statusEl.textContent =
      "Henüz orientation verisi gelmedi.";
    return;
  }

  neutralGamma = latestGamma;
  filteredForce = 0;

  renderForce(0, 0);
  sendMotion(0, true);

  statusEl.textContent =
    "Nötr konum kaydedildi. Telefonu sağa-sola eğ.";
}

function onOrientation(event) {
  if (typeof event.gamma !== "number") {
    return;
  }

  eventCount += 1;
  latestGamma = event.gamma;

  if (neutralGamma === null) {
    neutralGamma = latestGamma;
  }

  const relative =
    latestGamma - neutralGamma;

  const rawForce =
    -normalizeTilt(relative);

  filteredForce =
    filteredForce * (1 - FILTER_ALPHA) +
    rawForce * FILTER_ALPHA;

  if (Math.abs(filteredForce) < 0.005) {
    filteredForce = 0;
  }

  renderForce(
    filteredForce,
    relative
  );

  sendMotion(
    filteredForce,
    false
  );

  if (eventCount === 1) {
    statusEl.textContent =
      "Orientation aktif. Telefonu sağa-sola eğ.";

    calibrateBtn.disabled = false;
  }
}

function startOrientationListener() {
  window.removeEventListener(
    "deviceorientation",
    onOrientation,
    true
  );

  window.addEventListener(
    "deviceorientation",
    onOrientation,
    true
  );

  orientationActive = true;
  eventCount = 0;

  permissionBtn.textContent =
    "ORIENTATION AKTİF";

  permissionBtn.disabled = true;

  statusEl.textContent =
    "İzin verildi. İlk orientation verisi bekleniyor…";

  setTimeout(function () {
    if (
      orientationActive &&
      eventCount === 0
    ) {
      statusEl.textContent =
        "İzin verildi fakat orientation verisi gelmedi.";
    }
  }, 3000);
}

/*
 * Bu handler içinde requestPermission çağrısından önce:
 * - fetch yok
 * - WebSocket await yok
 * - başka permission isteği yok
 * - timeout yok
 *
 * iOS kullanıcı jestini doğrudan bu çağrıya aktarır.
 */
permissionBtn.addEventListener(
  "click",
  function () {
    statusEl.textContent =
      "Safari izin yanıtı bekleniyor…";

    if (
      typeof DeviceOrientationEvent === "undefined"
    ) {
      statusEl.textContent =
        "DeviceOrientationEvent desteklenmiyor.";
      return;
    }

    if (
      typeof DeviceOrientationEvent.requestPermission !==
      "function"
    ) {
      startOrientationListener();
      return;
    }

    DeviceOrientationEvent
      .requestPermission()
      .then(function (result) {
        statusEl.textContent =
          "Permission sonucu: " + result;

        if (result !== "granted") {
          permissionBtn.textContent =
            "TEKRAR DENE";
          return;
        }

        startOrientationListener();
      })
      .catch(function (error) {
        statusEl.textContent =
          "Permission hatası: " +
          error.name +
          " — " +
          error.message;

        permissionBtn.textContent =
          "TEKRAR DENE";
      });
  }
);

calibrateBtn.addEventListener(
  "click",
  calibrateNeutral
);

document.addEventListener(
  "visibilitychange",
  function () {
    if (document.hidden) {
      sendMotion(0, true);
    }
  }
);

window.addEventListener(
  "pagehide",
  function () {
    sendMotion(0, true);
  }
);

connectWebSocket();
updateRole("none");
renderForce(0, 0);
</script>
</body>
</html>
)HTML";
