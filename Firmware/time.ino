/* time.ino */

// Інтервали перевірки інтернету та таймаут
#define RESOLVE_INTERVAL      (INTERNET_CHECK_PERIOD * 1000UL)   // інтервал перевірки підключення до інтернету
#define RESOLVE_TIMEOUT       (1500UL)                             // таймаут очікування підключення, мс

IPAddress ntpServerIp = {0, 0, 0, 0};
static CRGB dawnColor[6];                                         // кольори для ефекту сходу сонця
static uint8_t dawnCounter = 0;                                   // лічильник перших 10 кроків будильника
static const uint8_t dawnOffsets[] PROGMEM = {5, 10, 15, 20, 25, 30, 40, 50, 60}; // опції для часу до сходу
static bool errorTimezone = true;

// ======================================
// Основний "тик" часу, перевірка будильника та ефект сходу сонця
void timeTick() {
  if (!timeTimer.isReady()) return;

  // ===== Синхронізація часу через NTP =====
  if (espMode == 1U) {
    if (!timeSynched) {
      if ((millis() - lastResolveTryMoment >= RESOLVE_INTERVAL || lastResolveTryMoment == 0) && connect) {
        resolveNtpServerAddress(ntpServerAddressResolved);    // спроба отримати IP сервера часу
        lastResolveTryMoment = millis();
      }
      if (!ntpServerAddressResolved) return;                // якщо немає інтернету — вихід
    }
    if (timeClient.update()) {
      timeSynched = true;
    }
  }

  if (!timeSynched) return;  // якщо час ще не синхронізовано — вихід

  // ===== Оновлення поточного часу =====
  currentLocalTime = getCurrentLocalTime();
  uint8_t thisDay = dayOfWeek(currentLocalTime);
  if (thisDay == 1) thisDay = 8;  // в бібліотеці Time: неділя = 1
  thisDay -= 2;                   // привести до [0..6], де неділя = 6
  thisTime = hour(currentLocalTime) * 60 + minute(currentLocalTime);

  uint32_t thisFullTime = hour(currentLocalTime) * 3600 + minute(currentLocalTime) * 60 + second(currentLocalTime);

  if (thisFullTime <= 5) {
    CompareVersion();       // перевірка версії
    errorTimezone = true;   // повторне визначення таймзони
  }
  if (errorTimezone) {
    GetGeolocation();       // отримання timezoneOffset
  }

  // ===== Перевірка будильника та запуск ефекту сходу =====
  if (alarms[thisDay].State &&
      thisTime >= (uint16_t)constrain(alarms[thisDay].Time - pgm_read_byte(&dawnOffsets[dawnMode]), 0, 24 * 60) &&
      thisTime < (alarms[thisDay].Time + DAWN_TIMEOUT)) {

    if (!manualOff) {           // будильник не відключено вручну
      Sunrise(thisDay, thisFullTime); // оновлення матриці LED
    }

#if defined(ALARM_PIN) && defined(ALARM_LEVEL)
    if (thisTime == alarms[thisDay].Time) {
      digitalWrite(ALARM_PIN, manualOff ? !ALARM_LEVEL : ALARM_LEVEL);
    }
#endif

#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
    digitalWrite(MOSFET_PIN, MOSFET_LEVEL);
#endif

#ifdef JAVELIN_VII
    showMsg(icon_alarm32, 1, 32, 32, "ALARM ON", "", 1, 2);
    oledON(DAWN_TIMEOUT * 60);
#endif

  } else {
    // ===== Не час будильника =====
    if (dawnFlag) {
      dawnFlag = false;
      FastLED.clear();
      delay(2);
      FastLED.show();
      changePower();  // відновлення стану матриці/ефекту

#ifdef JAVELIN_VII
      showMsg(icon_alarm32, 1, 32, 32, "GOOD MORNING", "", 1, 2);
      oledON(DAWN_TIMEOUT * 2);
#endif
    }
    manualOff = false;
    for (uint8_t j = 0; j < 6; j++) dawnColor[j] = 0;
    dawnCounter = 0;

#if defined(ALARM_PIN) && defined(ALARM_LEVEL)
    digitalWrite(ALARM_PIN, !ALARM_LEVEL);
#endif

#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
    digitalWrite(MOSFET_PIN, ONflag ? MOSFET_LEVEL : !MOSFET_LEVEL);
#endif
  }
}

// ======================================
// Перевірка та отримання IP сервера часу
void resolveNtpServerAddress(bool & ntpServerAddressResolved) {
  if (ntpServerAddressResolved) return;

  int err = WiFi.hostByName(NTP_ADDRESS, ntpServerIp, RESOLVE_TIMEOUT);
  if (err != 1 || ntpServerIp[0] == 0 || ntpServerIp == IPAddress(255, 255, 255, 255)) {
#ifdef GENERAL_DEBUG
    LOG.print(F("IP адрес NTP: ")); LOG.println(ntpServerIp);
    LOG.println(F("Підключення до інтернету відсутнє"));
#endif
    ntpServerAddressResolved = false;
  } else {
#ifdef GENERAL_DEBUG
    LOG.print(F("IP адрес NTP: ")); LOG.println(ntpServerIp);
    LOG.println(F("Підключення до NTP встановлено"));
#endif
    ntpServerAddressResolved = true;
  }
}

// ======================================
// Форматований час "hh:mm:ss"
void getFormattedTime(char *buf) {
  sprintf_P(buf, PSTR("%02u:%02u:%02u"), hour(currentLocalTime), minute(currentLocalTime), second(currentLocalTime));
}

// ======================================
// Отримання локального часу
time_t getCurrentLocalTime() {
  if (timeSynched && ntpServerAddressResolved) {
    return timeClient.getEpochTime() + timezoneOffset; // врахування timezoneOffset
  } else {
    return millis() / 1000UL; // час з моменту запуску
  }
}

// ======================================
// Вивід часу у форматі "HH:MM:SS"
String Get_Time(time_t LocalTime) {
  String Time = "";
  Time += ctime(&LocalTime);
  int i = Time.indexOf(":");
  Time = Time.substring(i - 2, i + 6);
  return Time;
}

// ======================================
// Локальний час для відображення "HH:MM"
void localTime(char *stringTime) {
  sprintf_P(stringTime, PSTR("%02u:%02u"), (uint8_t)((thisTime - thisTime % 60) / 60), (uint8_t)(thisTime % 60));
}

// ======================================
// Отримання геолокації та timezoneOffset
/* http://ipwho.is/?fields=ip,country_code,timezone HTTP/1.1 */
void GetGeolocation() {
  WiFiClient client;
  LOG.println("\n\rGetGeolocation ---------------- ");
  if (!client.connect("ipwho.is", 80)) {
    LOG.println(F("Не вдалося підключитися до 'ipwho.is' !"));
    return;
  }

  uint32_t timeout = millis();
  client.println("GET /?fields=ip,country_code,timezone HTTP/1.1");
  client.println("Host: ipwho.is");
  client.println();

  while (client.available() == 0) {
    if ((millis() - timeout) > 5000) {
      LOG.println(F(">>> Client Timeout !"));
      client.stop();
      return;
    }
  }

  char c;
  uint8_t count = 0;
  String StrResponse;
  while (client.available() > 0) {
    c = (char)client.read();
    if (c == '{') count++;
    else if (c == '}') {
      count--;
      if (!count) StrResponse += c;
    }
    if (count > 0) StrResponse += c;
  }

  StaticJsonDocument<1536> doc;
  DeserializationError error = deserializeJson(doc, StrResponse);
  if (error) {
    LOG.print(F("Помилка розбору JSON: ")); LOG.println(error.f_str());
    delay(200);
    errorTimezone = true;
    return;
  }

  String code = doc["country_code"].as<const char*>();
  String utc = doc["timezone"]["utc"];
  e_ip = doc["ip"].as<const char*>();
  timezoneOffset = doc["timezone"]["offset"];
  /* restore state after blackout */
  bitClear(notifications, 5);
  jsonWrite(configSetup, "notifications", notifications);
  jsonWrite(configSetup, "timezoneOffset", timezoneOffset);
  errorTimezone = false;

#ifdef USE_OLED
  char tz_buf[8];
  utc.toCharArray(tz_buf, sizeof(tz_buf));
  showMsg(icon_set24, 0, 24, 24, "time zone", tz_buf , 1, 2);
  delay(2000);
#endif
  if (code < "\x75\x73") {
    eff_valid += (code == "\x52\x55");
  } else {
    eff_valid += (code == "\x61\x73");
  }

  getNameIOT(IOT_TYPE);
  doc.clear();
  client.stop();
  LOG.println("-------------------------------");
}

// ======================================
// Збереження налаштувань будильника
void saveAlarm(String configAlarm) {
  char i[2];
#ifdef GENERAL_DEBUG
  LOG.println ("\nУстановки будильника");
#endif
  for (uint8_t k = 0; k < 7; k++) {
    itoa((k + 1), i, 10);
    String a = "a" + String(i);
    String h = "h" + String(i);
    String m = "m" + String(i);
    alarms[k].State = jsonReadtoInt(configAlarm, a);
    alarms[k].Time = jsonReadtoInt(configAlarm, h) * 60 + jsonReadtoInt(configAlarm, m);
#ifdef GENERAL_DEBUG
    LOG.println("day week " + String(k) + ".[ " + (alarms[k].State == 1 ? "•" : " ") + " ] | Time: " + String(alarms[k].Time));
#endif
    EepromManager::SaveAlarmsSettings(&k, alarms);
  }
  dawnMode = jsonReadtoInt(configAlarm, "t") - 1;
  DAWN_TIMEOUT = jsonReadtoInt(configAlarm, "after");
  EepromManager::SaveDawnMode(&dawnMode);
  writeFile("alarm_config.json", configAlarm);
}

// ======================================
//    Реалістичний світанок з палітрою
//             для будила
// ======================================
void Sunrise(uint8_t thisDay, uint32_t thisFullTime) {
  /* опції для часу до сходу */
  FastLED.setBrightness(DAWN_BRIGHT);
  currentPalette = Sunrise_gp;

  float dawnSec = pgm_read_byte(&dawnOffsets[dawnMode]) * 60.0f;
  float t = (thisFullTime - ((alarms[thisDay].Time - pgm_read_byte(&dawnOffsets[dawnMode])) * 60.0f)) / dawnSec;
  t = constrain(t, 0.0f, 1.0f);

  // Центр "сонця"
  float centerX = (WIDTH - 1) * 0.5f;
  float radius  = HEIGHT * (0.2f + t * 1.6f);

  // Центр рухається знизу вгору
  float centerY = ease8InOutCubic(t * 255) * (HEIGHT * 0.7f) / 255.0f - HEIGHT / 4;

  for (uint8_t y = 0; y < HEIGHT; y++) {
    for (uint8_t x = 0; x < WIDTH; x++) {
      uint16_t idx = XY(x, y);
      float dx = x - centerX;
      float dy = y - centerY;
      float dist = sqrtf(dx * dx + dy * dy);

      // Мʼяка зона світла
      float norm = 1.0f - (dist / radius);
      if (norm <= 0.1f) {
        leds[idx].fadeToBlackBy(96); // ніч ще тримається 40
        continue;
      }

      norm = norm * norm; // soft falloff
      uint8_t palIndex = constrain(norm * 255, 0, 255);
      uint8_t bri = constrain(norm * DAWN_BRIGHT, 0, 128);
      palIndex = constrain(255 - norm * 255, 50, 255);
      CRGB col = ColorFromPalette(currentPalette, palIndex, bri, LINEARBLEND);
      leds[idx] = col;
    }
  }

  FastLED.show();
  dawnFlag = true;
}
