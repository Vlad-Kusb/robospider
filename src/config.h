#pragma once
#include <Arduino.h>

#ifndef USE_SERVO_FLAG // флаг отладки, отключающий серво
#define USE_SERVO_FLAG 1
#endif
#ifndef DEBUG_MODE // флаг дебаг режима
#define DEBUG_MODE 0
#endif
#ifndef DEBUG_INF_MSGS
#define DEBUG_INF_MSGS 0
#endif

constexpr uint8_t HC_TRIG = 11;                 // триггер пин
constexpr uint8_t HC_ECHO = 12;                 // эхо пин
constexpr uint16_t SONAR_MAX_DISTANCE_CM = 300; // было uint8_t, 500 не влезало

constexpr uint8_t SERVOS_COUNT = 8;          // кол-во серв
constexpr uint8_t KNEE_START_POS = 125;      // начальная позиция сервоприводов для коленей
constexpr uint8_t BEDR_START_POS = 40;       // начальная позиция для бедер
constexpr bool KNEE_AUTODETACH = false;      // автоотключение серв коленей при достижении цели
constexpr bool BEDR_AUTODETACH = true;       // автооткл бедер
constexpr uint8_t BEDR_MIN_POS = 0;          // минимальная позиция для бедер
constexpr uint8_t BEDR_MAX_POS = 70;         // максимальная позиция бедер
constexpr uint8_t KNEE_MIN_POS = 0;          // минимум коленей
constexpr uint8_t KNEE_MAX_POS = 140;        // максимум коленей
constexpr uint8_t SERVO_DEFAULT_SPEED = 250; // град/сек
constexpr float SERVO_DEFAULT_ACCEL = 2.1f;  // ускорение

constexpr uint8_t LED = 13;      // пин светодиода
constexpr bool LED_STAT = false; // светодиод по умолчанию горит всегда и меняет состояние на время нажатия кнопки

constexpr uint8_t BUTTON_PIN = 10; // пин кнопки, выбор режимов должен быть по ней, сколько раз нажата -- такой режим и выбран, повторное одинарное нажатие в любом режиме просто вернет робота в стартовую позицию и будет ждать указаний
constexpr uint8_t BTN_DEB = 100;   // дребезг кнопки

enum SpiderState
{                      // объект состояний робота
    STATE_START_POS,   // 0 Стартовая позиция
    STATE_DANCE,       // 1 танец
    STATE_PATROL,      // 2 патруль
    STATE_SLEEP,       // 3 спать
    STATE_CHANGE_STATE // 4 изменение состояния
};

unsigned long debugMsgTimer = 0; // таймер

// общие переменные, определены в main.cpp
extern bool servosMoved;
extern unsigned int sonarDistanceCm;
extern SpiderState currentState;
extern SpiderState prevState;