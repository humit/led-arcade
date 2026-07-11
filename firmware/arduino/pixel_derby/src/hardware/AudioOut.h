#pragma once
#include <Arduino.h>
#include <esp_arduino_version.h>
#include "../Config.h"
#include "../assets/sounds/ArcadeSounds.h"

enum class AudioProfile : uint8_t { MUTE, QUIET, NORMAL };

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

  void update() {
    if (!playing || profile == AudioProfile::MUTE) return;
    const uint32_t now = millis();
    if (now < nextChangeMs) return;
    if (toneOn) {
      stopTone();
      toneOn = false;
      nextChangeMs = now + current.steps[index].gapMs;
      if (++index >= current.count) playing = false;
      return;
    }
    if (index >= current.count) { playing = false; return; }
    const ToneStep& step = current.steps[index];
    startTone(step.frequencyHz, effectiveDuty(step.duty));
    toneOn = true;
    nextChangeMs = now + step.durationMs;
  }

  void play(SoundId id, bool interrupt = true) {
    if (profile == AudioProfile::MUTE) return;
    if (playing && !interrupt) return;
    stopTone();
    current = soundPattern(id);
    index = 0;
    toneOn = false;
    playing = current.count > 0;
    nextChangeMs = millis();
  }

  void setProfile(AudioProfile next) { profile = next; if (profile == AudioProfile::MUTE) { playing = false; stopTone(); } }
  AudioProfile getProfile() const { return profile; }

  void playerJoined() { play(SoundId::PLAYER_JOINED); }
  void ready() { play(SoundId::READY); }
  void countdown(uint8_t value) { play(value == 0 ? SoundId::GO : SoundId::COUNTDOWN); }
  void tap() {}
  void winner() { play(SoundId::WINNER); }
  void powerUp() { play(SoundId::POWER_UP); }
  void crash() { play(SoundId::CRASH); }
  void defeat() { play(SoundId::DEFEAT); }
  void restart() { play(SoundId::RESTART); }
  void bossIntro() { play(SoundId::BOSS_INTRO); }
  void bossDefeated() { play(SoundId::BOSS_DEFEATED); }
  void menuSelect() { play(SoundId::MENU_SELECT); }
  void gameStart() { play(SoundId::GAME_START); }
  void newRecord() { play(SoundId::NEW_RECORD); }
  void attractChime() { play(SoundId::ATTRACT_CHIME, false); }
  void demoCue() { play(SoundId::DEMO_CUE, false); }
  void inviteCue() { play(SoundId::INVITE_CUE, false); }
  bool isPlaying() const { return playing; }

private:
  AudioProfile profile = AudioProfile::NORMAL;
  SoundPattern current{nullptr,0};
  uint8_t index = 0;
  uint32_t nextChangeMs = 0;
  bool playing = false;
  bool toneOn = false;

  uint8_t effectiveDuty(uint8_t requested) const {
    if (profile == AudioProfile::QUIET) return requested / 3 < 2 ? 2 : requested / 3;
    return requested;
  }

  void startTone(uint16_t frequency, uint8_t duty) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcWriteTone(AUDIO_PIN, frequency);
    ledcWrite(AUDIO_PIN, duty);
#else
    ledcWriteTone(0, frequency);
    ledcWrite(0, duty);
#endif
  }

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
