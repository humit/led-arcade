#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include "../Config.h"
#include "../Types.h"
#include "../SystemState.h"
#include "../controller/CaptiveLaunchPage.h"
#include "../session/SessionManager.h"
#include "../games/color_rally/ColorRallyGame.h"
#include "../hardware/AudioOut.h"

class CaptivePortal {
public:
  WebServer server{80};
  DNSServer dns;

  SessionManager* sessions = nullptr;
  ColorRallyGame* game = nullptr;
  AudioOut* audio = nullptr;
  SystemState* systemState = nullptr;

  void begin(SessionManager& sessionRef, ColorRallyGame& gameRef, AudioOut& audioRef, SystemState& systemRef) {
    sessions = &sessionRef;
    game = &gameRef;
    audio = &audioRef;
    systemState = &systemRef;

    WiFi.mode(WIFI_AP);

    if (!WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET)) {
      Serial.println("[ERROR] softAPConfig failed");
      return;
    }

    bool apStarted = AP_OPEN ? WiFi.softAP(AP_SSID) : WiFi.softAP(AP_SSID, AP_PASSWORD);

    if (!apStarted) {
      Serial.println("[ERROR] AP start failed");
      return;
    }

    dns.start(DNS_PORT, "*", AP_IP);

    server.on("/", HTTP_GET, [this]() { handleRoot(); });
    server.on("/join", HTTP_GET, [this]() { handleJoin(); });
    server.on("/state", HTTP_GET, [this]() { handleState(); });
    server.on("/press", HTTP_GET, [this]() { handlePress(); });
    server.on("/game_pause", HTTP_GET, [this]() { handleGamePause(); });
    server.on("/mute", HTTP_GET, [this]() { handleMute(); });

    server.on("/generate_204", HTTP_GET, [this]() { handleCaptiveRedirect(); });
    server.on("/gen_204", HTTP_GET, [this]() { handleCaptiveRedirect(); });
    server.on("/generate204", HTTP_GET, [this]() { handleCaptiveRedirect(); });

    // Captive-network probes must receive a redirect to the portal.
    // Returning the controller with 200 OK can cause some operating systems
    // to consider the connectivity check complete without opening the CNA.
    server.on("/hotspot-detect.html", HTTP_GET, [this]() { handleCaptiveRedirect(); });
    server.on("/library/test/success.html", HTTP_GET, [this]() { handleCaptiveRedirect(); });
    server.on("/success.html", HTTP_GET, [this]() { handleCaptiveRedirect(); });
    server.on("/success.txt", HTTP_GET, [this]() { handleCaptiveRedirect(); });
    server.on("/canonical.html", HTTP_GET, [this]() { handleCaptiveRedirect(); });

    server.on("/ncsi.txt", HTTP_GET, [this]() { handleCaptiveRedirect(); });
    server.on("/connecttest.txt", HTTP_GET, [this]() { handleCaptiveRedirect(); });
    server.on("/redirect", HTTP_GET, [this]() { handleRoot(); });

    server.on("/captive-portal/api", HTTP_GET, [this]() { handleCaptiveApi(); });
    server.on("/api", HTTP_GET, [this]() { handleCaptiveApi(); });

    server.onNotFound([this]() { handleNotFound(); });

    server.begin();

    Serial.println("[OK] Captive portal started");
    Serial.print("[INFO] SSID: ");
    Serial.println(AP_SSID);
    Serial.print("[INFO] Auth: ");
    Serial.println(AP_OPEN ? "open" : "WPA");
    Serial.print("[INFO] AP IP: ");
    Serial.println(WiFi.softAPIP());
    Serial.println("[INFO] QR payload:");
    Serial.println("       WIFI:T:nopass;S:LED-Arcade;;");
  }

  void loop() {
    dns.processNextRequest();
    server.handleClient();
  }

private:
  String clientIp() {
    return server.client().remoteIP().toString();
  }

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
    Serial.println(clientIp());
  }

  void handleRoot() {
    logRequest("root");
    sendNoCacheHeaders();
    server.send_P(200, "text/html", CAPTIVE_LAUNCH_HTML);
  }

  void redirectToRoot(const char* name) {
    logRequest(name);
    sendNoCacheHeaders();

    String rootUrl = String("http://") + AP_IP.toString() + "/";
    server.sendHeader("Location", rootUrl, true);
    server.send(302, "text/html", "<html><body>LED Arcade</body></html>");
  }

  void handleCaptiveRedirect() {
    redirectToRoot("captive");
  }

  void handleCaptiveApi() {
    logRequest("captive_api");
    sendNoCacheHeaders();

    String body = String("{\"captive\":true,\"user-portal-url\":\"http://") + AP_IP.toString() + "/\"}";
    server.send(200, "application/json", body);
  }

  void handleNotFound() {
    redirectToRoot("not_found");
  }

  void handleJoin() {
    String cid = server.arg("cid");
    String ip = clientIp();

    if (cid.length() == 0) {
      sendJson("{\"role\":\"none\",\"message\":\"missing cid\"}");
      return;
    }

    uint32_t now = millis();

    int existingSlot = sessions->findSlotByCidOrIp(cid, ip);
    if (existingSlot >= 0) {
      sessions->slots[existingSlot].cid = cid;
      sessions->slots[existingSlot].ip = ip;
      sessions->slots[existingSlot].lastSeenMs = now;
      sendJson(String("{\"role\":\"") + sideName(existingSlot) + "\",\"message\":\"reconnected\"}");
      return;
    }

    int existingSpectator = sessions->findSpectatorByCidOrIp(cid, ip);
    if (existingSpectator >= 0) {
      sessions->spectators[existingSpectator].cid = cid;
      sessions->spectators[existingSpectator].ip = ip;
      sessions->spectators[existingSpectator].lastSeenMs = now;
    }

    int cpuSlot = sessions->firstCpuSlot();

    if (cpuSlot >= 0) {
      if (existingSpectator >= 0) {
        sessions->removeSpectator(existingSpectator);
      }

      sessions->assignHumanToSlot(cpuSlot, cid, ip, *audio);
      sendJson(String("{\"role\":\"") + sideName(cpuSlot) + "\",\"message\":\"assigned\"}");
      return;
    }

    sessions->addOrUpdateSpectator(cid, ip);
    sendJson("{\"role\":\"none\",\"message\":\"spectator queue\"}");
  }

  void handleState() {
    String cid = server.arg("cid");
    String ip = clientIp();
    uint32_t now = millis();

    int slot = sessions->findSlotByCidOrIp(cid, ip);
    int spectator = sessions->findSpectatorByCidOrIp(cid, ip);

    if (slot >= 0) {
      sessions->slots[slot].cid = cid;
      sessions->slots[slot].ip = ip;
      sessions->slots[slot].lastSeenMs = now;
    } else if (spectator >= 0) {
      sessions->spectators[spectator].cid = cid;
      sessions->spectators[spectator].ip = ip;
      sessions->spectators[spectator].lastSeenMs = now;
    }

    bool inZone = false;
    if (slot >= 0) {
      inZone = game->state.targetSide == slot && game->isBallInHitZone(slot);
    }

    String json =
      String("{") +
      "\"role\":\"" + sideName(slot) + "\"," +
      "\"mode\":\"" + modeName(game->state.mode) + "\"," +
      "\"targetSide\":\"" + sideName(game->state.targetSide) + "\"," +
      "\"ballColor\":\"" + colorName(game->state.ballColorType) + "\"," +
      "\"gamePaused\":" + String(systemState->gamePaused ? "true" : "false") + "," +
      "\"audioMuted\":" + String(systemState->audioMuted ? "true" : "false") + "," +
      "\"charged\":" + String(game->state.ballCharged ? "true" : "false") + "," +
      "\"inZone\":" + String(inZone ? "true" : "false") + "," +
      "\"blueHuman\":" + String(sessions->slots[BLUE_SIDE].human ? "true" : "false") + "," +
      "\"pinkHuman\":" + String(sessions->slots[PINK_SIDE].human ? "true" : "false") + "," +
      "\"blueHp\":" + String(sessions->slots[BLUE_SIDE].hp) + "," +
      "\"pinkHp\":" + String(sessions->slots[PINK_SIDE].hp) + "," +
      "\"blueCombo\":" + String(sessions->slots[BLUE_SIDE].combo) + "," +
      "\"pinkCombo\":" + String(sessions->slots[PINK_SIDE].combo) +
      "}";

    sendJson(json);
  }

  void handlePress() {
    String cid = server.arg("cid");
    String ip = clientIp();
    String colorArg = server.arg("color");

    int slot = sessions->findSlotByCidOrIp(cid, ip);

    if (slot < 0) {
      sendJson("{\"ok\":false,\"message\":\"not player\"}");
      return;
    }

    sessions->slots[slot].cid = cid;
    sessions->slots[slot].ip = ip;
    sessions->slots[slot].lastSeenMs = millis();
    sessions->slots[slot].lastActionMs = millis();

    if (systemState->gamePaused) {
      sendJson("{\"ok\":false,\"message\":\"game paused\"}");
      return;
    }

    uint8_t pressedColor = parseColorArg(colorArg);

    if (pressedColor > C_BLUE) {
      sendJson("{\"ok\":false,\"message\":\"bad color\"}");
      return;
    }

    String msg;
    bool ok = game->handlePress(slot, pressedColor, *sessions, *audio, msg);

    sendJson(String("{\"ok\":") + (ok ? "true" : "false") + ",\"message\":\"" + msg + "\"}");
  }

  void handleGamePause() {
    String pausedArg = server.arg("paused");

    if (pausedArg == "1" || pausedArg == "true") {
      systemState->gamePaused = true;
      systemState->audioMuted = true;
      audio->setMuted(true);
      Serial.println("[SYSTEM] game paused");
      sendJson("{\"ok\":true,\"gamePaused\":true,\"audioMuted\":true}");
      return;
    }

    if (pausedArg == "0" || pausedArg == "false") {
      systemState->gamePaused = false;
      Serial.println("[SYSTEM] game resumed");
      sendJson(String("{\"ok\":true,\"gamePaused\":false,\"audioMuted\":") + (systemState->audioMuted ? "true" : "false") + "}");
      return;
    }

    sendJson(String("{\"ok\":false,\"message\":\"bad paused value\",\"gamePaused\":") + (systemState->gamePaused ? "true" : "false") + "}");
  }

  void handleMute() {
    String mutedArg = server.arg("muted");

    if (mutedArg == "1" || mutedArg == "true") {
      systemState->audioMuted = true;
      audio->setMuted(true);
      Serial.println("[SYSTEM] audio muted");
      sendJson("{\"ok\":true,\"audioMuted\":true}");
      return;
    }

    if (mutedArg == "0" || mutedArg == "false") {
      systemState->audioMuted = false;
      audio->setMuted(false);
      Serial.println("[SYSTEM] audio unmuted");
      sendJson("{\"ok\":true,\"audioMuted\":false}");
      return;
    }

    sendJson(String("{\"ok\":false,\"message\":\"bad muted value\",\"audioMuted\":") + (systemState->audioMuted ? "true" : "false") + "}");
  }
};
