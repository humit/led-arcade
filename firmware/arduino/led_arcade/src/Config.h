#pragma once
#include <Arduino.h>

// Network
static const char* AP_SSID = "! OYUNA KATIL !";
static const byte DNS_PORT = 53;
static const uint16_t WS_PORT = 81;
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

// Audio
#define AUDIO_PIN 25
#define AUDIO_RES_BITS 8
#define AUDIO_DUTY 10

// Session/game tuning
static const uint8_t MAX_PLAYERS = 8;
static const uint8_t FINISH_X = MATRIX_WIDTH - 1;
static const uint32_t PLAYER_RECONNECT_MS = 180000;
static const uint32_t PLAYER_HEARTBEAT_TIMEOUT_MS = 30000;
static const uint32_t TAP_DEBOUNCE_MS = 65;
static const uint32_t COUNTDOWN_STEP_MS = 1000;
static const uint32_t STATE_BROADCAST_MS = 200;
static const uint8_t MATCH_WIN_SCORE = 2;

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

// Color Clash
static const uint8_t CLASH_MIN_PLAYERS = 2;
static const uint8_t CLASH_MAX_PLAYERS = 4;
static const uint32_t CLASH_DURATION_MS = 30000;
static const uint32_t CLASH_TICK_MS = 250;
static const uint32_t CLASH_CPU_DECISION_MS = 250;
