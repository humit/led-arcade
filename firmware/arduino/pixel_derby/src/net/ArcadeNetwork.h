#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <WebSocketsServer.h>
#include "../Config.h"
#include "../Types.h"
#include "../controller/ArcadePage.h"
#include "../session/PlayerManager.h"
#include "../games/pixel_derby/PixelDerbyGame.h"
#include "../hardware/AudioOut.h"

class ArcadeNetwork {
public:
  WebServer http{80};
  DNSServer dns;
  WebSocketsServer ws{WS_PORT};

  void begin(PlayerManager& playerRef, PixelDerbyGame& gameRef, AudioOut& audioRef) {
    players = &playerRef;
    game = &gameRef;
    audio = &audioRef;

    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);
    WiFi.softAP(AP_SSID);
    dns.start(DNS_PORT, "*", AP_IP);

    http.on("/", HTTP_GET, [this]() { sendRoot(); });
    const char* captivePaths[] = {"/generate_204","/gen_204","/generate204","/hotspot-detect.html","/library/test/success.html","/success.html","/ncsi.txt","/connecttest.txt","/redirect"};
    for (const char* path : captivePaths) http.on(path, HTTP_GET, [this]() { sendRoot(); });
    http.onNotFound([this]() {
      http.sendHeader("Location", String("http://") + AP_IP.toString() + "/", true);
      http.send(302, "text/plain", "LED Arcade");
    });
    http.begin();

    ws.begin();
    ws.onEvent([this](uint8_t client, WStype_t type, uint8_t* payload, size_t length) {
      handleWs(client, type, payload, length);
    });

    Serial.println("[OK] LED Arcade captive portal + WebSocket started");
    Serial.print("[INFO] HTTP: http://"); Serial.println(AP_IP);
    Serial.print("[INFO] WebSocket port: "); Serial.println(WS_PORT);
  }

  void loop() {
    dns.processNextRequest();
    http.handleClient();
    ws.loop();

    const uint32_t now = millis();
    players->disconnectStale(now);
    players->expire(now);
    if (now - lastBroadcastMs >= STATE_BROADCAST_MS) {
      lastBroadcastMs = now;
      broadcastState();
    }
  }

  void broadcastState() {
    for (uint8_t client = 0; client < WEBSOCKETS_SERVER_CLIENT_MAX; client++) {
      if (ws.clientIsConnected(client)) sendState(client);
    }
  }

private:
  PlayerManager* players = nullptr;
  PixelDerbyGame* game = nullptr;
  AudioOut* audio = nullptr;
  uint32_t lastBroadcastMs = 0;

  void sendRoot() {
    http.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
    http.send_P(200, "text/html", ARCADE_HTML);
  }

  String commandArg(const String& message) {
    const int separator = message.indexOf('|');
    return separator < 0 ? "" : message.substring(separator + 1);
  }

  void handleWs(uint8_t client, WStype_t type, uint8_t* payload, size_t length) {
    if (type == WStype_DISCONNECTED) {
      players->disconnect(client, millis());
      broadcastState();
      return;
    }
    if (type != WStype_TEXT) return;

    String message;
    message.reserve(length + 1);
    for (size_t i = 0; i < length; i++) message += char(payload[i]);

    if (message.startsWith("HELLO|")) {
      const int slot = players->connect(commandArg(message), client, ws.remoteIP(client), millis());
      if (slot >= 0) audio->playerJoined();
      sendState(client);
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

    if (message == "SELECT_PLATFORM|matrix_8x32") game->selectPlatform();
    else if (message == "SELECT_GAME|pixel_derby") game->selectGame(GameId::PIXEL_DERBY, *players);
    else if (message == "SELECT_GAME|tron_arena") game->selectGame(GameId::TRON_ARENA, *players);
    else if (message == "SELECT_GAME|pixel_raider") game->selectGame(GameId::PIXEL_RAIDER, *players);
    else if (message == "SELECT_GAME|color_clash") game->selectGame(GameId::COLOR_CLASH, *players);
    else if (message.startsWith("READY|")) game->setReady(slot, commandArg(message) == "1", *players, *audio);
    else if (message == "START") game->start(slot, *players, *audio);
    else if (message == "TAP") game->tap(slot, *players, *audio);
    else if (message == "TURN_LEFT") game->turn(slot, true, *players);
    else if (message == "TURN_RIGHT") game->turn(slot, false, *players);
    else if (message == "MOVE_UP") game->raiderMove(slot, -1, *players, *audio);
    else if (message == "MOVE_DOWN") game->raiderMove(slot, 1, *players, *audio);
    else if (message == "CLASH_UP") game->clashMove(slot, TronDirection::UP, *players);
    else if (message == "CLASH_RIGHT") game->clashMove(slot, TronDirection::RIGHT, *players);
    else if (message == "CLASH_DOWN") game->clashMove(slot, TronDirection::DOWN, *players);
    else if (message == "CLASH_LEFT") game->clashMove(slot, TronDirection::LEFT, *players);
    else if (message == "WAIT|1") game->setWaiting(slot, true, *players);
    else if (message == "WAIT|0") game->setWaiting(slot, false, *players);
    else if (message == "LEAVE") { players->leave(slot); sendState(client); broadcastState(); return; }
    else if (message == "REMATCH") game->rematch(slot, *players);

    broadcastState();
  }

  void sendState(uint8_t client) {
    const int you = players->findByClient(client);
    String json;
    json.reserve(1900);
    json = "{\"stage\":\"";
    json += stageName(game->stage);
    json += "\",\"game\":\"" + String(gameName(game->selectedGame)) + "\"";
    json += ",\"you\":" + String(you);
    json += ",\"host\":" + String(players->hostSlot());
    json += ",\"connected\":" + String(players->connectedCount());
    json += ",\"active\":" + String(players->activeCount());
    json += ",\"readyCount\":" + String(players->readyCount());
    json += ",\"countdown\":" + String(game->countdownValue);
    json += ",\"winner\":" + String(game->winner);
    json += ",\"matchWinScore\":" + String(MATCH_WIN_SCORE);
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
    json += ",\"clashCounts\":[";
    for (uint8_t i = 0; i < MAX_PLAYERS; i++) { if (i) json += ','; json += String(game->clashCounts[i]); }
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
      json += ",\"tronX\":" + String(p.tronX);
      json += ",\"tronY\":" + String(p.tronY);
      json += ",\"tronAlive\":" + String(p.tronAlive ? "true" : "false");
      json += ",\"clashX\":" + String(p.clashX);
      json += ",\"clashY\":" + String(p.clashY) + "}";
    }
    json += "]}";
    ws.sendTXT(client, json);
  }
};
