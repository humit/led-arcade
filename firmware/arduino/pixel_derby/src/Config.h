#pragma once
#include <Arduino.h>

// Network
static const char* AP_SSID = "LED-Arcade";
static const byte DNS_PORT = 53;
static const uint16_t WS_PORT = 81;
static const IPAddress AP_IP(10, 10, 10, 1);
static const IPAddress AP_GATEWAY(10, 10, 10, 1);
static const IPAddress AP_SUBNET(255, 255, 255, 0);

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
static const uint32_t STATE_BROADCAST_MS = 250;
static const uint8_t MATCH_WIN_SCORE = 2;

// Bonus + boss MVP
static const uint8_t TURBO_X_1 = 10;
static const uint8_t TURBO_X_2 = 21;
static const uint8_t TURBO_TAPS = 3;
static const uint8_t RACES_PER_BOSS = 2;
static const uint32_t BOSS_DURATION_MS = 18000;
static const uint8_t BOSS_BASE_HP = 25;
static const uint8_t BOSS_HP_PER_RAIDER = 22;
static const uint8_t BOSS_PULSE_TAPS = 12;
static const uint32_t BOSS_PULSE_STUN_MS = 900;
