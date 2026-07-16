#pragma once
#include <Arduino.h>

enum class SoundId : uint8_t {
  NONE,
  MENU_SELECT,
  GAME_START,
  COUNTDOWN,
  GO,
  POWER_UP,
  CRASH,
  DEFEAT,
  RESTART,
  WINNER,
  NEW_RECORD,
  BOSS_INTRO,
  BOSS_DEFEATED,
  PLAYER_JOINED,
  READY,
  PONG_HIT,
  PONG_SCORE,
  ATTRACT_CHIME,
  DEMO_CUE,
  INVITE_CUE
};

struct ToneStep {
  uint16_t frequencyHz;
  uint16_t durationMs;
  uint16_t gapMs;
  uint8_t duty;
};

struct SoundPattern {
  const ToneStep* steps;
  uint8_t count;
};

static const ToneStep S_MENU_SELECT[] = {{440,24,10,7},{660,34,0,8}};
static const ToneStep S_GAME_START[] = {{330,32,8,8},{440,32,8,8},{660,36,8,9},{880,55,0,10}};
static const ToneStep S_COUNTDOWN[] = {{520,42,0,9}};
static const ToneStep S_GO[] = {{1047,55,10,10},{1568,85,0,10}};
static const ToneStep S_POWER_UP[] = {{523,26,6,8},{659,26,6,9},{784,45,0,10}};
static const ToneStep S_CRASH[] = {{400,32,6,10},{240,42,6,9},{120,70,0,8}};
static const ToneStep S_DEFEAT[] = {{440,55,12,8},{330,70,12,8},{220,110,0,8}};
static const ToneStep S_RESTART[] = {{262,35,8,7},{330,35,8,8},{392,55,0,9}};
static const ToneStep S_WINNER[] = {{523,42,8,9},{659,42,8,9},{784,52,8,10},{1047,110,0,10}};
static const ToneStep S_NEW_RECORD[] = {{659,28,5,9},{784,28,5,9},{988,40,8,10},{1319,85,0,10}};
static const ToneStep S_BOSS_INTRO[] = {{110,90,45,8},{110,90,45,9},{165,150,0,10}};
static const ToneStep S_BOSS_DEFEATED[] = {{180,45,4,10},{130,55,8,9},{523,34,6,9},{784,45,6,10},{1047,100,0,10}};
static const ToneStep S_PLAYER_JOINED[] = {{660,24,7,7},{880,32,0,8}};
static const ToneStep S_READY[] = {{980,28,0,7}};
static const ToneStep S_PONG_HIT[] = {{760,18,0,6}};
static const ToneStep S_PONG_SCORE[] = {{392,32,5,7},{784,62,0,9}};
static const ToneStep S_ATTRACT_CHIME[] = {{392,42,10,7},{523,42,10,8},{659,75,0,8}};
static const ToneStep S_DEMO_CUE[] = {{262,36,7,7},{392,36,7,8},{523,54,0,8}};
static const ToneStep S_INVITE_CUE[] = {{784,55,25,8},{988,80,0,9}};

inline SoundPattern soundPattern(SoundId id) {
  switch (id) {
    case SoundId::MENU_SELECT: return {S_MENU_SELECT,2};
    case SoundId::GAME_START: return {S_GAME_START,4};
    case SoundId::COUNTDOWN: return {S_COUNTDOWN,1};
    case SoundId::GO: return {S_GO,2};
    case SoundId::POWER_UP: return {S_POWER_UP,3};
    case SoundId::CRASH: return {S_CRASH,3};
    case SoundId::DEFEAT: return {S_DEFEAT,3};
    case SoundId::RESTART: return {S_RESTART,3};
    case SoundId::WINNER: return {S_WINNER,4};
    case SoundId::NEW_RECORD: return {S_NEW_RECORD,4};
    case SoundId::BOSS_INTRO: return {S_BOSS_INTRO,3};
    case SoundId::BOSS_DEFEATED: return {S_BOSS_DEFEATED,5};
    case SoundId::PLAYER_JOINED: return {S_PLAYER_JOINED,2};
    case SoundId::READY: return {S_READY,1};
    case SoundId::PONG_HIT: return {S_PONG_HIT,1};
    case SoundId::PONG_SCORE: return {S_PONG_SCORE,2};
    case SoundId::ATTRACT_CHIME: return {S_ATTRACT_CHIME,3};
    case SoundId::DEMO_CUE: return {S_DEMO_CUE,3};
    case SoundId::INVITE_CUE: return {S_INVITE_CUE,2};
    default: return {nullptr,0};
  }
}
