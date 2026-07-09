#pragma once

#include <Arduino.h>
#include <WebSocketsServer.h>

#include "../Config.h"
#include "../Types.h"
#include "../session/SessionManager.h"
#include "../games/motion_duel/MotionDuelGame.h"

class RealtimeTransport {
public:
  RealtimeTransport()
    : webSocket(WS_PORT) {
    resetClients();
  }

  void begin(
    SessionManager& sessionManager,
    MotionDuelGame& motionGame
  ) {
    sessions = &sessionManager;
    motion = &motionGame;

    webSocket.begin();

    webSocket.onEvent(
      [this](
        uint8_t clientNum,
        WStype_t type,
        uint8_t* payload,
        size_t length
      ) {
        handleEvent(clientNum, type, payload, length);
      }
    );

    Serial.print("[OK] WebSocket started on port ");
    Serial.println(WS_PORT);
  }

  void loop() {
    webSocket.loop();
  }

private:
  static const uint8_t MAX_WS_CLIENTS = 8;

  struct ClientBinding {
    bool connected = false;
    String cid = "";
    int8_t side = -1;
  };

  WebSocketsServer webSocket;
  SessionManager* sessions = nullptr;
  MotionDuelGame* motion = nullptr;
  ClientBinding clients[MAX_WS_CLIENTS];

  void resetClients() {
    for (uint8_t i = 0; i < MAX_WS_CLIENTS; i++) {
      clients[i] = ClientBinding();
    }
  }

  void handleEvent(
    uint8_t clientNum,
    WStype_t type,
    uint8_t* payload,
    size_t length
  ) {
    if (clientNum >= MAX_WS_CLIENTS) return;

    switch (type) {
      case WStype_CONNECTED:
        handleConnected(clientNum);
        break;

      case WStype_DISCONNECTED:
        handleDisconnected(clientNum);
        break;

      case WStype_TEXT:
        handleText(clientNum, payload, length);
        break;

      case WStype_PING:
        break;

      case WStype_PONG:
        break;

      default:
        break;
    }
  }

  void handleConnected(uint8_t clientNum) {
    clients[clientNum] = ClientBinding();
    clients[clientNum].connected = true;

    Serial.print("[WS] connected #");
    Serial.print(clientNum);
    Serial.print(" ip=");
    Serial.println(webSocket.remoteIP(clientNum));
  }

  void handleDisconnected(uint8_t clientNum) {
    const int8_t side = clients[clientNum].side;

    if (motion != nullptr && side >= 0 && side <= PINK_SIDE) {
      motion->clearForce((uint8_t)side);
    }

    Serial.print("[WS] disconnected #");
    Serial.print(clientNum);

    if (clients[clientNum].cid.length() > 0) {
      Serial.print(" cid=");
      Serial.print(clients[clientNum].cid);
    }

    Serial.println();

    clients[clientNum] = ClientBinding();
  }

  void handleText(
    uint8_t clientNum,
    uint8_t* payload,
    size_t length
  ) {
    String message;

    message.reserve(length);

    for (size_t i = 0; i < length; i++) {
      message += (char)payload[i];
    }

    if (message.startsWith("HELLO|")) {
      bindClient(clientNum, message.substring(6));
      return;
    }

    if (message.startsWith("MOTION|")) {
      handleMotion(clientNum, message.substring(7));
      return;
    }

    if (message == "PING") {
      webSocket.sendTXT(clientNum, "PONG");
      return;
    }

    Serial.print("[WS] unknown message #");
    Serial.print(clientNum);
    Serial.print(": ");
    Serial.println(message);
  }

  void bindClient(uint8_t clientNum, String cid) {
    cid.trim();

    if (cid.length() == 0 || sessions == nullptr) {
      webSocket.sendTXT(clientNum, "ROLE|none");
      return;
    }

    clients[clientNum].cid = cid;

    const String remoteIp = webSocket.remoteIP(clientNum).toString();

    int side = sessions->findSlotByCidOrIp(cid, remoteIp);

    if (side < 0) {
      // HTTP /join may have stored a different representation of the IP.
      // CID remains the authoritative reconnect identity.
      side = sessions->findSlotByCidOrIp(cid, "");
    }

    clients[clientNum].side = side;

    if (side >= 0 && side <= PINK_SIDE) {
      sessions->slots[side].lastSeenMs = millis();

      String response = "ROLE|";
      response += sideName((uint8_t)side);

      webSocket.sendTXT(clientNum, response);

      Serial.print("[WS] bind #");
      Serial.print(clientNum);
      Serial.print(" cid=");
      Serial.print(cid);
      Serial.print(" role=");
      Serial.println(sideName((uint8_t)side));
    } else {
      webSocket.sendTXT(clientNum, "ROLE|none");

      Serial.print("[WS] spectator #");
      Serial.print(clientNum);
      Serial.print(" cid=");
      Serial.println(cid);
    }
  }

  void handleMotion(uint8_t clientNum, String valueText) {
    if (
      sessions == nullptr ||
      motion == nullptr ||
      !clients[clientNum].connected
    ) {
      return;
    }

    const int8_t side = clients[clientNum].side;

    if (side < 0 || side > PINK_SIDE) {
      return;
    }

    valueText.trim();

    const float value = constrain(
      valueText.toFloat(),
      -1.0f,
      1.0f
    );

    const uint32_t now = millis();

    sessions->slots[side].lastSeenMs = now;
    sessions->slots[side].lastActionMs = now;

    motion->setForce((uint8_t)side, value, now);
  }
};
