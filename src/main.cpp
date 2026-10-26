#include <Arduino.h>
#include <ServoSmooth.h> // основная библиотека для плавных и независимых серв
// #include <NewPing.h> // не понадобилось

#define USE_SERVO_FLAG true  // флаг отладки, отключающий серво
#define DEBUG_MODE false     // флаг дебаг режима
#define DEBUG_INF_MSGS false // флаг регулярные сообщения дебага в дебаг режиме

#include "config.h"
#include "legs.h"
#include "sonar.h"
#include "button.h"
#include "led.h"
#include "gaits.h"

bool servosMoved;             // флаг определения достижения целевой позиции всеми сервами
unsigned int sonarDistanceCm; // расстояние с датчика в см
// unsigned int sonarDistanceCmFilteredSmooth;

SpiderState currentState = STATE_START_POS; // задаем состояние в момент запуска
SpiderState prevState = STATE_START_POS;    // предыдущее состояние в момент запуска

void setup()
{
    pinMode(LED, OUTPUT); // настройка всех пинов
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(HC_TRIG, OUTPUT); // trig выход
    pinMode(HC_ECHO, INPUT);  // echo вход
    // digitalWrite(LED, LOW);

#if DEBUG_MODE // если дебаг, то подрубаем вывод в консоль
    {
        Serial.begin(9600);
        Serial.println("Spider bro is here!");
        // Serial.println("0 - sleep\n1 - dance");
    }
#endif

#if USE_SERVO_FLAG // если флаг разрешает -- врубаем сервы
    {
        init_servos();
    }
#endif
    led_blink(); // моргаем

    // fr_kn.attach(FR_KN_PIN, 0); //исторически оставшийся тут код
    // fr_kn.smoothStart();
    // fr_kn.setSpeed(40);
    // fr_kn.setAccel(0.1);
    // fl_kn.attach(FL_KN_PIN);
    // Serial.begin(9600);
}

void loop()
{
    digitalWrite(LED, currentState == STATE_SLEEP ? false : (LED_STAT ? digitalRead(BUTTON_PIN) : !digitalRead(BUTTON_PIN))); // если не спим, то смотрим на значение флага

    servosMoved = currentState != STATE_CHANGE_STATE ? tickAllServos() : servosMoved; // обоновляем переменную законченности фаз, если состояние робота не STATE_CHANGE_STATE, иначе -- предыдущее значение

    // if (servosMoved) sonarDistanceCm = getSonarDistance();
    if (millis() - sonarTimer > 500) // по таймеру узнаем расстояние (опционально стоит переделать чтобы это было только в необходимых режимах)
    {
        sonarDistanceCm = getSonarDistance();
        sonarTimer = millis();
    }

    currentState = button_handler(); // обновляем состояние через обработчик кнопки

    if (prevState != currentState && currentState != STATE_CHANGE_STATE) // если состояние поменялось, то сначала занимаем начальную позицию, затем моргаем
    {
        set_default_pos();
        led_blink(currentState); // моргаем по номеру состояния
    }

    switch (currentState) // в зависимости от состояния выбираем че делать, в какую функцию передать упралвение и тд
    {
    case STATE_START_POS: // стартовое положение
        if (currentState != prevState)
        {
            set_default_pos();
        }
        break;

    case STATE_SLEEP: // позиция сна
        if (currentState != prevState)
        {
            // init_servos(false);
            for (byte i = 0; i < SERVOS_COUNT / 2; i++)
            {
                // servos[i].stop();  // поменяй на занимение положения спанья ### ПЕРЕДЕЛАЙ
                //  servos[i].setTargetDeg(servos[i].getCurrentDeg() + 10);
                //  servos[i].setAutoDetach(true);
                setServoPos(i, KNEE_START_POS - 65);
            }
        }
        break;

    case STATE_DANCE: // танцульки
        dancing();
        break;
        // case STATE_CHANGE_STATE: // исторический код
        //   if (currentState != prevState) {
        //     for (byte i = 0; i < SERVOS_COUNT; i++) {
        //       servos[i].stop();  // поменяй на занимение положения спанья
        //     }
        //   }
        //   break;
    case STATE_PATROL: // ходьба
        // static bool stepDone = false;
        if (sonarDistanceCm > 20)
        {
            // stepDone = movement(); // временно отключено за ненадобностью
            movement();
        }
        else
        {
            set_default_pos();
        }
        break;

    default:
        break;
    }

#if DEBUG_MODE
    if (DEBUG_MODE & millis() - debugMsgTimer >= 1000 && DEBUG_INF_MSGS) // регулярное сообщение дебага
    {
        static int msgCounter = 0;
        Serial.print("Sonar distance: ");
        Serial.println(sonarDistanceCm);
        Serial.print("Millis: ");
        Serial.println(millis());
        Serial.print("Current state: ");
        Serial.println(currentState);
        Serial.print("Servos moved: ");
        Serial.println(servosMoved);
        Serial.print("Avg loop time: ");
        Serial.println("###");
        Serial.print("Message number: ");
        Serial.println(msgCounter);

        Serial.println();
        debugMsgTimer = millis();
        msgCounter++;
    }
#endif
    prevState = currentState != STATE_CHANGE_STATE ? currentState : prevState; // предыыдущее состояние с учетом того, что это не может быть состояние смены состояния
}
