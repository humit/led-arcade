#pragma once
#include <Arduino.h>
#include "../../Config.h"
#include "../../session/PlayerManager.h"
#include "../../hardware/AudioOut.h"

class PixelPongGame {
public:
  int8_t leftSlot = -1;
  int8_t rightSlot = -1;
  int8_t leftPaddleY = 2;
  int8_t rightPaddleY = 2;
  int8_t ballX = MATRIX_WIDTH / 2 - 1;
  int8_t ballY = MATRIX_HEIGHT / 2 - 1;
  int8_t ballDx = 1;
  int8_t ballDy = 0;
  uint8_t leftScore = 0;
  uint8_t rightScore = 0;
  uint16_t rallyHits = 0;
  uint32_t stepMs = PONG_STEP_START_MS;
  uint32_t resumeAtMs = 0;
  bool pointPause = false;
  bool matchFinished = false;
  int8_t winnerSlot = -1;

  void clear() {
    leftSlot = -1;
    rightSlot = -1;
    leftPaddleY = centeredPaddleY();
    rightPaddleY = centeredPaddleY();
    leftScore = 0;
    rightScore = 0;
    rallyHits = 0;
    stepMs = PONG_STEP_START_MS;
    resumeAtMs = 0;
    pointPause = false;
    matchFinished = false;
    winnerSlot = -1;
    nextServeDirection = 1;
    resetBallPreview();
  }

  bool prepare(const PlayerManager& players) {
    clear();
    uint8_t active = 0;
    for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
      const PlayerSlot& player = players.players[i];
      if (!isActive(player)) continue;
      if (active == 0) leftSlot = i;
      else if (active == 1) rightSlot = i;
      active++;
    }
    if (active != 2) {
      leftSlot = -1;
      rightSlot = -1;
      return false;
    }
    nextServeDirection = random(2) == 0 ? -1 : 1;
    return true;
  }

  void start(uint32_t now) {
    matchFinished = false;
    winnerSlot = -1;
    serve(now, nextServeDirection);
    cpuNextMoveMs = now + random(PONG_CPU_MOVE_MIN_MS, PONG_CPU_MOVE_MAX_MS + 1);
  }

  bool move(uint8_t slot, int8_t delta, PlayerManager& players) {
    if (slot >= MAX_PLAYERS || delta == 0) return false;
    PlayerSlot& player = players.players[slot];
    if (!isActive(player) || player.isCpu) return false;
    if (slot != leftSlot && slot != rightSlot) return false;

    const uint32_t now = millis();
    if (now - player.lastTapMs < PONG_INPUT_DEBOUNCE_MS) return false;
    player.lastTapMs = now;

    int8_t& paddleY = slot == leftSlot ? leftPaddleY : rightPaddleY;
    const int8_t next = constrain(int(paddleY) + int(delta), 0, maxPaddleY());
    if (next == paddleY) return false;
    paddleY = next;
    return true;
  }

  void update(PlayerManager& players, AudioOut& audio, uint32_t now) {
    if (matchFinished) return;
    const bool leftActive = slotActive(players, leftSlot);
    const bool rightActive = slotActive(players, rightSlot);
    if (!leftActive || !rightActive) {
      matchFinished = true;
      winnerSlot = leftActive ? leftSlot : rightActive ? rightSlot : -1;
      return;
    }

    updateCpu(players, now);

    if (pointPause) {
      if (now < resumeAtMs) return;
      serve(now, nextServeDirection);
      return;
    }
    if (now - lastStepMs < stepMs) return;
    lastStepMs = now;

    int8_t nextX = ballX + ballDx;
    int8_t nextY = ballY + ballDy;

    if (nextY < 0) {
      ballDy = 1;
      nextY = 1;
    } else if (nextY >= MATRIX_HEIGHT) {
      ballDy = -1;
      nextY = MATRIX_HEIGHT - 2;
    }

    if (ballDx < 0 && nextX == PONG_LEFT_X && paddleContains(leftPaddleY, nextY)) {
      ballDx = 1;
      nextX = PONG_LEFT_X + 1;
      applyPaddleImpact(leftPaddleY, nextY);
      registerHit(audio);
    } else if (ballDx > 0 && nextX == PONG_RIGHT_X && paddleContains(rightPaddleY, nextY)) {
      ballDx = -1;
      nextX = PONG_RIGHT_X - 1;
      applyPaddleImpact(rightPaddleY, nextY);
      registerHit(audio);
    }

    if (nextX < 0) {
      scorePoint(false, audio, now);
      return;
    }
    if (nextX >= MATRIX_WIDTH) {
      scorePoint(true, audio, now);
      return;
    }

    ballX = nextX;
    ballY = nextY;
  }

private:
  uint32_t lastStepMs = 0;
  uint32_t cpuNextMoveMs = 0;
  int8_t nextServeDirection = 1;

  static bool isActive(const PlayerSlot& player) {
    return player.occupied && player.connected && !player.waiting;
  }

  bool slotActive(const PlayerManager& players, int8_t slot) const {
    return slot >= 0 && slot < MAX_PLAYERS && isActive(players.players[slot]);
  }

  static int8_t maxPaddleY() { return MATRIX_HEIGHT - PONG_PADDLE_HEIGHT; }
  static int8_t centeredPaddleY() { return (MATRIX_HEIGHT - PONG_PADDLE_HEIGHT) / 2; }

  static bool paddleContains(int8_t paddleY, int8_t y) {
    return y >= paddleY && y < paddleY + PONG_PADDLE_HEIGHT;
  }

  void resetBallPreview() {
    ballX = MATRIX_WIDTH / 2 - 1;
    ballY = MATRIX_HEIGHT / 2 - 1;
    ballDx = nextServeDirection;
    ballDy = 0;
  }

  void serve(uint32_t now, int8_t direction) {
    pointPause = false;
    rallyHits = 0;
    stepMs = PONG_STEP_START_MS;
    ballX = direction < 0 ? MATRIX_WIDTH / 2 - 1 : MATRIX_WIDTH / 2;
    ballY = random(2, MATRIX_HEIGHT - 2);
    ballDx = direction < 0 ? -1 : 1;
    const int8_t verticalOptions[3] = {-1, 0, 1};
    ballDy = verticalOptions[random(3)];
    lastStepMs = now;
  }

  void applyPaddleImpact(int8_t paddleY, int8_t impactY) {
    const int8_t offset = impactY - paddleY;
    if (offset <= 0) ballDy = -1;
    else if (offset >= PONG_PADDLE_HEIGHT - 1) ballDy = 1;
    else if (ballDy == 0) ballDy = random(2) == 0 ? -1 : 1;
  }

  void registerHit(AudioOut& audio) {
    rallyHits++;
    const uint32_t speedup = min<uint32_t>(
        uint32_t(rallyHits) * PONG_SPEEDUP_PER_HIT_MS,
        PONG_STEP_START_MS - PONG_STEP_MIN_MS);
    stepMs = PONG_STEP_START_MS - speedup;
    audio.pongHit();
  }

  void scorePoint(bool leftScored, AudioOut& audio, uint32_t now) {
    if (leftScored) leftScore++;
    else rightScore++;
    audio.pongScore();

    if (leftScore >= PONG_SCORE_TO_WIN || rightScore >= PONG_SCORE_TO_WIN) {
      matchFinished = true;
      winnerSlot = leftScore >= PONG_SCORE_TO_WIN ? leftSlot : rightSlot;
      return;
    }

    nextServeDirection = leftScored ? 1 : -1;
    pointPause = true;
    resumeAtMs = now + PONG_POINT_PAUSE_MS;
    resetBallPreview();
  }

  void updateCpu(PlayerManager& players, uint32_t now) {
    if (now < cpuNextMoveMs) return;
    cpuNextMoveMs = now + random(PONG_CPU_MOVE_MIN_MS, PONG_CPU_MOVE_MAX_MS + 1);

    int8_t cpuSlot = -1;
    int8_t* paddle = nullptr;
    bool ballApproaching = false;
    if (leftSlot >= 0 && players.players[leftSlot].isCpu) {
      cpuSlot = leftSlot;
      paddle = &leftPaddleY;
      ballApproaching = ballDx < 0;
    } else if (rightSlot >= 0 && players.players[rightSlot].isCpu) {
      cpuSlot = rightSlot;
      paddle = &rightPaddleY;
      ballApproaching = ballDx > 0;
    }
    if (cpuSlot < 0 || paddle == nullptr || pointPause) return;
    if (ballApproaching && random(100) < PONG_CPU_SKIP_PERCENT) return;

    int8_t targetCenter = ballApproaching ? ballY + int8_t(random(-1, 2)) : MATRIX_HEIGHT / 2;
    targetCenter = constrain(targetCenter, 0, MATRIX_HEIGHT - 1);
    const int8_t currentCenter = *paddle + PONG_PADDLE_HEIGHT / 2;
    if (targetCenter < currentCenter) (*paddle)--;
    else if (targetCenter > currentCenter) (*paddle)++;
    *paddle = constrain(*paddle, int8_t(0), maxPaddleY());
  }
};
