#pragma once

#include <Arduino.h>
#include "../../Config.h"
#include "../../session/SessionManager.h"
#include "../../hardware/AudioOut.h"

class MotionDuelGame {
public:
  // Legacy motion-demo fields are retained so existing renderer helpers compile.
  float ballPos = (LED_COUNT - 1) * 0.5f;
  float velocity = 0.0f;

  void begin() {
    ballPos = (LED_COUNT - 1) * 0.5f;
    velocity = 0.0f;

    for (uint8_t side = 0; side < 2; side++) {
      forces[side] = 0.0f;
      lastMotionMs[side] = 0;
      leftTiltArmed[side] = true;
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
    leftTiltArmed[side] = true;
  }

  float activeForce(uint8_t side, uint32_t now) const {
    if (side > PINK_SIDE) return 0.0f;
    if (lastMotionMs[side] == 0) return 0.0f;

    if (now - lastMotionMs[side] > MOTION_STALE_MS) {
      return 0.0f;
    }

    return forces[side];
  }

  /*
   * Detect one deliberate left-tilt gesture.
   *
   * Gesture lifecycle:
   *   neutral/right  -> armed
   *   cross left threshold -> one gesture
   *   remain tilted -> no repeated gestures
   *   return neutral -> armed again
   *
   * If the player tilts too early, the gesture is consumed. The phone must
   * return to neutral before another attempt. This prevents holding the phone
   * left and getting an automatic hit when the ball later enters the zone.
   */
  bool consumeLeftTilt(
    uint8_t side,
    uint32_t now,
    bool hitAllowed
  ) {
    if (side > PINK_SIDE) return false;

    const float force = activeForce(side, now);

    if (force >= LEFT_TILT_REARM_THRESHOLD) {
      leftTiltArmed[side] = true;
    }

    if (
      leftTiltArmed[side] &&
      force <= LEFT_TILT_TRIGGER_THRESHOLD
    ) {
      leftTiltArmed[side] = false;

      Serial.print(
        hitAllowed
          ? "[MOTION HIT GESTURE] "
          : "[MOTION EARLY GESTURE] "
      );
      Serial.print(sideName(side));
      Serial.print(" force=");
      Serial.println(force, 3);

      return hitAllowed;
    }

    return false;
  }

  /*
   * Legacy continuous-force demo physics. Retained but no longer called by
   * the main Color Rally loop.
   */
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
  // Controller force is normalized to -1.0 ... +1.0.
  static constexpr float LEFT_TILT_TRIGGER_THRESHOLD = -0.55f;

  // Player must return close to neutral before the next gesture.
  static constexpr float LEFT_TILT_REARM_THRESHOLD = -0.18f;

  float forces[2] = {0.0f, 0.0f};
  uint32_t lastMotionMs[2] = {0, 0};
  bool leftTiltArmed[2] = {true, true};
};
