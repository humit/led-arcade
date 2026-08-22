#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <esp_attr.h>
#include <esp_system.h>
#include <stdarg.h>
#include "../Config.h"

struct FieldPersistentState {
  uint32_t magic = 0;
  uint32_t bootCount = 0;
  uint32_t lastUptimeMs = 0;
  uint32_t lastFreeHeap = 0;
  char lastEvent[96] = {};
};

RTC_DATA_ATTR FieldPersistentState fieldPersistentState;

class FieldDiagnostics {
public:
  struct Counters {
    uint32_t wifiConnects = 0;
    uint32_t wifiDisconnects = 0;
    uint32_t wsConnects = 0;
    uint32_t wsDisconnects = 0;
    uint32_t clientHellos = 0;
    uint32_t heartbeatTimeouts = 0;
    uint32_t expiredSessions = 0;
    uint32_t queueDrops = 0;
    uint32_t gestureAccepted = 0;
    uint32_t gestureRejected = 0;
    uint32_t httpRequests = 0;
    uint32_t captiveRequests = 0;
    uint8_t maxWifiStations = 0;
    uint8_t maxWsClients = 0;
  } counters;

  void begin() {
    resetReason = esp_reset_reason();
    if (fieldPersistentState.magic != PERSISTENT_MAGIC) {
      memset(&fieldPersistentState, 0, sizeof(fieldPersistentState));
      fieldPersistentState.magic = PERSISTENT_MAGIC;
    }

    previousBootUptimeMs = fieldPersistentState.lastUptimeMs;
    previousBootFreeHeap = fieldPersistentState.lastFreeHeap;
    strlcpy(previousBootLastEvent, fieldPersistentState.lastEvent, sizeof(previousBootLastEvent));
    fieldPersistentState.bootCount++;
    bootCount = fieldPersistentState.bootCount;
    minimumFreeHeap = ESP.getFreeHeap();

    logf(
        "INFO",
        "BOOT",
        "build=%s boot=%lu reset=%s previousUptimeMs=%lu previousHeap=%lu previousEvent=%s",
        FIELD_TEST_BUILD,
        static_cast<unsigned long>(bootCount),
        resetReasonName(resetReason),
        static_cast<unsigned long>(previousBootUptimeMs),
        static_cast<unsigned long>(previousBootFreeHeap),
        previousBootLastEvent[0] ? previousBootLastEvent : "none"
    );
    checkpoint();
  }

  void loop() {
    const uint32_t now = millis();
    const uint32_t freeHeap = ESP.getFreeHeap();
    if (freeHeap < minimumFreeHeap) minimumFreeHeap = freeHeap;

    if (now - lastCheckpointMs >= DIAGNOSTICS_CHECKPOINT_MS) {
      lastCheckpointMs = now;
      checkpoint();
    }

    if (now - lastHealthLogMs >= DIAGNOSTICS_HEALTH_LOG_MS) {
      lastHealthLogMs = now;
      logf(
          freeHeap < DIAGNOSTICS_LOW_HEAP_BYTES ? "WARN" : "INFO",
          "HEALTH",
          "uptimeMs=%lu heap=%lu minHeap=%lu wifiStations=%u",
          static_cast<unsigned long>(now),
          static_cast<unsigned long>(freeHeap),
          static_cast<unsigned long>(minimumFreeHeap),
          WiFi.softAPgetStationNum()
      );
    }
  }

  void recordWifiConnect(const uint8_t* mac, uint8_t aid) {
    counters.wifiConnects++;
    updateStationPeak();
    char macText[18];
    formatMac(mac, macText, sizeof(macText));
    logf("INFO", "WIFI", "station-connected mac=%s aid=%u stations=%u", macText, aid, WiFi.softAPgetStationNum());
  }

  void recordWifiDisconnect(const uint8_t* mac, uint8_t aid, uint8_t reason) {
    counters.wifiDisconnects++;
    char macText[18];
    formatMac(mac, macText, sizeof(macText));
    logf("WARN", "WIFI", "station-disconnected mac=%s aid=%u reason=%u stations=%u", macText, aid, reason, WiFi.softAPgetStationNum());
  }

  void recordWsConnect(uint32_t clientId, const IPAddress& ip, uint8_t connectedClients) {
    counters.wsConnects++;
    if (connectedClients > counters.maxWsClients) counters.maxWsClients = connectedClients;
    logf("INFO", "WS", "connected client=%lu ip=%s clients=%u", static_cast<unsigned long>(clientId), ip.toString().c_str(), connectedClients);
  }

  void recordWsDisconnect(uint32_t clientId, const IPAddress& ip, int slot) {
    counters.wsDisconnects++;
    logf("WARN", "WS", "disconnected client=%lu ip=%s slot=%d", static_cast<unsigned long>(clientId), ip.toString().c_str(), slot);
  }

  void recordHello(uint32_t clientId, int slot, const IPAddress& ip, const String& cid) {
    counters.clientHellos++;
    ClientRecord& record = clientRecord(clientId);
    record.clientId = clientId;
    record.slot = slot;
    record.ip = ip;
    record.lastSeenMs = millis();
    copyText(record.cid, sizeof(record.cid), cid);
    logf("INFO", "CLIENT", "hello client=%lu slot=%d ip=%s cid=%s", static_cast<unsigned long>(clientId), slot, ip.toString().c_str(), record.cid);
  }

  void recordClientInfo(uint32_t clientId, int slot, const IPAddress& ip, const String& profile) {
    ClientRecord& record = clientRecord(clientId);
    record.clientId = clientId;
    record.slot = slot;
    record.ip = ip;
    record.lastSeenMs = millis();
    copyText(record.profile, sizeof(record.profile), profile);
    logf("INFO", "CLIENT", "profile client=%lu slot=%d ip=%s %s", static_cast<unsigned long>(clientId), slot, ip.toString().c_str(), record.profile);
  }

  void touchClient(uint32_t clientId) {
    for (auto& record : clients) {
      if (record.clientId == clientId) {
        record.lastSeenMs = millis();
        return;
      }
    }
  }

  void recordHeartbeatTimeout(uint32_t clientId, int slot, const IPAddress& ip, uint32_t silentMs) {
    counters.heartbeatTimeouts++;
    logf("WARN", "CLIENT", "heartbeat-timeout client=%lu slot=%d ip=%s silentMs=%lu", static_cast<unsigned long>(clientId), slot, ip.toString().c_str(), static_cast<unsigned long>(silentMs));
  }

  void recordExpiredSession(int slot, const IPAddress& ip, uint32_t disconnectedMs) {
    counters.expiredSessions++;
    logf("INFO", "CLIENT", "session-expired slot=%d ip=%s disconnectedMs=%lu", slot, ip.toString().c_str(), static_cast<unsigned long>(disconnectedMs));
  }

  void recordQueueDrops(uint32_t total) {
    counters.queueDrops = total;
    logf("WARN", "WS", "queue-drops total=%lu", static_cast<unsigned long>(total));
  }

  void recordGesture(uint32_t clientId, int slot, const String& payload) {
    if (payload.indexOf("|accepted|") >= 0) counters.gestureAccepted++;
    else counters.gestureRejected++;
    logf("INFO", "GESTURE", "client=%lu slot=%d %s", static_cast<unsigned long>(clientId), slot, payload.c_str());
  }

  void recordClientEvent(uint32_t clientId, int slot, const String& payload) {
    logf("INFO", "BROWSER", "client=%lu slot=%d %s", static_cast<unsigned long>(clientId), slot, payload.c_str());
  }

  void recordHttpRequest(const IPAddress& ip, const char* responseKind, const char* path) {
    counters.httpRequests++;
    if (strcmp(responseKind, "root") != 0) counters.captiveRequests++;
    logf("INFO", "HTTP", "ip=%s response=%s path=%s", ip.toString().c_str(), responseKind, path);
  }

  void recordControl(uint32_t clientId, int slot, const String& command) {
    logf("INFO", "CONTROL", "client=%lu slot=%d command=%s", static_cast<unsigned long>(clientId), slot, command.c_str());
  }

  void logf(const char* level, const char* category, const char* format, ...) {
    char message[112];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    LogEntry& entry = logs[nextLogIndex];
    entry.sequence = ++sequence;
    entry.uptimeMs = millis();
    copyText(entry.level, sizeof(entry.level), level);
    copyText(entry.category, sizeof(entry.category), category);
    copyText(entry.message, sizeof(entry.message), message);
    nextLogIndex = (nextLogIndex + 1) % DIAGNOSTICS_LOG_CAPACITY;
    if (logCount < DIAGNOSTICS_LOG_CAPACITY) logCount++;

    snprintf(
        fieldPersistentState.lastEvent,
        sizeof(fieldPersistentState.lastEvent),
        "%s/%s %s",
        entry.level,
        entry.category,
        entry.message
    );
    Serial.printf("[FIELD][%lu][%s][%s] %s\n", static_cast<unsigned long>(entry.uptimeMs), entry.level, entry.category, entry.message);
  }

  String summaryJson() const {
    String json;
    json.reserve(5000);
    json = "{\"build\":";
    appendJsonString(json, FIELD_TEST_BUILD);
    json += ",\"bootCount\":" + String(bootCount);
    json += ",\"resetReason\":";
    appendJsonString(json, resetReasonName(resetReason));
    json += ",\"uptimeMs\":" + String(millis());
    json += ",\"previousBootUptimeMs\":" + String(previousBootUptimeMs);
    json += ",\"previousBootFreeHeap\":" + String(previousBootFreeHeap);
    json += ",\"previousBootLastEvent\":";
    appendJsonString(json, previousBootLastEvent);
    json += ",\"freeHeap\":" + String(ESP.getFreeHeap());
    json += ",\"minimumFreeHeap\":" + String(minimumFreeHeap);
    json += ",\"wifiStations\":" + String(WiFi.softAPgetStationNum());
    json += ",\"wifiMaxConnections\":" + String(AP_MAX_CONNECTIONS);
    json += ",\"apIp\":";
    appendJsonString(json, WiFi.softAPIP().toString());
    json += ",\"apMac\":";
    appendJsonString(json, WiFi.softAPmacAddress());
    json += ",\"counters\":{";
    json += "\"wifiConnects\":" + String(counters.wifiConnects);
    json += ",\"wifiDisconnects\":" + String(counters.wifiDisconnects);
    json += ",\"wsConnects\":" + String(counters.wsConnects);
    json += ",\"wsDisconnects\":" + String(counters.wsDisconnects);
    json += ",\"clientHellos\":" + String(counters.clientHellos);
    json += ",\"heartbeatTimeouts\":" + String(counters.heartbeatTimeouts);
    json += ",\"expiredSessions\":" + String(counters.expiredSessions);
    json += ",\"queueDrops\":" + String(counters.queueDrops);
    json += ",\"gestureAccepted\":" + String(counters.gestureAccepted);
    json += ",\"gestureRejected\":" + String(counters.gestureRejected);
    json += ",\"httpRequests\":" + String(counters.httpRequests);
    json += ",\"captiveRequests\":" + String(counters.captiveRequests);
    json += ",\"maxWifiStations\":" + String(counters.maxWifiStations);
    json += ",\"maxWsClients\":" + String(counters.maxWsClients) + "}";
    json += ",\"clients\":[";
    bool first = true;
    for (const auto& record : clients) {
      if (record.clientId == INVALID_CLIENT) continue;
      if (!first) json += ',';
      first = false;
      json += "{\"clientId\":" + String(record.clientId);
      json += ",\"slot\":" + String(record.slot);
      json += ",\"ip\":";
      appendJsonString(json, record.ip.toString());
      json += ",\"cid\":";
      appendJsonString(json, record.cid);
      json += ",\"lastSeenMs\":" + String(record.lastSeenMs);
      json += ",\"profile\":";
      appendJsonString(json, record.profile);
      json += "}";
    }
    json += "]}";
    return json;
  }

  String logText() const {
    String text;
    text.reserve(logCount * 150 + 256);
    text += "LED Arcade field-test log\n";
    text += "build=" + String(FIELD_TEST_BUILD) + " boot=" + String(bootCount) + " reset=" + String(resetReasonName(resetReason)) + "\n";
    for (uint8_t offset = 0; offset < logCount; offset++) {
      const uint8_t index = (nextLogIndex + DIAGNOSTICS_LOG_CAPACITY - logCount + offset) % DIAGNOSTICS_LOG_CAPACITY;
      const LogEntry& entry = logs[index];
      text += String(entry.sequence) + "\t" + String(entry.uptimeMs) + "\t" + entry.level + "\t" + entry.category + "\t" + entry.message + "\n";
    }
    return text;
  }

private:
  static constexpr uint32_t PERSISTENT_MAGIC = 0x4C454446;
  static constexpr uint32_t INVALID_CLIENT = 0xFFFFFFFF;

  struct LogEntry {
    uint32_t sequence = 0;
    uint32_t uptimeMs = 0;
    char level[6] = {};
    char category[12] = {};
    char message[112] = {};
  };

  struct ClientRecord {
    uint32_t clientId = INVALID_CLIENT;
    int8_t slot = -1;
    IPAddress ip;
    uint32_t lastSeenMs = 0;
    char cid[32] = {};
    char profile[240] = {};
  };

  LogEntry logs[DIAGNOSTICS_LOG_CAPACITY];
  ClientRecord clients[DIAGNOSTICS_CLIENT_CAPACITY];
  uint8_t nextLogIndex = 0;
  uint8_t logCount = 0;
  uint32_t sequence = 0;
  uint32_t bootCount = 0;
  esp_reset_reason_t resetReason = ESP_RST_UNKNOWN;
  uint32_t previousBootUptimeMs = 0;
  uint32_t previousBootFreeHeap = 0;
  char previousBootLastEvent[96] = {};
  uint32_t minimumFreeHeap = 0;
  uint32_t lastCheckpointMs = 0;
  uint32_t lastHealthLogMs = 0;

  void checkpoint() {
    fieldPersistentState.lastUptimeMs = millis();
    fieldPersistentState.lastFreeHeap = ESP.getFreeHeap();
  }

  void updateStationPeak() {
    const uint8_t stations = WiFi.softAPgetStationNum();
    if (stations > counters.maxWifiStations) counters.maxWifiStations = stations;
  }

  ClientRecord& clientRecord(uint32_t clientId) {
    for (auto& record : clients) if (record.clientId == clientId) return record;
    for (auto& record : clients) if (record.clientId == INVALID_CLIENT) return record;
    ClientRecord& oldest = clients[0];
    for (auto& record : clients) if (record.lastSeenMs < oldest.lastSeenMs) oldest = record;
    oldest = ClientRecord();
    return oldest;
  }

  static void copyText(char* target, size_t capacity, const String& source) {
    if (capacity == 0) return;
    strlcpy(target, source.c_str(), capacity);
  }

  static void copyText(char* target, size_t capacity, const char* source) {
    if (capacity == 0) return;
    strlcpy(target, source ? source : "", capacity);
  }

  static void formatMac(const uint8_t* mac, char* output, size_t capacity) {
    if (mac == nullptr) {
      strlcpy(output, "unknown", capacity);
      return;
    }
    snprintf(output, capacity, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  }

  static const char* resetReasonName(esp_reset_reason_t reason) {
    switch (reason) {
      case ESP_RST_POWERON: return "power-on";
      case ESP_RST_EXT: return "external-pin";
      case ESP_RST_SW: return "software";
      case ESP_RST_PANIC: return "panic-exception";
      case ESP_RST_INT_WDT: return "interrupt-watchdog";
      case ESP_RST_TASK_WDT: return "task-watchdog";
      case ESP_RST_WDT: return "other-watchdog";
      case ESP_RST_DEEPSLEEP: return "deep-sleep";
      case ESP_RST_BROWNOUT: return "brownout";
      case ESP_RST_SDIO: return "sdio";
      default: return "unknown";
    }
  }

  static void appendJsonString(String& json, const String& value) {
    json += '"';
    for (size_t i = 0; i < value.length(); i++) {
      const char c = value[i];
      if (c == '"' || c == '\\') {
        json += '\\';
        json += c;
      } else if (c == '\n') json += "\\n";
      else if (c == '\r') json += "\\r";
      else if (c == '\t') json += "\\t";
      else if (uint8_t(c) >= 0x20) json += c;
    }
    json += '"';
  }
};
