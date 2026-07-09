#pragma once
#include <Arduino.h>
#include "../Types.h"
#include "../Config.h"
#include "../hardware/AudioOut.h"

static const uint8_t MAX_SPECTATORS = 6;

class SessionManager {
public:
  Slot slots[2];
  Spectator spectators[MAX_SPECTATORS];

  void begin() {
    makeCpu(BLUE_SIDE);
    makeCpu(PINK_SIDE);

    for (int i = 0; i < MAX_SPECTATORS; i++) {
      spectators[i] = Spectator();
    }
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

  void makeCpu(uint8_t side) {
    slots[side] = Slot();
    slots[side].human = false;
    slots[side].hp = MAX_HP;
    slots[side].combo = 0;
  }

  void assignHumanToSlot(uint8_t side, const String& cid, const String& ip, AudioOut& audio) {
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

    audio.assign();
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

  void removeSpectator(int index) {
    if (index < 0 || index >= MAX_SPECTATORS) return;
    spectators[index] = Spectator();
  }

  void promoteSpectatorToSlot(uint8_t side, AudioOut& audio) {
    int sp = firstSpectator();

    if (sp < 0) {
      makeCpu(side);
      return;
    }

    String cid = spectators[sp].cid;
    String ip = spectators[sp].ip;

    removeSpectator(sp);
    assignHumanToSlot(side, cid, ip, audio);
  }

  void expire(uint32_t now, AudioOut& audio) {
    for (int side = 0; side < 2; side++) {
      if (
        slots[side].human &&
        now - slots[side].lastSeenMs > HUMAN_DISCONNECT_TIMEOUT_MS
      ) {
        Serial.print("[DISCONNECTED] ");
        Serial.println(sideName(side));

        if (firstSpectator() >= 0) {
          promoteSpectatorToSlot(side, audio);
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
};
