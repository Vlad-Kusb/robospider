#pragma once
#include "config.h"

int cycleNum(int num, bool isBedr = true) // вспомогательная функция на случай переполнения в циклах ходьбы
{
    const int start = isBedr ? 4 : 0;
    const int length = 4; // Количество чисел в диапазоне (4, 5, 6, 7)

    // Нормализуем число в диапазон [0, length-1] с зацикливанием
    int normalized = (num - start) % length;

    // Корректируем отрицательные значения
    if (normalized < 0)
    {
        normalized += length;
    }

    return start + normalized;
}

bool movement(byte phase = 0) // ходьба 0 1 2 (перед лево право), основная функция для ходьбы
{
    static uint32_t lastStep = millis();
    // static byte phase = 0;
    static byte again = 0;
    static int8_t leg = 0;
    static byte count = 0;

    if (prevState != currentState || millis() - lastStep > 100)
    {
        // phase = 0;
        again = 0;
        leg = 0;
        count = 0;
    }
    lastStep = millis();

    switch (phase)
    {
    case 0:
        if (servosMoved)
        {
            switch (again)
            {
            case 0:
                setServoPos(cycleNum(leg + 4 + 1), leg % 2 == 0 ? BEDR_START_POS + 15 : BEDR_START_POS - 15); // подгибаем лапки чтоб не упасть на поднятую ногу
                setServoPos(cycleNum(leg + 4 - 1), leg % 2 == 0 ? BEDR_START_POS - 15 : BEDR_START_POS + 15);
                again++;
                // break;
            case 1:
                setServoPos(leg, KNEE_START_POS - 100); // поднимаем ногу переднюю правую
                again++;
                break;
            case 2:
                setServoPos(leg + 4, BEDR_START_POS + 50); // опускаем ногу
                setServoPos(leg, KNEE_START_POS - 20);     // опускаем не до конца
                leg += 2;
                again++;
                break;
            case 3:
                setServoPos(cycleNum(leg + 4 + 1), leg % 2 == 0 ? BEDR_START_POS + 50 : BEDR_START_POS - 50); // подгибаем лапки чтоб не упасть на поднятую ногу
                setServoPos(cycleNum(leg + 4 - 1), leg % 2 == 0 ? BEDR_START_POS - 50 : BEDR_START_POS + 50);
                again++;
                break;
            case 4:
                setServoPos(leg, KNEE_START_POS - 100); // поднимаем заднюю левую
                setServoPos(leg - 2, KNEE_START_POS);   // опустим до конца
                again++;
                break;
            case 5:
                setServoPos(leg + 4, BEDR_START_POS - 50); // опускаем ногу
                setServoPos(leg, KNEE_START_POS);
                leg -= 1;
                again++;
                break;
            case 6:
                setServoPos(cycleNum(leg + 4 + 1), leg % 2 == 0 ? BEDR_START_POS + 50 : BEDR_START_POS - 50); // подгибаем лапки чтоб не упасть на поднятую ногу
                setServoPos(cycleNum(leg + 4 - 1), leg % 2 == 0 ? BEDR_START_POS - 50 : BEDR_START_POS + 50);
                again++;
                break;
            case 7:
                setServoPos(leg, KNEE_START_POS - 100); // поднимаем переднюю левую
                again++;
                break;
            case 8:
                setServoPos(leg + 4, BEDR_START_POS + 50); // опускаем ногу
                setServoPos(leg, KNEE_START_POS - 20);
                leg += 2;
                again++;
                break;
            case 9:
                setServoPos(cycleNum(leg + 4 + 1), leg % 2 == 0 ? BEDR_START_POS + 50 : BEDR_START_POS - 50); // подгибаем лапки чтоб не упасть на поднятую ногу
                setServoPos(cycleNum(leg + 4 - 1), leg % 2 == 0 ? BEDR_START_POS - 50 : BEDR_START_POS + 50);
                again++;
                break;
            case 10:
                setServoPos(leg, KNEE_START_POS - 100); // поднимаем заднюю правую
                setServoPos(leg - 2, KNEE_START_POS);
                again++;
                break;
            case 11:
                setServoPos(leg + 4, BEDR_START_POS - 50); // опускаем ногу
                setServoPos(leg, KNEE_START_POS);
                again = 1;
                leg = 0;
                count++;
                return true;
                break;
            }
        }

        break;

    case 1: // left
        if (servosMoved)
        {
        }
        break;
    case 2: // right
        if (servosMoved)
        {
        }
        break;
    }
    return false;
}

void dancing() // функция танца
{
    // static uint32_t lastDance = millis();
    static byte phase = 0;
    static byte again = 0;
    static int8_t leg = 0;
    static byte count = 0;

    if (prevState != currentState)
    {
        phase = 0;
        again = 0;
        leg = 0;
        count = 0;
    }

    switch (phase)
    {
    case 0:
        if (servosMoved)
        {
            switch (again)
            {
            case 0:
                setServoPos(leg, KNEE_START_POS - 100);                                                       // поднимаем ногу
                setServoPos(cycleNum(leg + 4 + 1), leg % 2 == 0 ? BEDR_START_POS + 15 : BEDR_START_POS - 15); // подгибаем лапки чтоб не упасть на поднятую ногу
                setServoPos(cycleNum(leg + 4 - 1), leg % 2 == 0 ? BEDR_START_POS - 15 : BEDR_START_POS + 15);
                again++;
                break;
            case 1:
                setServoPos(leg + 4, BEDR_START_POS - 40); // трясем поднятой
                again++;
                break;
            case 2:
                setServoPos(leg + 4, BEDR_START_POS + 40); // трясем в другую сторону
                again++;
                break;
            case 3:
                setServoPos(leg + 4, BEDR_START_POS); // обратно
                again++;
                break;
            case 4:
                setServoPos(leg, KNEE_START_POS); // опускаем
                again++;
                break;
            case 5:
                set_default_pos();
                again = 0;
                leg++;
                count++;
                if (leg == 4)
                    leg = 0;
                if (count >= 4)
                {
                    phase++;
                    leg = 0;
                    again = 0;
                    count = 0;
                }
                break;
            }
        }
        break;

    case 1: // анжумания
        if (servosMoved)
        {
            switch (again)
            {
            case 0:
                setServoPos(leg, KNEE_START_POS - 65);
                setServoPos(leg + 1, KNEE_START_POS - 65);
                setServoPos(leg + 2, KNEE_START_POS - 65);
                setServoPos(leg + 3, KNEE_START_POS - 65);
                again++;
                count++;
                break;
            case 1:
                set_default_pos();
                again = 0;
                if (count >= 2)
                {
                    phase++;
                    count = 0;
                }
                break;
            }
        }
        break;
    case 2: // анжумания только перед
        if (servosMoved)
        {
            switch (again)
            {
            case 0:
                setServoPos(leg, KNEE_START_POS - 65);
                setServoPos(leg + 1, KNEE_START_POS - 65);
                again++;
                count++;
                break;
            case 1:
                set_default_pos();
                again = 0;
                if (count >= 2)
                {
                    phase = 0;
                    count = 0;
                }
                break;
            }
        }
        break;
    }
}
