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
    ATTRACT_PAC_CHASE,
    ATTRACT_PONG_CPU,
    ATTRACT_STACK_CPU,
    ATTRACT_RAIDER_CPU,
    ATTRACT_JOIN
  };

  void begin(AudioOut& audioRef) {
    audio = &audioRef;
    cueStartedMs = millis();
    nextAttractAudioMs = cueStartedMs + ATTRACT_AUDIO_INITIAL_DELAY_MS;
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
  static constexpr uint8_t ATTRACT_PHASE_COUNT = 5;

  AudioOut* audio = nullptr;
  ArcadeStage previousStage = ArcadeStage::PLATFORM_SELECT;
  GameId previousGame = GameId::NONE;
  int8_t previousWinner = -2;
  bool previousRecord = false;
  bool initialized = false;
  bool attractRunning = false;
  uint8_t attractPhase = 255;
  uint32_t attractPhaseStartedMs = 0;
  uint32_t nextAttractAudioMs = 0;
  uint8_t attractAudioCue = 0;
  VisualCue activeCue = VisualCue::MENU_ATTRACT;
  uint32_t cueStartedMs = 0;

  void updateAttract() {
    const uint32_t now = millis();
    if (!attractRunning) {
      attractRunning = true;
      attractPhase = 255;
      attractPhaseStartedMs = now;
      attractAudioCue = 0;
      nextAttractAudioMs = now + ATTRACT_AUDIO_INITIAL_DELAY_MS;
    }

    if (attractPhase == 255 || now - attractPhaseStartedMs >= attractPhaseDurationMs(attractPhase)) {
      attractPhase = attractPhase == 255 ? 0 : uint8_t((attractPhase + 1) % ATTRACT_PHASE_COUNT);
      attractPhaseStartedMs = now;
      switch (attractPhase) {
        case 0:
          setCue(VisualCue::ATTRACT_PAC_CHASE);
          break;
        case 1:
          setCue(VisualCue::ATTRACT_PONG_CPU);
          break;
        case 2:
          setCue(VisualCue::ATTRACT_STACK_CPU);
          break;
        case 3:
          setCue(VisualCue::ATTRACT_RAIDER_CPU);
          break;
        default:
          setCue(VisualCue::ATTRACT_JOIN);
          break;
      }
    }

    maybePlayAttractAudio(now);
  }

  uint32_t attractPhaseDurationMs(uint8_t phase) const {
    switch (phase) {
      case 0: return ATTRACT_PAC_CHASE_MS;
      case 1: return ATTRACT_PONG_CPU_MS;
      case 2: return ATTRACT_STACK_CPU_MS;
      case 3: return ATTRACT_RAIDER_CPU_MS;
      default: return ATTRACT_JOIN_MS;
    }
  }

  void maybePlayAttractAudio(uint32_t now) {
    if (!audio || int32_t(now - nextAttractAudioMs) < 0) return;

    switch (attractAudioCue % 3) {
      case 0:
        audio->attractChime();
        break;
      case 1:
        audio->inviteCue();
        break;
      default:
        audio->demoCue();
        break;
    }

    attractAudioCue++;
    nextAttractAudioMs = now + ATTRACT_AUDIO_INTERVAL_MS;
  }

  void updateGameCues(const ArcadeGameEngine& game) {
    const bool browserAudio = game.selectedArena == ArenaType::SCREEN_ARCADE;
    if (!initialized) {
      previousStage = game.stage;
      previousGame = game.selectedGame;
      previousWinner = game.winner;
      previousRecord = game.newDeviceRecord || game.raiderNewRecord || game.stack.newRecord;
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
      if (game.stage == ArcadeStage::COUNTDOWN && old != ArcadeStage::ANNOUNCE && audio) audio->gameStart();
      if (game.stage == ArcadeStage::ANNOUNCE && audio) audio->bossIntro();
      if (game.stage == ArcadeStage::RESULT) {
        setCue(VisualCue::WINNER);
        if (audio && !browserAudio) {
          if (game.newDeviceRecord || game.raiderNewRecord || game.stack.newRecord) audio->newRecord();
          else if (game.winner >= 0) audio->winner();
          else audio->defeat();
        }
      }
      if (game.stage == ArcadeStage::BOSS_RESULT && audio) {
        if (game.bossDefeated) audio->bossDefeated(); else audio->winner();
      }
      if (game.stage == ArcadeStage::LOBBY && (old == ArcadeStage::RESULT || old == ArcadeStage::BOSS_RESULT) && audio) audio->restart();
    }

    previousWinner = game.winner;
    previousRecord = game.newDeviceRecord || game.raiderNewRecord || game.stack.newRecord;
  }

  void setCue(VisualCue cue) {
    activeCue = cue;
    cueStartedMs = millis();
  }
};
