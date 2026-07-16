#pragma once
#include <Arduino.h>
#include "Config.h"

enum class ArcadeStage : uint8_t {
  PLATFORM_SELECT,
  GAME_SELECT,
  LOBBY,
  ANNOUNCE,
  COUNTDOWN,
  RACING,
  RESULT,
  BOSS,
  BOSS_RESULT
};

enum class ArenaType : uint8_t { NONE, MATRIX_8X32, STRIP_1D, SCREEN_ARCADE };

enum class GameId : uint8_t { NONE, PIXEL_DERBY, TRON_ARENA, PIXEL_RAIDER, COLOR_CLASH, PIXEL_PONG, REFLEX_RALLY, POWER_PUSH, TAP_CLASH };

enum class TronDirection : uint8_t { UP, RIGHT, DOWN, LEFT };

static constexpr uint32_t INVALID_WS_CLIENT = UINT32_MAX;

struct PlayerSlot {
  bool occupied = false;
  bool connected = false;
  bool isCpu = false;
  bool ready = false;
  bool waiting = false;
  uint32_t wsClient = INVALID_WS_CLIENT;
  String cid;
  IPAddress remoteIp;
  uint32_t lastSeenMs = 0;
  uint32_t disconnectedAtMs = 0;
  uint32_t lastTapMs = 0;
  uint8_t position = 0;
  uint8_t score = 0;
  uint32_t totalPoints = 0;
  uint16_t wins = 0;
  uint8_t streak = 0;
  uint8_t bestStreak = 0;
  uint32_t personalBestMs = 0;
  uint32_t lastRaceMs = 0;
  bool newPersonalRecord = false;
  uint8_t turboTaps = 0;
  bool turbo1Claimed = false;
  bool turbo2Claimed = false;
  uint16_t bossDamage = 0;
  uint16_t bossCandidateScore = 0;
  uint8_t bossEnergy = 0;
  uint32_t stunnedUntilMs = 0;

  int8_t tronX = -1;
  int8_t tronY = -1;
  TronDirection tronDirection = TronDirection::RIGHT;
  bool tronAlive = false;

  int8_t clashX = -1;
  int8_t clashY = -1;
  TronDirection clashDirection = TronDirection::RIGHT;
};

inline const char* stageName(ArcadeStage stage) {
  switch (stage) {
    case ArcadeStage::PLATFORM_SELECT: return "platform";
    case ArcadeStage::GAME_SELECT: return "games";
    case ArcadeStage::LOBBY: return "lobby";
    case ArcadeStage::ANNOUNCE: return "announce";
    case ArcadeStage::COUNTDOWN: return "countdown";
    case ArcadeStage::RACING: return "race";
    case ArcadeStage::RESULT: return "result";
    case ArcadeStage::BOSS: return "boss";
    case ArcadeStage::BOSS_RESULT: return "boss_result";
  }
  return "platform";
}

inline const char* gameName(GameId game) {
  switch (game) {
    case GameId::PIXEL_DERBY: return "pixel_derby";
    case GameId::TRON_ARENA: return "tron_arena";
    case GameId::PIXEL_RAIDER: return "pixel_raider";
    case GameId::COLOR_CLASH: return "color_clash";
    case GameId::PIXEL_PONG: return "pixel_pong";
    case GameId::REFLEX_RALLY: return "reflex_rally";
    case GameId::POWER_PUSH: return "power_push";
    case GameId::TAP_CLASH: return "tap_clash";
    default: return "none";
  }
}

inline const char* arenaName(ArenaType arena) {
  switch (arena) {
    case ArenaType::MATRIX_8X32: return "matrix_8x32";
    case ArenaType::STRIP_1D: return "strip_1d";
    case ArenaType::SCREEN_ARCADE: return "screen_arcade";
    default: return "none";
  }
}
