#include <config.h>
constexpr uint8_t SERVOS_COUNT;
ServoSmooth servos[SERVOS_COUNT];
byte servoPins[SERVOS_COUNT];

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