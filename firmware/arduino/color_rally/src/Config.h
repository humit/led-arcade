#pragma once
#include <Arduino.h>

// -------------------- Network --------------------

static const char* AP_SSID = "LED-Arcade";
static const bool AP_OPEN = true;
static const char* AP_PASSWORD = "";

static const byte DNS_PORT = 53;

static const IPAddress AP_IP(10, 10, 10, 1);
static const IPAddress AP_GATEWAY(10, 10, 10, 1);
static const IPAddress AP_SUBNET(255, 255, 255, 0);

// -------------------- Hardware --------------------

#define LED_PIN       23
#define LED_COUNT     200
#define LED_TYPE      WS2812B
#define COLOR_ORDER   GRB
#define LED_BRIGHTNESS 38

#define AUDIO_PIN      22
#define AUDIO_RES_BITS 8
#define AUDIO_DUTY     10   // lower = quieter. Try 6-18.

// -------------------- Game tuning --------------------

static const int BALL_HEAD_SIZE = 1;
static const int BALL_TAIL_NORMAL = 8;
static const int BALL_TAIL_CHARGED = 13;

static const int HIT_ZONE_SIZE = 42;

static const float BALL_SPEED_BASE = 38.0f;
static const float BALL_SPEED_COMBO = 48.0f;
static const float BALL_SPEED_CHARGED = 58.0f;

static const uint8_t MAX_HP = 5;
static const uint8_t COMBO_FAST_AT = 3;
static const uint8_t COMBO_CHARGED_AT = 5;

static const uint32_t HUMAN_DISCONNECT_TIMEOUT_MS = 30000;
static const uint32_t SPECTATOR_TIMEOUT_MS = 20000;
static const uint32_t POINT_PAUSE_MS = 900;
static const uint32_t CPU_REACTION_MS = 300;
static const uint8_t CPU_CORRECT_PERCENT = 78;

// -------------------- Zones / colors --------------------

static const uint8_t BLUE_SIDE = 0;
static const uint8_t PINK_SIDE = 1;
static const uint8_t NO_SIDE = 255;

static const uint8_t C_RED = 0;
static const uint8_t C_GREEN = 1;
static const uint8_t C_BLUE = 2;

static const int BLUE_HIT_A = 0;
static const int BLUE_HIT_B = HIT_ZONE_SIZE - 1;

static const int PINK_HIT_A = LED_COUNT - HIT_ZONE_SIZE;
static const int PINK_HIT_B = LED_COUNT - 1;
