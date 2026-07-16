#pragma once
#include <Arduino.h>
#include "../../Config.h"
#include "../../session/PlayerManager.h"

class TapClashGame {
public:
  static constexpr uint8_t NO_CELL = 255;
  static constexpr uint8_t EVENT_NONE = 0;
  static constexpr uint8_t EVENT_TARGET = 1;
  static constexpr uint8_t EVENT_SCORE = 2;
  static constexpr uint8_t EVENT_MISS = 3;

  uint8_t targetCell = NO_CELL;
  uint32_t targetId = 0;
  uint32_t targetExpiresAtMs = 0;
  uint32_t roundEndsAtMs = 0;
  uint32_t eventId = 0;
  uint8_t eventType = EVENT_NONE;
  int8_t eventSlot = -1;
  uint32_t lockedUntilMs[MAX_PLAYERS] = {0};

  void clear() {
    targetCell = NO_CELL;
    targetId = 0;
    targetExpiresAtMs = 0;
    roundEndsAtMs = 0;
    nextTargetAtMs = 0;
    eventId = 0;
    eventType = EVENT_NONE;
    eventSlot = -1;
    running = false;
    memset(lockedUntilMs, 0, sizeof(lockedUntilMs));
    memset(cpuDueMs, 0, sizeof(cpuDueMs));
  }

  void prepare(PlayerManager& players) {
    clear();
    for (auto& player : players.players) {
      if (!player.occupied || !player.connected || player.waiting) continue;
      player.score = 0;
      player.lastTapMs = 0;
    }
  }

  void start(PlayerManager& players, uint32_t now) {
    running = true;
    roundEndsAtMs = now + TAP_CLASH_DURATION_MS;
    nextTargetAtMs = now + TAP_CLASH_FIRST_TARGET_DELAY_MS;
    scheduleCpuActions(players, 0);
  }

  bool update(PlayerManager& players, uint32_t now) {
    if (!running) return false;

    if (now >= roundEndsAtMs) {
      running = false;
      targetCell = NO_CELL;
      targetExpiresAtMs = 0;
      memset(cpuDueMs, 0, sizeof(cpuDueMs));
      return true;
    }

    if (targetCell != NO_CELL && now >= targetExpiresAtMs) {
      targetCell = NO_CELL;
      targetExpiresAtMs = 0;
      nextTargetAtMs = now + random(TAP_CLASH_GAP_MIN_MS, TAP_CLASH_GAP_MAX_MS + 1);
      memset(cpuDueMs, 0, sizeof(cpuDueMs));
    }

    if (targetCell == NO_CELL && now >= nextTargetAtMs) {
      spawnTarget(players, now);
    }

    if (targetCell == NO_CELL) return false;

    for (uint8_t slot = 0; slot < MAX_PLAYERS; slot++) {
      PlayerSlot& player = players.players[slot];
      if (!player.occupied || !player.connected || player.waiting || !player.isCpu) continue;
      if (cpuDueMs[slot] == 0 || now < cpuDueMs[slot]) continue;
      claim(slot, players, now);
      break;
    }

    return false;
  }

  bool tap(
      uint8_t slot,
      uint32_t submittedTargetId,
      uint8_t submittedCell,
      PlayerManager& players,
      uint32_t now
  ) {
    if (!running || slot >= MAX_PLAYERS) return false;
    PlayerSlot& player = players.players[slot];
    if (!player.occupied || !player.connected || player.waiting || player.isCpu) return false;
    if (now < lockedUntilMs[slot]) return false;
    if (now - player.lastTapMs < TAP_CLASH_INPUT_DEBOUNCE_MS) return false;
    player.lastTapMs = now;

    // Stale input means another player already claimed or replaced the target.
    // Ignore it without punishing normal network latency.
    if (targetCell == NO_CELL || submittedTargetId != targetId) return false;

    if (submittedCell != targetCell) {
      lockedUntilMs[slot] = now + TAP_CLASH_WRONG_LOCK_MS;
      publish(EVENT_MISS, slot);
      return true;
    }

    return claim(slot, players, now);
  }

  uint32_t remainingMs(uint32_t now) const {
    if (!running || now >= roundEndsAtMs) return 0;
    return roundEndsAtMs - now;
  }

  uint32_t targetRemainingMs(uint32_t now) const {
    if (targetCell == NO_CELL || now >= targetExpiresAtMs) return 0;
    return targetExpiresAtMs - now;
  }

  uint32_t lockRemainingMs(uint8_t slot, uint32_t now) const {
    if (slot >= MAX_PLAYERS || now >= lockedUntilMs[slot]) return 0;
    return lockedUntilMs[slot] - now;
  }

private:
  bool running = false;
  uint32_t nextTargetAtMs = 0;
  uint32_t cpuDueMs[MAX_PLAYERS] = {0};

  void publish(uint8_t type, int8_t slot) {
    eventId++;
    eventType = type;
    eventSlot = slot;
  }

  void spawnTarget(PlayerManager& players, uint32_t now) {
    targetCell = uint8_t(random(TAP_CLASH_GRID_CELLS));
    targetId++;
    targetExpiresAtMs = now + random(TAP_CLASH_TARGET_MIN_MS, TAP_CLASH_TARGET_MAX_MS + 1);
    publish(EVENT_TARGET, -1);
    scheduleCpuActions(players, now);
  }

  void scheduleCpuActions(PlayerManager& players, uint32_t now) {
    memset(cpuDueMs, 0, sizeof(cpuDueMs));
    if (now == 0) return;
    for (uint8_t slot = 0; slot < MAX_PLAYERS; slot++) {
      const PlayerSlot& player = players.players[slot];
      if (!player.occupied || !player.connected || player.waiting || !player.isCpu) continue;
      cpuDueMs[slot] = now + random(TAP_CLASH_CPU_MIN_MS, TAP_CLASH_CPU_MAX_MS + 1);
    }
  }

  bool claim(uint8_t slot, PlayerManager& players, uint32_t now) {
    if (targetCell == NO_CELL || slot >= MAX_PLAYERS) return false;
    PlayerSlot& player = players.players[slot];
    if (!player.occupied || !player.connected || player.waiting) return false;

    if (player.score < 255) player.score++;
    player.totalPoints += TAP_CLASH_POINTS_PER_TARGET;
    targetCell = NO_CELL;
    targetExpiresAtMs = 0;
    nextTargetAtMs = now + random(TAP_CLASH_GAP_MIN_MS, TAP_CLASH_GAP_MAX_MS + 1);
    memset(cpuDueMs, 0, sizeof(cpuDueMs));
    publish(EVENT_SCORE, slot);
    return true;
  }
};
