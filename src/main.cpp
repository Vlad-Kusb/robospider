#include <Arduino.h>
#include <ServoSmooth.h> // основная библиотека для плавных и независимых серв
// #include <NewPing.h> // не понадобилось

#define USE_SERVO_FLAG true  // флаг отладки, отключающий серво
#define DEBUG_MODE false     // флаг дебаг режима
#define DEBUG_INF_MSGS false // флаг регулярные сообщения дебага в дебаг режиме

constexpr uint8_t HC_TRIG = 11;     // триггер пин
constexpr uint8_t HC_ECHO = 12;     // эхо пин
constexpr uint8_t SERVOS_COUNT = 8; // количество сервоприводов

constexpr uint8_t KNEE_START_POS = 125;        // начальная позиция сервоприводов для коленей
constexpr uint8_t BEDR_START_POS = 40;         // начальная позиция для бедер
constexpr bool KNEE_AUTODETACH = false;        // автоотключение серв коленей при достижении цели
constexpr bool BEDR_AUTODETACH = true;         // автооткл бедер
constexpr uint8_t BEDR_MIN_POS = 0;            // минимальная позиция для бедер
constexpr uint8_t BEDR_MAX_POS = 70;           // максимальная позиция бедер
constexpr uint8_t KNEE_MIN_POS = 0;            // минимум коленей
constexpr uint8_t KNEE_MAX_POS = 140;          // максимум коленей
constexpr uint8_t SERVO_DEFAULT_SPEED = 250;   // град/сек
constexpr float SERVO_DEFAULT_ACCEL = 2.1f;    // ускорение
constexpr uint8_t LED = 13;                    // пин светодиода
constexpr bool LED_STAT = false;               // светодиод по умолчанию горит всегда и меняет состояние на время нажатия кнопки
constexpr uint8_t SONAR_MAX_DISTANCE_CM = 500; // ограничение максимального измерения расстояния
constexpr uint8_t BUTTON_PIN = 10;             // пин кнопки, выбор режимов должен быть по ней, сколько раз нажата -- такой режим и выбран, повторное одинарное нажатие в любом режиме просто вернет робота в стартовую позицию и будет ждать указаний
constexpr uint8_t BTN_DEB = 100;               // дребезг кнопки

ServoSmooth servos[SERVOS_COUNT];                                                   // сервоприводы
const byte servoPins[SERVOS_COUNT] = {4, 9, 5, 7, 8, 3, 6, 2};                      // пины сервоприводов подключенные против часовой стрелки, начиная с правого переднего колена, заканчивая правым передним бедром (По этому же списку нумерация)
const int8_t servoAngleCorrections[SERVOS_COUNT] = {15, -15, -5, -15, -4, 0, 6, 0}; // углы коррекции для нулевого положения

unsigned long servoTimer = 0;    // Глобальный Серво таймер для ручных тиков
unsigned long debugMsgTimer = 0; // таймер
unsigned long sonarTimer = 0;    // таймер

bool servosMoved; // флаг определения достижения целевой позиции всеми сервами

unsigned int sonarDistanceCm; // расстояние с датчика в см
// unsigned int sonarDistanceCmFilteredSmooth;

enum SpiderState
{                      // объект состояний робота
    STATE_START_POS,   // 0 Стартовая позиция
    STATE_DANCE,       // 1 танец
    STATE_PATROL,      // 2 патруль
    STATE_SLEEP,       // 3 спать
    STATE_CHANGE_STATE // 4 изменение состояния
};

SpiderState currentState = STATE_START_POS; // задаем состояние в момент запуска
SpiderState prevState = STATE_START_POS;    // предыдущее состояние в момент запуска

// put function declarations here:
void led_blink(uint8_t count);             // вспомогательная функция, которая моргает светодиодом заданное число раз
void init_servos();                            // функция инициализации
int cycleNum(int num, bool isBedr = true);     // вспомогательная функция на случай переполнения в циклах ходьбы
bool movement(byte phase = 0);                 // ходьба 0 1 2 (перед лево право), основная функция для ходьбы
int getSonarDistance();                        // получение дистанции с датчика
SpiderState button_handler();                  // обработчик кнопки
void dancing();                                // функция танца
void set_default_pos();                        // начальная позиция серв
void setServoPos(byte servoNum, int pos);      // функция для задания положения серво. (НОМЕР серво, угол в формате 0-180) номер в соответствии с массивом servoPins
int convertAngle(byte servoNumber, int angle); // Функция для перобразования углов четных-нечетных лап в формат 0-180 и коррекция углов ()
bool tickAllServos();                          // функция для двига всех серв разом, возвращает true, если все сервы на целевых позициях!

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
        static bool stepDone = false;
        if (sonarDistanceCm > 20)
        {
            stepDone = movement();
        }
        else
        {
            set_default_pos();
        }
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
