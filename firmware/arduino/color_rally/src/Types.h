#pragma once
#include <Arduino.h>
#include "Config.h"

struct Slot {
  bool human = false;
  String cid = "";
  String ip = "";
  uint32_t lastSeenMs = 0;
  uint32_t lastActionMs = 0;
  uint8_t hp = MAX_HP;
  uint8_t combo = 0;
};

struct Spectator {
  bool active = false;
  String cid = "";
  String ip = "";
  uint32_t lastSeenMs = 0;
};

enum Mode {
  MODE_PLAYING,
  MODE_POINT_PAUSE
};

struct GameState {
  Mode mode = MODE_PLAYING;

  float ballPos = 5.0f;
  int ballDir = 1;
  uint8_t targetSide = PINK_SIDE;
  uint8_t ballColorType = C_RED;
  bool ballCharged = false;

  uint8_t lastPointWinner = NO_SIDE;
  uint8_t lastDamagedSide = NO_SIDE;

  uint32_t pointPauseUntil = 0;

  bool cpuArmed = false;
  uint32_t cpuHitAtMs = 0;
  uint8_t cpuPlannedColor = C_RED;
};

inline String sideName(uint8_t side) {
  if (side == BLUE_SIDE) return "blue";
  if (side == PINK_SIDE) return "pink";
  return "none";
}

inline String colorName(uint8_t colorType) {
  if (colorType == C_RED) return "red";
  if (colorType == C_GREEN) return "green";
  if (colorType == C_BLUE) return "blue";
  return "unknown";
}

inline String modeName(Mode mode) {
  if (mode == MODE_PLAYING) return "playing";
  if (mode == MODE_POINT_PAUSE) return "point";
  return "unknown";
}

inline uint8_t parseColorArg(const String& color) {
  if (color == "red") return C_RED;
  if (color == "green") return C_GREEN;
  if (color == "blue") return C_BLUE;
  return 99;
}
