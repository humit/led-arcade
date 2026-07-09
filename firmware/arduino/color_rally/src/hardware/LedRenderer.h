#pragma once
#include <Arduino.h>
#include <FastLED.h>
#include "../Config.h"
#include "../Types.h"
#include "../SystemState.h"
#include "../session/SessionManager.h"
#include "../games/color_rally/ColorRallyGame.h"

class LedRenderer {
public:
  CRGB leds[LED_COUNT];

  void begin() {
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, LED_COUNT);
    FastLED.setBrightness(LED_BRIGHTNESS);
    FastLED.clear(true);
  }

  void render(const SessionManager& sessions, const ColorRallyGame& game, const SystemState& systemState) {
    if (systemState.gamePaused) {
      drawPaused(sessions, game);
      FastLED.show();
      return;
    }

    if (game.state.mode == MODE_POINT_PAUSE) {
      drawPointPause(sessions, game);
      FastLED.show();
      return;
    }

    fadeAll(72);
    drawHitZones(sessions, game);
    drawHpBars(sessions);
    drawBall(game);
    FastLED.show();
  }

private:
  CRGB gameColor(uint8_t colorType) {
    if (colorType == C_RED) return CRGB::Red;
    if (colorType == C_GREEN) return CRGB::Green;
    if (colorType == C_BLUE) return CRGB::Blue;
    return CRGB::White;
  }

  CRGB sideColor(const SessionManager& sessions, uint8_t side) {
    if (side == BLUE_SIDE) {
      return sessions.slots[BLUE_SIDE].human ? CRGB::Blue : CRGB::Cyan;
    }

    if (side == PINK_SIDE) {
      return sessions.slots[PINK_SIDE].human ? CRGB::DeepPink : CRGB::Orange;
    }

    return CRGB::White;
  }

  void addPixelSafe(int idx, CRGB c) {
    if (idx < 0 || idx >= LED_COUNT) return;
    leds[idx] += c;
  }

  void setPixelSafe(int idx, CRGB c) {
    if (idx < 0 || idx >= LED_COUNT) return;
    leds[idx] = c;
  }

  void dimSet(int idx, CRGB c, uint8_t scale) {
    c.nscale8_video(scale);
    setPixelSafe(idx, c);
  }

  void fadeAll(uint8_t amount) {
    for (int i = 0; i < LED_COUNT; i++) {
      leds[i].fadeToBlackBy(amount);
    }
  }

  void drawHpBars(const SessionManager& sessions) {
    CRGB blue = sideColor(sessions, BLUE_SIDE);
    CRGB pink = sideColor(sessions, PINK_SIDE);

    for (int i = 0; i < MAX_HP; i++) {
      uint8_t scale = i < sessions.slots[BLUE_SIDE].hp ? 90 : 8;
      dimSet(i, blue, scale);
    }

    for (int i = 0; i < MAX_HP; i++) {
      uint8_t scale = i < sessions.slots[PINK_SIDE].hp ? 90 : 8;
      dimSet(LED_COUNT - 1 - i, pink, scale);
    }
  }

  void drawHitZones(const SessionManager& sessions, const ColorRallyGame& game) {
    CRGB blue = sideColor(sessions, BLUE_SIDE);
    CRGB pink = sideColor(sessions, PINK_SIDE);

    uint8_t blueScale = game.state.targetSide == BLUE_SIDE ? 70 : 10;
    uint8_t pinkScale = game.state.targetSide == PINK_SIDE ? 70 : 10;

    for (int i = BLUE_HIT_A + MAX_HP; i <= BLUE_HIT_B; i++) {
      dimSet(i, blue, blueScale);
    }

    for (int i = PINK_HIT_A; i <= PINK_HIT_B - MAX_HP; i++) {
      dimSet(i, pink, pinkScale);
    }

    // Middle marker intentionally removed.
  }

  void drawBall(const ColorRallyGame& game) {
    int head = (int)roundf(game.ballHeadPos());
    int tailLen = game.currentBallTailLength();

    CRGB base = gameColor(game.state.ballColorType);

    for (int i = 0; i <= BALL_HEAD_SIZE; i++) {
      int idx = game.state.ballDir > 0 ? (head - i) : (head + i);

      CRGB c = base;
      c.nscale8_video(255);

      if (game.state.ballCharged) {
        c += CRGB(70, 70, 70);
      }

      addPixelSafe(idx, c);
    }

    for (int t = 1; t <= tailLen; t++) {
      int idx = game.state.ballDir > 0
        ? (head - BALL_HEAD_SIZE - t)
        : (head + BALL_HEAD_SIZE + t);

      uint8_t scale = game.state.ballCharged
        ? map(t, 1, tailLen, 180, 10)
        : map(t, 1, tailLen, 145, 8);

      CRGB c = base;
      c.nscale8_video(scale);

      addPixelSafe(idx, c);
    }
  }

  void drawPointPause(const SessionManager& sessions, const ColorRallyGame& game) {
    fadeAll(74);

    CRGB c = sideColor(sessions, game.state.lastPointWinner);
    uint8_t pulse = (millis() / 90) % 2 ? 120 : 20;

    for (int i = 0; i < LED_COUNT; i++) {
      if (game.state.lastPointWinner == BLUE_SIDE && i < LED_COUNT / 2) {
        dimSet(i, c, pulse);
      }

      if (game.state.lastPointWinner == PINK_SIDE && i >= LED_COUNT / 2) {
        dimSet(i, c, pulse);
      }
    }
  }

  void drawPaused(const SessionManager& sessions, const ColorRallyGame& game) {
    fadeAll(88);

    drawHitZones(sessions, game);
    drawHpBars(sessions);
    drawBall(game);

    uint8_t pulse = (millis() / 450) % 2 ? 18 : 4;

    for (int i = 0; i < LED_COUNT; i++) {
      CRGB c = CRGB::White;
      c.nscale8_video(pulse);
      leds[i] += c;
    }

    dimSet(0, sideColor(sessions, BLUE_SIDE), 110);
    dimSet(LED_COUNT - 1, sideColor(sessions, PINK_SIDE), 110);
  }
};
