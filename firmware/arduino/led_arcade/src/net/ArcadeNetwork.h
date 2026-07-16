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

class ArcadeNetwork {
public:
  AsyncWebServer http{80};
  AsyncWebServer socketHttp{WS_PORT};
  DNSServer dns;
  AsyncWebSocket ws{"/"};

  void begin(
      PlayerManager& playerRef,
      ArcadeGameEngine& gameRef,
      AudioOut& audioRef
  ) {
    players = &playerRef;
    game = &gameRef;
    audio = &audioRef;

    wsEvents = xQueueCreate(WS_EVENT_QUEUE_SIZE, sizeof(WsEvent));
    if (wsEvents == nullptr) {
      Serial.println("[FATAL] Could not create WebSocket event queue");
      return;
    }

    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);
    WiFi.softAP(AP_SSID);
    dns.start(DNS_PORT, "*", AP_IP);

    http.on("/", HTTP_GET, [this](AsyncWebServerRequest* request) {
      sendRoot(request);
    });

    const char* captivePaths[] = {
        "/generate_204",
        "/gen_204",
        "/generate204",
        "/hotspot-detect.html",
        "/library/test/success.html",
        "/success.html",
        "/ncsi.txt",
        "/connecttest.txt",
        "/redirect"
    };
    for (const char* path : captivePaths) {
      http.on(path, HTTP_GET, [this](AsyncWebServerRequest* request) {
        sendRoot(request);
      });
    }

    http.onNotFound([this](AsyncWebServerRequest* request) {
      AsyncWebServerResponse* response = request->beginResponse(
          302,
          "text/plain",
          "LED Arcade"
      );
      response->addHeader(
          "Location",
          String("http://") + AP_IP.toString() + "/"
      );
      response->addHeader(
          "Cache-Control",
          "no-store, no-cache, must-revalidate, max-age=0"
      );
      request->send(response);
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

    processWsEvents();

    const uint32_t now = millis();
    players->disconnectStale(now);
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
    TEXT,
    DISCONNECTED
  };

  static constexpr size_t WS_EVENT_TEXT_MAX = 192;
  static constexpr uint8_t WS_EVENT_QUEUE_SIZE = 24;
  static constexpr uint32_t WS_CLEANUP_INTERVAL_MS = 1000;

  struct WsEvent {
    WsEventType type = WsEventType::TEXT;
    uint32_t clientId = INVALID_WS_CLIENT;
    uint8_t remoteIp[4] = {0, 0, 0, 0};
    uint16_t length = 0;
    char text[WS_EVENT_TEXT_MAX] = {};
  };

  PlayerManager* players = nullptr;
  ArcadeGameEngine* game = nullptr;
  AudioOut* audio = nullptr;
  QueueHandle_t wsEvents = nullptr;
  volatile uint32_t droppedWsEvents = 0;
  uint32_t reportedDroppedWsEvents = 0;
  uint32_t lastBroadcastMs = 0;
  uint32_t lastWsCleanupMs = 0;

  void sendRoot(AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse_P(
        200,
        "text/html",
        ARCADE_HTML
    );
    response->addHeader(
        "Cache-Control",
        "no-store, no-cache, must-revalidate, max-age=0"
    );
    request->send(response);
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

    if (type == WS_EVT_DISCONNECT) {
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

  void processWsEvents() {
    if (wsEvents == nullptr) return;

    WsEvent event;
    while (xQueueReceive(wsEvents, &event, 0) == pdTRUE) {
      if (event.type == WsEventType::DISCONNECTED) {
        players->disconnect(event.clientId, millis());
        broadcastState();
        continue;
      }

      const IPAddress remoteIp(
          event.remoteIp[0],
          event.remoteIp[1],
          event.remoteIp[2],
          event.remoteIp[3]
      );
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

  void handleWsMessage(
      uint32_t client,
      const IPAddress& remoteIp,
      const String& message
  ) {
    if (message.startsWith("HELLO|")) {
      const int slot = players->connect(
          commandArg(message),
          client,
          remoteIp,
          millis()
      );
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
    else if (message == "SELECT_GAME|reflex_rally") game->selectGame(GameId::REFLEX_RALLY, *players);
    else if (message == "SELECT_GAME|power_push") game->selectGame(GameId::POWER_PUSH, *players);
    else if (message == "SELECT_GAME|tap_clash") game->selectGame(GameId::TAP_CLASH, *players);
    else if (message.startsWith("READY|")) game->setReady(slot, commandArg(message) == "1", *players, *audio);
    else if (message == "START") game->start(slot, *players, *audio);
    else if (message == "TAP") game->tap(slot, *players, *audio);
    else if (message.startsWith("TAP_CLASH|")) {
      uint32_t targetId = 0;
      uint8_t cell = 0;
      if (parseTapClashInput(message, targetId, cell)) {
        game->tapClashCell(slot, targetId, cell, *players);
      }
    }
    else if (message == "TURN_LEFT") game->turn(slot, true, *players);
    else if (message == "TURN_RIGHT") game->turn(slot, false, *players);
    else if (message == "MOVE_UP") game->raiderMove(slot, -1, *players, *audio);
    else if (message == "MOVE_DOWN") game->raiderMove(slot, 1, *players, *audio);
    else if (message == "PONG_UP") game->pongMove(slot, -1, *players);
    else if (message == "PONG_DOWN") game->pongMove(slot, 1, *players);
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
    json.reserve(2600);
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
