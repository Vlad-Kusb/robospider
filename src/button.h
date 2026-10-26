#pragma once
#include "config.h"

SpiderState button_handler() // обработчик кнопки
{
    static bool pbtn = false;
    static uint32_t tmr = millis();
    static uint32_t lastClick = 0;
    static uint8_t countClicks = 0;
    bool btn = !digitalRead(BUTTON_PIN);

    if (pbtn != btn && millis() - tmr >= BTN_DEB)
    {
        tmr = millis();
        pbtn = btn;
        if (btn)
        {
            if (millis() - lastClick > 1000)
                countClicks = 0;
            countClicks++;
            lastClick = millis();
            return STATE_CHANGE_STATE;
        };
        // else Serial.println("Кнопка отпущена");
    }

    // кнопка удерживается дольше 500 мс
    if (pbtn && millis() - tmr >= 500)
    {
        // tmr = millis();  // сброс таймера
        return STATE_CHANGE_STATE;
        // Serial.println("Кнопка удержана");
    }

    if (millis() - lastClick > 1000) // перестали нажимать и возвращаем какое состояние накликали
    {
        switch (countClicks)
        {
        case 1:
            return STATE_START_POS;
            break;
        case 2:
            return STATE_DANCE;
            break;
        case 3:
            return STATE_PATROL;
            break;
        case 4:
            return STATE_SLEEP;
            break;
        default:
            return STATE_START_POS;
            break;
        }
    }
    else
        return STATE_CHANGE_STATE;
    return STATE_START_POS;
}