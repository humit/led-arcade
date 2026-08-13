#pragma once
#include <Arduino.h>

// Network
static const char* AP_SSID = "! OYUNA KATIL !";
static const byte DNS_PORT = 53;
static const uint16_t WS_PORT = 81;
static const uint8_t AP_MAX_CONNECTIONS = 8;
static const IPAddress AP_IP(10, 10, 10, 10);
static const IPAddress AP_GATEWAY(10, 10, 10, 10);
static const IPAddress AP_SUBNET(255, 255, 255, 0);

// Venue display language: true = Turkish, false = English.
static const bool DISPLAY_LANGUAGE_TR = true;

// 8x32 WS2812B matrix
#define LED_PIN 23
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 8
#define LED_COUNT (MATRIX_WIDTH * MATRIX_HEIGHT)
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define LED_BRIGHTNESS 36

// 1D WS2812B strip arena
#define STRIP_LED_PIN 22
#define STRIP_LED_COUNT 150
#define STRIP_LED_BRIGHTNESS 36

// 1D games
static const uint16_t STRIP_HIT_ZONE_LEDS = 14;
static const uint32_t STRIP_RALLY_STEP_START_MS = 36;
static const uint32_t STRIP_RALLY_STEP_MIN_MS = 14;
static const uint32_t STRIP_PUSH_DURATION_MS = 20000;
static const uint32_t STRIP_PUSH_TAP_DEBOUNCE_MS = 120;
static const uint32_t STRIP_CPU_PUSH_MIN_MS = 185;
static const uint32_t STRIP_CPU_PUSH_MAX_MS = 310;

// Screen Arcade
static const uint8_t SCREEN_ARCADE_PLAYER_COUNT = 3;
static const uint8_t TAP_CLASH_GRID_CELLS = 9;
static const uint32_t TAP_CLASH_DURATION_MS = 30000;
static const uint32_t TAP_CLASH_FIRST_TARGET_DELAY_MS = 350;
static const uint32_t TAP_CLASH_TARGET_MIN_MS = 850;
static const uint32_t TAP_CLASH_TARGET_MAX_MS = 1250;
static const uint32_t TAP_CLASH_GAP_MIN_MS = 180;
static const uint32_t TAP_CLASH_GAP_MAX_MS = 360;
static const uint32_t TAP_CLASH_INPUT_DEBOUNCE_MS = 80;
static const uint32_t TAP_CLASH_WRONG_LOCK_MS = 650;
static const uint32_t TAP_CLASH_CPU_MIN_MS = 430;
static const uint32_t TAP_CLASH_CPU_MAX_MS = 920;
static const uint8_t TAP_CLASH_POINTS_PER_TARGET = 10;
static const uint8_t TAP_CLASH_WIN_BONUS_POINTS = 50;
static const uint8_t BRAIN_DUEL_PLAYER_COUNT = 2;
static const uint8_t BRAIN_DUEL_OPTION_COUNT = 4;
static const uint8_t BRAIN_DUEL_QUESTION_COUNT = 10;
static const uint32_t BRAIN_DUEL_EASY_QUESTION_MS = 8500;
static const uint32_t BRAIN_DUEL_NORMAL_QUESTION_MS = 9500;
static const uint32_t BRAIN_DUEL_CHALLENGE_QUESTION_MS = 10500;
static const uint32_t BRAIN_DUEL_REVEAL_MS = 1700;
static const uint8_t BRAIN_DUEL_CORRECT_POINTS = 10;
static const uint8_t BRAIN_DUEL_FIRST_BONUS_POINTS = 2;
static const uint8_t BRAIN_DUEL_WIN_BONUS_POINTS = 40;

// Audio
#define AUDIO_PIN 25
#define AUDIO_RES_BITS 8
#define AUDIO_DUTY 10

// Idle attract audio. Visual attract phases continue normally, but sound is
// intentionally sparse so an unattended installation does not become noisy.
static const uint32_t ATTRACT_AUDIO_INITIAL_DELAY_MS = 20000;
static const uint32_t ATTRACT_AUDIO_INTERVAL_MS = 180000;

// Idle attract playlist. Each scene is long enough to read as a tiny game
// session rather than a rapidly changing decorative animation.
static const uint32_t ATTRACT_PAC_CHASE_MS = 14000;
static const uint32_t ATTRACT_PONG_CPU_MS = 16000;
static const uint32_t ATTRACT_STACK_CPU_MS = 18000;
static const uint32_t ATTRACT_RAIDER_CPU_MS = 16000;
static const uint32_t ATTRACT_JOIN_MS = 8000;

// Session/game tuning
static const uint8_t MAX_PLAYERS = 8;
static const uint8_t FINISH_X = MATRIX_WIDTH - 1;
static const uint32_t PLAYER_RECONNECT_MS = 180000;
static const uint32_t PLAYER_HEARTBEAT_TIMEOUT_MS = 30000;
static const uint32_t TAP_DEBOUNCE_MS = 65;
static const uint32_t COUNTDOWN_STEP_MS = 1000;
static const uint32_t STATE_BROADCAST_MS = 200;
static const uint8_t MATCH_WIN_SCORE = 2;

// Field-test diagnostics
static const char* FIELD_TEST_BUILD = "field-test-diagnostics-v2";
static const uint32_t DIAGNOSTICS_CHECKPOINT_MS = 5000;
static const uint32_t DIAGNOSTICS_HEALTH_LOG_MS = 60000;
static const uint32_t DIAGNOSTICS_LOW_HEAP_BYTES = 32000;
static const uint8_t DIAGNOSTICS_LOG_CAPACITY = 64;
static const uint8_t DIAGNOSTICS_CLIENT_CAPACITY = MAX_PLAYERS;

// Bonus + boss
static const uint8_t TURBO_X_1 = 10;
static const uint8_t TURBO_X_2 = 21;
static const uint8_t TURBO_TAPS = 3;
static const uint8_t RACES_PER_BOSS = 2;
static const uint32_t BOSS_DURATION_MS = 18000;
static const uint8_t BOSS_BASE_HP = 25;
static const uint8_t BOSS_HP_PER_RAIDER = 22;
static const uint8_t BOSS_PULSE_TAPS = 12;
static const uint32_t BOSS_PULSE_STUN_MS = 900;
static const uint32_t ANNOUNCE_STEP_MS = 700;

// Tron Arena
static const uint8_t TRON_MIN_PLAYERS = 2;
static const uint8_t TRON_MAX_PLAYERS = 4;
static const uint32_t TRON_TICK_START_MS = 300;
static const uint32_t TRON_TICK_MID_MS = 250;
static const uint32_t TRON_TICK_FAST_MS = 200;

// Automatic CPU opponent
static const uint32_t CPU_DERBY_TAP_MIN_MS = 220;
static const uint32_t CPU_DERBY_TAP_MAX_MS = 390;
static const uint32_t CPU_BOSS_TAP_MIN_MS = 180;
static const uint32_t CPU_BOSS_TAP_MAX_MS = 320;

// Pixel Raider
static const uint8_t RAIDER_PLAYER_X = 3;
static const uint32_t RAIDER_WORLD_TICK_START_MS = 340;
static const uint32_t RAIDER_WORLD_TICK_FAST_MS = 190;
static const uint32_t RAIDER_FIRE_START_MS = 650;
static const uint32_t RAIDER_FIRE_RAPID_1_MS = 420;
static const uint32_t RAIDER_FIRE_RAPID_2_MS = 270;
static const uint32_t RAIDER_BULLET_TICK_MS = 85;
static const uint8_t RAIDER_MAX_BULLETS = 12;

// Pixel Pong
static const uint8_t PONG_PADDLE_HEIGHT = 3;
static const uint8_t PONG_LEFT_X = 1;
static const uint8_t PONG_RIGHT_X = MATRIX_WIDTH - 2;
static const uint8_t PONG_SCORE_TO_WIN = 5;
static const uint32_t PONG_STEP_START_MS = 145;
static const uint32_t PONG_STEP_MIN_MS = 70;
static const uint32_t PONG_SPEEDUP_PER_HIT_MS = 7;
static const uint32_t PONG_POINT_PAUSE_MS = 850;
static const uint32_t PONG_INPUT_DEBOUNCE_MS = 65;
static const uint32_t PONG_CPU_MOVE_MIN_MS = 90;
static const uint32_t PONG_CPU_MOVE_MAX_MS = 150;
static const uint8_t PONG_CPU_SKIP_PERCENT = 12;

// Stack Shift (vertical 8x32 falling-block game)
static const uint8_t STACK_WIDTH = 8;
static const uint8_t STACK_HEIGHT = 32;
static const bool STACK_ROTATE_CLOCKWISE = true;
static const uint32_t STACK_FALL_START_MS = 650;
static const uint32_t STACK_FALL_MIN_MS = 110;
static const uint8_t STACK_LEVEL_LINES = 5;
static const uint32_t STACK_SPEEDUP_MS = 55;
static const uint32_t STACK_INPUT_DEBOUNCE_MS = 45;
static const uint32_t STACK_PAUSE_DEBOUNCE_MS = 250;
static const uint32_t STACK_LINE_CLEAR_FLASH_MS = 90;
static const uint8_t STACK_LINE_CLEAR_FLASH_PHASES = 4;
static const uint32_t STACK_LINE_CLEAR_DURATION_MS =
    STACK_LINE_CLEAR_FLASH_MS * STACK_LINE_CLEAR_FLASH_PHASES;
static const uint8_t STACK_GHOST_START_PIECES = 10;
static const uint8_t STACK_GHOST_TETRIS_REWARD = 8;
static const uint8_t STACK_GHOST_STREAK_THRESHOLD = 3;
static const uint8_t STACK_GHOST_STREAK_REWARD = 5;
static const uint8_t STACK_GHOST_MAX_PIECES = 24;
static const uint8_t STACK_BLACK_CLAMP_MAX = 32;
static const uint8_t STACK_PLAYFIELD_BLACK_CLAMP_MAX = 180;
static const uint16_t STACK_LEVEL_EMPTY_ROW_POINTS = 25;
static const uint32_t STACK_LEVEL_INTRO_MS = 1050;
static const uint32_t STACK_LEVEL_SCAN_LEAD_MS = 180;
static const uint32_t STACK_LEVEL_SCAN_ROW_MS = 85;
static const uint32_t STACK_LEVEL_SCAN_HOLD_MS = 720;
static const uint16_t STACK_PERFECT_CLEAR_POINTS = 2500;
static const uint8_t STACK_PERFECT_CLEAR_GHOST_REWARD = 12;
static const uint32_t STACK_PERFECT_CLEAR_MS = 2200;

// Color Clash
static const uint8_t CLASH_MIN_PLAYERS = 2;
static const uint8_t CLASH_MAX_PLAYERS = 4;
static const uint32_t CLASH_DURATION_MS = 30000;
static const uint32_t CLASH_TICK_MS = 250;
static const uint32_t CLASH_CPU_DECISION_MS = 250;
