#pragma once
#include <Arduino.h>
#include "../../Types.h"
#include "../../Config.h"
#include "../../session/SessionManager.h"
#include "../../hardware/AudioOut.h"

class ColorRallyGame {
public:
  GameState state;

  void begin() {
    randomSeed(esp_random());
    serveFrom(BLUE_SIDE);
  }

  int currentBallTailLength() const {
    return state.ballCharged ? BALL_TAIL_CHARGED : BALL_TAIL_NORMAL;
  }

  float ballHeadPos() const {
    if (state.ballDir > 0) return state.ballPos + BALL_HEAD_SIZE;
    return state.ballPos - BALL_HEAD_SIZE;
  }

  bool isBallInHitZone(uint8_t side) const {
    float head = ballHeadPos();

    if (side == BLUE_SIDE) {
      return head >= BLUE_HIT_A && head <= BLUE_HIT_B;
    }

    if (side == PINK_SIDE) {
      return head >= PINK_HIT_A && head <= PINK_HIT_B;
    }

    return false;
  }

  float currentBallSpeed(const SessionManager& sessions) const {
    if (state.ballCharged) return BALL_SPEED_CHARGED;

    uint8_t attacker = state.targetSide == BLUE_SIDE ? PINK_SIDE : BLUE_SIDE;

    if (sessions.slots[attacker].combo >= COMBO_FAST_AT) {
      return BALL_SPEED_COMBO;
    }

    return BALL_SPEED_BASE;
  }

  void serveFrom(uint8_t side) {
    if (side == BLUE_SIDE) {
      state.ballPos = 5.0f;
      state.ballDir = 1;
      state.targetSide = PINK_SIDE;
    } else {
      state.ballPos = LED_COUNT - 6;
      state.ballDir = -1;
      state.targetSide = BLUE_SIDE;
    }

    state.ballColorType = random(0, 3);
    state.ballCharged = false;
    resetCpuPlan();

    Serial.print("[SERVE] from ");
    Serial.print(sideName(side));
    Serial.print(" color=");
    Serial.println(colorName(state.ballColorType));
  }

  void bounceFrom(uint8_t side, bool cpu, SessionManager& sessions, AudioOut& audio) {
    sessions.slots[side].combo++;

    bool makeCharged = sessions.slots[side].combo >= COMBO_CHARGED_AT;

    if (side == BLUE_SIDE) {
      state.ballDir = 1;
      state.targetSide = PINK_SIDE;
      state.ballPos = max(state.ballPos, 2.0f);
    } else {
      state.ballDir = -1;
      state.targetSide = BLUE_SIDE;
      state.ballPos = min(state.ballPos, (float)(LED_COUNT - 3));
    }

    state.ballColorType = random(0, 3);
    state.ballCharged = makeCharged;

    if (state.ballCharged) audio.charged();
    else audio.good(state.ballColorType);

    resetCpuPlan();

    Serial.print(cpu ? "[CPU HIT] " : "[HIT] ");
    Serial.print(sideName(side));
    Serial.print(" combo=");
    Serial.print(sessions.slots[side].combo);
    Serial.print(" nextColor=");
    Serial.print(colorName(state.ballColorType));
    Serial.print(" charged=");
    Serial.println(state.ballCharged ? "yes" : "no");
  }

  void damageSide(uint8_t side, SessionManager& sessions, AudioOut& audio) {
    if (sessions.slots[side].hp > 0) {
      sessions.slots[side].hp--;
    }

    sessions.slots[side].combo = 0;
    state.lastDamagedSide = side;
    state.lastPointWinner = side == BLUE_SIDE ? PINK_SIDE : BLUE_SIDE;

    Serial.print("[DAMAGE] ");
    Serial.print(sideName(side));
    Serial.print(" hp=");
    Serial.println(sessions.slots[side].hp);

    audio.damage();

    if (sessions.slots[side].hp == 0) {
      Serial.print("[ROUND RESET] winner=");
      Serial.println(sideName(state.lastPointWinner));

      sessions.slots[BLUE_SIDE].hp = MAX_HP;
      sessions.slots[PINK_SIDE].hp = MAX_HP;
      sessions.slots[BLUE_SIDE].combo = 0;
      sessions.slots[PINK_SIDE].combo = 0;
    }

    state.mode = MODE_POINT_PAUSE;
    state.pointPauseUntil = millis() + POINT_PAUSE_MS;
  }

  void updateCpu(uint32_t now, SessionManager& sessions, AudioOut& audio) {
    if (sessions.slots[state.targetSide].human) return;

    if (!isBallInHitZone(state.targetSide)) {
      resetCpuPlan();
      return;
    }

    if (!state.cpuArmed) {
      state.cpuArmed = true;
      state.cpuHitAtMs = now + CPU_REACTION_MS;

      bool correct = random(0, 100) < CPU_CORRECT_PERCENT;

      if (correct) {
        state.cpuPlannedColor = state.ballColorType;
      } else {
        state.cpuPlannedColor = random(0, 3);

        if (state.cpuPlannedColor == state.ballColorType) {
          state.cpuPlannedColor = (state.ballColorType + 1) % 3;
        }
      }

      return;
    }

    if (now >= state.cpuHitAtMs) {
      if (state.cpuPlannedColor == state.ballColorType) {
        bounceFrom(state.targetSide, true, sessions, audio);
      } else {
        sessions.slots[state.targetSide].combo = 0;
        audio.bad();
        resetCpuPlan();
      }
    }
  }

  void update(uint32_t dtMs, SessionManager& sessions, AudioOut& audio) {
    uint32_t now = millis();

    sessions.expire(now, audio);

    if (state.mode == MODE_POINT_PAUSE) {
      if (now >= state.pointPauseUntil) {
        state.mode = MODE_PLAYING;
        serveFrom(state.lastPointWinner);
      }

      return;
    }

    float step = currentBallSpeed(sessions) * ((float)dtMs / 1000.0f);
    state.ballPos += state.ballDir * step;

    updateCpu(now, sessions, audio);

    float head = ballHeadPos();

    if (head < 0) {
      damageSide(BLUE_SIDE, sessions, audio);
    } else if (head > LED_COUNT - 1) {
      damageSide(PINK_SIDE, sessions, audio);
    }
  }

  bool handleMotionHit(
    uint8_t side,
    SessionManager& sessions,
    AudioOut& audio,
    String& message
  ) {
    if (state.mode != MODE_PLAYING) {
      message = "wait";
      return false;
    }

    if (!sessions.slots[side].human) {
      message = "cpu slot";
      return false;
    }

    if (state.targetSide != side) {
      message = "not your turn";
      return false;
    }

    if (!isBallInHitZone(side)) {
      message = "outside hit zone";
      return false;
    }

    bounceFrom(side, false, sessions, audio);
    message = "motion hit";
    return true;
  }

  bool handlePress(uint8_t side, uint8_t pressedColor, SessionManager& sessions, AudioOut& audio, String& message) {
    if (state.mode != MODE_PLAYING) {
      message = "wait";
      return false;
    }

    if (state.targetSide != side) {
      audio.bad();
      message = "not your turn";
      return false;
    }

    if (!isBallInHitZone(side)) {
      audio.bad();
      message = "too early";
      return false;
    }

    if (pressedColor != state.ballColorType) {
      sessions.slots[side].combo = 0;
      audio.bad();
      message = "wrong color";
      return false;
    }

    bounceFrom(side, false, sessions, audio);
    message = "hit";
    return true;
  }

private:
  void resetCpuPlan() {
    state.cpuArmed = false;
    state.cpuHitAtMs = 0;
    state.cpuPlannedColor = C_RED;
  }
};
