#pragma once
#include <Arduino.h>
#include "../../Config.h"
#include "../../Types.h"
#include "../../session/PlayerManager.h"
#include "../../hardware/AudioOut.h"

class PixelDerbyGame {
public:
  ArcadeStage stage = ArcadeStage::PLATFORM_SELECT;
  int8_t winner = -1;
  uint8_t countdownValue = 3;
  uint32_t countdownChangedAtMs = 0;
  uint32_t raceStartedAtMs = 0;
  uint32_t deviceBestRaceMs = 0;
  bool newDeviceRecord = false;
  uint8_t racesSinceBoss = 0;

  int8_t bossSlot = -1;
  int8_t previousBossSlot = -1;
  int8_t lastBossHitSlot = -1;
  int8_t topRaiderSlot = -1;
  uint16_t bossHp = 0;
  uint16_t bossMaxHp = 0;
  uint32_t bossEndsAtMs = 0;
  bool bossDefeated = false;
  uint8_t bossPulseCount = 0;

  void selectPlatform() {
    if (stage == ArcadeStage::PLATFORM_SELECT) stage = ArcadeStage::GAME_SELECT;
  }

  void selectGame(PlayerManager& players) {
    if (stage != ArcadeStage::GAME_SELECT) return;
    players.resetMatch();
    winner = -1;
    newDeviceRecord = false;
    racesSinceBoss = 0;
    bossSlot = -1;
    previousBossSlot = -1;
    stage = ArcadeStage::LOBBY;
  }

  bool setReady(uint8_t slot, bool ready, PlayerManager& players, AudioOut& audio) {
    if (stage != ArcadeStage::LOBBY || slot >= MAX_PLAYERS) return false;
    PlayerSlot& p = players.players[slot];
    if (!p.occupied || !p.connected || p.waiting) return false;
    p.ready = ready;
    audio.ready();
    return true;
  }

  bool setWaiting(uint8_t slot, bool waiting, PlayerManager& players) {
    if (slot >= MAX_PLAYERS) return false;
    return players.setWaiting(slot, waiting);
  }

  bool start(uint8_t requester, PlayerManager& players, AudioOut& audio) {
    if (stage != ArcadeStage::LOBBY) return false;
    if (requester >= MAX_PLAYERS || !players.players[requester].connected || players.players[requester].waiting) return false;
    if (!players.allConnectedReady()) return false;
    players.resetRound(false);
    winner = -1;
    newDeviceRecord = false;
    countdownValue = 3;
    countdownChangedAtMs = millis();
    raceStartedAtMs = 0;
    stage = ArcadeStage::COUNTDOWN;
    audio.countdown(countdownValue);
    return true;
  }

  bool tap(uint8_t slot, PlayerManager& players, AudioOut& audio) {
    if (slot >= MAX_PLAYERS) return false;
    PlayerSlot& p = players.players[slot];
    if (!p.occupied || !p.connected || p.waiting) return false;

    const uint32_t now = millis();
    if (now - p.lastTapMs < TAP_DEBOUNCE_MS) return false;
    p.lastTapMs = now;

    if (stage == ArcadeStage::RACING) {
      uint8_t step = p.turboTaps > 0 ? 2 : 1;
      if (p.turboTaps > 0) p.turboTaps--;
      const uint8_t oldPosition = p.position;
      const uint16_t advanced = uint16_t(p.position) + step;
      p.position = advanced > FINISH_X ? FINISH_X : uint8_t(advanced);
      claimTurbo(p, oldPosition, p.position);
      if (p.position >= FINISH_X) finishRace(slot, players, audio, now);
      return true;
    }

    if (stage == ArcadeStage::BOSS) {
      if (slot == bossSlot) {
        p.bossEnergy++;
        if (p.bossEnergy >= BOSS_PULSE_TAPS) {
          p.bossEnergy = 0;
          bossPulseCount++;
          for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
            PlayerSlot& raider = players.players[i];
            if (!raider.occupied || !raider.connected || raider.waiting || i == bossSlot) continue;
            raider.stunnedUntilMs = now + BOSS_PULSE_STUN_MS;
          }
          audio.ready();
        }
        return true;
      }

      if (now < p.stunnedUntilMs || bossHp == 0) return false;
      const uint8_t damage = p.turboTaps > 0 ? 2 : 1;
      if (p.turboTaps > 0) p.turboTaps--;
      p.bossDamage += damage;
      lastBossHitSlot = slot;
      bossHp = damage >= bossHp ? 0 : bossHp - damage;
      if (bossHp == 0) finishBoss(players, audio, true);
      return true;
    }

    return false;
  }

  bool rematch(uint8_t requester, PlayerManager& players) {
    if (requester >= MAX_PLAYERS || !players.players[requester].connected || players.players[requester].waiting) return false;

    if (stage == ArcadeStage::RESULT) {
      if (racesSinceBoss >= RACES_PER_BOSS) {
        startBoss(players);
        return true;
      }
      const bool matchOver = winner >= 0 && players.players[winner].score >= MATCH_WIN_SCORE;
      if (matchOver) players.resetMatch();
      else players.resetRound(true);
      winner = -1;
      newDeviceRecord = false;
      stage = ArcadeStage::LOBBY;
      return true;
    }

    if (stage == ArcadeStage::BOSS_RESULT) {
      players.resetRound(true);
      winner = -1;
      newDeviceRecord = false;
      stage = ArcadeStage::LOBBY;
      return true;
    }

    return false;
  }

  void update(PlayerManager& players, AudioOut& audio) {
    const uint32_t now = millis();

    if (stage == ArcadeStage::COUNTDOWN) {
      if (now - countdownChangedAtMs < COUNTDOWN_STEP_MS) return;
      countdownChangedAtMs = now;
      if (countdownValue > 1) {
        countdownValue--;
        audio.countdown(countdownValue);
        return;
      }
      countdownValue = 0;
      audio.countdown(0);
      raceStartedAtMs = now;
      stage = ArcadeStage::RACING;
      return;
    }

    if (stage == ArcadeStage::BOSS && now >= bossEndsAtMs) finishBoss(players, audio, false);
  }

  uint32_t bossRemainingMs() const {
    if (stage != ArcadeStage::BOSS) return 0;
    const uint32_t now = millis();
    return now >= bossEndsAtMs ? 0 : bossEndsAtMs - now;
  }

  uint32_t stunRemainingMs(const PlayerSlot& p) const {
    const uint32_t now = millis();
    return now >= p.stunnedUntilMs ? 0 : p.stunnedUntilMs - now;
  }

private:
  void claimTurbo(PlayerSlot& p, uint8_t oldPosition, uint8_t newPosition) {
    if (!p.turbo1Claimed && oldPosition < TURBO_X_1 && newPosition >= TURBO_X_1) {
      p.turbo1Claimed = true;
      p.turboTaps = TURBO_TAPS;
      p.totalPoints += 10;
      p.bossCandidateScore++;
    }
    if (!p.turbo2Claimed && oldPosition < TURBO_X_2 && newPosition >= TURBO_X_2) {
      p.turbo2Claimed = true;
      p.turboTaps = TURBO_TAPS;
      p.totalPoints += 10;
      p.bossCandidateScore++;
    }
  }

  void addRaceCandidateScores(uint8_t winningSlot, PlayerManager& players) {
    int8_t order[MAX_PLAYERS];
    uint8_t count = 0;
    for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
      const PlayerSlot& p = players.players[i];
      if (!p.occupied || !p.connected || p.waiting) continue;
      order[count++] = i;
    }
    for (uint8_t i = 0; i < count; i++) {
      for (uint8_t j = i + 1; j < count; j++) {
        const PlayerSlot& a = players.players[order[i]];
        const PlayerSlot& b = players.players[order[j]];
        if (b.position > a.position || (b.position == a.position && order[j] == winningSlot)) {
          const int8_t tmp = order[i]; order[i] = order[j]; order[j] = tmp;
        }
      }
    }
    if (count > 0) players.players[order[0]].bossCandidateScore += 5;
    if (count > 1) players.players[order[1]].bossCandidateScore += 3;
    if (count > 2) players.players[order[2]].bossCandidateScore += 2;
  }

  void finishRace(uint8_t winningSlot, PlayerManager& players, AudioOut& audio, uint32_t now) {
    winner = winningSlot;
    racesSinceBoss++;
    const uint32_t duration = raceStartedAtMs > 0 ? now - raceStartedAtMs : 0;

    addRaceCandidateScores(winningSlot, players);

    for (uint8_t slot = 0; slot < MAX_PLAYERS; slot++) {
      PlayerSlot& p = players.players[slot];
      if (!p.occupied || !p.connected || p.waiting) continue;
      p.totalPoints += 20;
      p.newPersonalRecord = false;
      if (slot != winningSlot) p.streak = 0;
    }

    PlayerSlot& champion = players.players[winningSlot];
    champion.score++;
    champion.wins++;
    champion.streak++;
    if (champion.streak > champion.bestStreak) champion.bestStreak = champion.streak;
    champion.totalPoints += 80;
    champion.lastRaceMs = duration;

    if (duration > 0 && (champion.personalBestMs == 0 || duration < champion.personalBestMs)) {
      champion.personalBestMs = duration;
      champion.newPersonalRecord = true;
      champion.bossCandidateScore += 3;
    }

    newDeviceRecord = duration > 0 && (deviceBestRaceMs == 0 || duration < deviceBestRaceMs);
    if (newDeviceRecord) deviceBestRaceMs = duration;

    stage = ArcadeStage::RESULT;
    audio.winner();
  }

  int8_t chooseBoss(const PlayerManager& players) const {
    int8_t selected = -1;
    int32_t best = -32768;
    for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
      const PlayerSlot& p = players.players[i];
      if (!p.occupied || !p.connected || p.waiting) continue;
      int32_t candidate = p.bossCandidateScore;
      if (i == previousBossSlot) candidate -= 4;
      candidate = candidate * 1000 + p.totalPoints;
      if (selected < 0 || candidate > best) {
        selected = i;
        best = candidate;
      }
    }
    return selected;
  }

  void startBoss(PlayerManager& players) {
    racesSinceBoss = 0;
    players.resetRound(false);
    bossSlot = chooseBoss(players);
    previousBossSlot = bossSlot;
    lastBossHitSlot = -1;
    topRaiderSlot = -1;
    bossPulseCount = 0;

    const uint8_t active = players.activeCount();
    const uint8_t raiders = active > 0 ? active - 1 : 0;
    bossMaxHp = BOSS_BASE_HP + uint16_t(raiders) * BOSS_HP_PER_RAIDER;
    bossHp = bossMaxHp;
    bossEndsAtMs = millis() + BOSS_DURATION_MS;
    bossDefeated = false;

    for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
      PlayerSlot& p = players.players[i];
      p.bossDamage = 0;
      p.bossEnergy = 0;
      p.stunnedUntilMs = 0;
      p.bossCandidateScore = 0;
    }
    stage = ArcadeStage::BOSS;
  }

  void finishBoss(PlayerManager& players, AudioOut& audio, bool defeated) {
    if (stage != ArcadeStage::BOSS) return;
    bossDefeated = defeated;

    if (defeated) {
      uint16_t topDamage = 0;
      for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
        const PlayerSlot& p = players.players[i];
        if (!p.occupied || !p.connected || p.waiting || i == bossSlot) continue;
        if (p.bossDamage > topDamage) { topDamage = p.bossDamage; topRaiderSlot = i; }
      }
      for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
        PlayerSlot& p = players.players[i];
        if (!p.occupied || !p.connected || p.waiting || i == bossSlot) continue;
        p.totalPoints += 50 + p.bossDamage;
      }
      if (topRaiderSlot >= 0) players.players[topRaiderSlot].totalPoints += 30;
      if (lastBossHitSlot >= 0 && lastBossHitSlot != topRaiderSlot) players.players[lastBossHitSlot].totalPoints += 20;
      audio.winner();
    } else if (bossSlot >= 0) {
      PlayerSlot& boss = players.players[bossSlot];
      boss.totalPoints += 100;
      audio.winner();
    }

    stage = ArcadeStage::BOSS_RESULT;
  }
};
