#include "src/Config.h"
#include "src/Types.h"
#include "src/SystemState.h"
#include "src/hardware/AudioOut.h"
#include "src/hardware/LedRenderer.h"
#include "src/session/SessionManager.h"
#include "src/games/color_rally/ColorRallyGame.h"
#include "src/games/motion_duel/MotionDuelGame.h"
#include "src/net/RealtimeTransport.h"
#include "src/net/CaptivePortal.h"

AudioOut audio;
SessionManager sessions;
SystemState systemState;
ColorRallyGame game;
MotionDuelGame motionDemo;
LedRenderer renderer;
CaptivePortal portal;
RealtimeTransport realtime;

uint32_t lastFrameMs = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("===================================");
  Serial.println("LED Arcade Platform");
  Serial.println("Game: Motion Controller Demo");
  Serial.println("Layered Arduino repo baseline");
  Serial.println("===================================");

  audio.begin();
  renderer.begin();

  sessions.begin();
  game.begin();
  motionDemo.begin();

  portal.begin(sessions, game, audio, systemState);
  realtime.begin(sessions, motionDemo);

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
  realtime.loop();

  uint32_t now = millis();

  if (lastFrameMs == 0) {
    lastFrameMs = now;
  }

  uint32_t dtMs = now - lastFrameMs;

  if (dtMs >= 20) {
    lastFrameMs = now;

    if (!systemState.gamePaused) {
      motionDemo.update(dtMs, sessions, audio);
    }

    renderer.renderMotionDemo(sessions, motionDemo);
  }
}
