#include "src/Config.h"
#include "src/Types.h"
#include "src/hardware/AudioOut.h"
#include "src/hardware/MatrixRenderer.h"
#include "src/session/PlayerManager.h"
#include "src/games/pixel_derby/PixelDerbyGame.h"
#include "src/net/ArcadeNetwork.h"

AudioOut audio;
MatrixRenderer renderer;
PlayerManager players;
PixelDerbyGame game;
ArcadeNetwork network;

uint32_t lastFrameMs = 0;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("===================================");
  Serial.println("LED Arcade / 8x32 Pixel Derby");
  Serial.println("WebSocket multiplayer MVP");
  Serial.println("===================================");

  audio.begin();
  renderer.begin();
  network.begin(players, game, audio);
}

void loop() {
  network.loop();
  game.update(players, audio);

  const uint32_t now = millis();
  if (now - lastFrameMs >= 20) {
    lastFrameMs = now;
    renderer.render(players, game);
  }
}
