#pragma once
#include "config.h"
static ServoSmooth servos[SERVOS_COUNT];
static const byte servoPins[SERVOS_COUNT] = {4, 9, 5, 7, 8, 3, 6, 2};
static const int8_t servoAngleCorrections[SERVOS_COUNT] = {15, -15, -5, -15, -4, 0, 6, 0};
static unsigned long servoTimer = 0;// Глобальный Серво таймер для ручных тиков

// unsigned long servoTimer = 0;
void init_servos();
void set_default_pos();
void setServoPos(byte servoNum, int pos);
int convertAngle(byte servoNumber, int angle);
bool tickAllServos();

void init_servos() // функция инициализации
{
    for (byte i = 0; i < SERVOS_COUNT / 2; i++)
    {                                                                    // инициализируем колени Первые 4 серво - колени; Для четных верхнее положение - 0
        servos[i].setAutoDetach(KNEE_AUTODETACH);                        // авто детач не нужен в связи с нагрузкой
        servos[i].attach(servoPins[i], convertAngle(i, KNEE_START_POS)); // Для нечетных верхнее положение - 180
        // servos[i].smoothStart();                                          // плавный старт
    }

    delay(500);

    for (byte i = 4; i < SERVOS_COUNT; i++)
    {                                             // инициализируем бедра следующие 4 сервы - бедра
        servos[i].setAutoDetach(BEDR_AUTODETACH); // автодетач им супер нужен для экономии батарейки
        servos[i].attach(servoPins[i], convertAngle(i, BEDR_START_POS));
        // servos[i].smoothStart();  // плавный старт не дает весомых результатов
    }

    for (byte i = 0; i < SERVOS_COUNT; i++)
    {
        servos[i].setSpeed(SERVO_DEFAULT_SPEED); // Задаем скорость для всех серв
        servos[i].setAccel(SERVO_DEFAULT_ACCEL);
        // servos[i].setAutoDetach(i>=SERVOS_COUNT/2 ? true : false);
    }

    set_default_pos(); // зададим начальную позицию сервам
}

void set_default_pos() // начальная позиция серв
{
    for (byte i = 0; i < SERVOS_COUNT / 2; i++)
    { // дополнительно выставляем стартовый угол колен
        // servos[i].setTargetDeg(convertAngle(i, KNEE_START_POS));
        setServoPos(i, KNEE_START_POS);
        // servos[i].setAutoDetach(false);
    }

    for (byte i = 4; i < SERVOS_COUNT; i++)
    { // стартовый угол бедер
        // servos[i].setTargetDeg(convertAngle(i, BEDR_START_POS));
        setServoPos(i, BEDR_START_POS);
        // servos[i].setAutoDetach(true);
    }
}

void setServoPos(byte servoNum, int pos) // функция для задания положения серво. (НОМЕР серво, угол в формате 0-180) номер в соответствии с массивом servoPins
{
    // servoNum = servoNum
    if (servos[servoNum].getTargetDeg() != convertAngle(servoNum, pos))
    {
        servos[servoNum].setTargetDeg(convertAngle(servoNum, pos));
        servosMoved = false;
    }
}

int convertAngle(byte servoNumber, int angle) // Функция для перобразования углов четных-нечетных лап в формат 0-180 и коррекция углов ()
{
    bool isKnee = servoNumber < SERVOS_COUNT / 2; //!!! Принимает НОМЕР серво, а не пин!!!!
    angle += servoAngleCorrections[servoNumber];
    angle = isKnee ? constrain(angle, KNEE_MIN_POS + servoAngleCorrections[servoNumber], KNEE_MAX_POS + servoAngleCorrections[servoNumber])
                   : constrain(angle, BEDR_MIN_POS + servoAngleCorrections[servoNumber], BEDR_MAX_POS + servoAngleCorrections[servoNumber]); // ограничение максимального угла серво с учетом коррекции угла
    return (servoNumber % 2 == 0 ? angle : 180 - angle);                                                                                     // возврат угла с учетом преобразования по четности машинки
}

bool tickAllServos() // функция для двига всех серв разом, возвращает true, если все сервы на целевых позициях!
{
    uint8_t movedCount = 0; // для подсчета серв в таргетном положении
    if (millis() - servoTimer >= 20)
    { // взводим таймер на 20 мс (как в библиотеке)
        servoTimer = millis();
        for (byte i = 0; i < SERVOS_COUNT; i++)
        {
            movedCount += servos[i].tickManual(); // двигаем все сервы. Такой вариант эффективнее отдельных тиков
        }
        return movedCount == SERVOS_COUNT;
    }
    else
        return servosMoved;
}
