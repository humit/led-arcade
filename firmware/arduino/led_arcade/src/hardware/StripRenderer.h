#pragma once
#include <FastLED.h>
#include "../Config.h"
#include "../Types.h"
#include "../session/PlayerManager.h"
#include "../core/ArcadeGameEngine.h"

class StripRenderer {
public:
  CRGB leds[STRIP_LED_COUNT];

  void begin() {
    FastLED.addLeds<LED_TYPE, STRIP_LED_PIN, COLOR_ORDER>(leds, STRIP_LED_COUNT);
    clearPixels();
    FastLED.show();
  }

  void clearPixels() { fill_solid(leds, STRIP_LED_COUNT, CRGB::Black); }

  void render(const PlayerManager& players, const ArcadeGameEngine& game, uint32_t nowMs) {
    clearPixels();
    if (game.stage == ArcadeStage::GAME_SELECT || game.selectedGame == GameId::NONE) renderArenaDemo(nowMs);
    else if (game.selectedGame == GameId::REFLEX_RALLY) renderRally(players, game);
    else if (game.selectedGame == GameId::POWER_PUSH) renderPush(players, game, nowMs);
    FastLED.show();
  }

private:
  CRGB playerColor(int8_t slot) const {
    static const CRGB colors[MAX_PLAYERS] = {CRGB(20,80,255),CRGB(255,25,120),CRGB(25,220,100),CRGB(255,210,25),CRGB(20,210,230),CRGB(255,110,20),CRGB(150,70,255),CRGB::White};
    return slot >= 0 && slot < MAX_PLAYERS ? colors[slot] : CRGB(60,60,60);
  }

  void zones(const ArcadeGameEngine& game) {
    const CRGB left = playerColor(game.stripLeftSlot), right = playerColor(game.stripRightSlot);
    for (uint16_t i=0;i<STRIP_HIT_ZONE_LEDS;i++) {
      leds[i] = left; leds[i].nscale8(75);
      leds[STRIP_LED_COUNT-1-i] = right; leds[STRIP_LED_COUNT-1-i].nscale8(75);
    }
  }

  void renderArenaDemo(uint32_t nowMs) {
    const uint16_t zoneLength = STRIP_HIT_ZONE_LEDS;
    for (uint16_t i=0;i<zoneLength;i++) { leds[i]=CRGB(0,0,50); leds[STRIP_LED_COUNT-1-i]=CRGB(55,0,24); }
    const uint32_t travel=STRIP_LED_COUNT-1, phase=(nowMs/18)%(travel*2);
    const uint16_t head=phase<=travel?phase:(travel*2)-phase;
    leds[head]=CRGB::White;
  }

  void renderRally(const PlayerManager&, const ArcadeGameEngine& game) {
    zones(game);
    const int16_t x=constrain(game.stripBall,0,STRIP_LED_COUNT-1);
    leds[x]=CRGB::White;
    if (x>0) leds[x-1]=CRGB(45,45,45);
    if (x+1<STRIP_LED_COUNT) leds[x+1]=CRGB(18,18,18);
  }

  void renderPush(const PlayerManager&, const ArcadeGameEngine& game, uint32_t nowMs) {
    const int16_t marker=constrain(game.stripMarker,0,STRIP_LED_COUNT-1);
    const CRGB left=playerColor(game.stripLeftSlot), right=playerColor(game.stripRightSlot);
    for (int16_t i=0;i<marker;i++) { leds[i]=left; leds[i].nscale8(80); }
    for (int16_t i=marker+1;i<STRIP_LED_COUNT;i++) { leds[i]=right; leds[i].nscale8(80); }
    leds[marker]=(nowMs/120)%2?CRGB::White:CRGB(120,120,120);
  }
};
