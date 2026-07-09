#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <FastLED.h>

// ================================================================
// LED Arcade Color Rally v0.1
// Rule: press the same color as the incoming ball.
// ================================================================

// -------------------- WiFi --------------------

const char* AP_SSID = "LED-Arcade-08";
const char* AP_PASSWORD = "playleds";

const byte DNS_PORT = 53;

IPAddress apIP(10, 10, 10, 1);
IPAddress gatewayIP(10, 10, 10, 1);
IPAddress subnetMask(255, 255, 255, 0);

DNSServer dnsServer;
WebServer server(80);

// -------------------- Hardware --------------------

#define LED_PIN       23
#define LED_COUNT     200
#define LED_TYPE      WS2812B
#define COLOR_ORDER   GRB
#define BRIGHTNESS    38

#define AUDIO_PIN     22
#define AUDIO_RES_BITS 8
#define AUDIO_DUTY     12   // 0-255 arası. Küçük = daha kısık ses.

CRGB leds[LED_COUNT];

// -------------------- Game tuning --------------------

const int BALL_HEAD_SIZE = 1;              // baş kısmı: 1 = tek LED gibi davranır
const int BALL_TAIL_NORMAL = 8;            // normal top kuyruğu
const int BALL_TAIL_CHARGED = 13;          // charged top kuyruğu

const int HIT_ZONE_SIZE = 42;

const float BALL_SPEED_BASE = 38.0f;
const float BALL_SPEED_COMBO = 48.0f;
const float BALL_SPEED_CHARGED = 58.0f;

const uint8_t MAX_HP = 5;
const uint8_t COMBO_FAST_AT = 3;
const uint8_t COMBO_CHARGED_AT = 5;

const uint32_t HUMAN_IDLE_TIMEOUT_MS = 10000;
const uint32_t SPECTATOR_TIMEOUT_MS = 20000;
const uint32_t POINT_PAUSE_MS = 900;
const uint32_t CPU_REACTION_MS = 300;

// CPU accuracy. Higher = stronger CPU.
const uint8_t CPU_CORRECT_PERCENT = 78;

// -------------------- Sides / colors --------------------

const uint8_t BLUE_SIDE = 0;
const uint8_t PINK_SIDE = 1;
const uint8_t NO_SIDE = 255;

const uint8_t C_RED = 0;
const uint8_t C_GREEN = 1;
const uint8_t C_BLUE = 2;

const int BLUE_HIT_A = 0;
const int BLUE_HIT_B = HIT_ZONE_SIZE - 1;

const int PINK_HIT_A = LED_COUNT - HIT_ZONE_SIZE;
const int PINK_HIT_B = LED_COUNT - 1;

// -------------------- Session state --------------------

struct Slot {
  bool human;
  String cid;
  String ip;
  uint32_t lastSeenMs;
  uint32_t lastActionMs;
  uint8_t hp;
  uint8_t combo;
};

struct Spectator {
  bool active;
  String cid;
  String ip;
  uint32_t lastSeenMs;
};

const uint8_t MAX_SPECTATORS = 6;

Slot slots[2];
Spectator spectators[MAX_SPECTATORS];

// -------------------- Game state --------------------

enum Mode {
  MODE_PLAYING,
  MODE_POINT_PAUSE
};

Mode mode = MODE_PLAYING;

float ballPos = 4.0f;
int ballDir = 1;                 // +1 -> pink side, -1 -> blue side
uint8_t targetSide = PINK_SIDE;  // side that must defend
uint8_t ballColorType = C_RED;
bool ballCharged = false;

uint8_t lastPointWinner = NO_SIDE;
uint8_t lastDamagedSide = NO_SIDE;

uint32_t pointPauseUntil = 0;
uint32_t lastFrameMs = 0;

bool cpuArmed = false;
uint32_t cpuHitAtMs = 0;
uint8_t cpuPlannedColor = C_RED;

// -------------------- Audio --------------------
// Daha kısık ses için LEDC + düşük duty kullanıyoruz.

void playToneSoft(uint16_t freq, uint16_t durationMs) {
  if (freq == 0 || durationMs == 0) return;

  // Arduino-ESP32 v3.x: LEDC API pin tabanlıdır.
  ledcWriteTone(AUDIO_PIN, freq);
  ledcWrite(AUDIO_PIN, AUDIO_DUTY);
  delay(durationMs);
  ledcWriteTone(AUDIO_PIN, 0);
  ledcWrite(AUDIO_PIN, 0);
}

void sfxJoin() {
  playToneSoft(880, 35);
}

void sfxAssign() {
  playToneSoft(660, 28);
  delay(10);
  playToneSoft(990, 34);
}

void sfxGood(uint8_t colorType) {
  if (colorType == C_RED) playToneSoft(740, 26);
  else if (colorType == C_GREEN) playToneSoft(920, 26);
  else playToneSoft(1180, 26);
}

void sfxBad() {
  playToneSoft(180, 40);
}

void sfxDamage() {
  playToneSoft(130, 85);
}

void sfxCharged() {
  playToneSoft(1400, 42);
}

void sfxReset() {
  playToneSoft(520, 32);
}

// -------------------- Helpers --------------------

String sideName(uint8_t side) {
  if (side == BLUE_SIDE) return "blue";
  if (side == PINK_SIDE) return "pink";
  return "none";
}

String colorName(uint8_t colorType) {
  if (colorType == C_RED) return "red";
  if (colorType == C_GREEN) return "green";
  if (colorType == C_BLUE) return "blue";
  return "unknown";
}

String modeName() {
  if (mode == MODE_PLAYING) return "playing";
  if (mode == MODE_POINT_PAUSE) return "point";
  return "unknown";
}

CRGB gameColor(uint8_t colorType) {
  if (colorType == C_RED) return CRGB::Red;
  if (colorType == C_GREEN) return CRGB::Green;
  if (colorType == C_BLUE) return CRGB::Blue;
  return CRGB::White;
}

CRGB sideColor(uint8_t side) {
  if (side == BLUE_SIDE) {
    return slots[BLUE_SIDE].human ? CRGB::Blue : CRGB::Cyan;
  }

  if (side == PINK_SIDE) {
    return slots[PINK_SIDE].human ? CRGB::DeepPink : CRGB::Orange;
  }

  return CRGB::White;
}

String currentClientIp() {
  return server.client().remoteIP().toString();
}

int findSlotByCidOrIp(const String& cid, const String& ip) {
  for (int i = 0; i < 2; i++) {
    if (!slots[i].human) continue;
    if (cid.length() > 0 && slots[i].cid == cid) return i;
    if (ip.length() > 0 && slots[i].ip == ip) return i;
  }
  return -1;
}

int findSpectatorByCidOrIp(const String& cid, const String& ip) {
  for (int i = 0; i < MAX_SPECTATORS; i++) {
    if (!spectators[i].active) continue;
    if (cid.length() > 0 && spectators[i].cid == cid) return i;
    if (ip.length() > 0 && spectators[i].ip == ip) return i;
  }
  return -1;
}

int firstCpuSlot() {
  if (!slots[BLUE_SIDE].human) return BLUE_SIDE;
  if (!slots[PINK_SIDE].human) return PINK_SIDE;
  return -1;
}

int firstSpectator() {
  for (int i = 0; i < MAX_SPECTATORS; i++) {
    if (spectators[i].active) return i;
  }
  return -1;
}

void removeSpectator(int index) {
  if (index < 0 || index >= MAX_SPECTATORS) return;

  spectators[index].active = false;
  spectators[index].cid = "";
  spectators[index].ip = "";
  spectators[index].lastSeenMs = 0;
}

int addOrUpdateSpectator(const String& cid, const String& ip) {
  int existing = findSpectatorByCidOrIp(cid, ip);

  if (existing >= 0) {
    spectators[existing].cid = cid;
    spectators[existing].ip = ip;
    spectators[existing].lastSeenMs = millis();
    return existing;
  }

  for (int i = 0; i < MAX_SPECTATORS; i++) {
    if (!spectators[i].active) {
      spectators[i].active = true;
      spectators[i].cid = cid;
      spectators[i].ip = ip;
      spectators[i].lastSeenMs = millis();
      return i;
    }
  }

  return -1;
}

void makeCpu(uint8_t side) {
  slots[side].human = false;
  slots[side].cid = "";
  slots[side].ip = "";
  slots[side].lastSeenMs = 0;
  slots[side].lastActionMs = 0;
  slots[side].hp = MAX_HP;
  slots[side].combo = 0;
}

void assignHumanToSlot(uint8_t side, const String& cid, const String& ip) {
  slots[side].human = true;
  slots[side].cid = cid;
  slots[side].ip = ip;
  slots[side].lastSeenMs = millis();
  slots[side].lastActionMs = millis();
  slots[side].hp = MAX_HP;
  slots[side].combo = 0;

  Serial.print("[ASSIGN] ");
  Serial.print(sideName(side));
  Serial.print(" cid=");
  Serial.print(cid);
  Serial.print(" ip=");
  Serial.println(ip);

  sfxAssign();
}

void promoteSpectatorToSlot(uint8_t side) {
  int sp = firstSpectator();

  if (sp < 0) {
    makeCpu(side);
    return;
  }

  String cid = spectators[sp].cid;
  String ip = spectators[sp].ip;

  removeSpectator(sp);
  assignHumanToSlot(side, cid, ip);
}

bool isBallInHitZone(uint8_t side) {
  float head = ballHeadPos();

  if (side == BLUE_SIDE) {
    return head >= BLUE_HIT_A && head <= BLUE_HIT_B;
  }

  if (side == PINK_SIDE) {
    return head >= PINK_HIT_A && head <= PINK_HIT_B;
  }

  return false;
}

void resetCpuPlan() {
  cpuArmed = false;
  cpuHitAtMs = 0;
  cpuPlannedColor = C_RED;
}

float currentBallSpeed() {
  if (ballCharged) return BALL_SPEED_CHARGED;

  uint8_t attacker = targetSide == BLUE_SIDE ? PINK_SIDE : BLUE_SIDE;
  if (slots[attacker].combo >= COMBO_FAST_AT) {
    return BALL_SPEED_COMBO;
  }

  return BALL_SPEED_BASE;
}

uint8_t randomBallColor() {
  return random(0, 3);
}

// -------------------- HTTP helpers --------------------

void sendNoCacheHeaders() {
  server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
}

void sendJson(const String& json) {
  sendNoCacheHeaders();
  server.send(200, "application/json", json);
}

void logRequest(const char* name) {
  Serial.print("[HTTP] ");
  Serial.print(name);
  Serial.print(" ");
  Serial.print(server.uri());
  Serial.print(" ip=");
  Serial.println(currentClientIp());
}

// -------------------- Controller HTML --------------------

String controllerPage() {
  return R"HTML(
<!doctype html>
<html lang="tr">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<title>LED Arcade Color Rally</title>
<style>
html,body{
  margin:0;height:100%;background:#080b12;color:white;
  font-family:system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif;
  user-select:none;-webkit-user-select:none;touch-action:manipulation;overscroll-behavior:none;
}
body{display:flex;align-items:center;justify-content:center;padding:14px;box-sizing:border-box}
.card{width:min(94vw,520px);background:#151b2a;border:1px solid #2b3448;border-radius:28px;padding:18px;text-align:center;box-sizing:border-box}
h1{margin:0 0 6px;font-size:26px}
.role{font-size:27px;font-weight:1000;margin:6px 0}
.blueText{color:#1f8ed7}.pinkText{color:#d72a78}.gray{color:#9ca8bd}
.score{display:grid;grid-template-columns:1fr 1fr;gap:10px;margin:12px 0}
.box{background:#0f1420;border:1px solid #273044;border-radius:18px;padding:10px}
.label{font-size:12px;color:#9ca8bd;font-weight:800}
.num{font-size:30px;font-weight:1000;line-height:1.05}
.hint{min-height:62px;margin:12px 0;color:#ffe066;font-size:19px;font-weight:950;line-height:1.25}
.target{font-size:16px;color:#d7dde8;font-weight:800;min-height:24px}
.buttons{display:grid;grid-template-rows:1fr 1fr 1fr;gap:10px;margin-top:12px}
button{width:100%;height:24vh;min-height:82px;max-height:128px;border:0;border-radius:22px;color:white;font-size:32px;font-weight:1000;letter-spacing:.05em;box-shadow:0 12px 30px rgba(0,0,0,.32)}
button:active{transform:scale(.985);filter:brightness(1.25)}
.red{background:linear-gradient(180deg,#f04b43,#7d1714)}
.green{background:linear-gradient(180deg,#24c66a,#126436)}
.blueBtn{background:linear-gradient(180deg,#2687ff,#123a77)}
.disabled{filter:grayscale(.72);opacity:.55}
.status{margin-top:12px;color:#9ca8bd;font-weight:800;min-height:22px}
.tiny{margin-top:6px;color:#667085;font-size:12px;line-height:1.35}
</style>
</head>
<body>
<main class="card">
  <h1>Color Rally</h1>
  <div id="role" class="role gray">BAĞLANIYOR</div>

  <div class="score">
    <div class="box">
      <div class="label">MAVİ</div>
      <div id="blueStats" class="num blueText">5 HP</div>
      <div id="blueWho" class="label">CPU</div>
    </div>
    <div class="box">
      <div class="label">PEMBE</div>
      <div id="pinkStats" class="num pinkText">5 HP</div>
      <div id="pinkWho" class="label">CPU</div>
    </div>
  </div>

  <div id="target" class="target">Gelen renge bas.</div>
  <div id="hint" class="hint">Top senin alana gelince aynı renge bas.</div>

  <div class="buttons">
    <button id="redBtn" class="red">KIRMIZI</button>
    <button id="greenBtn" class="green">YEŞİL</button>
    <button id="blueBtn" class="blueBtn">MAVİ</button>
  </div>

  <div id="status" class="status">Hazır</div>
  <div class="tiny">SSID: LED-Arcade-08 · IP: 10.10.10.10</div>
</main>

<script>
function getCid(){
  let cid=localStorage.getItem("ledArcadeColorRallyCid");
  if(!cid){
    cid="c"+Math.random().toString(16).slice(2)+Date.now().toString(16);
    localStorage.setItem("ledArcadeColorRallyCid",cid);
  }
  return cid;
}

const cid=getCid();

let role="none";
let targetSide="none";
let ballColor="red";
let inZone=false;
let mode="playing";
let lastPress=0;

const roleEl=document.getElementById("role");
const targetEl=document.getElementById("target");
const hintEl=document.getElementById("hint");
const statusEl=document.getElementById("status");
const blueStatsEl=document.getElementById("blueStats");
const pinkStatsEl=document.getElementById("pinkStats");
const blueWhoEl=document.getElementById("blueWho");
const pinkWhoEl=document.getElementById("pinkWho");
const redBtn=document.getElementById("redBtn");
const greenBtn=document.getElementById("greenBtn");
const blueBtn=document.getElementById("blueBtn");

function colorTr(c){
  if(c==="red") return "KIRMIZI";
  if(c==="green") return "YEŞİL";
  if(c==="blue") return "MAVİ";
  return "";
}

function roleText(r){
  if(r==="blue") return "MAVİ OYUNCU";
  if(r==="pink") return "PEMBE OYUNCU";
  return "İZLEYİCİ";
}

function paint(){
  roleEl.className="role gray";
  if(role==="blue") roleEl.className="role blueText";
  if(role==="pink") roleEl.className="role pinkText";
  roleEl.textContent=roleText(role);

  redBtn.classList.remove("disabled");
  greenBtn.classList.remove("disabled");
  blueBtn.classList.remove("disabled");

  if(role==="none" || mode==="point"){
    redBtn.classList.add("disabled");
    greenBtn.classList.add("disabled");
    blueBtn.classList.add("disabled");
  }

  targetEl.textContent="GELEN RENK: " + colorTr(ballColor);

  if(role==="none"){
    hintEl.textContent="İzleyicisin. Slot boşalırsa otomatik oyuna alınırsın.";
  }else if(mode==="point"){
    hintEl.textContent="Hasar! Yeni rally başlıyor.";
  }else if(targetSide===role && inZone){
    hintEl.textContent="ŞİMDİ BAS: " + colorTr(ballColor);
  }else if(targetSide===role){
    hintEl.textContent="Hazırlan. Top sana geliyor: " + colorTr(ballColor);
  }else{
    hintEl.textContent="Bekle. Rakip savunuyor.";
  }
}

async function join(){
  try{
    const r=await fetch("/join?cid="+encodeURIComponent(cid)+"&t="+Date.now(),{cache:"no-store"});
    const d=await r.json();
    role=d.role||"none";
    statusEl.textContent=d.message||"Bağlandı";
    paint();
  }catch(e){
    statusEl.textContent="Bağlantı yok";
  }
}

async function state(){
  try{
    const r=await fetch("/state?cid="+encodeURIComponent(cid)+"&t="+Date.now(),{cache:"no-store"});
    const d=await r.json();

    role=d.role||"none";
    targetSide=d.targetSide||"none";
    ballColor=d.ballColor||"red";
    inZone=!!d.inZone;
    mode=d.mode;

    blueStatsEl.textContent=d.blueHp + " HP · x" + d.blueCombo;
    pinkStatsEl.textContent=d.pinkHp + " HP · x" + d.pinkCombo;
    blueWhoEl.textContent=d.blueHuman ? "PLAYER" : "CPU";
    pinkWhoEl.textContent=d.pinkHuman ? "PLAYER" : "CPU";

    paint();
  }catch(e){
    statusEl.textContent="State alınamadı";
  }
}

async function fire(color){
  if(role==="none"){
    statusEl.textContent="İzleyici oynayamaz";
    return;
  }

  const now=performance.now();
  if(now-lastPress<80) return;
  lastPress=now;

  try{
    const t0=performance.now();
    const r=await fetch("/press?cid="+encodeURIComponent(cid)+"&color="+encodeURIComponent(color)+"&t="+Date.now(),{cache:"no-store"});
    const d=await r.json();
    const ms=Math.round(performance.now()-t0);
    statusEl.textContent=(d.ok ? "HIT" : d.message)+" · "+ms+" ms";
  }catch(e){
    statusEl.textContent="Basış gitmedi";
  }
}

redBtn.addEventListener("pointerdown",function(ev){ev.preventDefault();fire("red");},{passive:false});
greenBtn.addEventListener("pointerdown",function(ev){ev.preventDefault();fire("green");},{passive:false});
blueBtn.addEventListener("pointerdown",function(ev){ev.preventDefault();fire("blue");},{passive:false});

join();
setInterval(state,180);
</script>
</body>
</html>
)HTML";
}

// -------------------- Game actions --------------------

void serveFrom(uint8_t side) {
  if (side == BLUE_SIDE) {
    ballPos = 5.0f;
    ballDir = 1;
    targetSide = PINK_SIDE;
  } else {
    ballPos = LED_COUNT - 6;
    ballDir = -1;
    targetSide = BLUE_SIDE;
  }

  ballColorType = randomBallColor();
  ballCharged = false;
  resetCpuPlan();

  Serial.print("[SERVE] from ");
  Serial.print(sideName(side));
  Serial.print(" color=");
  Serial.println(colorName(ballColorType));
}

void bounceFrom(uint8_t side, bool cpu) {
  uint8_t attacker = side;

  slots[side].combo++;

  bool makeCharged = slots[side].combo >= COMBO_CHARGED_AT;

  if (side == BLUE_SIDE) {
    ballDir = 1;
    targetSide = PINK_SIDE;
    ballPos = max(ballPos, 2.0f);
  } else {
    ballDir = -1;
    targetSide = BLUE_SIDE;
    ballPos = min(ballPos, (float)(LED_COUNT - 3));
  }

  ballColorType = randomBallColor();
  ballCharged = makeCharged;

  if (ballCharged) {
    sfxCharged();
  } else {
    sfxGood(ballColorType);
  }

  resetCpuPlan();

  Serial.print(cpu ? "[CPU HIT] " : "[HIT] ");
  Serial.print(sideName(attacker));
  Serial.print(" combo=");
  Serial.print(slots[side].combo);
  Serial.print(" nextColor=");
  Serial.print(colorName(ballColorType));
  Serial.print(" charged=");
  Serial.println(ballCharged ? "yes" : "no");
}

void damageSide(uint8_t side) {
  if (slots[side].hp > 0) {
    slots[side].hp--;
  }

  slots[side].combo = 0;
  lastDamagedSide = side;
  lastPointWinner = side == BLUE_SIDE ? PINK_SIDE : BLUE_SIDE;

  Serial.print("[DAMAGE] ");
  Serial.print(sideName(side));
  Serial.print(" hp=");
  Serial.println(slots[side].hp);

  sfxDamage();

  if (slots[side].hp == 0) {
    Serial.print("[ROUND RESET] winner=");
    Serial.println(sideName(lastPointWinner));

    slots[BLUE_SIDE].hp = MAX_HP;
    slots[PINK_SIDE].hp = MAX_HP;
    slots[BLUE_SIDE].combo = 0;
    slots[PINK_SIDE].combo = 0;
  }

  mode = MODE_POINT_PAUSE;
  pointPauseUntil = millis() + POINT_PAUSE_MS;
}

void updateCpu(uint32_t now) {
  if (slots[targetSide].human) return;

  if (!isBallInHitZone(targetSide)) {
    resetCpuPlan();
    return;
  }

  if (!cpuArmed) {
    cpuArmed = true;
    cpuHitAtMs = now + CPU_REACTION_MS;

    bool correct = random(0, 100) < CPU_CORRECT_PERCENT;
    if (correct) {
      cpuPlannedColor = ballColorType;
    } else {
      cpuPlannedColor = randomBallColor();
      if (cpuPlannedColor == ballColorType) {
        cpuPlannedColor = (ballColorType + 1) % 3;
      }
    }

    return;
  }

  if (now >= cpuHitAtMs) {
    if (cpuPlannedColor == ballColorType) {
      bounceFrom(targetSide, true);
    } else {
      slots[targetSide].combo = 0;
      sfxBad();
      resetCpuPlan();
    }
  }
}

void updateSessions(uint32_t now) {
  for (int side = 0; side < 2; side++) {
    if (slots[side].human && now - slots[side].lastActionMs > HUMAN_IDLE_TIMEOUT_MS) {
      Serial.print("[IDLE] ");
      Serial.println(sideName(side));

      if (firstSpectator() >= 0) {
        promoteSpectatorToSlot(side);
      } else {
        makeCpu(side);
      }
    }
  }

  for (int i = 0; i < MAX_SPECTATORS; i++) {
    if (spectators[i].active && now - spectators[i].lastSeenMs > SPECTATOR_TIMEOUT_MS) {
      removeSpectator(i);
    }
  }
}

void updateGame(uint32_t dtMs) {
  uint32_t now = millis();

  updateSessions(now);

  if (mode == MODE_POINT_PAUSE) {
    if (now >= pointPauseUntil) {
      mode = MODE_PLAYING;
      serveFrom(lastPointWinner);
    }
    return;
  }

  float step = currentBallSpeed() * ((float)dtMs / 1000.0f);
  ballPos += ballDir * step;

  updateCpu(now);

  float head = ballHeadPos();

  if (head < 0) {
    damageSide(BLUE_SIDE);
  } else if (head > LED_COUNT - 1) {
    damageSide(PINK_SIDE);
  }
}

// -------------------- HTTP handlers --------------------

void handleRoot() {
  logRequest("root");
  sendNoCacheHeaders();
  server.send(200, "text/html", controllerPage());
}

void redirectToRoot(const char* name) {
  logRequest(name);
  sendNoCacheHeaders();

  String rootUrl = String("http://") + apIP.toString() + "/";
  server.sendHeader("Location", rootUrl, true);
  server.send(302, "text/html", "<html><body>LED Arcade</body></html>");
}

void handleCaptiveRedirect() {
  redirectToRoot("captive");
}

void handleCaptiveApi() {
  logRequest("captive_api");
  sendNoCacheHeaders();

  String body = String("{\"captive\":true,\"user-portal-url\":\"http://") + apIP.toString() + "/\"}";
  server.send(200, "application/json", body);
}

void handleNotFound() {
  redirectToRoot("not_found");
}

void handleJoin() {
  String cid = server.arg("cid");
  String ip = currentClientIp();

  if (cid.length() == 0) {
    sendJson("{\"role\":\"none\",\"message\":\"missing cid\"}");
    return;
  }

  uint32_t now = millis();

  int existingSlot = findSlotByCidOrIp(cid, ip);
  if (existingSlot >= 0) {
    slots[existingSlot].cid = cid;
    slots[existingSlot].ip = ip;
    slots[existingSlot].lastSeenMs = now;

    sendJson(String("{\"role\":\"") + sideName(existingSlot) + "\",\"message\":\"reconnected\"}");
    return;
  }

  int existingSpectator = findSpectatorByCidOrIp(cid, ip);
  if (existingSpectator >= 0) {
    spectators[existingSpectator].cid = cid;
    spectators[existingSpectator].ip = ip;
    spectators[existingSpectator].lastSeenMs = now;
  }

  int cpuSlot = firstCpuSlot();

  if (cpuSlot >= 0) {
    if (existingSpectator >= 0) {
      removeSpectator(existingSpectator);
    }

    assignHumanToSlot(cpuSlot, cid, ip);
    sendJson(String("{\"role\":\"") + sideName(cpuSlot) + "\",\"message\":\"assigned\"}");
    return;
  }

  addOrUpdateSpectator(cid, ip);
  sendJson("{\"role\":\"none\",\"message\":\"spectator queue\"}");
}

void handleState() {
  String cid = server.arg("cid");
  String ip = currentClientIp();

  uint32_t now = millis();

  int slot = findSlotByCidOrIp(cid, ip);
  int spectator = findSpectatorByCidOrIp(cid, ip);

  if (slot >= 0) {
    slots[slot].cid = cid;
    slots[slot].ip = ip;
    slots[slot].lastSeenMs = now;
  } else if (spectator >= 0) {
    spectators[spectator].cid = cid;
    spectators[spectator].ip = ip;
    spectators[spectator].lastSeenMs = now;
  }

  bool inZone = false;
  if (slot >= 0) {
    inZone = targetSide == slot && isBallInHitZone(slot);
  }

  String json =
    String("{") +
    "\"role\":\"" + sideName(slot) + "\"," +
    "\"mode\":\"" + modeName() + "\"," +
    "\"targetSide\":\"" + sideName(targetSide) + "\"," +
    "\"ballColor\":\"" + colorName(ballColorType) + "\"," +
    "\"charged\":" + String(ballCharged ? "true" : "false") + "," +
    "\"inZone\":" + String(inZone ? "true" : "false") + "," +
    "\"blueHuman\":" + String(slots[BLUE_SIDE].human ? "true" : "false") + "," +
    "\"pinkHuman\":" + String(slots[PINK_SIDE].human ? "true" : "false") + "," +
    "\"blueHp\":" + String(slots[BLUE_SIDE].hp) + "," +
    "\"pinkHp\":" + String(slots[PINK_SIDE].hp) + "," +
    "\"blueCombo\":" + String(slots[BLUE_SIDE].combo) + "," +
    "\"pinkCombo\":" + String(slots[PINK_SIDE].combo) +
    "}";

  sendJson(json);
}

uint8_t parseColorArg(const String& color) {
  if (color == "red") return C_RED;
  if (color == "green") return C_GREEN;
  if (color == "blue") return C_BLUE;
  return 99;
}

void handlePress() {
  String cid = server.arg("cid");
  String ip = currentClientIp();
  String colorArg = server.arg("color");

  int slot = findSlotByCidOrIp(cid, ip);

  if (slot < 0) {
    sendJson("{\"ok\":false,\"message\":\"not player\"}");
    return;
  }

  slots[slot].cid = cid;
  slots[slot].ip = ip;
  slots[slot].lastSeenMs = millis();
  slots[slot].lastActionMs = millis();

  uint8_t pressedColor = parseColorArg(colorArg);

  if (pressedColor > C_BLUE) {
    sendJson("{\"ok\":false,\"message\":\"bad color\"}");
    return;
  }

  if (mode != MODE_PLAYING) {
    sendJson("{\"ok\":false,\"message\":\"wait\"}");
    return;
  }

  if (targetSide != slot) {
    sfxBad();
    sendJson("{\"ok\":false,\"message\":\"not your turn\"}");
    return;
  }

  if (!isBallInHitZone(slot)) {
    sfxBad();
    sendJson("{\"ok\":false,\"message\":\"too early\"}");
    return;
  }

  if (pressedColor != ballColorType) {
    slots[slot].combo = 0;
    sfxBad();
    sendJson("{\"ok\":false,\"message\":\"wrong color\"}");
    return;
  }

  bounceFrom(slot, false);
  sendJson("{\"ok\":true,\"message\":\"hit\"}");
}

// -------------------- LED render --------------------

void addPixelSafe(int idx, CRGB c) {
  if (idx < 0 || idx >= LED_COUNT) return;
  leds[idx] += c;
}

void setPixelSafe(int idx, CRGB c) {
  if (idx < 0 || idx >= LED_COUNT) return;
  leds[idx] = c;
}

void dimSet(int idx, CRGB c, uint8_t scale) {
  c.nscale8_video(scale);
  setPixelSafe(idx, c);
}

void fadeAll(uint8_t amount) {
  for (int i = 0; i < LED_COUNT; i++) {
    leds[i].fadeToBlackBy(amount);
  }
}

void drawHpBars() {
  CRGB blue = sideColor(BLUE_SIDE);
  CRGB pink = sideColor(PINK_SIDE);

  for (int i = 0; i < MAX_HP; i++) {
    uint8_t scale = i < slots[BLUE_SIDE].hp ? 90 : 8;
    dimSet(i, blue, scale);
  }

  for (int i = 0; i < MAX_HP; i++) {
    uint8_t scale = i < slots[PINK_SIDE].hp ? 90 : 8;
    dimSet(LED_COUNT - 1 - i, pink, scale);
  }
}

void drawHitZones() {
  CRGB blue = sideColor(BLUE_SIDE);
  CRGB pink = sideColor(PINK_SIDE);

  uint8_t blueScale = targetSide == BLUE_SIDE ? 70 : 10;
  uint8_t pinkScale = targetSide == PINK_SIDE ? 70 : 10;

  for (int i = BLUE_HIT_A + MAX_HP; i <= BLUE_HIT_B; i++) {
    dimSet(i, blue, blueScale);
  }

  for (int i = PINK_HIT_A; i <= PINK_HIT_B - MAX_HP; i++) {
    dimSet(i, pink, pinkScale);
  }

  int midA = LED_COUNT / 2 - 1;
  int midB = LED_COUNT / 2;
  dimSet(midA, CRGB::White, 16);
  dimSet(midB, CRGB::White, 16);
}

void drawBall() {
  int head = (int)roundf(ballHeadPos());
  int tailLen = currentBallTailLength();

  CRGB base = gameColor(ballColorType);

  // Baş: parlak ve etkili temas noktası
  for (int i = 0; i <= BALL_HEAD_SIZE; i++) {
    int idx = ballDir > 0 ? (head - i) : (head + i);

    CRGB c = base;
    c.nscale8_video(255);

    if (ballCharged) {
      c += CRGB(70, 70, 70);
    }

    addPixelSafe(idx, c);
  }

  // Kuyruk: hareket yönünün tersine uzayan, koyudan açığa gradient
  for (int t = 1; t <= tailLen; t++) {
    int idx = ballDir > 0
      ? (head - BALL_HEAD_SIZE - t)
      : (head + BALL_HEAD_SIZE + t);

    // Başa yakın koyu/parlak, arkaya doğru daha açık/silik
    uint8_t scale;
    if (ballCharged) {
      scale = map(t, 1, tailLen, 180, 10);
    } else {
      scale = map(t, 1, tailLen, 145, 8);
    }

    CRGB c = base;
    c.nscale8_video(scale);

    // İstersen kuyruğa hafif beyaz karışım vererek badminton hissi artır
    // c += CRGB(scale / 10, scale / 10, scale / 10);

    addPixelSafe(idx, c);
  }
}

void drawPointPause() {
  fadeAll(74);

  CRGB c = sideColor(lastPointWinner);
  uint8_t pulse = (millis() / 90) % 2 ? 120 : 20;

  for (int i = 0; i < LED_COUNT; i++) {
    if (lastPointWinner == BLUE_SIDE && i < LED_COUNT / 2) {
      dimSet(i, c, pulse);
    }

    if (lastPointWinner == PINK_SIDE && i >= LED_COUNT / 2) {
      dimSet(i, c, pulse);
    }
  }
}

void renderLeds() {
  if (mode == MODE_POINT_PAUSE) {
    drawPointPause();
    FastLED.show();
    return;
  }

  fadeAll(72);
  drawHitZones();
  drawHpBars();
  drawBall();
  FastLED.show();
}

int currentBallTailLength() {
  return ballCharged ? BALL_TAIL_CHARGED : BALL_TAIL_NORMAL;
}

float ballHeadPos() {
  // Hareket yönünde öndeki gerçek temas noktası
  if (ballDir > 0) {
    // sağa gidiyor -> baş sağda
    return ballPos + BALL_HEAD_SIZE;
  } else {
    // sola gidiyor -> baş solda
    return ballPos - BALL_HEAD_SIZE;
  }
}

// -------------------- Setup / Loop --------------------

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("===================================");
  Serial.println("LED Arcade Color Rally v0.1");
  Serial.println("Rule: press the same color as incoming ball");
  Serial.println("===================================");

bool audioOk = ledcAttach(AUDIO_PIN, 1000, AUDIO_RES_BITS);
if (!audioOk) {
  Serial.println("[WARN] LEDC audio attach failed");
}
ledcWrite(AUDIO_PIN, 0);

  randomSeed(esp_random());

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, LED_COUNT);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear(true);

  makeCpu(BLUE_SIDE);
  makeCpu(PINK_SIDE);

  for (int i = 0; i < MAX_SPECTATORS; i++) {
    spectators[i].active = false;
    spectators[i].cid = "";
    spectators[i].ip = "";
    spectators[i].lastSeenMs = 0;
  }

  WiFi.mode(WIFI_AP);

  if (!WiFi.softAPConfig(apIP, gatewayIP, subnetMask)) {
    Serial.println("[ERROR] softAPConfig failed");
    return;
  }

  if (!WiFi.softAP(AP_SSID, AP_PASSWORD)) {
    Serial.println("[ERROR] AP start failed");
    return;
  }

  dnsServer.start(DNS_PORT, "*", apIP);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/join", HTTP_GET, handleJoin);
  server.on("/state", HTTP_GET, handleState);
  server.on("/press", HTTP_GET, handlePress);

  // Android
  server.on("/generate_204", HTTP_GET, handleCaptiveRedirect);
  server.on("/gen_204", HTTP_GET, handleCaptiveRedirect);
  server.on("/generate204", HTTP_GET, handleCaptiveRedirect);

  // Apple
  server.on("/hotspot-detect.html", HTTP_GET, handleCaptiveRedirect);
  server.on("/library/test/success.html", HTTP_GET, handleCaptiveRedirect);
  server.on("/success.html", HTTP_GET, handleCaptiveRedirect);
  server.on("/success.txt", HTTP_GET, handleCaptiveRedirect);
  server.on("/canonical.html", HTTP_GET, handleCaptiveRedirect);

  // Windows / generic
  server.on("/ncsi.txt", HTTP_GET, handleCaptiveRedirect);
  server.on("/connecttest.txt", HTTP_GET, handleCaptiveRedirect);
  server.on("/redirect", HTTP_GET, handleRoot);

  // Captive portal API
  server.on("/captive-portal/api", HTTP_GET, handleCaptiveApi);
  server.on("/api", HTTP_GET, handleCaptiveApi);

  server.onNotFound(handleNotFound);
  server.begin();

  serveFrom(BLUE_SIDE);

  Serial.println("[OK] Color Rally started");
  Serial.print("[INFO] SSID: ");
  Serial.println(AP_SSID);
  Serial.print("[INFO] Password: ");
  Serial.println(AP_PASSWORD);
  Serial.print("[INFO] AP IP: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("[INFO] LED_COUNT: ");
  Serial.println(LED_COUNT);
  Serial.print("[INFO] HIT_ZONE_SIZE: ");
  Serial.println(HIT_ZONE_SIZE);
  Serial.print("[INFO] BALL_HEAD_SIZE: ");
  Serial.println(BALL_HEAD_SIZE);
  Serial.print("[INFO] BALL_TAIL_NORMAL: ");
  Serial.println(BALL_TAIL_NORMAL);
  Serial.print("[INFO] BALL_TAIL_CHARGED: ");
  Serial.println(BALL_TAIL_CHARGED);
  Serial.println("[INFO] QR payload:");
  Serial.print("       WIFI:T:WPA;S:");
  Serial.print(AP_SSID);
  Serial.print(";P:");
  Serial.print(AP_PASSWORD);
  Serial.println(";;");
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();

  uint32_t now = millis();

  if (lastFrameMs == 0) {
    lastFrameMs = now;
  }

  uint32_t dtMs = now - lastFrameMs;

  if (dtMs >= 20) {
    lastFrameMs = now;
    updateGame(dtMs);
    renderLeds();
  }
}