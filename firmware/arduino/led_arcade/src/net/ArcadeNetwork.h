#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "../Config.h"
#include "../Types.h"
#include "../controller/ArcadePage.h"
#include "../session/PlayerManager.h"
#include "../core/ArcadeGameEngine.h"
#include "../hardware/AudioOut.h"
#include "../diagnostics/FieldDiagnostics.h"

class ArcadeNetwork {
public:
  AsyncWebServer http{80};
  AsyncWebServer socketHttp{WS_PORT};
  DNSServer dns;
  AsyncWebSocket ws{"/"};

  void begin(
      PlayerManager& playerRef,
      ArcadeGameEngine& gameRef,
      AudioOut& audioRef,
      FieldDiagnostics& diagnosticsRef
  ) {
    players = &playerRef;
    game = &gameRef;
    audio = &audioRef;
    diagnostics = &diagnosticsRef;

    wsEvents = xQueueCreate(WS_EVENT_QUEUE_SIZE, sizeof(WsEvent));
    wifiEvents = xQueueCreate(WIFI_EVENT_QUEUE_SIZE, sizeof(WifiEvent));
    if (wsEvents == nullptr || wifiEvents == nullptr) {
      Serial.println("[FATAL] Could not create network event queues");
      return;
    }

    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);
    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
      if (event == ARDUINO_EVENT_WIFI_AP_STACONNECTED) {
        enqueueWifiEvent(
            WifiEventType::CONNECTED,
            info.wifi_ap_staconnected.mac,
            info.wifi_ap_staconnected.aid,
            0
        );
      } else if (event == ARDUINO_EVENT_WIFI_AP_STADISCONNECTED) {
        enqueueWifiEvent(
            WifiEventType::DISCONNECTED,
            info.wifi_ap_stadisconnected.mac,
            info.wifi_ap_stadisconnected.aid,
            info.wifi_ap_stadisconnected.reason
        );
      }
    });
    WiFi.softAP(AP_SSID, nullptr, 1, 0, AP_MAX_CONNECTIONS);
    dns.start(DNS_PORT, "*", AP_IP);

    http.on("/", HTTP_GET, [this](AsyncWebServerRequest* request) {
      sendRoot(request);
    });

    http.on("/debug", HTTP_GET, [this](AsyncWebServerRequest* request) {
      sendDebugPage(request);
    });

    http.on("/debug.json", HTTP_GET, [this](AsyncWebServerRequest* request) {
      AsyncWebServerResponse* response = request->beginResponse(
          200,
          "application/json",
          diagnostics->summaryJson()
      );
      addNoCacheHeaders(response);
      request->send(response);
    });

    http.on("/debug.log", HTTP_GET, [this](AsyncWebServerRequest* request) {
      AsyncWebServerResponse* response = request->beginResponse(
          200,
          "text/plain; charset=utf-8",
          diagnostics->logText()
      );
      response->addHeader("Content-Disposition", "attachment; filename=led-arcade-field-test.log");
      addNoCacheHeaders(response);
      request->send(response);
    });

    const char* captivePaths[] = {
        "/generate_204",
        "/gen_204",
        "/generate204",
        "/hotspot-detect.html",
        "/library/test/success.html",
        "/success.html",
        "/success.txt",
        "/canonical.html",
        "/ncsi.txt",
        "/connecttest.txt"
    };
    for (const char* path : captivePaths) {
      http.on(path, HTTP_GET, [this](AsyncWebServerRequest* request) {
        sendCaptiveRedirect(request);
      });
    }

    http.on("/redirect", HTTP_GET, [this](AsyncWebServerRequest* request) {
      sendRoot(request);
    });

    http.on("/captive-portal/api", HTTP_GET, [this](AsyncWebServerRequest* request) {
      sendCaptiveApi(request);
    });

    http.on("/api", HTTP_GET, [this](AsyncWebServerRequest* request) {
      sendCaptiveApi(request);
    });

    http.onNotFound([this](AsyncWebServerRequest* request) {
      sendCaptiveRedirect(request);
    });

    ws.onEvent(
        [this](
            AsyncWebSocket*,
            AsyncWebSocketClient* client,
            AwsEventType type,
            void* arg,
            uint8_t* data,
            size_t length
        ) {
          enqueueWsEvent(client, type, arg, data, length);
        }
    );

    http.begin();
    socketHttp.addHandler(&ws);
    socketHttp.begin();

    Serial.println("[OK] LED Arcade captive portal + async WebSocket started");
    Serial.print("[INFO] HTTP: http://");
    Serial.println(AP_IP);
    Serial.print("[INFO] WebSocket port: ");
    Serial.println(WS_PORT);
  }

  void loop() {
    const uint32_t startedUs = micros();
    dns.processNextRequest();
    const uint32_t dnsUs = micros() - startedUs;

    processWifiEvents();
    processWsEvents();

    const uint32_t now = millis();
    for (uint8_t slot = 0; slot < MAX_PLAYERS; slot++) {
      const PlayerSlot& player = players->players[slot];
      if (!player.occupied || !player.connected || player.isCpu) continue;
      const uint32_t silentMs = now - player.lastSeenMs;
      if (silentMs >= PLAYER_HEARTBEAT_TIMEOUT_MS) {
        diagnostics->recordHeartbeatTimeout(
            player.wsClient,
            slot,
            player.remoteIp,
            silentMs
        );
      }
    }
    players->disconnectStale(now);
    for (uint8_t slot = 0; slot < MAX_PLAYERS; slot++) {
      const PlayerSlot& player = players->players[slot];
      if (!player.occupied || player.connected || player.isCpu) continue;
      const uint32_t disconnectedMs = now - player.disconnectedAtMs;
      if (disconnectedMs >= PLAYER_RECONNECT_MS) {
        diagnostics->recordExpiredSession(slot, player.remoteIp, disconnectedMs);
      }
    }
    players->expire(now);

    if (now - lastWsCleanupMs >= WS_CLEANUP_INTERVAL_MS) {
      lastWsCleanupMs = now;
      ws.cleanupClients(MAX_PLAYERS + 2);
    }

    if (now - lastBroadcastMs >= STATE_BROADCAST_MS) {
      lastBroadcastMs = now;
      broadcastState();
    }

    if (dnsUs > 50000) {
      Serial.printf(
          "[NET-BLOCK] dns=%lu us\n",
          static_cast<unsigned long>(dnsUs)
      );
    }

    if (droppedWsEvents != reportedDroppedWsEvents) {
      reportedDroppedWsEvents = droppedWsEvents;
      Serial.printf(
          "[NET-WARN] dropped WebSocket events=%lu\n",
          static_cast<unsigned long>(reportedDroppedWsEvents)
      );
      diagnostics->recordQueueDrops(reportedDroppedWsEvents + reportedDroppedWifiEvents);
    }

    if (droppedWifiEvents != reportedDroppedWifiEvents) {
      reportedDroppedWifiEvents = droppedWifiEvents;
      Serial.printf(
          "[NET-WARN] dropped Wi-Fi events=%lu\n",
          static_cast<unsigned long>(reportedDroppedWifiEvents)
      );
      diagnostics->recordQueueDrops(reportedDroppedWsEvents + reportedDroppedWifiEvents);
    }
  }

  void broadcastState() {
    for (uint8_t slot = 0; slot < MAX_PLAYERS; ++slot) {
      const PlayerSlot& player = players->players[slot];
      if (!player.occupied ||
          !player.connected ||
          player.isCpu ||
          player.wsClient == INVALID_WS_CLIENT) {
        continue;
      }

      if (!ws.hasClient(player.wsClient) ||
          !ws.availableForWrite(player.wsClient)) {
        continue;
      }

      sendState(player.wsClient);
    }
  }

private:
  enum class WsEventType : uint8_t {
    CONNECTED,
    TEXT,
    DISCONNECTED
  };

  enum class WifiEventType : uint8_t {
    CONNECTED,
    DISCONNECTED
  };

  static constexpr size_t WS_EVENT_TEXT_MAX = 512;
  static constexpr uint8_t WS_EVENT_QUEUE_SIZE = 24;
  static constexpr uint8_t WIFI_EVENT_QUEUE_SIZE = 16;
  static constexpr uint32_t WS_CLEANUP_INTERVAL_MS = 1000;

  struct WsEvent {
    WsEventType type = WsEventType::TEXT;
    uint32_t clientId = INVALID_WS_CLIENT;
    uint8_t remoteIp[4] = {0, 0, 0, 0};
    uint16_t length = 0;
    char text[WS_EVENT_TEXT_MAX] = {};
  };

  struct WifiEvent {
    WifiEventType type = WifiEventType::CONNECTED;
    uint8_t mac[6] = {};
    uint8_t aid = 0;
    uint8_t reason = 0;
  };

  PlayerManager* players = nullptr;
  ArcadeGameEngine* game = nullptr;
  AudioOut* audio = nullptr;
  FieldDiagnostics* diagnostics = nullptr;
  QueueHandle_t wsEvents = nullptr;
  QueueHandle_t wifiEvents = nullptr;
  volatile uint32_t droppedWsEvents = 0;
  volatile uint32_t droppedWifiEvents = 0;
  uint32_t reportedDroppedWsEvents = 0;
  uint32_t reportedDroppedWifiEvents = 0;
  uint32_t lastBroadcastMs = 0;
  uint32_t lastWsCleanupMs = 0;

  void sendRoot(AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse_P(
        200,
        "text/html",
        ARCADE_HTML
    );
    addNoCacheHeaders(response);
    request->send(response);
  }

  void sendCaptiveRedirect(AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse(
        302,
        "text/html",
        "<html><body>LED Arcade</body></html>"
    );
    response->addHeader("Location", String("http://") + AP_IP.toString() + "/");
    addNoCacheHeaders(response);
    request->send(response);
  }

  void sendCaptiveApi(AsyncWebServerRequest* request) {
    const String body = String("{\"captive\":true,\"user-portal-url\":\"http://") +
        AP_IP.toString() + "/\"}";
    AsyncWebServerResponse* response = request->beginResponse(
        200,
        "application/json",
        body
    );
    addNoCacheHeaders(response);
    request->send(response);
  }

  void sendDebugPage(AsyncWebServerRequest* request) {
    static const char DEBUG_PAGE[] PROGMEM = R"HTML(
<!doctype html><html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><meta http-equiv="refresh" content="5"><title>LED Arcade Field Test</title><style>body{font:14px system-ui;background:#080a12;color:#f4f6ff;margin:0;padding:18px}h1{font-size:22px}a{color:#8fb3ff}.card{background:#141827;border:1px solid #303854;border-radius:14px;padding:14px;margin:12px 0}pre{white-space:pre-wrap;word-break:break-word;font-size:12px}</style></head><body><h1>LED Arcade Field Test</h1><div class="card"><a href="/debug.json">Open JSON summary</a> · <a href="/debug.log" download>Download event log</a></div><div class="card"><pre id="out">Loading…</pre></div><script>fetch('/debug.json',{cache:'no-store'}).then(r=>r.json()).then(v=>out.textContent=JSON.stringify(v,null,2)).catch(e=>out.textContent=String(e))</script></body></html>
)HTML";
    AsyncWebServerResponse* response = request->beginResponse_P(200, "text/html", DEBUG_PAGE);
    addNoCacheHeaders(response);
    request->send(response);
  }

  void addNoCacheHeaders(AsyncWebServerResponse* response) {
    response->addHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
  }

  void enqueueWsEvent(
      AsyncWebSocketClient* client,
      AwsEventType type,
      void* arg,
      const uint8_t* data,
      size_t length
  ) {
    if (client == nullptr || wsEvents == nullptr) return;

    WsEvent event;
    event.clientId = client->id();

    const IPAddress remoteIp = client->remoteIP();
    for (uint8_t i = 0; i < 4; ++i) {
      event.remoteIp[i] = remoteIp[i];
    }

    if (type == WS_EVT_CONNECT) {
      event.type = WsEventType::CONNECTED;
    } else if (type == WS_EVT_DISCONNECT) {
      event.type = WsEventType::DISCONNECTED;
    } else if (type == WS_EVT_DATA) {
      const AwsFrameInfo* info = static_cast<const AwsFrameInfo*>(arg);
      if (info == nullptr ||
          !info->final ||
          info->index != 0 ||
          info->len != length ||
          info->opcode != WS_TEXT) {
        return;
      }

      if (length == 0 || length >= WS_EVENT_TEXT_MAX) {
        ++droppedWsEvents;
        return;
      }

      event.type = WsEventType::TEXT;
      event.length = static_cast<uint16_t>(length);
      memcpy(event.text, data, length);
      event.text[length] = '\0';
    } else {
      return;
    }

    if (xQueueSend(wsEvents, &event, 0) != pdTRUE) {
      ++droppedWsEvents;
    }
  }

  void enqueueWifiEvent(
      WifiEventType type,
      const uint8_t* mac,
      uint8_t aid,
      uint8_t reason
  ) {
    if (wifiEvents == nullptr) return;
    WifiEvent event;
    event.type = type;
    if (mac != nullptr) memcpy(event.mac, mac, sizeof(event.mac));
    event.aid = aid;
    event.reason = reason;
    if (xQueueSend(wifiEvents, &event, 0) != pdTRUE) ++droppedWifiEvents;
  }

  void processWifiEvents() {
    if (wifiEvents == nullptr) return;
    WifiEvent event;
    while (xQueueReceive(wifiEvents, &event, 0) == pdTRUE) {
      if (event.type == WifiEventType::CONNECTED) {
        diagnostics->recordWifiConnect(event.mac, event.aid);
      } else {
        diagnostics->recordWifiDisconnect(event.mac, event.aid, event.reason);
      }
    }
  }

  void processWsEvents() {
    if (wsEvents == nullptr) return;

    WsEvent event;
    while (xQueueReceive(wsEvents, &event, 0) == pdTRUE) {
      const IPAddress remoteIp(
          event.remoteIp[0],
          event.remoteIp[1],
          event.remoteIp[2],
          event.remoteIp[3]
      );
      if (event.type == WsEventType::CONNECTED) {
        diagnostics->recordWsConnect(event.clientId, remoteIp, ws.count());
        continue;
      }
      if (event.type == WsEventType::DISCONNECTED) {
        const int slot = players->findByClient(event.clientId);
        diagnostics->recordWsDisconnect(event.clientId, remoteIp, slot);
        players->disconnect(event.clientId, millis());
        broadcastState();
        continue;
      }

      handleWsMessage(
          event.clientId,
          remoteIp,
          String(event.text)
      );
    }
  }

  String commandArg(const String& message) {
    const int separator = message.indexOf('|');
    return separator < 0 ? "" : message.substring(separator + 1);
  }

  bool parseTapClashInput(
      const String& message,
      uint32_t& targetId,
      uint8_t& cell
  ) {
    const int first = message.indexOf('|');
    const int second = message.indexOf('|', first + 1);
    if (first < 0 || second < 0) return false;

    const String targetPart = message.substring(first + 1, second);
    const String cellPart = message.substring(second + 1);
    if (targetPart.length() == 0 || cellPart.length() == 0) return false;

    const long parsedCell = cellPart.toInt();
    if (parsedCell < 0 || parsedCell >= TAP_CLASH_GRID_CELLS) return false;
    targetId = strtoul(targetPart.c_str(), nullptr, 10);
    cell = uint8_t(parsedCell);
    return true;
  }

  bool parseBrainDuelInput(
      const String& message,
      uint32_t& questionId,
      uint8_t& answerIndex
  ) {
    const int first = message.indexOf('|');
    const int second = message.indexOf('|', first + 1);
    if (first < 0 || second < 0) return false;

    const String questionPart = message.substring(first + 1, second);
    const String answerPart = message.substring(second + 1);
    if (questionPart.length() == 0 || answerPart.length() == 0) return false;

    const long parsedAnswer = answerPart.toInt();
    if (parsedAnswer < 0 || parsedAnswer >= BRAIN_DUEL_OPTION_COUNT) return false;
    questionId = strtoul(questionPart.c_str(), nullptr, 10);
    answerIndex = uint8_t(parsedAnswer);
    return true;
  }

  void appendJsonString(String& json, const String& value) {
    json += '"';
    for (size_t i = 0; i < value.length(); i++) {
      const char c = value[i];
      if (c == '"' || c == '\\') {
        json += '\\';
        json += c;
      } else if (c == '\n') {
        json += "\\n";
      } else if (c == '\r') {
        json += "\\r";
      } else if (c == '\t') {
        json += "\\t";
      } else if (uint8_t(c) >= 0x20) {
        json += c;
      }
    }
    json += '"';
  }

  void handleWsMessage(
      uint32_t client,
      const IPAddress& remoteIp,
      const String& message
  ) {
    if (message.startsWith("HELLO|")) {
      const String cid = commandArg(message);
      const int slot = players->connect(
          cid,
          client,
          remoteIp,
          millis()
      );
      diagnostics->recordHello(client, slot, remoteIp, cid);
      if (slot >= 0 && game->selectedArena != ArenaType::SCREEN_ARCADE) audio->playerJoined();
      sendState(client);
      broadcastState();
      return;
    }

    if (message == "BACK_TO_PLATFORMS") {
      game->backToPlatforms(*players);
      broadcastState();
      return;
    }

    if (message == "BACK_TO_GAMES") {
      game->backToGames(*players);
      broadcastState();
      return;
    }

    const int slot = players->findByClient(client);
    if (slot < 0) return;
    players->touch(client, millis());
    diagnostics->touchClient(client);

    if (message.startsWith("CLIENT_INFO|")) {
      diagnostics->recordClientInfo(client, slot, remoteIp, commandArg(message));
      return;
    }

    if (message.startsWith("DIAG_GESTURE|")) {
      diagnostics->recordGesture(client, slot, commandArg(message));
      return;
    }

    if (message.startsWith("CLIENT_EVENT|")) {
      diagnostics->recordClientEvent(client, slot, commandArg(message));
      return;
    }

    if (message == "PING") {
      sendState(client);
      return;
    }

    if (message == "SELECT_PLATFORM|matrix_8x32") game->selectPlatform(ArenaType::MATRIX_8X32);
    else if (message == "SELECT_PLATFORM|strip_1d") game->selectPlatform(ArenaType::STRIP_1D);
    else if (message == "SELECT_PLATFORM|screen_arcade") game->selectPlatform(ArenaType::SCREEN_ARCADE);
    else if (message == "SELECT_GAME|pixel_derby") game->selectGame(GameId::PIXEL_DERBY, *players);
    else if (message == "SELECT_GAME|tron_arena") game->selectGame(GameId::TRON_ARENA, *players);
    else if (message == "SELECT_GAME|pixel_raider") game->selectGame(GameId::PIXEL_RAIDER, *players);
    else if (message == "SELECT_GAME|color_clash") game->selectGame(GameId::COLOR_CLASH, *players);
    else if (message == "SELECT_GAME|pixel_pong") game->selectGame(GameId::PIXEL_PONG, *players);
    else if (message == "SELECT_GAME|stack_shift") game->selectGame(GameId::STACK_SHIFT, *players);
    else if (message == "SELECT_GAME|reflex_rally") game->selectGame(GameId::REFLEX_RALLY, *players);
    else if (message == "SELECT_GAME|power_push") game->selectGame(GameId::POWER_PUSH, *players);
    else if (message.startsWith("READY|")) game->setReady(slot, commandArg(message) == "1", *players, *audio);
    else if (message == "START") game->start(slot, *players, *audio);
    else if (message == "TAP") game->tap(slot, *players, *audio);
    else if (message == "TURN_LEFT") game->turn(slot, true, *players);
    else if (message == "TURN_RIGHT") game->turn(slot, false, *players);
    else if (message == "MOVE_UP") game->raiderMove(slot, -1, *players, *audio);
    else if (message == "MOVE_DOWN") game->raiderMove(slot, 1, *players, *audio);
    else if (message == "PONG_UP") game->pongMove(slot, -1, *players);
    else if (message == "PONG_DOWN") game->pongMove(slot, 1, *players);
    else if (message == "STACK_LEFT") game->stackInput(slot, StackShiftInput::LEFT, *players, *audio);
    else if (message == "STACK_RIGHT") game->stackInput(slot, StackShiftInput::RIGHT, *players, *audio);
    else if (message == "STACK_ROTATE") game->stackInput(slot, StackShiftInput::ROTATE, *players, *audio);
    else if (message == "STACK_SOFT_DROP") game->stackInput(slot, StackShiftInput::SOFT_DROP, *players, *audio);
    else if (message == "STACK_HARD_DROP") game->stackInput(slot, StackShiftInput::HARD_DROP, *players, *audio);
    else if (message == "STACK_PAUSE") game->stackPause(slot, *players, *audio);
    else if (message == "CLASH_UP") game->clashMove(slot, TronDirection::UP, *players);
    else if (message == "CLASH_RIGHT") game->clashMove(slot, TronDirection::RIGHT, *players);
    else if (message == "CLASH_DOWN") game->clashMove(slot, TronDirection::DOWN, *players);
    else if (message == "CLASH_LEFT") game->clashMove(slot, TronDirection::LEFT, *players);
    else if (message == "WAIT|1") game->setWaiting(slot, true, *players);
    else if (message == "WAIT|0") game->setWaiting(slot, false, *players);
    else if (message == "LEAVE") {
      players->leave(slot);
      sendState(client);
      broadcastState();
      return;
    }
    else if (message == "REMATCH") game->rematch(slot, *players);

    broadcastState();
  }

  void sendState(uint32_t client) {
    if (!ws.hasClient(client) || !ws.availableForWrite(client)) return;

    const int you = players->findByClient(client);
    String json;
    json.reserve(5200);
    json = "{\"stage\":\"";
    json += stageName(game->stage);
    json += "\",\"arena\":\"" + String(arenaName(game->selectedArena)) + "\"";
    json += ",\"game\":\"" + String(gameName(game->selectedGame)) + "\"";
    json += ",\"you\":" + String(you);
    json += ",\"host\":" + String(players->hostSlot());
    json += ",\"connected\":" + String(players->connectedCount());
    json += ",\"active\":" + String(players->activeCount());
    json += ",\"readyCount\":" + String(players->readyCount());
    json += ",\"countdown\":" + String(game->countdownValue);
    json += ",\"winner\":" + String(game->winner);
    json += ",\"matchWinScore\":" + String(MATCH_WIN_SCORE);
    json += ",\"screenTargetPlayers\":" + String(SCREEN_ARCADE_PLAYER_COUNT);
    json += ",\"tapClashTarget\":" + String(game->tapClash.targetCell == TapClashGame::NO_CELL ? -1 : int(game->tapClash.targetCell));
    json += ",\"tapClashTargetId\":" + String(game->tapClash.targetId);
    json += ",\"tapClashRemainingMs\":" + String(game->tapClashRemainingMs());
    json += ",\"tapClashTargetRemainingMs\":" + String(game->tapClashTargetRemainingMs());
    json += ",\"tapClashEventId\":" + String(game->tapClash.eventId);
    json += ",\"tapClashEventType\":" + String(game->tapClash.eventType);
    json += ",\"tapClashEventSlot\":" + String(game->tapClash.eventSlot);
    json += ",\"brainQuestionId\":" + String(game->brainDuel.questionId);
    json += ",\"brainQuestionNumber\":" + String(game->brainDuel.questionNumber);
    json += ",\"brainTotalQuestions\":" + String(BRAIN_DUEL_QUESTION_COUNT);
    json += ",\"brainPhase\":" + String(uint8_t(game->brainDuel.phase));
    json += ",\"brainKind\":" + String(uint8_t(game->brainDuel.kind));
    json += ",\"brainDifficulty\":" + String(uint8_t(game->brainDuel.difficulty));
    json += ",\"brainRemainingMs\":" + String(game->brainDuelRemainingMs());
    json += ",\"brainCorrectIndex\":" + String(game->brainDuel.visibleCorrectIndex());
    json += ",\"brainEventId\":" + String(game->brainDuel.eventId);
    json += ",\"brainEventType\":" + String(game->brainDuel.eventType);
    json += ",\"brainEventSlot\":" + String(game->brainDuel.eventSlot);
    json += ",\"brainEventCorrect\":" + String(game->brainDuel.eventCorrect ? "true" : "false");
    json += ",\"brainPromptTr\":";
    appendJsonString(json, game->brainDuel.promptTr);
    json += ",\"brainPromptEn\":";
    appendJsonString(json, game->brainDuel.promptEn);
    json += ",\"brainOptionsTr\":[";
    for (uint8_t i = 0; i < BRAIN_DUEL_OPTION_COUNT; i++) {
      if (i) json += ',';
      appendJsonString(json, game->brainDuel.optionsTr[i]);
    }
    json += "]";
    json += ",\"brainOptionsEn\":[";
    for (uint8_t i = 0; i < BRAIN_DUEL_OPTION_COUNT; i++) {
      if (i) json += ',';
      appendJsonString(json, game->brainDuel.optionsEn[i]);
    }
    json += "]";
    json += ",\"deviceBestMs\":" + String(game->deviceBestRaceMs);
    json += ",\"newDeviceRecord\":" + String(game->newDeviceRecord ? "true" : "false");
    json += ",\"racesSinceBoss\":" + String(game->racesSinceBoss);
    json += ",\"bossSlot\":" + String(game->bossSlot);
    json += ",\"bossHp\":" + String(game->bossHp);
    json += ",\"bossMaxHp\":" + String(game->bossMaxHp);
    json += ",\"bossRemainingMs\":" + String(game->bossRemainingMs());
    json += ",\"bossDefeated\":" + String(game->bossDefeated ? "true" : "false");
    json += ",\"bossPulseCount\":" + String(game->bossPulseCount);
    json += ",\"announcePhase\":" + String(game->announcePhase);
    json += ",\"displayLanguage\":\"" + String(DISPLAY_LANGUAGE_TR ? "tr" : "en") + "\"";
    json += ",\"topRaiderSlot\":" + String(game->topRaiderSlot);
    json += ",\"clashRemainingMs\":" + String(game->clashRemainingMs());
    json += ",\"stripRally\":" + String(game->stripRally);
    json += ",\"stripMarker\":" + String(game->stripMarker);
    json += ",\"stripRemainingMs\":" + String(game->stripRemainingMs());
    json += ",\"pongLeftSlot\":" + String(game->pong.leftSlot);
    json += ",\"pongRightSlot\":" + String(game->pong.rightSlot);
    json += ",\"pongLeftScore\":" + String(game->pong.leftScore);
    json += ",\"pongRightScore\":" + String(game->pong.rightScore);
    json += ",\"pongPointPause\":" + String(game->pong.pointPause ? "true" : "false");
    json += ",\"stackScore\":" + String(game->stack.score);
    json += ",\"stackBestScore\":" + String(game->stack.bestScore);
    json += ",\"stackLines\":" + String(game->stack.clearedLines);
    json += ",\"stackLevel\":" + String(game->stack.level);
    json += ",\"stackNextPiece\":" + String(game->stack.nextPiece);
    json += ",\"stackPaused\":" + String(game->stack.paused ? "true" : "false");
    json += ",\"stackClearActive\":" + String(game->stack.lineClearActive ? "true" : "false");
    json += ",\"stackClearCount\":" + String(game->stack.lineClearCount);
    json += ",\"stackNewRecord\":" + String(game->stack.newRecord ? "true" : "false");
    json += ",\"stackGhostPieces\":" + String(game->stack.ghostPiecesRemaining);
    json += ",\"stackGhostAward\":" + String(game->stack.ghostAwardPieces);
    json += ",\"stackClearStreak\":" + String(game->stack.lineClearStreak);
    json += ",\"stackLevelBreak\":" + String(game->stack.levelBreakActive ? "true" : "false");
    json += ",\"stackLevelIntro\":" + String(game->stack.levelBreakIntroActive(millis()) ? "true" : "false");
    json += ",\"stackLevelBonus\":" + String(game->stack.levelBreakBonus);
    json += ",\"stackLevelEmptyRows\":" + String(game->stack.levelBreakEmptyRows);
    json += ",\"stackLevelScannedRows\":" + String(game->stack.levelBreakScannedRows(millis()));
    json += ",\"stackLevelScannedBonus\":" + String(game->stack.levelBreakScannedBonus(millis()));
    json += ",\"stackPerfectClear\":" + String(game->stack.perfectClearActive ? "true" : "false");
    json += ",\"stackPerfectClearBonus\":" + String(game->stack.perfectClearBonus);
    json += ",\"stackPerfectClearGhostAward\":" + String(game->stack.perfectClearGhostAward);
    json += ",\"clashCounts\":[";
    for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
      if (i) json += ',';
      json += String(game->clashCounts[i]);
    }
    json += "]";
    json += ",\"players\":[";

    bool first = true;
    for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
      const PlayerSlot& p = players->players[i];
      if (!p.occupied) continue;
      if (!first) json += ',';
      first = false;
      json += "{\"slot\":" + String(i);
      json += ",\"connected\":" + String(p.connected ? "true" : "false");
      json += ",\"isCpu\":" + String(p.isCpu ? "true" : "false");
      json += ",\"ready\":" + String(p.ready ? "true" : "false");
      json += ",\"waiting\":" + String(p.waiting ? "true" : "false");
      json += ",\"position\":" + String(p.position);
      json += ",\"score\":" + String(p.score);
      json += ",\"points\":" + String(p.totalPoints);
      json += ",\"wins\":" + String(p.wins);
      json += ",\"streak\":" + String(p.streak);
      json += ",\"bestStreak\":" + String(p.bestStreak);
      json += ",\"personalBestMs\":" + String(p.personalBestMs);
      json += ",\"lastRaceMs\":" + String(p.lastRaceMs);
      json += ",\"newPersonalRecord\":" + String(p.newPersonalRecord ? "true" : "false");
      json += ",\"turboTaps\":" + String(p.turboTaps);
      json += ",\"bossDamage\":" + String(p.bossDamage);
      json += ",\"bossCandidateScore\":" + String(p.bossCandidateScore);
      json += ",\"bossEnergy\":" + String(p.bossEnergy);
      json += ",\"stunnedMs\":" + String(game->stunRemainingMs(p));
      json += ",\"tapClashLockedMs\":" + String(game->tapClashLockRemainingMs(i));
      json += ",\"brainAnswered\":" + String(game->brainDuel.answered[i] ? "true" : "false");
      json += ",\"brainAnswerCorrect\":" + String(game->brainDuel.answerCorrect[i] ? "true" : "false");
      json += ",\"brainSelectedAnswer\":" + String(game->brainDuel.selectedAnswer[i] == BrainDuelGame::NO_ANSWER ? -1 : int(game->brainDuel.selectedAnswer[i]));
      json += ",\"tronX\":" + String(p.tronX);
      json += ",\"tronY\":" + String(p.tronY);
      json += ",\"tronAlive\":" + String(p.tronAlive ? "true" : "false");
      json += ",\"clashX\":" + String(p.clashX);
      json += ",\"clashY\":" + String(p.clashY) + "}";
    }
    json += "]}";

    ws.text(client, json);
  }
};
