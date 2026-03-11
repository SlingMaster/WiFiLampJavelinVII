/* utility.ino */

/************* НАЛАШТУВАННЯ МАТРИЦІ *************/
#if (CONNECTION_ANGLE == 0 && STRIP_DIRECTION == 0)
#define _WIDTH WIDTH
#define THIS_X x
#define THIS_Y y

#elif (CONNECTION_ANGLE == 0 && STRIP_DIRECTION == 1)
#define _WIDTH HEIGHT
#define THIS_X y
#define THIS_Y x

#elif (CONNECTION_ANGLE == 1 && STRIP_DIRECTION == 0)
#define _WIDTH WIDTH
#define THIS_X x
#define THIS_Y (HEIGHT - y - 1)

#elif (CONNECTION_ANGLE == 1 && STRIP_DIRECTION == 3)
#define _WIDTH HEIGHT
#define THIS_X (HEIGHT - y - 1)
#define THIS_Y x

#elif (CONNECTION_ANGLE == 2 && STRIP_DIRECTION == 2)
#define _WIDTH WIDTH
#define THIS_X (WIDTH - x - 1)
#define THIS_Y (HEIGHT - y - 1)

#elif (CONNECTION_ANGLE == 2 && STRIP_DIRECTION == 3)
#define _WIDTH HEIGHT
#define THIS_X (HEIGHT - y - 1)
#define THIS_Y (WIDTH - x - 1)

#elif (CONNECTION_ANGLE == 3 && STRIP_DIRECTION == 2)
#define _WIDTH WIDTH
#define THIS_X (WIDTH - x - 1)
#define THIS_Y y

#elif (CONNECTION_ANGLE == 3 && STRIP_DIRECTION == 1)
#define _WIDTH HEIGHT
#define THIS_X y
#define THIS_Y (WIDTH - x - 1)

#else
!!!!!!!!!!!!!!!!!!!!!!!!!!!   смотрите инструкцию: https://alexgyver.ru/wp-content/uploads/2018/11/scheme3.jpg
!!!!!!!!!!!!!!!!!!!!!!!!!!!   такого сочетания CONNECTION_ANGLE и STRIP_DIRECTION не бывает
#define _WIDTH WIDTH
#define THIS_X x
#define THIS_Y y
#pragma message "Wrong matrix parameters! Set to default"
#endif



#if defined(JAVELIN_VII) || defined(JAVELIN)
// =====================================
inline uint16_t CompositMatrix(uint8_t x, uint8_t y) {
  /* matrix 8x32 ---------- */
  const uint8_t panel = x >> 3;     // 0..2
  const uint8_t lx = x & 0x07;      // локальний X 0..7

  uint16_t base = panel * 256;      // 0 / 256 / 512

  if (!(y & 0x01)) {
    return base + y * 8 + lx;
  } else {
    return base + y * 8 + (7 - lx);
  }
}


//inline uint16_t CompositMatrixOLD(uint8_t x, uint8_t y) {
//
//  // 1nd matrix 8x32 -----
//  if (x < 8) {
//    if (y % 2 == 0) {   // even rows
//      return  y * 8 + x;
//    } else {            // odd rows
//      // return  y * 8 + 8 - x - 1;
//      return  y * 8 + 7 - x;
//    }
//  }
//
//  // 2st matrix 8x32 -----
//  if ((x >= 8) && (x < 16)) {
//    if (y % 2 == 0) {   // even rows
//      // return 256 + y * 8 + x - 8 + ;
//      return 248 + y * 8 + x;
//    } else {            // odd rows
//      // return 256 + y * 8 + 8 - (x - 8) - 1;
//      return 263 + y * 8 - (x - 8);
//    }
//  }
//
//  // 3st matrix 8x32 -----
//  if (x >= 16) {
//    if (y % 2 == 0) {   // even rows
//      // return 512 + y * 8 + x - 16;
//      return 496 + y * 8 + x;
//    } else {            // odd rows
//      // return 512 + y * 8 + 8 - (x - 16) - 1;
//      return 519 + y * 8 - (x - 16);
//    }
//  }
//}

#endif

#if defined(JAVELIN_VII)
// ====================================
inline uint16_t defMatrix(uint8_t THIS_X, uint8_t THIS_Y) {
  /* одна матриця або стрічка ------- */
  //  if (!(THIS_Y & 1) || MATRIX_TYPE) {
  //    return (THIS_Y * _WIDTH + THIS_X);                // Even rows run forwards
  //  } else {
  //    return (THIS_Y * _WIDTH + _WIDTH - THIS_X - 1);   // Odd rows run backwards
  //  }

  bool forward = !(THIS_Y & 1) || MATRIX_TYPE;
  //               Even rows run forwards   | Odd rows run backwards
  return forward ? THIS_Y * _WIDTH + THIS_X : THIS_Y * _WIDTH + (_WIDTH - THIS_X - 1);
}
#endif

#ifdef USE_ROBOT
// =====================================
inline uint16_t customMatrix(uint8_t x, uint8_t y) {
  // uint8_t XYTable[] = {
  const uint8_t PROGMEM XYTable[] = {
    0,  27,  28,  59,  60,  91,  92, 121, 122,
    1,  26,  29,  58,  61,  90,  93, 120, 123,
    2,  25,  30,  57,  62,  89,  94, 119, 124,
    3,  24,  31,  56,  63,  88,  95, 118, 125,
    4,  23,  32,  55,  64,  87,  96, 117, 126,
    5,  22,  33,  54,  65,  86,  97, 116, 127,
    6,  21,  34,  53,  66,  85,  98, 115, 128,
    7,  20,  35,  52,  67,  84,  99, 114, 129,
    8,  19,  36,  51,  68,  83, 100, 113, 130,
    9,  18,  37,  50,  69,  82, 101, 112, 131,
    10,  17,  38,  49,  70,  81, 102, 111, 132,
    11,  16,  39,  48,  71,  80, 103, 110, 133,
    12,  15,  40,  47,  72,  79, 104, 109, 134,
    13,  14,  41,  46,  73,  78, 105, 108, 135,
    136, 139,  42,  45,  74,  77, 106, 141, 142,
    137, 138,  43,  44,  75,  76, 107, 140, 143
  };


  // якщо у вас матриця незвичайної форми з проміжками/вирізами або просто маленька, тоді вам доведеться переписати функцію XY() під свої потреби
  // масив для перенаправлення можна сформувати за допомогою цього онлайн-сервісу: https://macetech.github.io/FastLED-XY-Map-Generator/
  /* Custom Matrix
     -  -  Х  Х  Х  Х  Х  Х  Х  Х  Х  -  -
     -  -  Х  Х  Х  Х  Х  Х  Х  Х  Х  -  -
     Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х
     Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х
     Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х
     Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х
     Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х
     Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х
     Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х
     Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х
     Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х
     Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х
     Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х
     Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х
     Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х
     Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х
  */

  if ( (x >= WIDTH) || (y >= HEIGHT) ) {
    // return (LAST_VISIBLE_LED + 1);
    return 136;
  }

  uint8_t i = (y *  WIDTH) + x;

  // uint8_t j = XYTable[i];
  uint8_t j = pgm_read_byte(&XYTable[i]);

  return j + 9;

}
#endif

// =====================================
/* получить номер пикселя в ленте по координатам
  библиотека FastLED тоже использует эту функцию */
uint16_t XY(uint8_t x, uint8_t y) {
#ifdef JAVELIN_VII
  /* складова матриця із трьох 8х32 та 2 стрічки по 24 led в рядку -- */
  /* обов'язково нижне ліве підключення стрічка перша зигзагом по 24 led в рядку */
  if (y > 1) {
    return  CompositMatrix(x, y - MATRIX_OFFSET) + 48;
  } else {
    return defMatrix(x, y);
  }
#endif

#ifdef JAVELIN
  /* складова матриця із двох або трьох 8х32 -- */
  return CompositMatrix(x, y);
#endif

#ifdef USE_ROBOT
  return customMatrix(x, y);
#endif

#ifdef ORG_LAMP
  /* одна матриця або стрічка ------- */
  return defMatrix(x, y);
#endif
  // return 0;
}



// ====================================
/* оставлено для совместимости со эффектами из старых прошивок */
uint16_t getPixelNumber(uint8_t x, uint8_t y) {
  return XY(x, y);
}

// =====================================
// restore settings
// =====================================
/* восстановление настроек эффектов на настройки по умолчанию */
void restoreSettings() {
  for (uint8_t i = 0; i < MODE_AMOUNT; i++) {
    setDefState(true, i);
    //EepromManager::SaveModesSettings(&currentMode, modes);
    // EepromManager::SaveModesSettings(&i, modes);

#ifdef GENERAL_DEBUG
    if (i % 10U == 0U) {
      LOG.println ("               • [ # ] | BRI | SPD | SCL |" );
    }
    LOG.printf_P(PSTR("Restore Settings [%03d] | %03d | %03d | %03d | \n\r"), i, modes[i].Brightness, modes[i].Speed, modes[i].Scale);
#endif
  }
}

// ======================================
void updateSets() {
  loadingFlag = true;
  settChanged = true;
  // modes[currentMode].changed = true;
  setFPS();
  eepromTimeout = millis();

#if (USE_MQTT)
  if (espMode == 1U) {
    MqttManager::needToPublish = true;
  }
#endif
}

// ======================================
void sendAlarms(char *outputBuffer) {
  strcpy_P(outputBuffer, PSTR("ALMS"));

  for (byte i = 0; i < 7; i++) {
    sprintf_P(outputBuffer, PSTR("%s %u"), outputBuffer, (uint8_t)alarms[i].State);
  }

  for (byte i = 0; i < 7; i++) {
    sprintf_P(outputBuffer, PSTR("%s %u"), outputBuffer, alarms[i].Time);
  }
  sprintf_P(outputBuffer, PSTR("%s %u"), outputBuffer, dawnMode + 1);
}

// =====================================
//           Code by © Stepko
CRGB rgb332ToCRGB(byte value) { // Tnx to Stepko
  CRGB color;
  color.r = value & 0xe0; // mask out the 3 bits of red at the start of the byte
  color.r |= (color.r >> 3); // extend limited 0-224 range to 0-252
  color.r |= (color.r >> 3); // extend limited 0-252 range to 0-255
  color.g = value & 0x1c; // mask out the 3 bits of green in the middle of the byte
  color.g |= (color.g << 3) | (color.r >> 3); // extend limited 0-34 range to 0-255
  color.b = value & 0x03; // mask out the 2 bits of blue at the end of the byte
  color.b |= color.b << 2; // extend 0-3 range to 0-15
  color.b |= color.b << 4; // extend 0-15 range to 0-255
  return color;
}

// =====================================
void CompareVersion() {
  // if (notifications > 5) {
  // notifications -= 8;
  if (bitRead(notifications, 4)) {
    bitClear(notifications, 4);
    jsonWrite(configSetup, "notifications", notifications);
    saveConfig();
    ResetDefaultEffects(false);
  }
  // if (notifications > 0) {
  if (bitRead(notifications, 1)) {
    // https://arduinogetstarted.com/tutorials/arduino-http-request
    if (!HTTPclient.connect("winecard.ltd.ua", 80)) {
#ifdef GENERAL_DEBUG
      Serial.println(F("Connection failed"));
#endif
      return;
    }
    Serial.println(" • Connected to server");

    // Send HTTP request
    HTTPclient.println(F("GET /dev/WifiLampRemote3/version.json HTTP/1.0"));
    HTTPclient.println(F("Host: winecard.ltd.ua"));
    HTTPclient.println(F("Connection: close"));
    if (HTTPclient.println() == 0) {
#ifdef GENERAL_DEBUG
      Serial.println(F("Failed to send request"));
#endif
      HTTPclient.stop();
      return;
    }

    // Check HTTP status
    if (deltaHue > 200U) {
      char status[32] = {0};
      HTTPclient.readBytesUntil('\r', status, sizeof(status));
      // It should be "HTTP/1.0 200 OK" or "HTTP/1.1 200 OK"
      if (strcmp(status + 9, "200 OK") != 0) {
#ifdef GENERAL_DEBUG
        Serial.print(F("Unexpected response: "));
        Serial.println(status);
#endif
        HTTPclient.stop();
        return;
      }
    }

    // Skip HTTP headers ----
    char endOfHeaders[] = "\r\n\r\n";
    if (!HTTPclient.find(endOfHeaders)) {
#ifdef GENERAL_DEBUG
      Serial.println(F("Invalid response"));
#endif
      HTTPclient.stop();
      return;
    }

    // Allocate the JSON document
    // Use https://arduinojson.org/v6/assistant to compute the capacity.
    const size_t capacity = 256;
    DynamicJsonDocument doc(capacity);

    // Parse JSON object ----
    DeserializationError error = deserializeJson(doc, HTTPclient);
    if (error) {
#ifdef GENERAL_DEBUG
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
#endif
      HTTPclient.stop();
      return;
    }

    // Extract values -----
    String latestVer = doc["ver"].as<const char*>();
    if (latestVer > VERSION) {
      currentMode = MODE_AMOUNT - 1;
      printMSG("New Firmware Released " + latestVer, false);
    }
#ifdef GENERAL_DEBUG
    LOG.print("New Firmware Released • ");
    LOG.print(latestVer);
    LOG.print(" | Current • ");
    LOG.println(VERSION);
#endif
    doc.clear();
    // Disconnect -------
    HTTPclient.stop();
  }
}

// ======================================
String getNameIOT(byte idx) {
  static const uint8_t id[8][23] PROGMEM = {
    {0x57, 0x69, 0x46, 0x69, 0x20, 0x4c, 0x61, 0x6d, 0x70, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00},
    {0x57, 0x69, 0x46, 0x69, 0x20, 0x4c, 0x61, 0x6d, 0x70, 0x20, 0x4a, 0x61, 0x76, 0x65, 0x6c, 0x69, 0x6e, 0x00, 0x20, 0x20, 0x20, 0x20, 0x21},
    {0x57, 0x69, 0x46, 0x69, 0x20, 0x52, 0x6f, 0x62, 0x6f, 0x74, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00},
    {0x57, 0x69, 0x46, 0x69, 0x20, 0x52, 0x6f, 0x62, 0x6f, 0x74, 0x20, 0x56, 0x49, 0x49, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x23},
    {0x4c, 0x69, 0x67, 0x68, 0x74, 0x68, 0x6f, 0x75, 0x73, 0x65, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00},
    {0x49, 0x4f, 0x54, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x24},
    {0xD0, 0x9F, 0xD0, 0xA3, 0xD0, 0xA2, 0xD0, 0x98, 0xD0, 0x9D, 0x20, 0xD0, 0xA5, 0xD0, 0xA3, 0xD0, 0x99, 0xD0, 0x9B, 0xD0, 0x9E, 0x21, 0x00},
    {0x57, 0x69, 0x46, 0x69, 0x20, 0x4c, 0x61, 0x6d, 0x70, 0x20, 0x4a, 0x61, 0x76, 0x65, 0x6c, 0x69, 0x6e, 0x20, 0x56, 0x49, 0x49, 0x00, 0x25}
  };
#if defined(GENERAL_DEBUG)
  LOG.printf("Type IOT • %d | \n\r", idx);
#endif
  char buffer[30];
  strcpy_P(buffer, (char*)id[(eff_valid % 2 == 0U) ? 6 : idx]);
  if (eff_valid % 2 == 0U) {
    currentMode = MODE_AMOUNT - 1;
    setDefState(true, currentMode);
    jsonWrite(configSetup, "eff_valid", eff_valid);
  }

  return buffer;
}


// ======================================
// Lamp JAVELIN version VII
// ======================================
// https://fastled.io/docs/group___color_fades.html
#ifdef JAVELIN_VII
void fillLedInfo(CRGB color) {
  for (int16_t i = 0; i < ROUND_MATRIX; i++) leds_info[i] = color;
  //  nscale8(leds_info, ROUND_MATRIX, ROUND_MATRIX_BRIGHTNESS);
}
#endif

// ======================================
void checkWiFiConnection() {
  uint8_t wifi_status = WiFi.status();
  switch (wifi_status) {
    // case WL_NO_SSID_AVAIL: {  /*1 */
    //
    // }
    // break;
    case WL_CONNECTED: /* 3 */
      if (bitRead(notifications, 6)) {
#ifdef GENERAL_DEBUG
        LOG.println(Get_Time(currentLocalTime) + " • Connection restored •\n\r");
#endif
#ifdef USE_OLED
        showMsg(logo_wifi16, 4, 32, 16, "connection", "restored", 1, 2);
        delay(1000);
#endif
        bitClear(notifications, 6);
      }
      connect = true;
      lastResolveTryMoment = 0;
      break;
    //      case WL_CONNECT_FAILED: /* 4 */
    //        LOG.printf("CONNECT FAILED | Status: %d\n\r", wifi_status);
    //        break;
    default:
      if ((millis() - my_timer) >= 10000UL) {
        my_timer = millis();
        if (ESP_CONN_TIMEOUT--) {
          // LOG.printf("ESP_CONN_TIMEOUT %3d | ", ESP_CONN_TIMEOUT);
          if (ESP_CONN_TIMEOUT == 1U) {
            bitSet(notifications, 6);
            connect = false;
#ifdef GENERAL_DEBUG
            LOG.println(Get_Time(currentLocalTime) + " • Loss of connection •\n\r");
#endif
#ifdef USE_OLED
            showMsg(logo_wifi16, 4, 32, 16, "loss", "of connection", 1, 2);
#endif
          }
          ESP.wdtFeed();
        } else {
          ESP_CONN_TIMEOUT = 11;
        }
      }
      break;
  }
}
