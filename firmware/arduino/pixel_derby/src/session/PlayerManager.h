#pragma once
#include <Arduino.h>
#include "../Config.h"
#include "../Types.h"

class PlayerManager {
public:
  PlayerSlot players[MAX_PLAYERS];

  int findByCid(const String& cid) const {
    if (cid.length() == 0) return -1;
    for (uint8_t i = 0; i < MAX_PLAYERS; i++) if (players[i].occupied && players[i].cid == cid) return i;
    return -1;
  }

  int findByIp(const IPAddress& ip) const {
    if (ip == IPAddress(0, 0, 0, 0)) return -1;
    for (uint8_t i = 0; i < MAX_PLAYERS; i++) if (players[i].occupied && players[i].remoteIp == ip) return i;
    return -1;
  }

  int findByClient(uint8_t client) const {
    for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
      if (players[i].occupied && players[i].connected && players[i].wsClient == client) return i;
    }
    return -1;
  }

  int findReusableDisconnectedSlot() const {
    int best = -1;
    uint32_t newestDisconnect = 0;
    for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
      const PlayerSlot& player = players[i];
      if (!player.occupied || player.connected) continue;
      if (best < 0 || player.disconnectedAtMs > newestDisconnect) {
        best = i;
        newestDisconnect = player.disconnectedAtMs;
      }
    }
    return best;
  }

  int connect(const String& cid, uint8_t client, const IPAddress& remoteIp, uint32_t now) {
    int slot = findByCid(cid);
    if (slot < 0) slot = findByIp(remoteIp);
    if (slot < 0) slot = findReusableDisconnectedSlot();
    if (slot < 0) {
      for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
        if (!players[i].occupied) {
          slot = i;
          players[i] = PlayerSlot();
          players[i].occupied = true;
          break;
        }
      }
    }
    if (slot < 0) return -1;

    PlayerSlot& player = players[slot];
    player.occupied = true;
    player.connected = true;
    player.wsClient = client;
    player.cid = cid;
    player.remoteIp = remoteIp;
    player.lastSeenMs = now;
    player.disconnectedAtMs = 0;
    return slot;
  }

  void touch(uint8_t client, uint32_t now) {
    const int slot = findByClient(client);
    if (slot >= 0) players[slot].lastSeenMs = now;
  }

  void disconnect(uint8_t client, uint32_t now) {
    const int slot = findByClient(client);
    if (slot >= 0) markDisconnected(slot, now);
  }

  void disconnectStale(uint32_t now) {
    for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
      PlayerSlot& player = players[i];
      if (!player.occupied || !player.connected) continue;
      if (now - player.lastSeenMs >= PLAYER_HEARTBEAT_TIMEOUT_MS) markDisconnected(i, now);
    }
  }

  void expire(uint32_t now) {
    for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
      if (!players[i].occupied || players[i].connected) continue;
      if (now - players[i].disconnectedAtMs >= PLAYER_RECONNECT_MS) players[i] = PlayerSlot();
    }
  }

  bool setWaiting(uint8_t slot, bool waiting) {
    if (slot >= MAX_PLAYERS || !players[slot].occupied || !players[slot].connected) return false;
    players[slot].waiting = waiting;
    players[slot].ready = false;
    return true;
  }

  bool leave(uint8_t slot) {
    if (slot >= MAX_PLAYERS || !players[slot].occupied) return false;
    players[slot] = PlayerSlot();
    return true;
  }

  uint8_t connectedCount() const {
    uint8_t count = 0;
    for (const auto& p : players) if (p.occupied && p.connected) count++;
    return count;
  }

  uint8_t activeCount() const {
    uint8_t count = 0;
    for (const auto& p : players) if (p.occupied && p.connected && !p.waiting) count++;
    return count;
  }

  uint8_t readyCount() const {
    uint8_t count = 0;
    for (const auto& p : players) if (p.occupied && p.connected && !p.waiting && p.ready) count++;
    return count;
  }

  int hostSlot() const {
    for (uint8_t i = 0; i < MAX_PLAYERS; i++) if (players[i].occupied && players[i].connected && !players[i].waiting) return i;
    return -1;
  }

  bool allConnectedReady() const {
    const uint8_t active = activeCount();
    return active >= 2 && readyCount() == active;
  }

  void resetRound(bool resetReady) {
    for (auto& p : players) {
      if (!p.occupied) continue;
      p.position = 0;
      p.lastTapMs = 0;
      p.newPersonalRecord = false;
      p.turboTaps = 0;
      p.turbo1Claimed = false;
      p.turbo2Claimed = false;
      p.bossDamage = 0;
      p.bossEnergy = 0;
      p.stunnedUntilMs = 0;
      if (resetReady) p.ready = false;
    }
  }

  void resetMatch() {
    for (auto& p : players) {
      if (!p.occupied) continue;
      p.score = 0;
      p.position = 0;
      p.ready = false;
      p.lastTapMs = 0;
      p.newPersonalRecord = false;
      p.turboTaps = 0;
      p.turbo1Claimed = false;
      p.turbo2Claimed = false;
      p.bossDamage = 0;
      p.bossCandidateScore = 0;
      p.bossEnergy = 0;
      p.stunnedUntilMs = 0;
    }
  }

private:
  void markDisconnected(uint8_t slot, uint32_t now) {
    PlayerSlot& p = players[slot];
    p.connected = false;
    p.ready = false;
    p.wsClient = 255;
    p.disconnectedAtMs = now;
  }
};
