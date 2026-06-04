#include <ServoSmooth.h>
// #include <NewPing.h>

#define HC_TRIG 11      //триггер пин
#define HC_ECHO 12      // эхо пин
#define SERVOS_COUNT 8  // количество сервоприводов

#define KNEE_START_POS 125   // начальная позиция сервоприводов для коленей
#define BEDR_START_POS 40    // начальная позиция для бедер
#define USE_SERVO_FLAG true  // флаг отладки, отключающий серво
#define LED 13               // пин светодиода
#define LED_STAT false       // светодиод по умолчанию горит всегда и меняет состояние на время нажатия кнопки
#define BUTTON_PIN 10        // пин кнопки, выбор режимов должен быть по ней, сколько раз нажата -- такой режим и выбран, 1 нажатие -- танец жука, 2 -- патрулирование, повторное одинарное нажатие в любом режиме просто вернет робота в стартовую позицию и будет ждать указаний
#define DEBUG_MODE false
#define DEBUG_INF_MSGS false  //регулярные сообщения дебага
#define SONAR_MAX_DISTANCE_CM 500
#define BTN_DEB 100              // дребезг кнопки
#define KNEE_AUTODETACH false    //автоотключение серв коленей при достижении цели
#define BEDR_AUTODETACH true     //автооткл бедер
#define BEDR_MIN_POS 0           // минимальная позиция для бедер
#define BEDR_MAX_POS 70          // максимальная позиция бедер
#define KNEE_MIN_POS 0           //минимум коленей
#define KNEE_MAX_POS 140         // максимум коленей
#define SERVO_DEFAULT_SPEED 250  //град/сек
#define SERVO_DEFAULT_ACCEL 2.1  //ускорение


ServoSmooth servos[SERVOS_COUNT];                                                      // сервоприводы
const byte servoPins[SERVOS_COUNT] = { 4, 9, 5, 7, 8, 3, 6, 2 };                       //пины сервоприводов подключенные против часовой стрелки, начиная с правого переднего колена, заканчивая правым передним бедром
const int8_t servoAngleCorrections[SERVOS_COUNT] = { 15, -15, -5, -15, -4, 0, 6, 0 };  // углы коррекции для нулевого положения

unsigned long servoTimer = 0;     // Глобальный Серво таймер для ручных тиков
unsigned long debugMsgTimer = 0;  //таймер
unsigned long SonarTimer = 0;

bool servosMoved;  // флаг определения достижения целевой позиции всеми сервами

unsigned int sonarDistanceCm;
// unsigned int sonarDistanceCmFilteredSmooth;

enum SpiderState {
  STATE_START_POS,    // 0 Стартовая позиция
  STATE_DANCE,        //1 танец
  STATE_PATROL,       // 2 патруль
  STATE_SLEEP,        // 3 спать
  STATE_CHANGE_STATE  //4 изменение состояния
};

SpiderState currentState = STATE_START_POS;
SpiderState prevState = STATE_START_POS;

void led_blink(uint8_t count = 5) {
  for (byte i = 0; i <= count * 2; i++) {
    digitalWrite(LED, i % 2 == 0);
    delay(150);
  }
}

void init_servos() {                                                  //(bool att = true)
                                                                      // if (att) {
  for (byte i = 0; i < SERVOS_COUNT / 2; i++) {                       // инициализируем колени Первые 4 серво - колени; Для четных верхнее положение - 0
    servos[i].setAutoDetach(KNEE_AUTODETACH);                         // авто детач не нужен в связи с нагрузкой
    servos[i].attach(servoPins[i], convertAngle(i, KNEE_START_POS));  //Для нечетных верхнее положение - 180
    //servos[i].smoothStart();                                          // плавный старт
  }

  delay(500);

  for (byte i = 4; i < SERVOS_COUNT; i++) {    // инициализируем бедра Некст 4 серво - бедра
    servos[i].setAutoDetach(BEDR_AUTODETACH);  // автодетач им супер нужен для экономии батарейки
    servos[i].attach(servoPins[i], convertAngle(i, BEDR_START_POS));
    //servos[i].smoothStart();  // плавный старт
  }

  for (byte i = 0; i < SERVOS_COUNT; i++) {
    servos[i].setSpeed(SERVO_DEFAULT_SPEED);  // Задаем скорость для всех серв
    servos[i].setAccel(SERVO_DEFAULT_ACCEL);
    // servos[i].setAutoDetach(i>=SERVOS_COUNT/2 ? true : false);
  }
  // } else {
  //   for (byte i = 0; i < SERVOS_COUNT; i++) {
  //     servos[i].detach();
  //   }
  // }

  set_default_pos();  //начальная позиция
}

int cycleNum(int num, bool isBedr = true) {
  const int start = isBedr ? 4 : 0;
  const int length = 4;  // Количество чисел в диапазоне (4, 5, 6, 7)

  // Нормализуем число в диапазон [0, length-1] с зацикливанием
  int normalized = (num - start) % length;

  // Корректируем отрицательные значения
  if (normalized < 0) {
    normalized += length;
  }

  return start + normalized;
}

bool movement(byte phase = 0) {  //ходьба 0 1 2 (перед лево право)
  static uint32_t lastStep = millis();
  // static byte phase = 0;
  static byte again = 0;
  static int8_t leg = 0;
  static byte count = 0;

  if (prevState != currentState || millis() - lastStep > 100) {
    // phase = 0;
    again = 0;
    leg = 0;
    count = 0;
  }
  lastStep = millis();

  switch (phase) {
    case 0:
      if (servosMoved) {
        switch (again) {
          case 0:
            setServoPos(cycleNum(leg + 4 + 1), leg % 2 == 0 ? BEDR_START_POS + 15 : BEDR_START_POS - 15);  //подгибаем лапки чтоб не упасть на поднятую ногу
            setServoPos(cycleNum(leg + 4 - 1), leg % 2 == 0 ? BEDR_START_POS - 15 : BEDR_START_POS + 15);
            again++;
            // break;
          case 1:
            setServoPos(leg, KNEE_START_POS - 100);  // поднимаем ногу переднюю правую
            again++;
            break;
          case 2:
            setServoPos(leg + 4, BEDR_START_POS + 50);  // опускаем ногу
            setServoPos(leg, KNEE_START_POS - 20);      //опускаем не до конца
            leg += 2;
            again++;
            break;
          case 3:
            setServoPos(cycleNum(leg + 4 + 1), leg % 2 == 0 ? BEDR_START_POS + 50 : BEDR_START_POS - 50);  //подгибаем лапки чтоб не упасть на поднятую ногу
            setServoPos(cycleNum(leg + 4 - 1), leg % 2 == 0 ? BEDR_START_POS - 50 : BEDR_START_POS + 50);
            again++;
            break;
          case 4:
            setServoPos(leg, KNEE_START_POS - 100);  // поднимаем заднюю левую
            setServoPos(leg - 2, KNEE_START_POS);    //опустим до конца
            again++;
            break;
          case 5:
            setServoPos(leg + 4, BEDR_START_POS - 50);  //опускаем ногу
            setServoPos(leg, KNEE_START_POS);
            leg -= 1;
            again++;
            break;
          case 6:
            setServoPos(cycleNum(leg + 4 + 1), leg % 2 == 0 ? BEDR_START_POS + 50 : BEDR_START_POS - 50);  //подгибаем лапки чтоб не упасть на поднятую ногу
            setServoPos(cycleNum(leg + 4 - 1), leg % 2 == 0 ? BEDR_START_POS - 50 : BEDR_START_POS + 50);
            again++;
            break;
          case 7:
            setServoPos(leg, KNEE_START_POS - 100);  // поднимаем переднюю левую
            again++;
            break;
          case 8:
            setServoPos(leg + 4, BEDR_START_POS + 50);  //опускаем ногу
            setServoPos(leg, KNEE_START_POS - 20);
            leg += 2;
            again++;
            break;
          case 9:
            setServoPos(cycleNum(leg + 4 + 1), leg % 2 == 0 ? BEDR_START_POS + 50 : BEDR_START_POS - 50);  //подгибаем лапки чтоб не упасть на поднятую ногу
            setServoPos(cycleNum(leg + 4 - 1), leg % 2 == 0 ? BEDR_START_POS - 50 : BEDR_START_POS + 50);
            again++;
            break;
          case 10:
            setServoPos(leg, KNEE_START_POS - 100);  // поднимаем заднюю правую
            setServoPos(leg - 2, KNEE_START_POS);
            again++;
            break;
          case 11:
            setServoPos(leg + 4, BEDR_START_POS - 50);  //опускаем ногу
            setServoPos(leg, KNEE_START_POS);
            again = 1;
            leg = 0;
            count++;
            return true;
            break;
        }
      }

      break;

    case 1:  //left
      if (servosMoved) {
      }
      break;
    case 2:  // right
      if (servosMoved) {
      }
      break;
  }
  return false;
}

int getSonarDistance() {
  int duration, cm;            // назначаем переменную "cm" и "duration" для показаний датчика
  digitalWrite(HC_TRIG, LOW);  // изначально датчик не посылает сигнал
  delayMicroseconds(2);        // ставим задержку в 2 ммикросекунд

  digitalWrite(HC_TRIG, HIGH);  // посылаем сигнал
  delayMicroseconds(10);        // ставим задержку в 10 микросекунд
  digitalWrite(HC_TRIG, LOW);   // выключаем сигнал

  duration = pulseIn(HC_ECHO, HIGH);  // включаем прием сигнала

  cm = duration / 58;  // вычисляем расстояние в сантиметрах

  if (cm > SONAR_MAX_DISTANCE_CM) return SONAR_MAX_DISTANCE_CM;

  return cm;
}

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(HC_TRIG, OUTPUT);  // trig выход
  pinMode(HC_ECHO, INPUT);   // echo вход
  // digitalWrite(LED, LOW);

  if (DEBUG_MODE) {
    Serial.begin(9600);
    Serial.println("Spider bro is here!");
    // Serial.println("0 - sleep\n1 - dance");
  }


  if (USE_SERVO_FLAG) {
    init_servos();
  }
  led_blink();

  //fr_kn.attach(FR_KN_PIN, 0);
  //fr_kn.smoothStart();
  //fr_kn.setSpeed(40);
  //fr_kn.setAccel(0.1);
  //fl_kn.attach(FL_KN_PIN);
  //Serial.begin(9600);
}

void loop() {
  digitalWrite(LED, currentState == STATE_SLEEP ? false : (LED_STAT ? digitalRead(BUTTON_PIN) : !digitalRead(BUTTON_PIN)));  //если не спим, то смотрим на значение флага

  servosMoved = currentState != STATE_CHANGE_STATE ? tickAllServos() : servosMoved;  //Двигаем все сервы

  // if (servosMoved) sonarDistanceCm = getSonarDistance();
  if (millis() - SonarTimer > 500) {
    sonarDistanceCm = getSonarDistance();
    SonarTimer = millis();
  }

  currentState = button_handler();

  if (prevState != currentState & currentState != STATE_CHANGE_STATE) {
    set_default_pos();
    led_blink(currentState);
  }

  switch (currentState) {
    case STATE_START_POS:
      if (currentState != prevState) {
        set_default_pos();
      }
      break;

    case STATE_SLEEP:
      if (currentState != prevState) {
        // init_servos(false);
        for (byte i = 0; i < SERVOS_COUNT / 2; i++) {
          //servos[i].stop();  // поменяй на занимение положения спанья ### ПЕРЕДЕЛАЙ
          // servos[i].setTargetDeg(servos[i].getCurrentDeg() + 10);
          // servos[i].setAutoDetach(true);
          setServoPos(i, KNEE_START_POS - 65);
        }
      }
      break;

    case STATE_DANCE:
      dancing();
      break;
      // case STATE_CHANGE_STATE:
      //   if (currentState != prevState) {
      //     for (byte i = 0; i < SERVOS_COUNT; i++) {
      //       servos[i].stop();  // поменяй на занимение положения спанья
      //     }
      //   }
      //   break;
    case STATE_PATROL:
      static bool stepDone = false;
      if (sonarDistanceCm > 20) {
        stepDone = movement();
      } else {
        set_default_pos();
      }
  }



  if (DEBUG_MODE & millis() - debugMsgTimer >= 1000 & DEBUG_INF_MSGS) {
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
  prevState = currentState != STATE_CHANGE_STATE ? currentState : prevState;  //предыыдущее состояние с учетом того, что это не может быть состояние смены состояния
}

SpiderState button_handler() {
  static bool pbtn = false;
  static uint32_t tmr = millis();
  static uint32_t lastClick = 0;
  static uint8_t countClicks = 0;
  bool btn = !digitalRead(BUTTON_PIN);

  if (pbtn != btn && millis() - tmr >= BTN_DEB) {
    tmr = millis();
    pbtn = btn;
    if (btn) {
      if (millis() - lastClick > 1000) countClicks = 0;
      countClicks++;
      lastClick = millis();
      return STATE_CHANGE_STATE;
    };
    //else Serial.println("Кнопка отпущена");
  }

  // кнопка удерживается дольше 500 мс
  if (pbtn && millis() - tmr >= 500) {
    // tmr = millis();  // сброс таймера
    return STATE_CHANGE_STATE;
    // Serial.println("Кнопка удержана");
  }

  if (millis() - lastClick > 1000) {
    switch (countClicks) {
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
  } else return STATE_CHANGE_STATE;
  return STATE_START_POS;
}

void dancing() {
  // static uint32_t lastDance = millis();
  static byte phase = 0;
  static byte again = 0;
  static int8_t leg = 0;
  static byte count = 0;

  if (prevState != currentState) {
    phase = 0;
    again = 0;
    leg = 0;
    count = 0;
  }

  switch (phase) {
    case 0:
      if (servosMoved) {
        switch (again) {
          case 0:
            setServoPos(leg, KNEE_START_POS - 100);                                                        // поднимаем ногу
            setServoPos(cycleNum(leg + 4 + 1), leg % 2 == 0 ? BEDR_START_POS + 15 : BEDR_START_POS - 15);  //подгибаем лапки чтоб не упасть на поднятую ногу
            setServoPos(cycleNum(leg + 4 - 1), leg % 2 == 0 ? BEDR_START_POS - 15 : BEDR_START_POS + 15);
            again++;
            break;
          case 1:
            setServoPos(leg + 4, BEDR_START_POS - 40);  // трясем поднятой
            again++;
            break;
          case 2:
            setServoPos(leg + 4, BEDR_START_POS + 40);  // трясем в другую сторону
            again++;
            break;
          case 3:
            setServoPos(leg + 4, BEDR_START_POS);  // обратно
            again++;
            break;
          case 4:
            setServoPos(leg, KNEE_START_POS);  // опускаем
            again++;
            break;
          case 5:
            set_default_pos();
            again = 0;
            leg++;
            count++;
            if (leg == 4) leg = 0;
            if (count >= 4) {
              phase++;
              leg = 0;
              again = 0;
              count = 0;
            }
            break;
        }
      }
      break;

    case 1:  //анжумания
      if (servosMoved) {
        switch (again) {
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
            if (count >= 2) {
              phase++;
              count = 0;
            }
            break;
        }
      }
      break;
    case 2:  // анжумания только перед
      if (servosMoved) {
        switch (again) {
          case 0:
            setServoPos(leg, KNEE_START_POS - 65);
            setServoPos(leg + 1, KNEE_START_POS - 65);
            again++;
            count++;
            break;
          case 1:
            set_default_pos();
            again = 0;
            if (count >= 2) {
              phase = 0;
              count = 0;
            }
            break;
        }
      }
      break;
  }
}

void set_default_pos() {
  for (byte i = 0; i < SERVOS_COUNT / 2; i++) {  //дополнительно выставляем стартовый угол колен
    // servos[i].setTargetDeg(convertAngle(i, KNEE_START_POS));
    setServoPos(i, KNEE_START_POS);
    // servos[i].setAutoDetach(false);
  }

  for (byte i = 4; i < SERVOS_COUNT; i++) {  //стартовый угол бедер
    // servos[i].setTargetDeg(convertAngle(i, BEDR_START_POS));
    setServoPos(i, BEDR_START_POS);
    // servos[i].setAutoDetach(true);
  }
}

void setServoPos(byte servoNum, int pos) {  //НОМЕР серво, угол в формате 0-180
  // servoNum = servoNum
  if (servos[servoNum].getTargetDeg() != convertAngle(servoNum, pos)) {
    servos[servoNum].setTargetDeg(convertAngle(servoNum, pos));
    servosMoved = false;
  }
}

int convertAngle(byte servoNumber, int angle) {  // Функция для перобразования углов четных-нечетных лап в формат 0-180 и коррекция углов ()
  bool isKnee = servoNumber < SERVOS_COUNT / 2;  //!!! Принимает НОМЕР серво, а не пин!!!!
  angle += servoAngleCorrections[servoNumber];
  angle = isKnee ? constrain(angle, KNEE_MIN_POS + servoAngleCorrections[servoNumber], KNEE_MAX_POS + servoAngleCorrections[servoNumber])
                 : constrain(angle, BEDR_MIN_POS + servoAngleCorrections[servoNumber], BEDR_MAX_POS + servoAngleCorrections[servoNumber]);  //ограничение максимального угла серво с учетом коррекции угла
  return (servoNumber % 2 == 0 ? angle : 180 - angle);                                                                                      //возврат угла с учетом преобразования по четности машинки
}

bool tickAllServos() {                // функция для двига всех серв разом, возвращает true, если все сервы на целевых позициях!
  uint8_t movedCount = 0;             //для подсчета серв в таргетном положении
  if (millis() - servoTimer >= 20) {  // взводим таймер на 20 мс (как в библиотеке)
    servoTimer = millis();
    for (byte i = 0; i < SERVOS_COUNT; i++) {
      movedCount += servos[i].tickManual();  // двигаем все сервы. Такой вариант эффективнее отдельных тиков
    }
    return movedCount == SERVOS_COUNT;
  } else return servosMoved;
}
