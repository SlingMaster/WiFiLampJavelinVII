/*
   Firmware.ino
  Скетч проєкту "WiFi Lamp Javelin VII & Remote Control • III"
  Автор ідеї та першої реалізації: AlexGyver, 2019 | https://AlexGyver.ru/
  -----------------------------------------------------------
  © Реалізація HTTP-сервера: Alexey Skoromnny (Alvisor)
  © Реалізація Remote Control у прошивці та програмному забезпеченні для всіх платформ: Alex Dovby (SlingMaster)

  ==========================================================================================
  Джерела останньої версії: https://github.com/SlingMaster/WiFiLamp-JavelinVII
  ==========================================================================================
  Розпакуйте вміст архіву у кореневу папку на диску (не на робочий стіл, будь ласка)
  і дійте, як показав Алекс Гайвер у відео: https://youtu.be/771-Okf0dYs?t=525.
  Лампа повинна бути спаяна за відповідною схемою (3 варіанти на вибір):
  • IOT [7] JAVELIN_VII
  • IOT [1] JAVELIN
  • IOT [0] ORG_LAMP

  В архіві є файл "Прочитай мене!!.doc". Його потрібно уважно прочитати.
  Для завантаження файлів з папки data у файлову систему контролера потрібно встановити Uploader:
  https://esp8266-arduinoide.ru/esp8266fs/
  Версію плати у "Менеджері плат" оберіть 2.7.4.
  При першому запуску лампа створить WiFi мережу з ім'ям «WiFi Lamp Javelin», пароль: 31415926.
  Підключившись до мережі «WiFi Lamp Javelin», у браузері відкрийте 192.168.4.1, щоб зайти на веб-сторінку лампи.
  Там можна змінити ім'я лампи (якщо їх кілька у мережі), налаштувати підключення до вашої домашньої WiFi мережі та перезавантажити лампу.
  Всі налаштування прошивки знаходяться у ConstantsUser.h (легко розберетеся) та data/config.json (можна нічого не змінювати, все можна редагувати з веб-сторінки).
  Якщо хочете, щоб лампа відразу підключилася до вашої WiFi мережі, введіть у data/config.json:
      "ssid": "ім'я вашої мережі",
      "password": "пароль вашої мережі",
      "ESP_mode": 1
  Потім збережіть файл та завантажте файлову систему. Лампа підключиться одразу.
  Інші налаштування можна виконати зі сторінки лампи.

  На YouTube каналі «SlingMasterJSC» https://www.youtube.com/user/SlingMasterJSC
  є підбірка відео про конструкцію лампи та програмне забезпечення: два плейлісти Wifi Lamp «JavelinVII» та Arduino Project, рекомендую переглянути.
  =============================================
*/

#define FASTLED_USE_PROGMEM 1 // просим библиотеку FASTLED экономить память контроллера на свои палитры

#include "pgmspace.h"
#include "ConstantsUser.h"
#include "Constants.h"
#include <FastLED.h>
#include "DataVar.h"
#include "eff_utils.h"
#include "json_utils.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include "Types.h"
#include "timerMinim.h"
#include "fonts.h"
#include <TimeLib.h>
#include "TimerManager.h"
#include "EepromManager.h"

#include <ESP8266SSDP.h>
#include <ArduinoJson.h>                //Установить из менеджера библиотек версию 6.X. https://arduinojson.org/
#include <ESP8266HTTPUpdateServer.h>    // Обновление с web страницы

#ifdef ESP_USE_BUTTON
#include <GyverButton.h>
#endif

#ifdef JAVELIN_VII
#include <EncButton.h>
#endif

#include <NTPClient.h>
#include <Timezone.h>


#ifdef OTA
#include "OtaManager.h"
#endif

#if USE_MQTT
#include "MqttManager.h"
#endif

#ifdef USE_LittleFS
#include <LittleFS.h>
#define SPIFFS LittleFS
#endif


#ifdef USE_OLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#endif

#include "data_pgm.h"

// --------- INIT OBJECTS ----------
#ifdef JAVELIN
CRGB leds[NUM_LEDS + ROUND_MATRIX + LIGHT_MATRIX];
#endif

#ifdef JAVELIN_VII
//EncButton enc(ENCODER_CLK, ENCODER_DT, ENCODER_KEY, INPUT);   // энкодер с кнопкой <A, B, KEY>
EncButton enc(ENCODER_CLK, ENCODER_DT, ENCODER_KEY);
CRGB leds[NUM_LEDS];
CRGB leds_info[ROUND_MATRIX];
#endif

#ifdef ORG_LAMP
CRGB leds[NUM_LEDS];
#endif

WiFiUDP Udp;

//WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP, NTP_ADDRESS, 0, NTP_INTERVAL); // объект, запрашивающий время с ntp сервера; в нём смещение часового пояса не используется (перенесено в объект localTimeZone); здесь всегда должно быть время UTC
//TimeChangeRule summerTime  = { SUMMER_TIMEZONE_NAME, SUMMER_WEEK_NUM, SUMMER_WEEKDAY, SUMMER_MONTH, SUMMER_HOUR, 0 };
//TimeChangeRule winterTime  = { WINTER_TIMEZONE_NAME, WINTER_WEEK_NUM, WINTER_WEEKDAY, WINTER_MONTH, WINTER_HOUR, 0 };
//Timezone localTimeZone(summerTime, winterTime);

/* Local Time */
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, 0, NTP_INTERVAL);     // завжди UTC
int32_t timezoneOffset = 0;       // години різниці від UTC, буде заповнено з GetGeolocation()
time_t currentLocalTime;

timerMinim timeTimer(3000);
bool ntpServerAddressResolved = false;
bool timeSynched = false;
uint32_t lastTimePrinted = 0U;
bool selectedSettings = false;

#ifdef ESP_USE_BUTTON
#if (BUTTON_IS_SENSORY == 1U)
GButton touch(BTN_PIN, LOW_PULL, NORM_OPEN);  // для сенсорной кнопки LOW_PULL
#else
GButton touch(BTN_PIN, HIGH_PULL, NORM_OPEN); // для физической (не сенсорной) кнопки HIGH_PULL. ну и кнопку нужно ставить без резистора в разрыв между пинами D2 и GND
#endif
#endif

#ifdef  JAVELIN
GButton touchJavelin(BTN_JAVELIN_PIN, HIGH_PULL, NORM_OPEN); // для фізичної кнопки управління режимами Javelin
#endif

#ifdef OTA
OtaManager otaManager(&showWarning);
OtaPhase OtaManager::OtaFlag = OtaPhase::None;
#endif

#if USE_MQTT
AsyncMqttClient* mqttClient = NULL;
AsyncMqttClient* MqttManager::mqttClient = NULL;
char* MqttManager::mqttServer = NULL;
char* MqttManager::mqttUser = NULL;
char* MqttManager::mqttPassword = NULL;
char* MqttManager::clientId = NULL;
char* MqttManager::lampInputBuffer = NULL;
char* MqttManager::topicInput = NULL;
char* MqttManager::topicOutput = NULL;
bool MqttManager::needToPublish = false;
char MqttManager::mqttBuffer[] = {};
uint32_t MqttManager::mqttLastConnectingAttempt = 0;
SendCurrentDelegate MqttManager::sendCurrentDelegate = NULL;
#endif

ESP8266HTTPUpdateServer httpUpdater;  // Объект для обнавления с web страницы
ESP8266WebServer HTTP (ESP_HTTP_PORT);//ESP8266WebServer HTTP;  // Web интерфейс для устройства
File fsUploadFile;  // Для файловой системы
WiFiClient HTTPclient;


// --- ИНИЦИАЛИЗАЦИЯ ПЕРЕМЕННЫХ -------
#ifdef  JAVELIN_VII
byte menu = 0;
#endif

uint32_t effTimer;

String e_ip;
uint8_t global_br = 50;
bool gb;
uint16_t localPort = ESP_UDP_PORT;
char packetBuffer[MAX_FRAME_BUFER + 1];
char inputBuffer[MAX_UDP_BUFFER_SIZE];
static const uint8_t maxDim = max(WIDTH, HEIGHT);
static int8_t progress = 0;

uint8_t currentMode = 0;
ModeType modes[MODE_AMOUNT];

AlarmType alarms[7];
uint8_t dawnMode;
bool dawnFlag = false;
uint32_t thisTime;
bool manualOff = false;
byte extCtrl = 0U;
uint8_t FPSdelay = DYNAMIC;
bool loadingFlag = true;
byte custom_eff = 0;
byte camouflage = 0;
bool ONflag = false;
uint32_t eepromTimeout;
bool settChanged = false;
bool buttonEnabled = true; // это важное первоначальное значение. нельзя делать false по умолчанию
bool diagnostic = false;

// uint8_t day_night = false;     // если день - true, ночь - false

unsigned char matrixValue[8][16]; //это массив для эффекта Огонь

bool TimerManager::TimerRunning = false;
bool TimerManager::TimerHasFired = false;
uint8_t TimerManager::TimerOption = 1U;
uint64_t TimerManager::TimeToFire = 0ULL;

// cycle effects settings -----------------
uint8_t eff_auto = 0;
uint8_t eff_interval = 30; //DEFAULT_FAVORITES_INTERVAL;
uint8_t eff_valid = 0;
uint8_t eff_rnd = 0;
bool lendLease = false;
byte notifications = 0;
bool motion_control = 0;
char TextTicker [80];
uint8_t espMode;
uint8_t random_on;
uint32_t auto_swap_timer;
uint32_t my_timer;
bool connect = false;
uint32_t lastResolveTryMoment = 0xFFFFFFFFUL;
uint8_t ESP_CONN_TIMEOUT;
uint8_t EFF_FAV;
uint8_t WORKGROUP;
String LAMP_LIST;
bool sleep = false;
byte brigthness;
bool develop = false;


// ======================================
void setup() {
  Serial.begin(115200);
  Serial.println();
  ESP.wdtEnable(WDTO_8S);

#ifdef  JAVELIN_VII
  display.begin(0x3C, true);
  delay(250); ESP.wdtFeed(); yield();
  showLogoDevice();
  delay(2000); ESP.wdtFeed(); yield();
#endif

  initPinIOT();

  LOG.print(F("\nСтарт файлової системи\n"));
  FS_init(); ESP.wdtFeed(); yield();

  initConfigure(); ESP.wdtFeed(); yield();

  // LOG.print(F("Старт SSDP\n\r"));
  SSDP_init(); ESP.wdtFeed(); yield();

  // LOG.print(F("Старт WebServer\n"));
  runServerHTTP(); ESP.wdtFeed(); yield();

#if defined(GENERAL_DEBUG) && GENERAL_DEBUG_TELNET
  telnetServer.begin();
  for (uint8_t i = 0; i < 100; i++) {
    handleTelnetClient();
    delay(100);
    ESP.wdtFeed();
  }
#endif

#ifdef JAVELIN_VII
  attachInterrupt(0, isr, CHANGE);
  attachInterrupt(1, isr, CHANGE);
  enc.setEncISR(true); yield();

  touch.setStepTimeout(BUTTON_STEP_TIMEOUT);
  touch.setClickTimeout(BUTTON_CLICK_TIMEOUT);
  touch.setDebounce(BUTTON_SET_DEBOUNCE);
#endif

#if defined(ESP_USE_BUTTON)
  touch.setStepTimeout(BUTTON_STEP_TIMEOUT);
  touch.setClickTimeout(BUTTON_CLICK_TIMEOUT);
  touch.setDebounce(BUTTON_SET_DEBOUNCE);

#if (BUTTON_IS_SENSORY == 1)
#if ESP_RESET_ON_START
  delay(500); ESP.wdtFeed();
  if (digitalRead(BTN_PIN)) {
    LOG.println(F("Налаштування WiFiManager скинуто"));
    jsonWrite(configSetup, "ssid", "");
    jsonWrite(configSetup, "password", "");
    saveConfig();
  }
  ESP.wdtFeed();
#elif defined(BUTTON_LOCK_ON_START)
  delay(500); ESP.wdtFeed();
  if (digitalRead(BTN_PIN)) buttonEnabled = false;
  ESP.wdtFeed();
#endif
#endif

#if (BUTTON_IS_SENSORY == 0)
#if ESP_RESET_ON_START
  delay(500); ESP.wdtFeed();
  if (!(digitalRead(BTN_PIN))) {
    LOG.println(F("Налаштування WiFiManager скинуто"));
    jsonWrite(configSetup, "ssid", "");
    jsonWrite(configSetup, "password", "");
    saveConfig();
  }
  ESP.wdtFeed();
#elif defined(BUTTON_LOCK_ON_START)
  delay(500); ESP.wdtFeed();
  if (!(digitalRead(BTN_PIN))) buttonEnabled = false;
  ESP.wdtFeed();
#endif
#endif
#endif

#ifdef ORG_LAMP
  FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
#endif

#ifdef JAVELIN
  FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS + ROUND_MATRIX + LIGHT_MATRIX);
  /* механічна кнопка тест Javelin ------------- */
  touchJavelin.setStepTimeout(BUTTON_STEP_TIMEOUT);
  touchJavelin.setClickTimeout(BUTTON_CLICK_TIMEOUT);
  touchJavelin.setDebounce(BUTTON_SET_DEBOUNCE);
#endif

#ifdef JAVELIN_VII
  FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.addLeds<WS2812B, LED_INFO, COLOR_ORDER>(leds_info, ROUND_MATRIX);
#endif

  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clearData(); ESP.wdtFeed(); yield();

  EepromManager::InitEepromSettings(
    modes, alarms, &ONflag, &dawnMode, &currentMode, & (restoreSettings)
  ); ESP.wdtFeed(); yield();

  sendAlarms(inputBuffer); ESP.wdtFeed(); yield();

  String configAlarm = readFile("alarm_config.json", 1024); ESP.wdtFeed(); yield();
  DAWN_TIMEOUT = jsonReadtoInt(configAlarm, "after");
  LOG.println(configAlarm);
  LOG.println("DAWN_TIMEOUT | afer : " + String(DAWN_TIMEOUT));
  configAlarm = "";

#ifdef JAVELIN_VII
  display.clearDisplay();
  showVersion(); ESP.wdtFeed(); yield();
  display.display(); ESP.wdtFeed(); yield();
  delay(3000); ESP.wdtFeed(); yield();
#endif

  LOG.printf_P(PSTR("Робочий режим лампи: ESP_MODE = %d\n"), espMode);
  WiFi.persistent(false);

  if (espMode == 0U) {
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    if (sizeof(AP_STATIC_IP)) {
      WiFi.softAPConfig(
        IPAddress(AP_STATIC_IP[0], AP_STATIC_IP[1], AP_STATIC_IP[2], AP_STATIC_IP[3]),
        IPAddress(AP_STATIC_IP[0], AP_STATIC_IP[1], AP_STATIC_IP[2], 1),
        IPAddress(255, 255, 255, 0)
      );
    }
    WiFi.softAP(AP_NAME, AP_PASS); ESP.wdtFeed(); yield();
    LOG.print(F("Старт WiFi в режимі точки доступу\n"));
    LOG.print(F("IP адрес: "));
    LOG.println(WiFi.softAPIP()); ESP.wdtFeed();

    //#ifdef GENERAL_DEBUG
    byte mac[6];
    WiFi.macAddress(mac);
    LOG.println("mac : " + String(mac[0], HEX) + ":" + String(mac[1], HEX) + ":" + String(mac[2], HEX) + ":" + String(mac[3], HEX) + ":" + String(mac[4], HEX) + ":" + String(mac[5], HEX));
    LOG.println("*******************************************");
    LOG.printf ("AP Mode | Heap Size after connection %d bytes\n\r", system_get_free_heap_size());
    LOG.println("*******************************************");
    LOG.println("     Version • " + VERSION + " effects");
    //#endif

#ifdef ORG_LAMP
    espModeState(); ESP.wdtFeed(); yield();
#endif

#if defined(JAVELIN_VII) || defined(JAVELIN)
    javelinConnect(); ESP.wdtFeed(); yield();
#endif

#ifdef USE_OLED
    outIP(WiFi.softAPIP()); ESP.wdtFeed(); yield();
#endif

    connect = true;
  } else {
    LOG.print("Старт WiFi в режимі клієнта | підключення до роутера ");
    WiFi.mode(WIFI_STA);
    String _ssid = jsonRead(configSetup, "ssid");
    String _password = jsonRead(configSetup, "password");

    if (_ssid == "" && _password == "") {
      espMode = 0;
      jsonWrite(configSetup, "ESP_mode", (int)espMode);
      saveConfig();
      ESP.restart();
    } else {
      WiFi.begin(_ssid.c_str(), _password.c_str());
      LOG.println(_ssid);
      unsigned long start = millis();
      while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
        ESP.wdtFeed();
        yield();
      }
    }

#ifdef ORG_LAMP
    espModeState(); ESP.wdtFeed(); yield();
#else
    javelinConnect(); ESP.wdtFeed(); yield();
#endif
    // ??? }

    ESP.wdtFeed();
    /* connect ok */
    // Інакше вдалося підключитися надсилаємо повідомлення
    // про підключення та виводимо адресу IP

    if (WiFi.status() == WL_CONNECTED) {
      LOG.printf("\n\rПідключення до роутера встановлено | status • %d\n\r", WiFi.status());
      LOG.print("IP адрес: ");
      LOG.println(WiFi.localIP());
    } else {
      LOG.printf("\n\rNot Connected | Wi-Fi Status • %d\n\r", WiFi.status());
    }
    LOG.printf_P(PSTR("Порт UDP сервера: %u\n\r"), localPort);
    Udp.begin(localPort); ESP.wdtFeed(); yield();

    //#ifdef GENERAL_DEBUG
    String chipID = String(ESP.getChipId(), HEX);
    chipID.toUpperCase();
    LOG.println ("ChipId • " + chipID + " | " + String(ESP.getChipId(), DEC) );
    LOG.println ("***********************************************");
    LOG.printf ("Heap Size after connection %d bytes\n\r", system_get_free_heap_size());
    LOG.println ("***********************************************");
    LOG.println("     Version • " + VERSION + " effects");
    //#endif
    timeClient.begin(); ESP.wdtFeed(); yield();

  }

  long rssi = WiFi.RSSI();
  LOG.print(F("Рівень сигналу мережі RSSI = "));
  LOG.print(rssi);
  LOG.println(F(" dbm"));

#if (USE_MQTT)
  if (espMode == 1U) {
    mqttClient = new AsyncMqttClient();
    MqttManager::setupMqtt(mqttClient, inputBuffer, &sendCurrent); ESP.wdtFeed(); yield();
  }
#endif

  memset(matrixValue, 0, sizeof(matrixValue)); ESP.wdtFeed(); yield();
  randomSeed(micros());
  CompareVersion();
  loadingFlag = true;
  delay(100); ESP.wdtFeed(); yield();
  my_timer = millis();
  auto_swap_timer = millis();
  setFPS(); ESP.wdtFeed(); yield();
}

// ========================================
void loop() {
  static uint32_t lastHTTP = 0;
  static uint32_t lastEffectSwap = 0;

  // WIFI connection management
  if (espMode) {
    /* перевірка підключення по wi-fi до роутера */
    checkWiFiConnection();
  }

  if (extCtrl == 0U) {
    // 1. Керування лампою
    controlActionTick();

    // 2. Ефекти
    effectsTick();
    yield();  // даємо час WiFi / OTA

    // Auto swap effects
    if ((millis() - lastEffectSwap) >= eff_interval * 1000UL) {
      lastEffectSwap = millis();
      autoSwapEff();
    }
  }
  if (connect) {
    // 3. HTTP обробка (низький пріоритет, кожні 10 мс)
    if ((millis() - lastHTTP) >= 10UL) {
      HTTP.handleClient();
      lastHTTP = millis();
    }
  }
  // EEPROM
  EepromManager::HandleEepromTick(&settChanged, &eepromTimeout, &ONflag, &currentMode, modes);
  // EepromManager::InitEepromSettings(modes, alarms, &ONflag, &dawnMode, &currentMode, restoreDefaultSettings);

  if (!connect) return;

  // UDP parsing
  parseUDP();

  // Time / Timer
  timeTick();
  TimerManager::HandleTimer(&ONflag, &settChanged, &eepromTimeout, &changePower);

#ifdef OTA
  otaManager.HandleOtaUpdate();
  yield();
#endif

#if USE_MQTT
  if (espMode == 1U && mqttClient && WiFi.isConnected() && !mqttClient->connected()) {
    MqttManager::mqttConnect();
    MqttManager::needToPublish = true;
  }

  if (MqttManager::needToPublish && strlen(inputBuffer) > 0) {
    commandDecode(inputBuffer, MqttManager::mqttBuffer, true);
    MqttManager::publishState();
  }
#endif


#if defined(GENERAL_DEBUG) && GENERAL_DEBUG_TELNET
  handleTelnetClient();
#endif

  ESP.wdtFeed();
}

// ========================================
void initConfigure() {
  configSetup = readFile("config.json", 2048);

  // JSC-LED тест помилки підключення
  // jsonWrite(configSetup, "ssid", "JSC-LED");

#if defined(GENERAL_DEBUG)
  LOG.print(F("Конфігурація зчитана\n\r"));
  LOG.println("configSetup : " + configSetup);
#endif

  // Ініціалізація змінних з config.json
  LAMP_NAME = jsonRead(configSetup, "SSDP");
  AP_NAME = jsonRead(configSetup, "ssidAP");
  AP_PASS = jsonRead(configSetup, "passwordAP");
  random_on = jsonReadtoInt(configSetup, "random_on");
  espMode = jsonReadtoInt(configSetup, "ESP_mode");
  custom_eff = jsonReadtoInt(configSetup, "custom_eff");
  camouflage = jsonReadtoInt(configSetup, "camouflage");
  notifications = jsonReadtoInt(configSetup, "notifications");
  motion_control = jsonReadtoInt(configSetup, "motion_control");
  gb = (jsonReadtoInt(configSetup, "gb") == 1);
  global_br = jsonReadtoInt(configSetup, "global_br");
  timezoneOffset = jsonReadtoInt(configSetup, "timezoneOffset");

  if (jsonReadtoInt(configSetup, "fav_effect") >= MODE_AMOUNT) {
    jsonWrite(configSetup, "fav_effect", EFF_MATRIX);
  } else {
    EFF_FAV = jsonReadtoInt(configSetup, "fav_effect");
  }

  eff_auto = jsonReadtoInt(configSetup, "eff_auto");
  eff_interval = jsonReadtoInt(configSetup, "eff_interval");
  eff_rnd = jsonReadtoInt(configSetup, "eff_rnd");
  eff_valid = jsonReadtoInt(configSetup, "eff_valid");
  WORKGROUP = jsonReadtoInt(configSetup, "workgroup");
  LAMP_LIST = jsonRead(configSetup, "lamp_list");

  buttonEnabled = jsonReadtoInt(configSetup, "button_on");
  ESP_CONN_TIMEOUT = 11; // jsonReadtoInt(configSetup, "TimeOut");
  (jsonRead(configSetup, "run_text")).toCharArray(TextTicker, (jsonRead(configSetup, "run_text")).length() + 1);
  DONT_TURN_ON_AFTER_SHUTDOWN = jsonReadtoInt(configSetup, "effect_always");
  AUTOMATIC_OFF_TIME = (SLEEP_TIMER * 60UL * 60UL * 1000UL) * (uint32_t)(jsonReadtoInt(configSetup, "timer5h"));
  jsonRead(configSetup, "ntp").toCharArray(NTP_ADDRESS, (jsonRead(configSetup, "ntp")).length() + 1);
}
