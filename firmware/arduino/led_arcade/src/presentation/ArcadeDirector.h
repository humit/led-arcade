#pragma once
#include <Arduino.h>
#include "../Types.h"
#include "../hardware/AudioOut.h"
#include "../core/ArcadeGameEngine.h"
#include "../session/PlayerManager.h"

class ArcadeDirector {
public:
  enum class VisualCue : uint8_t {
    NONE,
    MENU_ATTRACT,
    DERBY,
    RAIDER,
    PAINT,
    PONG,
    WINNER,
    ATTRACT_CHOMPER,
    ATTRACT_DERBY_DEMO,
    ATTRACT_RAIDER_DEMO,
    ATTRACT_PAINT_DEMO,
    ATTRACT_JOIN
  };

  void begin(AudioOut& audioRef) {
    audio = &audioRef;
    cueStartedMs = millis();
    attractStartedMs = cueStartedMs;
  }

  void update(const ArcadeGameEngine& game, const PlayerManager& players) {
    const bool menuStage = game.stage == ArcadeStage::PLATFORM_SELECT || game.stage == ArcadeStage::GAME_SELECT;
    const bool noHumans = players.humanActiveCount() == 0;

    if (menuStage && noHumans) {
      updateAttract();
    } else {
      if (attractRunning) {
        attractRunning = false;
        attractPhase = 255;
        setCue(VisualCue::MENU_ATTRACT);
      }
      updateGameCues(game);
    }
  }

  VisualCue cue() const { return activeCue; }
  uint32_t cueStartedAt() const { return cueStartedMs; }
  uint32_t cueAgeMs() const { return millis() - cueStartedMs; }
  bool cueActive(uint32_t durationMs) const { return cueAgeMs() < durationMs; }
  bool isAttractRunning() const { return attractRunning; }

private:
  static constexpr uint32_t ATTRACT_PHASE_MS = 7000;
  static constexpr uint8_t ATTRACT_PHASE_COUNT = 5;

  AudioOut* audio = nullptr;
  ArcadeStage previousStage = ArcadeStage::PLATFORM_SELECT;
  GameId previousGame = GameId::NONE;
  int8_t previousWinner = -2;
  bool previousRecord = false;
  bool initialized = false;
  bool attractRunning = false;
  uint8_t attractPhase = 255;
  uint32_t attractStartedMs = 0;
  VisualCue activeCue = VisualCue::MENU_ATTRACT;
  uint32_t cueStartedMs = 0;

  void updateAttract() {
    const uint32_t now = millis();
    if (!attractRunning) {
      attractRunning = true;
      attractStartedMs = now;
      attractPhase = 255;
    }

    const uint8_t phase = uint8_t(((now - attractStartedMs) / ATTRACT_PHASE_MS) % ATTRACT_PHASE_COUNT);
    if (phase == attractPhase) return;
    attractPhase = phase;

    switch (phase) {
      case 0:
        setCue(VisualCue::ATTRACT_CHOMPER);
        if (audio) audio->attractChime();
        break;
      case 1:
        setCue(VisualCue::ATTRACT_DERBY_DEMO);
        if (audio) audio->demoCue();
        break;
      case 2:
        setCue(VisualCue::ATTRACT_RAIDER_DEMO);
        if (audio) audio->demoCue();
        break;
      case 3:
        setCue(VisualCue::ATTRACT_PAINT_DEMO);
        if (audio) audio->demoCue();
        break;
      default:
        setCue(VisualCue::ATTRACT_JOIN);
        if (audio) audio->inviteCue();
        break;
    }
  }

  void updateGameCues(const ArcadeGameEngine& game) {
    const bool browserAudio = game.selectedArena == ArenaType::SCREEN_ARCADE;
    if (!initialized) {
      previousStage = game.stage;
      previousGame = game.selectedGame;
      previousWinner = game.winner;
      previousRecord = game.newDeviceRecord || game.raiderNewRecord;
      initialized = true;
      return;
    }

    if (game.selectedGame != previousGame) {
      previousGame = game.selectedGame;
      if (audio && !browserAudio) audio->menuSelect();
      if (game.selectedGame == GameId::PIXEL_DERBY) setCue(VisualCue::DERBY);
      else if (game.selectedGame == GameId::PIXEL_RAIDER) setCue(VisualCue::RAIDER);
      else if (game.selectedGame == GameId::COLOR_CLASH) setCue(VisualCue::PAINT);
      else if (game.selectedGame == GameId::PIXEL_PONG) setCue(VisualCue::PONG);
    }

    if (game.stage != previousStage) {
      const ArcadeStage old = previousStage;
      previousStage = game.stage;
      if (game.stage == ArcadeStage::PLATFORM_SELECT || game.stage == ArcadeStage::GAME_SELECT) setCue(VisualCue::MENU_ATTRACT);
      if (game.stage == ArcadeStage::COUNTDOWN && old != ArcadeStage::ANNOUNCE && audio && !browserAudio) audio->gameStart();
      if (game.stage == ArcadeStage::ANNOUNCE && audio && !browserAudio) audio->bossIntro();
      if (game.stage == ArcadeStage::RESULT) {
        setCue(VisualCue::WINNER);
        if (audio && !browserAudio) {
          if (game.newDeviceRecord || game.raiderNewRecord) audio->newRecord();
          else if (game.winner >= 0) audio->winner();
          else audio->defeat();
        }
      }
      if (game.stage == ArcadeStage::BOSS_RESULT && audio && !browserAudio) {
        if (game.bossDefeated) audio->bossDefeated(); else audio->winner();
      }
      if (game.stage == ArcadeStage::LOBBY && (old == ArcadeStage::RESULT || old == ArcadeStage::BOSS_RESULT) && audio && !browserAudio) audio->restart();
    }

    previousWinner = game.winner;
    previousRecord = game.newDeviceRecord || game.raiderNewRecord;
  }

  void setCue(VisualCue cue) {
    activeCue = cue;
    cueStartedMs = millis();
  }
};
