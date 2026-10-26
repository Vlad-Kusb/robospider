#pragma once
#include <Arduino.h>

// --- флаги (через #define, чтобы работал #if) ---
#ifndef DEBUG_MODE
  #define DEBUG_MODE 0
#endif
#ifndef DEBUG_INF_MSGS
  #define DEBUG_INF_MSGS 0
#endif
#ifndef USE_SERVO_FLAG
  #define USE_SERVO_FLAG 1
#endif

// --- пины ---
constexpr uint8_t HC_TRIG = 11;
constexpr uint8_t HC_ECHO = 12;
constexpr uint8_t LED = 13;
constexpr uint8_t BUTTON_PIN = 10;

// --- сервы ---
constexpr uint8_t SERVOS_COUNT = 8;
constexpr uint8_t KNEE_START_POS = 125;
constexpr uint8_t BEDR_START_POS = 40;
constexpr uint8_t KNEE_MIN_POS = 0;
constexpr uint8_t KNEE_MAX_POS = 140;
constexpr uint8_t BEDR_MIN_POS = 0;
constexpr uint8_t BEDR_MAX_POS = 70;
constexpr bool KNEE_AUTODETACH = false;
constexpr bool BEDR_AUTODETACH = true;
constexpr uint16_t SERVO_DEFAULT_SPEED = 250;
constexpr float SERVO_DEFAULT_ACCEL = 2.1f;

// --- сонар, кнопка, светодиод ---
constexpr uint16_t SONAR_MAX_DISTANCE_CM = 500;
constexpr uint16_t BTN_DEB = 100;
constexpr bool LED_STAT = false;