#pragma once
#include <Arduino.h>
#include <esp_arduino_version.h>
#include "../Config.h"

class AudioOut {
public:
  bool muted = false;

  void begin() {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    bool ok = ledcAttach(AUDIO_PIN, 1000, AUDIO_RES_BITS);
    if (!ok) {
      Serial.println("[WARN] LEDC audio attach failed");
    }
    ledcWrite(AUDIO_PIN, 0);
#else
    ledcSetup(0, 1000, AUDIO_RES_BITS);
    ledcAttachPin(AUDIO_PIN, 0);
    ledcWrite(0, 0);
#endif
  }

  void setMuted(bool value) {
    muted = value;
    stopTone();
  }

  void playToneSoft(uint16_t freq, uint16_t durationMs) {
    if (freq == 0 || durationMs == 0) return;
    if (muted) return;

#if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcWriteTone(AUDIO_PIN, freq);
    ledcWrite(AUDIO_PIN, AUDIO_DUTY);
    delay(durationMs);
#else
    ledcWriteTone(0, freq);
    ledcWrite(0, AUDIO_DUTY);
    delay(durationMs);
#endif

    stopTone();
  }

  void assign() {
    playToneSoft(660, 28);
    delay(10);
    playToneSoft(990, 34);
  }

  void good(uint8_t colorType) {
    if (colorType == C_RED) playToneSoft(740, 26);
    else if (colorType == C_GREEN) playToneSoft(920, 26);
    else playToneSoft(1180, 26);
  }

  void bad() {
    playToneSoft(180, 40);
  }

  void damage() {
    playToneSoft(130, 85);
  }

  void charged() {
    playToneSoft(1400, 42);
  }

private:
  void stopTone() {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcWriteTone(AUDIO_PIN, 0);
    ledcWrite(AUDIO_PIN, 0);
#else
    ledcWriteTone(0, 0);
    ledcWrite(0, 0);
#endif
  }
};
