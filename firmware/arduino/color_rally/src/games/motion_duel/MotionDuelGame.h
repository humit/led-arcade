#pragma once

#include <Arduino.h>
#include "../../Config.h"
#include "../../session/SessionManager.h"
#include "../../hardware/AudioOut.h"

class MotionDuelGame {
public:
  float ballPos = (LED_COUNT - 1) * 0.5f;
  float velocity = 0.0f;

  void begin() {
    ballPos = (LED_COUNT - 1) * 0.5f;
    velocity = 0.0f;

    for (uint8_t side = 0; side < 2; side++) {
      forces[side] = 0.0f;
      lastMotionMs[side] = 0;
    }
  }

  void setForce(uint8_t side, float value, uint32_t now) {
    if (side > PINK_SIDE) return;

    forces[side] = constrain(value, -1.0f, 1.0f);
    lastMotionMs[side] = now;
  }

  void clearForce(uint8_t side) {
    if (side > PINK_SIDE) return;

    forces[side] = 0.0f;
    lastMotionMs[side] = 0;
  }

  float activeForce(uint8_t side, uint32_t now) const {
    if (side > PINK_SIDE) return 0.0f;
    if (lastMotionMs[side] == 0) return 0.0f;
    if (now - lastMotionMs[side] > MOTION_STALE_MS) return 0.0f;

    return forces[side];
  }

  void update(
    uint32_t dtMs,
    SessionManager& sessions,
    AudioOut& audio
  ) {
    (void)sessions;
    (void)audio;

    const uint32_t now = millis();
    const float dt = min(dtMs, (uint32_t)100) / 1000.0f;

    const float blueForce = activeForce(BLUE_SIDE, now);
    const float pinkForce = activeForce(PINK_SIDE, now);

    // Both phones currently operate in the same world coordinate system:
    // negative force pushes left, positive force pushes right.
    const float netForce = constrain(
      blueForce + pinkForce,
      -1.0f,
      1.0f
    );

    velocity += netForce * MOTION_ACCELERATION * dt;

    const float dragFactor = max(
      0.0f,
      1.0f - MOTION_DRAG_PER_SECOND * dt
    );

    velocity *= dragFactor;
    velocity = constrain(
      velocity,
      -MOTION_MAX_SPEED,
      MOTION_MAX_SPEED
    );

    ballPos += velocity * dt;

    const float minPos = 1.0f;
    const float maxPos = LED_COUNT - 2.0f;

    if (ballPos < minPos) {
      ballPos = minPos;
      velocity = fabsf(velocity) * 0.55f;
    } else if (ballPos > maxPos) {
      ballPos = maxPos;
      velocity = -fabsf(velocity) * 0.55f;
    }
  }

private:
  float forces[2] = {0.0f, 0.0f};
  uint32_t lastMotionMs[2] = {0, 0};
};
