#pragma once
#include <Arduino.h>
#include <FastLED.h>
#include "../Config.h"

class SpriteRenderer {
public:
  using SetPixel = void (*)(void*, int, int, CRGB);

  static CRGB shade(CRGB base, uint8_t level) {
    if (level == 0) return CRGB::Black;
    CRGB c = base;
    c.nscale8_video(level == 1 ? 50 : level == 2 ? 120 : 255);
    return c;
  }

  static void drawRows(void* ctx, SetPixel setter, int ox, int oy,
                       const char* const* rows, uint8_t h, CRGB base) {
    for (uint8_t y = 0; y < h; ++y) {
      const char* row = rows[y];
      for (uint8_t x = 0; row[x]; ++x) {
        const char ch = row[x];
        if (ch < '1' || ch > '3') continue;
        setter(ctx, ox + x, oy + y, shade(base, uint8_t(ch - '0')));
      }
    }
  }
};
