/*
  ConstantsUser.h
  ===============================================================================================================
  https://doc.arduino.ua/ru/prog/StringToUpperCase
  !!! Рекомендується перед прошивкою лампи вказати параметри домашньої WiFi мережі
      (ssid та password) у файлі data/config.json,
      щоб уникнути роботи в режимі точки доступу та одразу після запуску
      налаштувати власні параметри через веб-інтерфейс.
      У цьому файлі зосереджені найчастіше змінювані налаштування,
      що залежать від конструкції лампи та вподобань користувача.
  ===============================================================================================================
*/

#include <ESP8266WebServer.h>

// === СХЕМОТЕХНІКА  ===================
// =====================================

/* | 0 • оригінальна лампа | 1 • лампа javelin | 2 • робот | 3 • рорбот 007 | 4 • IOT lighthouse | 7 • лампа javelin vII

     • IOT [7] JAVELIN_VII           • IOT [1] JAVELIN          • IOT [0] ORG_LAMP        • IOT [3] ROBOT_VII
  DEVICE ......... XX GPIO    DEVICE ......... XX GPIO    DEVICE ......... XX GPIO    DEVICE ......... XX GPIO
  ---------------- -- ----    ---------------- -- ----    ---------------- -- ----    ---------------- -- ----
  LED_INFO ....... D0 (16)    LED_INFO ....... D0 (16)    LED_INFO ....... D0 (16)    LED_INFO ....... D0 (16)
  OLED_SCL ....... D1 (05)    OLED_SCL ....... D1 (05)    OLED_SCL ....... D1 (05)    OLED_SCL ....... D1 (05)
  OLED_SDA ....... D2 (04)    OLED_SDA ....... D2 (04)    OLED_SDA ....... D2 (04)    OLED_SDA ....... D2 (04)
  LED_PIN ........ D3 (00)    LED_PIN ........ D3 (00)    LED_PIN ........ D3 (00)    LED_PIN ........ D3 (00)
  MOSFET ......... D4 (02)    BTN_PIN ........ D4 (02)    BTN_PIN ........ D4 (02)    BTN_PIN ........ D4 (02)
  ENCODER_SCL .... D5 (14)    ENCODER_SCL .... D5 (14)    ENCODER_SCL .... D5 (14)    ENCODER_SCL .... D5 (14)
  ENCODER_SDA .... D6 (12)    ENCODER_SDA .... D6 (12)    ENCODER_SDA .... D6 (12)    ENCODER_SDA .... D6 (12)
  ENCODER_KEY .... D7 (13)    ENCODER_KEY .... D7 (13)    ENCODER_KEY .... D7 (13)    ENCODER_KEY .... D7 (13)
  BTN_PIN ........ D8 (15)    MOSFET ......... D8 (15)    MOSFET ......... D8 (15)    MOSFET ......... D8 (15)

  SENSOR_MOTION... AO (AO)
*/



// Розкоментуйте потрібний варіант конструкції лампи ============================
// #define ROUND_LAMP                       // оригінальна схема лампи «Gyver» (Round Lamp)
// #define ORG_LAMP                         // оригінальна схема лампи «Gyver»
// #define JAVELIN                          // оригінальна схема лампи «JAVELIN»
#define JAVELIN_VII                         // PRO-схема лампи «JAVELIN VII»
// ==============================================================================

/* IOT_TYPE
   | 0 • оригінальна лампа | 1 • лампа javelin | 2 • робот
   | 3 • рорбот 007 | 4 • IOT lighthouse | 7 • лампа javelin vII */

/* WiFi Lamp Javelin ver. II ================================================ */
#ifdef JAVELIN_VII
static byte fps;

#define IOT_TYPE                (7U)       // введено поняття типу пристрою, потрібно для сумісності з прогамним забезпеченням

#define USE_ENCODER
#define USE_OLED

#define OLED_RESET -1                                       //   QT-PY / XIAO
#define SCREEN_WIDTH           (128)
#define SCREEN_HEIGHT           (64)
/* used pins */
#define LED_INFO                (16)                        // D0 | 16U •
//                                                             D1 |  5U • OLED SCL
//                                                             D2 |  4U • OLED SDA
#define LED_PIN                 (0U)                        // D3 | 00U •
#define MOSFET_PIN              (2U)                        // D4 |  2U • 
#define MB_LED_PIN              (2U)                        /* D4 |  2U • blue led на платі */

#ifdef USE_ENCODER
#define ENCODER_CLK            (14U)                        // D5 | 14U •
#define ENCODER_DT             (12U)                        // D6 | 12U •
#define ENCODER_KEY            (13U)                        // D7 | 13U •
#endif

#define BTN_PIN                 (15U)                       // D8 | GPIO15
#define SENSOR_MOTION           (A0)                        // A0 | вхід датчика руху
#define MOSFET_LEVEL            (HIGH)                      // логічний рівень для MOSFET_PIN,
// коли матриця увімкнена: HIGH або LOW

/* Використовувати сенсорну кнопку */
#define ESP_USE_BUTTON

// === LED МАТРИЦЯ ======================================================================
// Або одна матриця, або світлодіодна стрічка.
// Типове підключення: наприклад, матриця 16×16 або стрічка.
#define MATRIX_TYPE             (0U)                        // тип матриці:
// 0 – зигзаг (serpentine)
// 1 – паралельна (progressive)

#define MATRIX_OFFSET           (2U)                        // зсув, якщо додатково використовується
// дві LED-стрічки по 24 світлодіоди

static const uint8_t WIDTH     = 24;                        // ширина матриці (static const для
// сумісності з бібліотекою OLED)
static const uint8_t HEIGHT    = 34;                        // висота матриці (static const для
// сумісності з бібліотекою OLED)

#define ROUND_MATRIX            (24U)
#define ROUND_MATRIX_BRIGHTNESS (128)

#define COLOR_ORDER             (GRB)                       // порядок кольорів у стрічці.
// Якщо кольори відображаються неправильно —
// змініть (можна почати з RGB)

#define CONNECTION_ANGLE        (0U)                        // кут підключення:
// 0 – лівий нижній
// 1 – лівий верхній
// 2 – правий верхній
// 3 – правий нижній

#define STRIP_DIRECTION         (0U)                        // напрямок стрічки від кута:
// 0 – вправо
// 1 – вгору
// 2 – вліво
// 3 – вниз

#define CURRENT_LIMIT           (1500U)                     // обмеження струму (мА).
// Автоматично керує яскравістю
// (бережіть блок живлення!)
// 0 – вимкнути обмеження

#define TEXT_HEIGHT             (7U)                        // висота прокрутки тексту
// (від нижнього краю матриці)

#endif  /* END JAVELIN_VII */




// === JAVELIN ========================
// оригінальна конструкція лампи ======
#ifdef JAVELIN

#define IOT_TYPE                (1U)       // введено поняття типу пристрою, потрібно для сумісності з прогамним забезпеченням

/* used pins */
// #define LED_INFO               (16U)                        // D0 | 16U •
#define BACKLIGHT_PIN           (5U)                        // D1 |  5U • 
#define BTN_PIN                 (4U)                        // D2 |  4U • 
#define LED_PIN                 (0U)                        // D3 |  0U •
#define MB_LED_PIN              (2U)                        /* D4 |  2U • blue led на платі */
// #define NA                     (14U)                        // D5 | 14U •
#define OTA_PIN                (12U)                        // D6 | 12U •
#define BTN_JAVELIN_PIN        (13U)                        // D7 | 13U •
// #define NA                     (15U)                        // D8 | 15U
// #define SENSOR_MOTION           (A0)                        // A0 |     •

/* use sensory button */
#define ESP_USE_BUTTON

// === LED МАТРИЦЯ =====================                       або одна матриця або стрічка типове підключення наприклад матриця (16х16) або стрічка
#define MATRIX_TYPE             (0U)                        // тип матриці: 0 - зигзаг, 1 - параллельна      (D3)
static const uint8_t WIDTH = 24U;                           // ширина матриці
static const uint8_t HEIGHT = 32U;                          // висота матриці
#define ROUND_MATRIX            (35U)                       // кількість led в матриці індикатора стану
#define LIGHT_MATRIX             (3U)                       // кількість led в матриці підсвітки
#define MATRIX_LEVEL           (128U)                       // яскраість led в матриці індикатора стану

/* matrix type end connection   */
#define COLOR_ORDER             (GRB)                       // порядок цветов на ленте. Если цвет отображается некорректно - меняйте. Начать можно с RGB
#define CONNECTION_ANGLE         (0U)                       // угол подключения: 0 - левый нижний, 1 - левый верхний, 2 - правый верхний, 3 - правый нижний
#define STRIP_DIRECTION          (0U)                       // направление ленты из угла: 0 - вправо, 1 - вверх, 2 - влево, 3 - вниз
#define CURRENT_LIMIT         (2000U)                       // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

#define TEXT_HEIGHT             (5U)                        // висота, на якій біжить текст (від низу матриці)
/*
  #define ALARM_PIN               (..U)
  #define ALARM_LEVEL            (HIGH) */
#define MOSFET_PIN              (15U)                     // пин MOSFET транзистора (D8) - может быть использован для управления питанием матрицы/ленты (если раскомментировать строку)
//                                                             в оригинальной прошивке использовался (5U) - (D1) желаемо на ваш выбор
#define MOSFET_LEVEL           (HIGH)                     // логический уровень, в который будет установлен пин MOSFET_PIN, когда матрица включена - HIGH или LOW (если раскомментировать)

#endif /* END JAVELIN */



// === ORG_LAMP ========================
// оригінальна конструкція лампи =======
#ifdef ORG_LAMP
/* ORIGIN | ROUND_LAMP */
// #define ORIGIN
#define ROUND_LAMP

#define IOT_TYPE                (0U)       // введено поняття типу пристрою, потрібно для сумісності з прогамним забезпеченням

#ifdef ORIGIN
/* used pins */
// #define LED_INFO               (16U)                        // D0 | 16U •
#define BACKLIGHT_PIN           (5U)                        // D1 |  5U • 
#define BTN_PIN                 (4U)                        // D2 |  4U • 
#define LED_PIN                 (0U)                        // D3 |  0U •
#define MB_LED_PIN              (2U)                        /* D4 |  2U • blue led на платі */
// #define NA                     (14U)                        // D5 | 14U •
#define OTA_PIN                (12U)                        // D6 | 12U •
#define BTN_JAVELIN_PIN        (13U)                        // D7 | 13U •
// #define NA                     (15U)                        // D8 | 15U
// #define SENSOR_MOTION           (A0)                        // A0 |     •

/* use sensory button */
#define ESP_USE_BUTTON

// === LED МАТРИЦЯ =====================                     або одна матриця або стрічка типове підключення наприклад матриця (16х16) або стрічка
#define MATRIX_TYPE              (0U)                       // тип матриці: 0 - зигзаг, 1 - параллельна      (D3)
#define WIDTH                   (24U)                       // ширина матриці
#define HEIGHT                  (32U)                       // висота матриці
#define ROUND_MATRIX            (35U)                       // кількість led в матриці індикатора стану
#define LIGHT_MATRIX             (3U)                       // кількість led в матриці підсвітки
#define MATRIX_LEVEL           (128U)                       // яскраість led в матриці індикатора стану

/* matrix type end connection   */
#define COLOR_ORDER             (GRB)                       // порядок цветов на ленте. Если цвет отображается некорректно - меняйте. Начать можно с RGB
#define CONNECTION_ANGLE         (0U)                       // угол подключения: 0 - левый нижний, 1 - левый верхний, 2 - правый верхний, 3 - правый нижний
#define STRIP_DIRECTION          (0U)                       // направление ленты из угла: 0 - вправо, 1 - вверх, 2 - влево, 3 - вниз
#define CURRENT_LIMIT         (2000U)                       // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

#define TEXT_HEIGHT             (5U)                        // висота, на якій біжить текст (від низу матриці)
/*
  #define ALARM_PIN               (..U)
  #define ALARM_LEVEL            (HIGH)
  #define MOSFET_PIN              (15U)                       // пин MOSFET транзистора (D8) - может быть использован для управления питанием матрицы/ленты (если раскомментировать строку)
  //                                                             в оригинальной прошивке использовался (5U) - (D1) желаемо на ваш выбор
  #define MOSFET_LEVEL           (HIGH)                      // логический уровень, в который будет установлен пин MOSFET_PIN, когда матрица включена - HIGH или LOW (если раскомментировать)
*/
#endif /* ORIGIN */

// === ROUND_LAMP =====================
// ROUND_LAMP | round lamp 8x8 ======
#ifdef ROUND_LAMP

/* used pins */
// #define LED_INFO               (16U)                        // D0 | 16U •
// #define BACKLIGHT_PIN           (5U)                        // D1 |  5U •
#define BTN_PIN                 (4U)                        // D2 |  4U • 
#define LED_PIN                 (0U)                        // D3 |  0U •
#define MB_LED_PIN              (2U)                        /* D4 |  2U • blue led на платі */
// #define NA                     (14U)                        // D5 | 14U •
// #define OTA_PIN                (12U)                        // D6 | 12U •
// #define BTN_JAVELIN_PIN        (13U)                        // D7 | 13U •
// #define NA                     (15U)                        // D8 | 15U
// #define SENSOR_MOTION           (A0)                        // A0 |     •

/* use sensory button */
#define ESP_USE_BUTTON

// === LED МАТРИЦЯ =====================                      або одна матриця або стрічка типове підключення наприклад матриця (16х16) або стрічка
#define MATRIX_TYPE                (0U)                       // тип матриці: 0 - зигзаг, 1 - параллельна      (D3)
//static const uint8_t WIDTH = 8U;                              // ширина матриці
//static const uint8_t HEIGHT = 8U;                             // висота матриці
#define WIDTH                      (8U)                       // ширина матриці
#define HEIGHT                     (8U)                       // висота матриці
//#define ROUND_MATRIX            (35U)                       // кількість led в матриці індикатора стану
//#define LIGHT_MATRIX             (3U)                       // кількість led в матриці підсвітки
//#define MATRIX_LEVEL           (128U)                       // яскраість led в матриці індикатора стану

/* matrix type end connection   */
#define COLOR_ORDER               (GRB)                       // порядок цветов на ленте. Если цвет отображается некорректно - меняйте. Начать можно с RGB
#define CONNECTION_ANGLE           (3U)                       // угол подключения: 0 - левый нижний, 1 - левый верхний, 2 - правый верхний, 3 - правый нижний
#define STRIP_DIRECTION            (1U)                       // направление ленты из угла: 0 - вправо, 1 - вверх, 2 - влево, 3 - вниз
#define CURRENT_LIMIT           (1000U)                       // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

#define TEXT_HEIGHT                (0U)                        // висота, на якій біжить текст (від низу матриці)
/*
  #define ALARM_PIN               (..U)
  #define ALARM_LEVEL            (HIGH)
  #define MOSFET_PIN              (15U)                       // пин MOSFET транзистора (D8) - может быть использован для управления питанием матрицы/ленты (если раскомментировать строку)
  //                                                             в оригинальной прошивке использовался (5U) - (D1) желаемо на ваш выбор
  #define MOSFET_LEVEL           (HIGH)                      // логический уровень, в который будет установлен пин MOSFET_PIN, когда матрица включена - HIGH или LOW (если раскомментировать)
*/

#endif /* END ROUND_LAMP */
#endif /* END ORG_LAMP*/

// === СЕНСОРНА КНОПКА =================
// Якщо у вас не сенсорна, а звичайна кнопка — змініть значення 1 на 0
#ifdef ESP_USE_BUTTON

#define BUTTON_LOCK_ON_START                                 // Якщо під час увімкнення лампи
// в розетку кнопка натиснута,
// або кнопка не підключена —
// лампа ігнорує натискання кнопки
// до наступного перезапуску

#define BUTTON_IS_SENSORY        (1U)                         // 1 – сенсорна кнопка
// 0 – звичайна кнопка

#define BUTTON_CAN_SET_SLEEP_TIMER                            // Дозволити встановлення таймера сну
// за допомогою кнопки

#ifdef BUTTON_CAN_SET_SLEEP_TIMER
#define BUTTON_SET_SLEEP_TIMER1  (5U)                         // Таймер вимкнення лампи (у хвилинах).
// При одноразовому натисканні кнопки:
// • лампа двічі блимає синім
// • якщо лампа була вимкнена — вона увімкнеться
#endif

#endif


// -------------------------------------
//#define ALARM_PIN             (16U)                       // пин состояния будильника (D0) - может быть использован для управления каким-либо внешним устройством на время работы будильника (если раскомментировать)
//#define ALARM_LEVEL           (HIGH)                      // логический уровень, в который будет установлен пин ALARM_PIN, когда "рассвет"/будильник включен (если раскомментировать)

// === ФАЙЛОВА СИСТЕМА  ================
//#define USE_LittleFS                                      // Раскомментируйте эту строку, если вместо файловой системы SPIFFS хотите использовать файловую систему LittlFS
//                                                             на текущий момент предпочтительно использовать SPIFFS так как вы не будете видеть содержимое папок в LittlFS
//                                                             все остальное будет работать

// === LED СТРІЧКА  =====================
// если у вас не обычная матрица, а сборка из ленты или гирлянда, и возникают непонятной природы артефакты в рисунке эффекта, попробуйте менять следующие три строчки
//#define FASTLED_ALLOW_INTERRUPTS      (1)                 // default: 1; // Use this to force FastLED to allow interrupts in the clockless chipsets (or to force it to disallow), overriding the default on platforms that support this. Set the value to 1 to allow interrupts or 0 to disallow them.
#define FASTLED_INTERRUPT_RETRY_COUNT   (0)                 // default: 2; // Use this to determine how many times FastLED will attempt to re-transmit a frame if interrupted for too long by interrupts
#define FASTLED_ESP8266_RAW_PIN_ORDER                       // FASTLED_ESP8266_RAW_PIN_ORDER, FASTLED_ESP8266_D1_PIN_ORDER or FASTLED_ESP8266_NODEMCU_PIN_ORDER
#define NUM_LEDS                (uint16_t)(WIDTH * HEIGHT+1)
#define SEGMENTS                (1U)                        // диодов в одном "пикселе" (для создания матрицы из кусков ленты). вряд ли эффекты будут корректно работать, если изменить этот параметр

// === ПРОГРАМНЕ ЗАБЕЗПЕЧЕННЯ ===========
#define USE_ROUTER_ONLY                                     // не перемикатись в режим Access Point якщо вже було підключення до роутера при старті (актуально якщо довго стартуе роутер)
#define USE_SOUND_VISUALISER                                // візуалізатор звуку, закоментуйте якщо не використовуєте
#define USE_CPU_USAGE                                       // візуалізатор завантаження процесора PC при використанні додатку CpuUsage  

// === РЕЖИМЫ РАБОТЫ  ===================
uint8_t SLEEP_TIMER             (2U);                       // Час автовимкнення лампи (годин)
#define USE_MULTIPLE_LAMPS_CONTROL                          // Включити код для керування кількома лампами (workgroup)

// === НАЛАШТУВАННЯ ТЕКСТА З ПРОКРУТКОЮ =
// #define LOW_FONT                                            // розмір шрифта 5Х5 якщо закоментувати рядок, тоді 5Х8
#define TEXT_DIRECTION          (1U)                        // 1 - по горизонтали, 0 - по вертикали
#define MIRR_V                  (0U)                        // отразить текст по вертикали (0 / 1)
#define MIRR_H                  (0U)                        // отразить текст по горизонтали (0 / 1)                                   
#define SPACE                   (1U)                        // пробіл
#define LETTER_COLOR            (CRGB::White)               // колір літер за замовчуванням

// #define SHOW_IP_TO_START                                 // если строка не закомментирована, при удачном подключении к роутеру будет выводиться IP лампы



// === ЗОВНІШНЄ КЕРУВАННЯ ==============

#define USE_MQTT              (false)              // true – використовувати MQTT, false – вимкнено
#if USE_MQTT

static const char MqttServer[] PROGMEM = "195.162.64.104"; // Адреса MQTT-брокера
static const uint16_t MqttPort = 8883U;                    // Порт MQTT
#define MQTT_RECONNECT_TIME   (10U)                        // Затримка повторного підключення (с)
/* для індефікації на стороні брокера наприклад щоб програмно ідефікувати пристрій можна бути програмно
   вирахувати по data з топіку IoT/ MqttClientIdPrefix[]  + "/" + chipID "/" наприклад виходячи
   з функціоналу "LedLamp/", "Sensors/", "Remote/", "Gibrid/"  */
static const char MqttClientIdPrefix[] PROGMEM = "LedLamp/"; // Префікс ClientID + ESP.getChipId()
static const char MqttUser[] PROGMEM     = "";              // Користувач MQTT
static const char MqttPassword[] PROGMEM = "";              // Пароль MQTT
#endif


// =====================================
// Developers Constants
// =====================================
// === DEBUG ===========================
// #define GENERAL_DEBUG                          // Увімкнути налагоджувальні повідомлення

#define OTA                                       // Увімкнути OTA-прошивку
#ifdef OTA
#define ESP_OTA_PORT        (8266U)               // Порт OTA
#endif

#define GENERAL_DEBUG_TELNET (false)               // true – лог у Telnet, false – Serial
#define TELNET_PORT          (23U)                // Telnet-порт

#if (defined(GENERAL_DEBUG) && GENERAL_DEBUG_TELNET)
WiFiServer telnetServer(TELNET_PORT);
WiFiClient telnet;
bool telnetGreetingShown = false;
#define LOG                 telnet
#else
#define LOG                 Serial
#endif
//--------------------------------------
