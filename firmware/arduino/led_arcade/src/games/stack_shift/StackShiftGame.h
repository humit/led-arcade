#pragma once
#include <Arduino.h>
#include "../../Config.h"
#include "../../session/PlayerManager.h"
#include "../../hardware/AudioOut.h"

enum class StackShiftInput : uint8_t {
  LEFT,
  RIGHT,
  ROTATE,
  SOFT_DROP,
  HARD_DROP
};

struct StackBlockOffset {
  int8_t x;
  int8_t y;
};

static_assert(
    STACK_WIDTH == MATRIX_HEIGHT && STACK_HEIGHT == MATRIX_WIDTH,
    "Stack Shift virtual board must match the rotated physical matrix"
);

// Seven tetrominoes, four clockwise rotations, four blocks per rotation.
static constexpr StackBlockOffset STACK_SHAPES[7][4][4] = {
  { // I
    {{0,1},{1,1},{2,1},{3,1}},
    {{2,0},{2,1},{2,2},{2,3}},
    {{0,2},{1,2},{2,2},{3,2}},
    {{1,0},{1,1},{1,2},{1,3}}
  },
  { // O
    {{1,0},{2,0},{1,1},{2,1}},
    {{1,0},{2,0},{1,1},{2,1}},
    {{1,0},{2,0},{1,1},{2,1}},
    {{1,0},{2,0},{1,1},{2,1}}
  },
  { // T
    {{1,0},{0,1},{1,1},{2,1}},
    {{1,0},{1,1},{2,1},{1,2}},
    {{0,1},{1,1},{2,1},{1,2}},
    {{1,0},{0,1},{1,1},{1,2}}
  },
  { // S
    {{1,0},{2,0},{0,1},{1,1}},
    {{1,0},{1,1},{2,1},{2,2}},
    {{1,1},{2,1},{0,2},{1,2}},
    {{0,0},{0,1},{1,1},{1,2}}
  },
  { // Z
    {{0,0},{1,0},{1,1},{2,1}},
    {{2,0},{1,1},{2,1},{1,2}},
    {{0,1},{1,1},{1,2},{2,2}},
    {{1,0},{0,1},{1,1},{0,2}}
  },
  { // J
    {{0,0},{0,1},{1,1},{2,1}},
    {{1,0},{2,0},{1,1},{1,2}},
    {{0,1},{1,1},{2,1},{2,2}},
    {{1,0},{1,1},{0,2},{1,2}}
  },
  { // L
    {{2,0},{0,1},{1,1},{2,1}},
    {{1,0},{1,1},{1,2},{2,2}},
    {{0,1},{1,1},{2,1},{0,2}},
    {{0,0},{1,0},{1,1},{1,2}}
  }
};

class StackShiftGame {
public:
  uint8_t board[STACK_HEIGHT][STACK_WIDTH] = {};
  uint8_t activePiece = 0;
  uint8_t nextPiece = 0;
  uint8_t rotation = 0;
  int8_t pieceX = 2;
  int8_t pieceY = -1;
  int8_t playerSlot = -1;

  uint32_t score = 0;
  uint32_t bestScore = 0;
  uint16_t clearedLines = 0;
  uint8_t level = 1;
  uint32_t fallIntervalMs = STACK_FALL_START_MS;

  bool running = false;
  bool paused = false;
  bool matchFinished = false;
  bool newRecord = false;
  bool lineClearActive = false;
  uint8_t lineClearCount = 0;
  uint8_t lineClearRows[4] = {};
  uint32_t lineClearStartedMs = 0;
  uint8_t ghostPiecesRemaining = 0;
  uint8_t ghostAwardPieces = 0;
  uint8_t lineClearStreak = 0;
  bool levelBreakActive = false;
  uint8_t levelBreakLevel = 1;
  uint8_t levelBreakEmptyRows = 0;
  uint8_t levelBreakAwardedRows = 0;
  uint32_t levelBreakBonus = 0;
  uint32_t levelBreakStartedMs = 0;
  bool perfectClearActive = false;
  bool pendingLevelBreak = false;
  uint32_t perfectClearBonus = 0;
  uint8_t perfectClearGhostAward = 0;
  uint32_t perfectClearStartedMs = 0;

  void clear() {
    memset(board, 0, sizeof(board));
    activePiece = 0;
    nextPiece = 0;
    rotation = 0;
    pieceX = 2;
    pieceY = -1;
    playerSlot = -1;
    score = 0;
    clearedLines = 0;
    level = 1;
    fallIntervalMs = STACK_FALL_START_MS;
    running = false;
    paused = false;
    matchFinished = false;
    newRecord = false;
    lineClearActive = false;
    lineClearCount = 0;
    memset(lineClearRows, 0, sizeof(lineClearRows));
    lineClearStartedMs = 0;
    ghostPiecesRemaining = STACK_GHOST_START_PIECES;
    ghostAwardPieces = 0;
    lineClearStreak = 0;
    levelBreakActive = false;
    levelBreakLevel = 1;
    levelBreakEmptyRows = 0;
    levelBreakAwardedRows = 0;
    levelBreakBonus = 0;
    levelBreakStartedMs = 0;
    perfectClearActive = false;
    pendingLevelBreak = false;
    perfectClearBonus = 0;
    perfectClearGhostAward = 0;
    perfectClearStartedMs = 0;
    lastFallMs = 0;
    pauseChangedAtMs = 0;
    pausedAtMs = 0;
    bagIndex = 7;
  }

  bool prepare(const PlayerManager& players) {
    clear();
    uint8_t activeHumans = 0;
    for (uint8_t slot = 0; slot < MAX_PLAYERS; ++slot) {
      const PlayerSlot& player = players.players[slot];
      if (!isActiveHuman(player)) continue;
      playerSlot = slot;
      ++activeHumans;
    }
    if (activeHumans != 1) {
      playerSlot = -1;
      return false;
    }
    return true;
  }

  void start(uint32_t now) {
    if (playerSlot < 0) {
      matchFinished = true;
      return;
    }
    running = true;
    matchFinished = false;
    newRecord = false;
    lastFallMs = now;
    nextPiece = drawPiece();
    spawnPiece();
    if (!canPlace(activePiece, rotation, pieceX, pieceY)) finishGame();
  }

  void update(const PlayerManager& players, AudioOut& audio, uint32_t now) {
    if (!running || matchFinished) return;
    if (!slotActive(players, playerSlot)) {
      finishGame();
      return;
    }
    if (paused) return;
    if (perfectClearActive) {
      if (now - perfectClearStartedMs >= STACK_PERFECT_CLEAR_MS) finishPerfectClear(audio, now);
      return;
    }
    if (levelBreakActive) {
      advanceLevelBreakScore(now);
      if (now - levelBreakStartedMs >= levelBreakDurationMs()) finishLevelBreak(now);
      return;
    }
    if (lineClearActive) {
      if (now - lineClearStartedMs >= STACK_LINE_CLEAR_DURATION_MS) {
        finishLineClear(audio, now);
      }
      return;
    }
    if (now - lastFallMs < fallIntervalMs) return;
    lastFallMs = now;
    if (!tryMove(0, 1)) lockPiece(audio);
  }

  bool input(
      uint8_t slot,
      StackShiftInput action,
      PlayerManager& players,
      AudioOut& audio
  ) {
    if (!running || paused || lineClearActive || levelBreakActive || perfectClearActive || matchFinished || slot != playerSlot || slot >= MAX_PLAYERS) return false;
    PlayerSlot& player = players.players[slot];
    if (!isActiveHuman(player)) return false;

    const uint32_t now = millis();
    if (now - player.lastTapMs < STACK_INPUT_DEBOUNCE_MS) return false;
    player.lastTapMs = now;

    switch (action) {
      case StackShiftInput::LEFT:
        return tryMove(-1, 0);
      case StackShiftInput::RIGHT:
        return tryMove(1, 0);
      case StackShiftInput::ROTATE:
        if (tryRotate()) {
          audio.stackRotate();
          return true;
        }
        return false;
      case StackShiftInput::SOFT_DROP:
        if (tryMove(0, 1)) {
          score += 1;
          lastFallMs = now;
          return true;
        }
        lockPiece(audio);
        return true;
      case StackShiftInput::HARD_DROP: {
        uint8_t distance = 0;
        while (tryMove(0, 1)) ++distance;
        score += uint32_t(distance) * 2U;
        lockPiece(audio);
        return true;
      }
    }
    return false;
  }

  bool togglePause(uint8_t slot, const PlayerManager& players, AudioOut& audio) {
    if (!running || matchFinished || slot != playerSlot || !slotActive(players, slot)) return false;
    const uint32_t now = millis();
    if (now - pauseChangedAtMs < STACK_PAUSE_DEBOUNCE_MS) return false;
    pauseChangedAtMs = now;
    paused = !paused;
    if (paused) {
      pausedAtMs = now;
    } else {
      if (pausedAtMs > 0) {
        const uint32_t pauseDuration = now - pausedAtMs;
        if (lineClearActive) lineClearStartedMs += pauseDuration;
        if (levelBreakActive) levelBreakStartedMs += pauseDuration;
        if (perfectClearActive) perfectClearStartedMs += pauseDuration;
      }
      pausedAtMs = 0;
    }
    lastFallMs = now;
    audio.menuSelect();
    return true;
  }

  uint8_t cell(uint8_t x, uint8_t y) const {
    if (x >= STACK_WIDTH || y >= STACK_HEIGHT) return 0;
    return board[y][x];
  }

  bool lineMarked(uint8_t y) const {
    if (!lineClearActive) return false;
    for (uint8_t i = 0; i < lineClearCount; ++i) {
      if (lineClearRows[i] == y) return true;
    }
    return false;
  }

  uint8_t lineClearPhase(uint32_t now) const {
    if (!lineClearActive) return 0;
    const uint32_t visualNow = paused && pausedAtMs > 0 ? pausedAtMs : now;
    return uint8_t((visualNow - lineClearStartedMs) / STACK_LINE_CLEAR_FLASH_MS);
  }

  bool activeAt(uint8_t x, uint8_t y) const {
    if (!running || matchFinished || lineClearActive || levelBreakActive || perfectClearActive) return false;
    for (uint8_t block = 0; block < 4; ++block) {
      const StackBlockOffset& offset = STACK_SHAPES[activePiece][rotation][block];
      const int8_t blockX = pieceX + offset.x;
      const int8_t blockY = pieceY + offset.y;
      if (blockX == int8_t(x) && blockY == int8_t(y)) return true;
    }
    return false;
  }

  bool ghostVisible() const {
    return running && !paused && !matchFinished && !lineClearActive &&
        !levelBreakActive && !perfectClearActive && ghostPiecesRemaining > 0;
  }

  int8_t ghostPieceY() const {
    if (!ghostVisible()) return pieceY;
    int8_t landingY = pieceY;
    while (canPlace(activePiece, rotation, pieceX, landingY + 1)) ++landingY;
    return landingY;
  }

  bool ghostAt(uint8_t x, uint8_t y) const {
    if (!ghostVisible()) return false;
    const int8_t landingY = ghostPieceY();
    if (landingY == pieceY) return false;
    for (uint8_t block = 0; block < 4; ++block) {
      const StackBlockOffset& offset = STACK_SHAPES[activePiece][rotation][block];
      if (pieceX + offset.x == int8_t(x) && landingY + offset.y == int8_t(y)) return true;
    }
    return false;
  }

  uint32_t levelBreakAgeMs(uint32_t now) const {
    if (!levelBreakActive) return 0;
    const uint32_t visualNow = paused && pausedAtMs > 0 ? pausedAtMs : now;
    return visualNow - levelBreakStartedMs;
  }

  bool levelBreakIntroActive(uint32_t now) const {
    return levelBreakActive && levelBreakAgeMs(now) < STACK_LEVEL_INTRO_MS;
  }

  uint32_t levelBreakDurationMs() const {
    return STACK_LEVEL_INTRO_MS + STACK_LEVEL_SCAN_LEAD_MS +
        uint32_t(levelBreakEmptyRows) * STACK_LEVEL_SCAN_ROW_MS +
        STACK_LEVEL_SCAN_HOLD_MS;
  }

  uint8_t levelBreakScannedRows(uint32_t now) const {
    if (!levelBreakActive) return 0;
    const uint32_t age = levelBreakAgeMs(now);
    const uint32_t scanStart = STACK_LEVEL_INTRO_MS + STACK_LEVEL_SCAN_LEAD_MS;
    if (age < scanStart) return 0;
    const uint32_t elapsed = age - scanStart;
    const uint32_t scanned = elapsed / STACK_LEVEL_SCAN_ROW_MS + 1;
    return scanned > levelBreakEmptyRows
        ? levelBreakEmptyRows
        : uint8_t(scanned);
  }

  uint32_t levelBreakScannedBonus(uint32_t now) const {
    return uint32_t(levelBreakScannedRows(now)) *
        STACK_LEVEL_EMPTY_ROW_POINTS * levelBreakLevel;
  }

  uint32_t perfectClearAgeMs(uint32_t now) const {
    if (!perfectClearActive) return 0;
    const uint32_t visualNow = paused && pausedAtMs > 0 ? pausedAtMs : now;
    return visualNow - perfectClearStartedMs;
  }

private:
  uint8_t bag[7] = {0,1,2,3,4,5,6};
  uint8_t bagIndex = 7;
  uint32_t lastFallMs = 0;
  uint32_t pauseChangedAtMs = 0;
  uint32_t pausedAtMs = 0;

  static bool isActiveHuman(const PlayerSlot& player) {
    return player.occupied && player.connected && !player.waiting && !player.isCpu;
  }

  static bool slotActive(const PlayerManager& players, int8_t slot) {
    return slot >= 0 && slot < MAX_PLAYERS && isActiveHuman(players.players[slot]);
  }

  bool canPlace(uint8_t piece, uint8_t nextRotation, int8_t x, int8_t y) const {
    for (uint8_t block = 0; block < 4; ++block) {
      const StackBlockOffset& offset = STACK_SHAPES[piece][nextRotation][block];
      const int8_t blockX = x + offset.x;
      const int8_t blockY = y + offset.y;
      if (blockX < 0 || blockX >= STACK_WIDTH || blockY >= STACK_HEIGHT) return false;
      if (blockY >= 0 && board[blockY][blockX] != 0) return false;
    }
    return true;
  }

  bool tryMove(int8_t dx, int8_t dy) {
    const int8_t nextX = pieceX + dx;
    const int8_t nextY = pieceY + dy;
    if (!canPlace(activePiece, rotation, nextX, nextY)) return false;
    pieceX = nextX;
    pieceY = nextY;
    return true;
  }

  bool tryRotate() {
    const uint8_t nextRotation = (rotation + 1) & 0x03;
    static const int8_t kicks[] = {0, -1, 1, -2, 2};
    for (int8_t kick : kicks) {
      if (!canPlace(activePiece, nextRotation, pieceX + kick, pieceY)) continue;
      pieceX += kick;
      rotation = nextRotation;
      return true;
    }
    return false;
  }

  void refillBag() {
    for (uint8_t i = 0; i < 7; ++i) bag[i] = i;
    for (int8_t i = 6; i > 0; --i) {
      const uint8_t j = random(i + 1);
      const uint8_t temp = bag[i];
      bag[i] = bag[j];
      bag[j] = temp;
    }
    bagIndex = 0;
  }

  uint8_t drawPiece() {
    if (bagIndex >= 7) refillBag();
    return bag[bagIndex++];
  }

  void spawnPiece() {
    activePiece = nextPiece;
    nextPiece = drawPiece();
    rotation = 0;
    pieceX = (STACK_WIDTH - 4) / 2;
    pieceY = -1;
  }

  void lockPiece(AudioOut& audio) {
    if (!running || matchFinished) return;
    if (ghostPiecesRemaining > 0) --ghostPiecesRemaining;
    ghostAwardPieces = 0;
    for (uint8_t block = 0; block < 4; ++block) {
      const StackBlockOffset& offset = STACK_SHAPES[activePiece][rotation][block];
      const int8_t blockX = pieceX + offset.x;
      const int8_t blockY = pieceY + offset.y;
      if (blockY < 0) {
        finishGame();
        return;
      }
      board[blockY][blockX] = activePiece + 1;
    }

    lineClearCount = findFullLines();
    if (lineClearCount > 0) {
      ++lineClearStreak;
      if (lineClearCount >= 4) ghostAwardPieces += STACK_GHOST_TETRIS_REWARD;
      if (lineClearStreak % STACK_GHOST_STREAK_THRESHOLD == 0) {
        ghostAwardPieces += STACK_GHOST_STREAK_REWARD;
      }
      if (ghostAwardPieces > 0) {
        const uint16_t rewardedGhostPieces = uint16_t(ghostPiecesRemaining) + ghostAwardPieces;
        ghostPiecesRemaining = rewardedGhostPieces > STACK_GHOST_MAX_PIECES
            ? STACK_GHOST_MAX_PIECES
            : uint8_t(rewardedGhostPieces);
      }
      lineClearActive = true;
      lineClearStartedMs = millis();
      audio.stackLine(lineClearCount);
      return;
    }

    lineClearStreak = 0;
    audio.stackLock();
    spawnAfterLock(millis());
  }

  uint8_t findFullLines() {
    uint8_t found = 0;
    for (int8_t row = STACK_HEIGHT - 1; row >= 0 && found < 4; --row) {
      bool full = true;
      for (uint8_t x = 0; x < STACK_WIDTH; ++x) {
        if (board[row][x] == 0) {
          full = false;
          break;
        }
      }
      if (full) lineClearRows[found++] = uint8_t(row);
    }
    return found;
  }

  void finishLineClear(AudioOut& audio, uint32_t now) {
    bool marked[STACK_HEIGHT] = {};
    for (uint8_t i = 0; i < lineClearCount; ++i) marked[lineClearRows[i]] = true;

    int8_t writeRow = STACK_HEIGHT - 1;
    for (int8_t readRow = STACK_HEIGHT - 1; readRow >= 0; --readRow) {
      if (marked[readRow]) continue;
      if (writeRow != readRow) memcpy(board[writeRow], board[readRow], STACK_WIDTH);
      --writeRow;
    }
    while (writeRow >= 0) {
      memset(board[writeRow], 0, STACK_WIDTH);
      --writeRow;
    }

    static const uint16_t points[5] = {0, 100, 300, 500, 800};
    const uint8_t previousLevel = level;
    score += uint32_t(points[lineClearCount]) * level;
    clearedLines += lineClearCount;
    level = 1 + clearedLines / STACK_LEVEL_LINES;
    const uint32_t speedup = uint32_t(level - 1) * STACK_SPEEDUP_MS;
    fallIntervalMs = speedup >= STACK_FALL_START_MS - STACK_FALL_MIN_MS
        ? STACK_FALL_MIN_MS
        : STACK_FALL_START_MS - speedup;

    lineClearActive = false;
    lineClearCount = 0;
    memset(lineClearRows, 0, sizeof(lineClearRows));
    lineClearStartedMs = 0;

    const bool levelAdvanced = level > previousLevel;
    if (levelAdvanced) prepareLevelBreak();

    if (boardEmpty()) {
      perfectClearBonus = uint32_t(STACK_PERFECT_CLEAR_POINTS) * level;
      score += perfectClearBonus;
      const uint8_t beforeGhost = ghostPiecesRemaining;
      const uint16_t rewardedGhost = uint16_t(ghostPiecesRemaining) +
          STACK_PERFECT_CLEAR_GHOST_REWARD;
      ghostPiecesRemaining = rewardedGhost > STACK_GHOST_MAX_PIECES
          ? STACK_GHOST_MAX_PIECES
          : uint8_t(rewardedGhost);
      perfectClearGhostAward = ghostPiecesRemaining - beforeGhost;
      perfectClearActive = true;
      pendingLevelBreak = levelAdvanced;
      perfectClearStartedMs = now;
      audio.stackPerfectClear();
      return;
    }

    if (levelAdvanced) {
      startLevelBreak(audio, now);
      return;
    }

    spawnAfterLock(now);
  }

  bool boardEmpty() const {
    for (uint8_t y = 0; y < STACK_HEIGHT; ++y) {
      for (uint8_t x = 0; x < STACK_WIDTH; ++x) {
        if (board[y][x] != 0) return false;
      }
    }
    return true;
  }

  void prepareLevelBreak() {
    levelBreakLevel = level;
    levelBreakEmptyRows = countTopEmptyRows();
    levelBreakBonus = uint32_t(levelBreakEmptyRows) *
        STACK_LEVEL_EMPTY_ROW_POINTS * level;
    levelBreakAwardedRows = 0;
  }

  void startLevelBreak(AudioOut& audio, uint32_t now) {
    levelBreakActive = true;
    levelBreakAwardedRows = 0;
    levelBreakStartedMs = now;
    audio.powerUp();
  }

  void advanceLevelBreakScore(uint32_t now) {
    const uint8_t scanned = levelBreakScannedRows(now);
    if (scanned <= levelBreakAwardedRows) return;
    const uint8_t newlyAwarded = scanned - levelBreakAwardedRows;
    score += uint32_t(newlyAwarded) *
        STACK_LEVEL_EMPTY_ROW_POINTS * levelBreakLevel;
    levelBreakAwardedRows = scanned;
  }

  uint8_t countTopEmptyRows() const {
    uint8_t emptyRows = 0;
    for (uint8_t y = 0; y < STACK_HEIGHT; ++y) {
      bool occupied = false;
      for (uint8_t x = 0; x < STACK_WIDTH; ++x) {
        if (board[y][x] != 0) { occupied = true; break; }
      }
      if (occupied) break;
      ++emptyRows;
    }
    return emptyRows;
  }

  void finishLevelBreak(uint32_t now) {
    advanceLevelBreakScore(now);
    levelBreakActive = false;
    levelBreakAwardedRows = 0;
    levelBreakStartedMs = 0;
    spawnAfterLock(now);
  }

  void finishPerfectClear(AudioOut& audio, uint32_t now) {
    perfectClearActive = false;
    perfectClearStartedMs = 0;
    perfectClearBonus = 0;
    perfectClearGhostAward = 0;
    if (pendingLevelBreak) {
      pendingLevelBreak = false;
      startLevelBreak(audio, now);
      return;
    }
    spawnAfterLock(now);
  }

  void spawnAfterLock(uint32_t now) {
    spawnPiece();
    lastFallMs = now;
    if (!canPlace(activePiece, rotation, pieceX, pieceY)) finishGame();
  }

  void finishGame() {
    running = false;
    paused = false;
    lineClearActive = false;
    levelBreakActive = false;
    perfectClearActive = false;
    pendingLevelBreak = false;
    matchFinished = true;
    newRecord = score > bestScore;
    if (newRecord) bestScore = score;
  }
};
