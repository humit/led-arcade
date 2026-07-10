#pragma once
#include <Arduino.h>
#include <FastLED.h>
#include "../Config.h"
#include "../Types.h"
#include "../session/PlayerManager.h"
#include "../games/pixel_derby/PixelDerbyGame.h"

class MatrixRenderer {
public:
  CRGB leds[LED_COUNT];

  void begin() {
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, LED_COUNT);
    FastLED.setBrightness(LED_BRIGHTNESS);
    FastLED.clear(true);
  }

  void render(const PlayerManager& players, const PixelDerbyGame& game) {
    FastLED.clear();
    switch (game.stage) {
      case ArcadeStage::PLATFORM_SELECT: drawPlatformSelect(); break;
      case ArcadeStage::GAME_SELECT: drawGameSelect(); break;
      case ArcadeStage::LOBBY: drawLobby(players); break;
      case ArcadeStage::COUNTDOWN: drawCountdown(game.countdownValue); break;
      case ArcadeStage::RACING: drawRace(players); break;
      case ArcadeStage::RESULT: drawResult(players, game); break;
      case ArcadeStage::BOSS: drawBoss(players, game); break;
      case ArcadeStage::BOSS_RESULT: drawBossResult(players, game); break;
    }
    FastLED.show();
  }

private:
  uint16_t xy(uint8_t x, uint8_t y) const {
    return (x & 1) == 0 ? x * MATRIX_HEIGHT + y : x * MATRIX_HEIGHT + (MATRIX_HEIGHT - 1 - y);
  }

  CRGB playerColor(uint8_t slot) const {
    static const CRGB colors[MAX_PLAYERS] = {
      CRGB::Blue, CRGB::DeepPink, CRGB::Green, CRGB::Yellow,
      CRGB::Cyan, CRGB::Orange, CRGB::Purple, CRGB::White
    };
    return colors[slot % MAX_PLAYERS];
  }

  void set(uint8_t x, uint8_t y, CRGB color) {
    if (x >= MATRIX_WIDTH || y >= MATRIX_HEIGHT) return;
    leds[xy(x, y)] = color;
  }

  uint8_t activeCount(const PlayerManager& players) const {
    uint8_t count = 0;
    for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
      if (players.players[i].occupied && players.players[i].connected && !players.players[i].waiting) count++;
    }
    return count;
  }

  int8_t activeOrder(const PlayerManager& players, uint8_t slot) const {
    int8_t order = 0;
    for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
      if (!players.players[i].occupied || !players.players[i].connected || players.players[i].waiting) continue;
      if (i == slot) return order;
      order++;
    }
    return -1;
  }

  uint8_t displayRow(const PlayerManager& players, uint8_t slot) const {
    static const uint8_t layouts[MAX_PLAYERS][MAX_PLAYERS] = {
      {3,3,3,3,3,3,3,3},
      {2,5,5,5,5,5,5,5},
      {1,4,6,6,6,6,6,6},
      {0,2,5,7,7,7,7,7},
      {0,2,3,5,7,7,7,7},
      {0,1,3,4,6,7,7,7},
      {0,1,2,4,5,6,7,7},
      {0,1,2,3,4,5,6,7}
    };
    const uint8_t count = activeCount(players);
    const int8_t order = activeOrder(players, slot);
    if (count == 0 || order < 0) return 3;
    return layouts[count - 1][order];
  }

  bool rowUsedByAnother(const PlayerManager& players, uint8_t row, uint8_t exceptSlot) const {
    for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
      if (i == exceptSlot || !players.players[i].occupied || !players.players[i].connected || players.players[i].waiting) continue;
      if (displayRow(players, i) == row) return true;
    }
    return false;
  }

  int8_t leaderSlot(const PlayerManager& players) const {
    int8_t leader = -1;
    uint8_t best = 0;
    bool tied = false;
    for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
      const PlayerSlot& p = players.players[i];
      if (!p.occupied || !p.connected || p.waiting) continue;
      if (leader < 0 || p.position > best) {
        leader = i;
        best = p.position;
        tied = false;
      } else if (p.position == best) {
        tied = true;
      }
    }
    return tied ? -1 : leader;
  }

  void drawPlatformSelect() {
    const uint8_t sweep = (millis() / 70) % MATRIX_WIDTH;
    for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
      for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {
        CRGB c = ((x + y) % 3 == 0) ? CRGB::Blue : CRGB::Purple;
        c.nscale8_video(x == sweep ? 180 : 28);
        set(x, y, c);
      }
    }
  }

  void drawGameSelect() {
    const uint8_t p1 = (millis() / 90) % MATRIX_WIDTH;
    const uint8_t p2 = MATRIX_WIDTH - 1 - p1;
    for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
      CRGB track(8, 8, 8);
      set(x, 2, track);
      set(x, 5, track);
    }
    set(p1, 2, CRGB::Blue);
    set(p2, 5, CRGB::DeepPink);
    for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) set(FINISH_X, y, CRGB(28, 28, 28));
  }

  void drawLobby(const PlayerManager& players) {
    const uint8_t pulse = (millis() / 280) % 2 ? 150 : 55;
    for (uint8_t slot = 0; slot < MAX_PLAYERS; slot++) {
      const PlayerSlot& p = players.players[slot];
      if (!p.occupied || !p.connected || p.waiting) continue;
      const uint8_t row = displayRow(players, slot);
      CRGB color = playerColor(slot);
      color.nscale8_video(p.ready ? pulse : 38);
      for (uint8_t x = 0; x < (p.ready ? 8 : 3); x++) set(x, row, color);
    }
  }

  void drawCountdown(uint8_t value) {
    FastLED.clear();
    if (value == 0) {
      for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {
        for (uint8_t x = 0; x < MATRIX_WIDTH; x++) set(x, y, CRGB::Green);
      }
      return;
    }
    drawDigit(value, 13, 1, CRGB::White);
  }

  void drawDigit(uint8_t digit, uint8_t ox, uint8_t oy, CRGB color) {
    static const uint8_t glyphs[3][5] = {
      {0b010,0b110,0b010,0b010,0b111},
      {0b111,0b001,0b111,0b100,0b111},
      {0b111,0b001,0b111,0b001,0b111}
    };
    const uint8_t* glyph = digit == 1 ? glyphs[0] : digit == 2 ? glyphs[1] : glyphs[2];
    for (uint8_t row = 0; row < 5; row++) {
      for (uint8_t col = 0; col < 3; col++) {
        if (glyph[row] & (1 << (2 - col))) {
          set(ox + col * 2, oy + row, color);
          set(ox + col * 2 + 1, oy + row, color);
        }
      }
    }
  }

  void drawRace(const PlayerManager& players) {
    for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {
      set(FINISH_X, y, CRGB(45, 45, 45));
      set(TURBO_X_1, y, CRGB(55, 34, 0));
      set(TURBO_X_2, y, CRGB(55, 34, 0));
    }

    const int8_t leader = leaderSlot(players);
    for (uint8_t slot = 0; slot < MAX_PLAYERS; slot++) {
      const PlayerSlot& p = players.players[slot];
      if (!p.occupied || !p.connected || p.waiting) continue;
      const uint8_t row = displayRow(players, slot);
      const uint8_t head = p.position < FINISH_X ? p.position : FINISH_X;
      CRGB color = playerColor(slot);
      if (p.turboTaps > 0) color = CRGB::Gold;

      // Every non-leader remains a clean single-pixel racer.
      set(head, row, color);

      if (slot != leader) continue;
      if (head > 0) set(head - 1, row, color);
      if (head > 1) {
        CRGB tail = color;
        tail.nscale8_video(80);
        set(head - 2, row, tail);
      }

      CRGB glow = color;
      glow.nscale8_video((millis() / 110) % 2 ? 85 : 35);
      if (row > 0 && !rowUsedByAnother(players, row - 1, slot)) set(head, row - 1, glow);
      if (row + 1 < MATRIX_HEIGHT && !rowUsedByAnother(players, row + 1, slot)) set(head, row + 1, glow);
    }
  }

  void drawBoss(const PlayerManager& players, const PixelDerbyGame& game) {
    const uint8_t bossStart = 25;
    const uint8_t pulse = (millis() / 110) % 2 ? 230 : 110;

    if (game.bossSlot >= 0) {
      CRGB bossColor = playerColor(game.bossSlot);
      bossColor.nscale8_video(pulse);
      for (uint8_t x = bossStart; x < MATRIX_WIDTH; x++) {
        for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {
          if (x == bossStart || x == MATRIX_WIDTH - 1 || y == 1 || y == 6 || ((x + y + millis() / 140) % 3 == 0)) {
            set(x, y, bossColor);
          }
        }
      }
      // Crown
      set(27, 0, CRGB::Gold); set(29, 0, CRGB::Gold); set(31, 0, CRGB::Gold);
      set(28, 1, CRGB::Gold); set(30, 1, CRGB::Gold);
    }

    const uint8_t hpPixels = game.bossMaxHp == 0 ? 0 : uint8_t((uint32_t(game.bossHp) * 23 + game.bossMaxHp - 1) / game.bossMaxHp);
    for (uint8_t x = 0; x < 23; x++) set(x, 0, x < hpPixels ? CRGB::Red : CRGB(18, 0, 0));

    const bool pulseFlash = (millis() / 80) % 2 == 0;
    for (uint8_t slot = 0; slot < MAX_PLAYERS; slot++) {
      const PlayerSlot& p = players.players[slot];
      if (!p.occupied || !p.connected || p.waiting || slot == game.bossSlot) continue;
      const uint8_t row = displayRow(players, slot);
      CRGB c = playerColor(slot);
      if (game.stunRemainingMs(p) > 0) c = pulseFlash ? CRGB::White : CRGB(12, 12, 12);
      const uint8_t shotX = 2 + ((millis() / 75 + p.bossDamage * 3) % 21);
      set(0, row, c);
      set(shotX, row, c);
      if (p.turboTaps > 0 && shotX + 1 < bossStart) set(shotX + 1, row, CRGB::Gold);
    }

    // Boss pulse charge bar on bottom row.
    if (game.bossSlot >= 0) {
      const PlayerSlot& boss = players.players[game.bossSlot];
      const uint8_t chargePixels = uint8_t((uint16_t(boss.bossEnergy) * 23) / BOSS_PULSE_TAPS);
      for (uint8_t x = 0; x < chargePixels; x++) set(x, 7, CRGB::Purple);
    }
  }

  void drawBossResult(const PlayerManager& players, const PixelDerbyGame& game) {
    if (game.bossDefeated) {
      for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
        for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) set(x, y, CHSV(uint8_t(x * 8 + y * 22 + millis() / 9), 255, 210));
      }
      if (game.topRaiderSlot >= 0) {
        const uint8_t row = displayRow(players, game.topRaiderSlot);
        for (uint8_t x = 0; x < MATRIX_WIDTH; x++) set(x, row, playerColor(game.topRaiderSlot));
      }
    } else if (game.bossSlot >= 0) {
      CRGB c = playerColor(game.bossSlot);
      c.nscale8_video((millis() / 130) % 2 ? 255 : 80);
      for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
        for (uint8_t y = 1; y < 7; y++) if ((x + y) % 2 == 0) set(x, y, c);
      }
      set(14, 0, CRGB::Gold); set(16, 0, CRGB::Gold); set(18, 0, CRGB::Gold);
    }
  }

  void drawResult(const PlayerManager& players, const PixelDerbyGame& game) {
    if (game.winner < 0 || game.winner >= MAX_PLAYERS) return;
    const uint8_t winner = game.winner;
    const uint8_t row = displayRow(players, winner);
    const uint8_t radius = 2 + ((millis() / 170) % 7);

    for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
      CRGB color = game.newDeviceRecord
        ? CHSV(uint8_t(x * 8 + millis() / 8), 255, 220)
        : playerColor(winner);
      if (!game.newDeviceRecord) color.nscale8_video((millis() / 120) % 2 ? 220 : 70);
      set(x, row, color);
      if (x <= radius || x >= MATRIX_WIDTH - 1 - radius) {
        if (row > 0) set(x, row - 1, color);
        if (row + 1 < MATRIX_HEIGHT) set(x, row + 1, color);
      }
    }

    for (uint8_t slot = 0; slot < MAX_PLAYERS; slot++) {
      const PlayerSlot& p = players.players[slot];
      if (!p.occupied || slot == winner) continue;
      const uint8_t scoreRow = displayRow(players, slot);
      for (uint8_t score = 0; score < p.score && score < MATCH_WIN_SCORE; score++) {
        set(score, scoreRow, playerColor(slot));
      }
    }
  }
};
