#pragma once
#include <Arduino.h>
#include "Config.h"

enum class ArcadeStage : uint8_t {
  PLATFORM_SELECT,
  GAME_SELECT,
  LOBBY,
  COUNTDOWN,
  RACING,
  RESULT,
  BOSS,
  BOSS_RESULT
};

struct PlayerSlot {
  bool occupied = false;
  bool connected = false;
  bool ready = false;
  bool waiting = false;
  uint8_t wsClient = 255;
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
};

inline const char* stageName(ArcadeStage stage) {
  switch (stage) {
    case ArcadeStage::PLATFORM_SELECT: return "platform";
    case ArcadeStage::GAME_SELECT: return "games";
    case ArcadeStage::LOBBY: return "lobby";
    case ArcadeStage::COUNTDOWN: return "countdown";
    case ArcadeStage::RACING: return "race";
    case ArcadeStage::RESULT: return "result";
    case ArcadeStage::BOSS: return "boss";
    case ArcadeStage::BOSS_RESULT: return "boss_result";
  }
  return "platform";
}
