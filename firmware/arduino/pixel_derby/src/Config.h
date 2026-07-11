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

// Audio
#define AUDIO_PIN 22
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
