#pragma once
#include <Arduino.h>
#include <esp_arduino_version.h>
#include "../Config.h"

class AudioOut {
public:
  void begin() {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcAttach(AUDIO_PIN, 1000, AUDIO_RES_BITS);
    ledcWrite(AUDIO_PIN, 0);
#else
    ledcSetup(0, 1000, AUDIO_RES_BITS);
    ledcAttachPin(AUDIO_PIN, 0);
    ledcWrite(0, 0);
#endif
  }

  void playerJoined() { tone(660, 24); tone(880, 28); }
  void ready() { tone(980, 24); }
  void countdown(uint8_t value) { tone(value == 0 ? 1320 : 520 + value * 120, value == 0 ? 90 : 42); }
  void tap() { tone(1080, 9); }
  void winner() { tone(880, 45); tone(1180, 55); tone(1560, 90); }

private:
  void tone(uint16_t frequency, uint16_t durationMs) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcWriteTone(AUDIO_PIN, frequency);
    ledcWrite(AUDIO_PIN, AUDIO_DUTY);
    delay(durationMs);
    ledcWriteTone(AUDIO_PIN, 0);
    ledcWrite(AUDIO_PIN, 0);
#else
    ledcWriteTone(0, frequency);
    ledcWrite(0, AUDIO_DUTY);
    delay(durationMs);
    ledcWriteTone(0, 0);
    ledcWrite(0, 0);
#endif
  }
};
