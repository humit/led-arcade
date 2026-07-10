#include "src/Config.h"
#include "src/Types.h"
#include "src/SystemState.h"
#include "src/hardware/AudioOut.h"
#include "src/hardware/LedRenderer.h"
#include "src/session/SessionManager.h"
#include "src/games/color_rally/ColorRallyGame.h"
#include "src/games/motion_duel/MotionDuelGame.h"
#include "src/net/SecureMotionServer.h"
#include "src/net/CaptivePortal.h"

AudioOut audio;
SessionManager sessions;
SystemState systemState;
ColorRallyGame game;
MotionDuelGame motionDemo;
LedRenderer renderer;
CaptivePortal portal;
SecureMotionServer secureMotion;

uint32_t lastFrameMs = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("===================================");
  Serial.println("LED Arcade Platform");
  Serial.println("Game: Motion Badminton - Human vs CPU");
  Serial.println("Layered Arduino repo baseline");
  Serial.println("===================================");

  audio.begin();
  renderer.begin();

  sessions.begin();
  game.begin();
  motionDemo.begin();

  portal.begin(sessions, game, audio, systemState);
  secureMotion.begin(sessions, motionDemo, audio);

  Serial.print("[INFO] LED_COUNT: ");
  Serial.println(LED_COUNT);
  Serial.print("[INFO] BALL_HEAD_SIZE: ");
  Serial.println(BALL_HEAD_SIZE);
  Serial.print("[INFO] BALL_TAIL_NORMAL: ");
  Serial.println(BALL_TAIL_NORMAL);
  Serial.print("[INFO] BALL_TAIL_CHARGED: ");
  Serial.println(BALL_TAIL_CHARGED);
  Serial.print("[INFO] HIT_ZONE_SIZE: ");
  Serial.println(HIT_ZONE_SIZE);
  Serial.println("[INFO] AP mode: open");
  Serial.println("[INFO] QR payload: WIFI:T:nopass;S:LED-Arcade;;");
  Serial.println("[INFO] Middle white marker LEDs removed.");
  Serial.println("[INFO] Global pause and mute enabled.");
}

void loop() {
  portal.loop();

  uint32_t now = millis();

  if (lastFrameMs == 0) {
    lastFrameMs = now;
  }

  uint32_t dtMs = now - lastFrameMs;

  if (dtMs >= 20) {
    lastFrameMs = now;

    if (!systemState.gamePaused) {
      game.update(dtMs, sessions, audio);

      const uint32_t gestureNow = millis();

      for (uint8_t side = BLUE_SIDE; side <= PINK_SIDE; side++) {
        if (!sessions.slots[side].human) {
          continue;
        }

        const bool hitAllowed =
          game.state.mode == MODE_PLAYING &&
          game.state.targetSide == side &&
          game.isBallInHitZone(side);

        if (
          motionDemo.consumeLeftTilt(
            side,
            gestureNow,
            hitAllowed
          )
        ) {
          String message;

          const bool hit = game.handleMotionHit(
            side,
            sessions,
            audio,
            message
          );

          Serial.print("[MOTION RESULT] ");
          Serial.print(sideName(side));
          Serial.print(" hit=");
          Serial.print(hit ? "yes" : "no");
          Serial.print(" message=");
          Serial.println(message);
        }
      }
    }

    renderer.render(sessions, game, systemState);
  }
}
