#include "src/Config.h"
#include "src/Types.h"
#include "src/hardware/AudioOut.h"
#include "src/hardware/MatrixRenderer.h"
#include "src/hardware/StripRenderer.h"
#include "src/session/PlayerManager.h"
#include "src/games/pixel_derby/PixelDerbyGame.h"
#include "src/net/ArcadeNetwork.h"
#include "src/presentation/ArcadeDirector.h"

AudioOut audio;
MatrixRenderer renderer;
StripRenderer stripRenderer;
PlayerManager players;
PixelDerbyGame game;
ArcadeNetwork network;
ArcadeDirector director;

uint32_t lastFrameMs = 0;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("===================================");
  Serial.println("LED Arcade / 8x32 Multi-game");
  Serial.println("Pixel Derby + Pixel Raider + Color Clash + Pixel Pong");
  Serial.println("===================================");

  audio.begin();
  renderer.begin();
  stripRenderer.begin();
  director.begin(audio);
  network.begin(players, game, audio);
}

void loop() {
  network.loop();
  game.update(players, audio);
  director.update(game, players);
  audio.update();

  const uint32_t now = millis();
  if (now - lastFrameMs >= 20) {
    lastFrameMs = now;
    if (game.selectedArena == ArenaType::STRIP_1D) {
      renderer.clearPixels();
      stripRenderer.render(players, game, now);
    } else {
      stripRenderer.clearPixels();
      renderer.render(players, game, director);
    }
  }
}
