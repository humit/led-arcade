#pragma once

#include <Arduino.h>
#include <esp_https_server.h>
#include <esp_http_server.h>
#include <esp_heap_caps.h>

#include "../Config.h"
#include "../Types.h"
#include "../controller/ControllerPage.h"
#include "../generated/TlsCredentials.h"
#include "../session/SessionManager.h"
#include "../games/motion_duel/MotionDuelGame.h"
#include "../hardware/AudioOut.h"

class SecureMotionServer {
public:
  bool begin(
    SessionManager& sessionManager,
    MotionDuelGame& motionGame,
    AudioOut& audioOut
  ) {
    sessions = &sessionManager;
    motion = &motionGame;
    audio = &audioOut;
    instance = this;

    httpd_ssl_config_t config = HTTPD_SSL_CONFIG_DEFAULT();
    config.port_secure = 444;
    config.httpd.max_uri_handlers = 4;
    // One short-lived HTTPS page request plus one long-lived WSS connection.
    // Keeping this low avoids several simultaneous TLS handshakes exhausting
    // the ESP32 internal heap.
    config.httpd.max_open_sockets = 2;
    config.httpd.backlog_conn = 2;
    config.httpd.lru_purge_enable = true;
    config.httpd.recv_wait_timeout = 5;
    config.httpd.send_wait_timeout = 5;
    config.servercert = reinterpret_cast<const uint8_t*>(TLS_CERT_PEM);
    config.servercert_len = TLS_CERT_PEM_LEN;
    config.prvtkey_pem = reinterpret_cast<const uint8_t*>(TLS_PRIVATE_KEY_PEM);
    config.prvtkey_len = TLS_PRIVATE_KEY_PEM_LEN;

    const esp_err_t result = httpd_ssl_start(&server, &config);

    if (result != ESP_OK) {
      Serial.print("[ERROR] HTTPS server start failed: ");
      Serial.println(esp_err_to_name(result));
      server = nullptr;
      return false;
    }

    httpd_uri_t rootUri = {};
    rootUri.uri = "/";
    rootUri.method = HTTP_GET;
    rootUri.handler = handleRootStatic;
    rootUri.user_ctx = this;

    httpd_uri_t wsUri = {};
    wsUri.uri = "/ws";
    wsUri.method = HTTP_GET;
    wsUri.handler = handleWebSocketStatic;
    wsUri.user_ctx = this;
    wsUri.is_websocket = true;
    wsUri.handle_ws_control_frames = true;

    esp_err_t registerResult = httpd_register_uri_handler(server, &rootUri);
    if (registerResult == ESP_OK) {
      registerResult = httpd_register_uri_handler(server, &wsUri);
    }

    if (registerResult != ESP_OK) {
      Serial.print("[ERROR] HTTPS route registration failed: ");
      Serial.println(esp_err_to_name(registerResult));
      httpd_ssl_stop(server);
      server = nullptr;
      return false;
    }

    logHeap("HTTPS ready");
    Serial.println("[OK] HTTPS motion controller started");
    Serial.println("[INFO] URL: https://play.nevershire.net:444/");
    Serial.println("[INFO] WSS: wss://play.nevershire.net:444/ws");
    return true;
  }

private:
  static constexpr size_t MAX_MESSAGE_SIZE = 128;

  static void logHeap(const char* label) {
    Serial.print("[HEAP] ");
    Serial.print(label);
    Serial.print(" free=");
    Serial.print(ESP.getFreeHeap());
    Serial.print(" largest=");
    Serial.print(
      heap_caps_get_largest_free_block(MALLOC_CAP_8BIT)
    );
    Serial.print(" min=");
    Serial.println(ESP.getMinFreeHeap());
  }

  httpd_handle_t server = nullptr;
  SessionManager* sessions = nullptr;
  MotionDuelGame* motion = nullptr;
  AudioOut* audio = nullptr;

  static SecureMotionServer* instance;

  static esp_err_t handleRootStatic(httpd_req_t* request) {
    auto* self = static_cast<SecureMotionServer*>(request->user_ctx);
    return self != nullptr ? self->handleRoot(request) : ESP_FAIL;
  }

  static esp_err_t handleWebSocketStatic(httpd_req_t* request) {
    auto* self = static_cast<SecureMotionServer*>(request->user_ctx);
    return self != nullptr ? self->handleWebSocket(request) : ESP_FAIL;
  }

  esp_err_t handleRoot(httpd_req_t* request) {
    logHeap("HTTPS root");
    httpd_resp_set_type(request, "text/html; charset=utf-8");
    httpd_resp_set_hdr(request, "Cache-Control", "no-store");
    // The HTML request does not need to keep a TLS socket alive. Closing it
    // leaves heap available for the long-lived secure WebSocket session.
    httpd_resp_set_hdr(request, "Connection", "close");
    return httpd_resp_send(
      request,
      CONTROLLER_HTML,
      HTTPD_RESP_USE_STRLEN
    );
  }

  esp_err_t handleWebSocket(httpd_req_t* request) {
    if (request->method == HTTP_GET) {
      logHeap("WSS handshake");
      Serial.print("[WSS] handshake fd=");
      Serial.println(httpd_req_to_sockfd(request));
      return ESP_OK;
    }

    httpd_ws_frame_t frame = {};
    esp_err_t result = httpd_ws_recv_frame(request, &frame, 0);

    if (result != ESP_OK) {
      return result;
    }

    if (frame.type == HTTPD_WS_TYPE_PING) {
      frame.type = HTTPD_WS_TYPE_PONG;
      frame.payload = nullptr;
      frame.len = 0;
      return httpd_ws_send_frame(request, &frame);
    }

    if (frame.type == HTTPD_WS_TYPE_CLOSE) {
      clearForceForFd(httpd_req_to_sockfd(request));
      frame.payload = nullptr;
      frame.len = 0;
      return httpd_ws_send_frame(request, &frame);
    }

    if (frame.type != HTTPD_WS_TYPE_TEXT) {
      return ESP_OK;
    }

    if (frame.len == 0 || frame.len > MAX_MESSAGE_SIZE) {
      return ESP_ERR_INVALID_SIZE;
    }

    char payload[MAX_MESSAGE_SIZE + 1] = {};
    frame.payload = reinterpret_cast<uint8_t*>(payload);
    result = httpd_ws_recv_frame(request, &frame, frame.len);

    if (result != ESP_OK) {
      return result;
    }

    payload[frame.len] = '\0';
    return handleMessage(request, String(payload));
  }

  esp_err_t handleMessage(httpd_req_t* request, String message) {
    message.trim();

    if (message.startsWith("HELLO|")) {
      return bindClient(request, message.substring(6));
    }

    if (message.startsWith("MOTION|")) {
      return handleMotion(request, message.substring(7));
    }

    if (message == "PING") {
      return sendText(request, "PONG");
    }

    return sendText(request, "ERROR|unknown-message");
  }

  esp_err_t bindClient(httpd_req_t* request, String cid) {
    cid.trim();

    if (cid.length() == 0 || sessions == nullptr || audio == nullptr) {
      return sendText(request, "ROLE|none");
    }

    int side = sessions->findSlotByCidOrIp(cid, "");

    if (side < 0) {
      side = sessions->firstCpuSlot();

      if (side >= 0) {
        sessions->assignHumanToSlot(
          static_cast<uint8_t>(side),
          cid,
          "",
          *audio
        );
      } else {
        sessions->addOrUpdateSpectator(cid, "");
      }
    }

    if (side < 0 || side > PINK_SIDE) {
      return sendText(request, "ROLE|none");
    }

    const int fd = httpd_req_to_sockfd(request);
    bindings[side].fd = fd;
    bindings[side].cid = cid;
    sessions->slots[side].lastSeenMs = millis();

    String response = "ROLE|";
    response += sideName(static_cast<uint8_t>(side));

    Serial.print("[WSS] bind fd=");
    Serial.print(fd);
    Serial.print(" cid=");
    Serial.print(cid);
    Serial.print(" role=");
    Serial.println(sideName(static_cast<uint8_t>(side)));

    return sendText(request, response);
  }

  esp_err_t handleMotion(httpd_req_t* request, String valueText) {
    if (sessions == nullptr || motion == nullptr) {
      return ESP_FAIL;
    }

    const int side = sideForFd(httpd_req_to_sockfd(request));

    if (side < 0) {
      return sendText(request, "ERROR|hello-required");
    }

    valueText.trim();
    const float value = constrain(valueText.toFloat(), -1.0f, 1.0f);
    const uint32_t now = millis();

    sessions->slots[side].lastSeenMs = now;
    sessions->slots[side].lastActionMs = now;
    motion->setForce(static_cast<uint8_t>(side), value, now);

    return ESP_OK;
  }

  esp_err_t sendText(httpd_req_t* request, const String& text) {
    httpd_ws_frame_t frame = {};
    frame.type = HTTPD_WS_TYPE_TEXT;
    frame.payload = reinterpret_cast<uint8_t*>(
      const_cast<char*>(text.c_str())
    );
    frame.len = text.length();
    return httpd_ws_send_frame(request, &frame);
  }

  int sideForFd(int fd) const {
    for (uint8_t side = 0; side < 2; side++) {
      if (bindings[side].fd == fd) {
        return side;
      }
    }

    return -1;
  }

  void clearForceForFd(int fd) {
    const int side = sideForFd(fd);

    if (side < 0) return;

    if (motion != nullptr) {
      motion->clearForce(static_cast<uint8_t>(side));
    }

    bindings[side] = Binding();
  }

  struct Binding {
    int fd = -1;
    String cid = "";
  };

  Binding bindings[2];
};

SecureMotionServer* SecureMotionServer::instance = nullptr;
