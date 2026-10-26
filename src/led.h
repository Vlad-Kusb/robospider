#pragma once
#include "config.h"

void led_blink(uint8_t count = 5) // вспомогательная функция, которая моргает светодиодом заданное число раз
{
    for (byte i = 0; i <= count * 2; i++)
    {
        digitalWrite(LED, i % 2 == 0);
        delay(150);
    }
}