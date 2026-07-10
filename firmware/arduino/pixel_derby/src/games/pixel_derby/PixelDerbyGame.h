#pragma once
#include <Arduino.h>
#include "../../Config.h"
#include "../../Types.h"
#include "../../session/PlayerManager.h"
#include "../../hardware/AudioOut.h"

class PixelDerbyGame {
public:
  ArcadeStage stage = ArcadeStage::PLATFORM_SELECT;
  GameId selectedGame = GameId::NONE;
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

  uint8_t announcePhase = 0;
  bool pendingBoss = false;
  uint32_t announceChangedAtMs = 0;
  uint8_t tronTrail[LED_COUNT] = {0}; // 0 empty, slot+1 occupied
  uint32_t tronLastTickMs = 0;
  uint32_t tronStartedAtMs = 0;
  uint32_t cpuNextActionMs = 0;

  void selectPlatform() {
    if (stage == ArcadeStage::PLATFORM_SELECT) stage = ArcadeStage::GAME_SELECT;
  }

  void backToGames(PlayerManager& players) {
    players.removeAutomaticCpu();
    players.resetRound(true);
    selectedGame = GameId::NONE;
    winner = -1;
    bossSlot = -1;
    pendingBoss = false;
    stage = ArcadeStage::GAME_SELECT;
  }

  void selectGame(GameId game, PlayerManager& players) {
    if (stage != ArcadeStage::GAME_SELECT) return;
    selectedGame = game;
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
    players.ensureAutomaticCpu();
    if (!players.allConnectedReady()) return false;
    const uint8_t active = players.activeCount();
    if (selectedGame == GameId::TRON_ARENA && (active < TRON_MIN_PLAYERS || active > TRON_MAX_PLAYERS)) return false;

    players.resetRound(false);
    winner = -1;
    newDeviceRecord = false;
    countdownValue = 3;
    countdownChangedAtMs = millis();
    raceStartedAtMs = 0;
    if (selectedGame == GameId::TRON_ARENA) prepareTron(players);
    stage = ArcadeStage::COUNTDOWN;
    audio.countdown(countdownValue);
    return true;
  }

  bool tap(uint8_t slot, PlayerManager& players, AudioOut& audio) {
    if (selectedGame != GameId::PIXEL_DERBY || slot >= MAX_PLAYERS) return false;
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

  bool turn(uint8_t slot, bool left, PlayerManager& players) {
    if (selectedGame != GameId::TRON_ARENA || stage != ArcadeStage::RACING || slot >= MAX_PLAYERS) return false;
    PlayerSlot& p = players.players[slot];
    if (!p.occupied || !p.connected || p.waiting || !p.tronAlive) return false;
    const int dir = int(p.tronDirection);
    p.tronDirection = TronDirection((dir + (left ? 3 : 1)) % 4);
    return true;
  }

  bool rematch(uint8_t requester, PlayerManager& players) {
    if (requester >= MAX_PLAYERS || !players.players[requester].connected || players.players[requester].waiting) return false;
    if (stage == ArcadeStage::RESULT) {
      if (selectedGame == GameId::PIXEL_DERBY && racesSinceBoss >= RACES_PER_BOSS) {
        beginBossIntro(players);
        return true;
      }
      const bool matchOver = winner >= 0 && players.players[winner].score >= MATCH_WIN_SCORE;
      if (matchOver) players.resetMatch(); else players.resetRound(true);
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
    if (stage == ArcadeStage::LOBBY) players.ensureAutomaticCpu();
    if (stage == ArcadeStage::ANNOUNCE) {
      if (now - announceChangedAtMs < ANNOUNCE_STEP_MS) return;
      announceChangedAtMs = now;
      announcePhase++;
      if (announcePhase >= 4) {
        countdownValue = 3;
        countdownChangedAtMs = now;
        stage = ArcadeStage::COUNTDOWN;
        audio.countdown(3);
      }
      return;
    }
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
      if (pendingBoss) { pendingBoss = false; startBoss(players); return; }
      raceStartedAtMs = now;
      cpuNextActionMs = now + 300;
      if (selectedGame == GameId::TRON_ARENA) {
        tronStartedAtMs = now;
        tronLastTickMs = now;
      }
      stage = ArcadeStage::RACING;
      return;
    }
    updateCpu(players, audio, now);
    if (stage == ArcadeStage::BOSS && now >= bossEndsAtMs) finishBoss(players, audio, false);
    if (stage == ArcadeStage::RACING && selectedGame == GameId::TRON_ARENA) updateTron(players, audio, now);
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

  uint8_t tronAliveCount(const PlayerManager& players) const {
    uint8_t count = 0;
    for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
      const PlayerSlot& p = players.players[i];
      if (p.occupied && p.connected && !p.waiting && p.tronAlive) count++;
    }
    return count;
  }

private:
  void scheduleCpuAction(uint32_t now, uint32_t minMs, uint32_t maxMs) {
    cpuNextActionMs = now + random(minMs, maxMs + 1);
  }

  bool tronCellFree(int8_t x, int8_t y) const {
    return x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT && tronTrail[y * MATRIX_WIDTH + x] == 0;
  }

  uint8_t tronClearance(const PlayerSlot& p, TronDirection direction) const {
    int8_t x = p.tronX;
    int8_t y = p.tronY;
    uint8_t clear = 0;
    for (uint8_t step = 0; step < 8; step++) {
      switch (direction) {
        case TronDirection::UP: y--; break;
        case TronDirection::RIGHT: x++; break;
        case TronDirection::DOWN: y++; break;
        case TronDirection::LEFT: x--; break;
      }
      if (!tronCellFree(x, y)) break;
      clear++;
    }
    return clear;
  }

  void steerCpuTron(PlayerSlot& cpu) {
    const int dir = int(cpu.tronDirection);
    const TronDirection forward = cpu.tronDirection;
    const TronDirection left = TronDirection((dir + 3) % 4);
    const TronDirection right = TronDirection((dir + 1) % 4);
    const uint8_t forwardClear = tronClearance(cpu, forward);
    const uint8_t leftClear = tronClearance(cpu, left);
    const uint8_t rightClear = tronClearance(cpu, right);

    if (forwardClear >= 2 && random(100) >= 12) return;
    if (leftClear == 0 && rightClear == 0) return;
    if (leftClear > rightClear) cpu.tronDirection = left;
    else if (rightClear > leftClear) cpu.tronDirection = right;
    else cpu.tronDirection = random(2) == 0 ? left : right;
  }

  void updateCpu(PlayerManager& players, AudioOut& audio, uint32_t now) {
    const int slot = players.cpuSlot();
    if (slot < 0) return;
    PlayerSlot& cpu = players.players[slot];

    if (selectedGame == GameId::TRON_ARENA && stage == ArcadeStage::RACING) {
      return; // Steering happens immediately before each Tron movement tick.
    }

    if (selectedGame != GameId::PIXEL_DERBY) return;
    if (stage != ArcadeStage::RACING && stage != ArcadeStage::BOSS) return;
    if (now < cpuNextActionMs) return;

    tap(slot, players, audio);
    if (stage == ArcadeStage::BOSS) scheduleCpuAction(now, CPU_BOSS_TAP_MIN_MS, CPU_BOSS_TAP_MAX_MS);
    else scheduleCpuAction(now, CPU_DERBY_TAP_MIN_MS, CPU_DERBY_TAP_MAX_MS);
  }

  void claimTurbo(PlayerSlot& p, uint8_t oldPosition, uint8_t newPosition) {
    if (!p.turbo1Claimed && oldPosition < TURBO_X_1 && newPosition >= TURBO_X_1) {
      p.turbo1Claimed = true; p.turboTaps = TURBO_TAPS; p.totalPoints += 10; p.bossCandidateScore++;
    }
    if (!p.turbo2Claimed && oldPosition < TURBO_X_2 && newPosition >= TURBO_X_2) {
      p.turbo2Claimed = true; p.turboTaps = TURBO_TAPS; p.totalPoints += 10; p.bossCandidateScore++;
    }
  }

  void addRaceCandidateScores(uint8_t winningSlot, PlayerManager& players) {
    int8_t order[MAX_PLAYERS]; uint8_t count = 0;
    for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
      const PlayerSlot& p = players.players[i];
      if (p.occupied && p.connected && !p.waiting) order[count++] = i;
    }
    for (uint8_t i = 0; i < count; i++) for (uint8_t j = i + 1; j < count; j++) {
      const PlayerSlot& a = players.players[order[i]]; const PlayerSlot& b = players.players[order[j]];
      if (b.position > a.position || (b.position == a.position && order[j] == winningSlot)) { int8_t t=order[i]; order[i]=order[j]; order[j]=t; }
    }
    if (count > 0) players.players[order[0]].bossCandidateScore += 5;
    if (count > 1) players.players[order[1]].bossCandidateScore += 3;
    if (count > 2) players.players[order[2]].bossCandidateScore += 2;
  }

  void finishRace(uint8_t winningSlot, PlayerManager& players, AudioOut& audio, uint32_t now) {
    winner = winningSlot; racesSinceBoss++;
    const uint32_t duration = raceStartedAtMs > 0 ? now - raceStartedAtMs : 0;
    addRaceCandidateScores(winningSlot, players);
    for (uint8_t slot = 0; slot < MAX_PLAYERS; slot++) {
      PlayerSlot& p = players.players[slot];
      if (!p.occupied || !p.connected || p.waiting) continue;
      p.totalPoints += 20; p.newPersonalRecord = false; if (slot != winningSlot) p.streak = 0;
    }
    PlayerSlot& champion = players.players[winningSlot];
    champion.score++; champion.wins++; champion.streak++; champion.bestStreak = max(champion.bestStreak, champion.streak); champion.totalPoints += 80; champion.lastRaceMs = duration;
    if (duration > 0 && (champion.personalBestMs == 0 || duration < champion.personalBestMs)) { champion.personalBestMs = duration; champion.newPersonalRecord = true; champion.bossCandidateScore += 3; }
    newDeviceRecord = duration > 0 && (deviceBestRaceMs == 0 || duration < deviceBestRaceMs);
    if (newDeviceRecord) deviceBestRaceMs = duration;
    stage = ArcadeStage::RESULT; audio.winner();
  }

  int8_t chooseBoss(const PlayerManager& players) const {
    int8_t selected = -1; int32_t best = -32768;
    for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
      const PlayerSlot& p = players.players[i];
      if (!p.occupied || !p.connected || p.waiting) continue;
      int32_t candidate = p.bossCandidateScore; if (i == previousBossSlot) candidate -= 4; candidate = candidate * 1000 + p.totalPoints;
      if (selected < 0 || candidate > best) { selected = i; best = candidate; }
    }
    return selected;
  }

  void beginBossIntro(PlayerManager& players) {
    racesSinceBoss = 0; players.resetRound(false); bossSlot = chooseBoss(players); previousBossSlot = bossSlot;
    pendingBoss = true; announcePhase = 0; announceChangedAtMs = millis(); stage = ArcadeStage::ANNOUNCE;
  }

  void startBoss(PlayerManager& players) {
    lastBossHitSlot = -1; topRaiderSlot = -1; bossPulseCount = 0;
    const uint8_t active = players.activeCount(); const uint8_t raiders = active > 0 ? active - 1 : 0;
    bossMaxHp = BOSS_BASE_HP + uint16_t(raiders) * BOSS_HP_PER_RAIDER; bossHp = bossMaxHp;
    bossEndsAtMs = millis() + BOSS_DURATION_MS; bossDefeated = false;
    for (uint8_t i = 0; i < MAX_PLAYERS; i++) { PlayerSlot& p=players.players[i]; p.bossDamage=0; p.bossEnergy=0; p.stunnedUntilMs=0; p.bossCandidateScore=0; }
    stage = ArcadeStage::BOSS;
  }

  void finishBoss(PlayerManager& players, AudioOut& audio, bool defeated) {
    if (stage != ArcadeStage::BOSS) return; bossDefeated = defeated;
    if (defeated) {
      uint16_t topDamage = 0;
      for (uint8_t i=0;i<MAX_PLAYERS;i++){const PlayerSlot&p=players.players[i];if(!p.occupied||!p.connected||p.waiting||i==bossSlot)continue;if(p.bossDamage>topDamage){topDamage=p.bossDamage;topRaiderSlot=i;}}
      for (uint8_t i=0;i<MAX_PLAYERS;i++){PlayerSlot&p=players.players[i];if(!p.occupied||!p.connected||p.waiting||i==bossSlot)continue;p.totalPoints+=50+p.bossDamage;}
      if(topRaiderSlot>=0)players.players[topRaiderSlot].totalPoints+=30;if(lastBossHitSlot>=0&&lastBossHitSlot!=topRaiderSlot)players.players[lastBossHitSlot].totalPoints+=20;
    } else if (bossSlot >= 0) players.players[bossSlot].totalPoints += 100;
    stage = ArcadeStage::BOSS_RESULT; audio.winner();
  }

  void prepareTron(PlayerManager& players) {
    memset(tronTrail, 0, sizeof(tronTrail));
    static const int8_t xs[4] = {3, 28, 3, 28};
    static const int8_t ys[4] = {2, 5, 5, 2};
    static const TronDirection dirs[4] = {TronDirection::RIGHT, TronDirection::LEFT, TronDirection::RIGHT, TronDirection::LEFT};
    uint8_t order = 0;
    for (uint8_t i=0;i<MAX_PLAYERS;i++) {
      PlayerSlot& p=players.players[i];
      if(!p.occupied||!p.connected||p.waiting)continue;
      p.tronX=xs[order];p.tronY=ys[order];p.tronDirection=dirs[order];p.tronAlive=true;
      tronTrail[p.tronY*MATRIX_WIDTH+p.tronX]=i+1; order++;
    }
  }

  uint32_t tronTickMs(uint32_t now) const {
    const uint32_t elapsed = now - tronStartedAtMs;
    if (elapsed >= 20000) return TRON_TICK_FAST_MS;
    if (elapsed >= 10000) return TRON_TICK_MID_MS;
    return TRON_TICK_START_MS;
  }

  void updateTron(PlayerManager& players, AudioOut& audio, uint32_t now) {
    if (now - tronLastTickMs < tronTickMs(now)) return;
    tronLastTickMs = now;
    const int cpu = players.cpuSlot();
    if (cpu >= 0 && players.players[cpu].tronAlive) steerCpuTron(players.players[cpu]);
    int8_t nx[MAX_PLAYERS], ny[MAX_PLAYERS]; bool die[MAX_PLAYERS] = {false};
    for(uint8_t i=0;i<MAX_PLAYERS;i++){nx[i]=-1;ny[i]=-1;PlayerSlot&p=players.players[i];if(!p.occupied||!p.connected||p.waiting||!p.tronAlive)continue;nx[i]=p.tronX;ny[i]=p.tronY;switch(p.tronDirection){case TronDirection::UP:ny[i]--;break;case TronDirection::RIGHT:nx[i]++;break;case TronDirection::DOWN:ny[i]++;break;case TronDirection::LEFT:nx[i]--;break;}if(nx[i]<0||nx[i]>=MATRIX_WIDTH||ny[i]<0||ny[i]>=MATRIX_HEIGHT)die[i]=true;else if(tronTrail[ny[i]*MATRIX_WIDTH+nx[i]]!=0)die[i]=true;}
    for(uint8_t i=0;i<MAX_PLAYERS;i++)for(uint8_t j=i+1;j<MAX_PLAYERS;j++)if(nx[i]>=0&&nx[i]==nx[j]&&ny[i]==ny[j]){die[i]=true;die[j]=true;}
    for(uint8_t i=0;i<MAX_PLAYERS;i++){PlayerSlot&p=players.players[i];if(!p.tronAlive)continue;if(die[i]){p.tronAlive=false;continue;}p.tronX=nx[i];p.tronY=ny[i];tronTrail[p.tronY*MATRIX_WIDTH+p.tronX]=i+1;}
    const uint8_t alive=tronAliveCount(players);
    if(alive<=1){winner=-1;for(uint8_t i=0;i<MAX_PLAYERS;i++)if(players.players[i].tronAlive){winner=i;break;}if(winner>=0){PlayerSlot&w=players.players[winner];w.score++;w.wins++;w.totalPoints+=100;w.streak++;w.bestStreak=max(w.bestStreak,w.streak);}for(uint8_t i=0;i<MAX_PLAYERS;i++)if(i!=winner&&players.players[i].occupied)players.players[i].streak=0;stage=ArcadeStage::RESULT;audio.winner();}
  }

public:
  void enterBossAfterIntro(PlayerManager& players) { startBoss(players); }
};
