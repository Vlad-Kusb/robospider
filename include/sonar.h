#pragma once
#include "config.h"

unsigned long sonarTimer = 0; // таймер

int getSonarDistance() // получение дистанции с датчика
{
    unsigned int duration, cm;           // назначаем переменную "cm" и "duration" для показаний датчика
    digitalWrite(HC_TRIG, LOW); // изначально датчик не посылает сигнал
    delayMicroseconds(2);       // ставим задержку в 2 ммикросекунд

    digitalWrite(HC_TRIG, HIGH); // посылаем сигнал
    delayMicroseconds(10);       // ставим задержку в 10 микросекунд
    digitalWrite(HC_TRIG, LOW);  // выключаем сигнал

    duration = pulseIn(HC_ECHO, HIGH); // включаем прием сигнала

    cm = duration / 58; // вычисляем расстояние в сантиметрах

    if (cm > SONAR_MAX_DISTANCE_CM)
        return SONAR_MAX_DISTANCE_CM;

    return cm;
}