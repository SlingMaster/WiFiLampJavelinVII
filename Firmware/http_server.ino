// http_server.ino
void runServerHTTP(void) {
  // Ниже две функции предварительной настройка WiFi в режиме точки доступа
  // и установка ESP_mode -------------------------------------------------
  HTTP.on("/ESP_mode", HTTP_GET, []() {   // Установка ESP Mode
    jsonWrite(configSetup, "ESP_mode", HTTP.arg("ESP_mode").toInt());
    saveConfig();
    espMode = jsonReadtoInt(configSetup, "ESP_mode");
    HTTP.send(200, "text/plain", "OK");
  });

  // SSID | Pass | timout -------------------------------------------------
  HTTP.on("/ssid", HTTP_GET, []() {          // сохранение настроек роутера
    jsonWrite(configSetup, "ssid", HTTP.arg("ssid"));
    jsonWrite(configSetup, "password", HTTP.arg("password"));
    jsonWrite(configSetup, "TimeOut", HTTP.arg("TimeOut").toInt());
    ESP_CONN_TIMEOUT = jsonReadtoInt(configSetup, "TimeOut");
    saveConfig();                 // Функция сохранения данных во Flash
    HTTP.send(200, "text/plain", "OK"); // отправляем ответ о выполнении
  });
  // ======================================================================

  // Выдаем данные configSetup ===========
  HTTP.on("/config.setup.json", HTTP_GET, []() {
    HTTP.send(200, "application/json", configSetup);
  });

  // Обработка Restart ===================
  HTTP.on("/restart", HTTP_GET, []() {
    String restart = HTTP.arg("device");            // Получаем значение device из запроса
    if (restart == "ok") {                          // Если значение равно Ок
      HTTP.send(200, "text / plain", "Reset OK");   // Oтправляем ответ Reset OK
      ESP.restart();                                // перезагружаем модуль
    }
    else {                                          // иначе
      HTTP.send(200, "text / plain", "No Reset");   // Oтправляем ответ No Reset
    }
  });

  // Remote Control =====================
  HTTP.on("/cmd", handle_cmd);                      // web управление лампой http:ipLamp/cmd?cmd=• команда •&val=• значение •
  // ------------------------------------
  httpUpdater.setup(&HTTP);                         // Добавляем функцию Update для перезаписи прошивки по WiFi при 4М(1M SPIFFS) и выше
  HTTP.begin();                                     // Запускаем HTTP сервер
}

// ======================================
void warnDinamicColor(uint8_t val) {
  switch (val) {
    case 0: showWarning(CRGB::Blue, 1000U, 250U); break;
    case 1: showWarning(CRGB::Yellow, 1000U, 250U); break;
    case 2: showWarning(CRGB::Red, 1000U, 250U); break;
    case 3: showWarning(CRGB::Cyan, 1000U, 250U); break;
    case 4: showWarning(CRGB::Gold, 1000U, 250U); break;
    case 5: showWarning(0xFF3F00, 1000U, 250U); break; // orange
    case 6: showWarning(CRGB::Green, 1000U, 250U); break;
    case 7: showWarning(CRGB::Magenta, 1000U, 250U); break;
    default: showWarning(CRGB::White, 1000U, 250U); break;
  }
}

// ======================================
void printMSG(String temStr, bool def) {
  temStr.toCharArray(TextTicker, temStr.length() + 1);
  currentMode = MODE_AMOUNT - 1;
  if (def) {
    setDefState(true, currentMode);
  }
  runEffect();
}

// ======================================
void testMatrix(uint8_t val) {
  ONflag = false;
#ifdef JAVELIN_VII
  const StrData* sd = &strdataPROGMEM[0];
#endif
  FastLED.setBrightness(10);
  FastLED.clear();
#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)         // установка сигнала в пин, управляющий MOSFET транзистором, соответственно состоянию вкл/выкл матрицы
  digitalWrite(MOSFET_PIN, MOSFET_LEVEL);
#endif
  /* test coordinates */
  //  drawPixelXY(0, 0, 0x00FF00);
  //  drawPixelXY(1, 0, 0x1F4F00);
  //  drawPixelXY(2, 0, 0x2F2F00);
  //  drawPixelXY(WIDTH - 1, HEIGHT - 1, 0xFF0000);
  //  drawPixelXY(WIDTH - 2, HEIGHT - 1, 0x4F1F00);
  //  drawPixelXY(WIDTH - 3, HEIGHT - 1, 0x2F2F00);

  gradientRect(0, HEIGHT - 1, WIDTH, HEIGHT - 1, 0, 0, 0, 200, 255U, 0);  /* end • */
  gradientRect(0, 0, WIDTH, 0, 96, 96, 200, 0, 255U, 0);                    /* • start */


  if (WIDTH % 2 == 0) { /*center */
    drawPixelXY(CENTER_X_MINOR, CENTER_Y_MINOR, 0xFF7F00);
    drawPixelXY(CENTER_X_MINOR + 1, CENTER_Y_MINOR, 0xFF7F00);
  } else {
    drawPixelXY(CENTER_X_MAJOR, CENTER_Y_MINOR, 0xFF7F00);
  }
  if (HEIGHT % 2 == 0) {
    drawPixelXY(CENTER_X_MINOR, CENTER_Y_MINOR + 1, 0xFF7F00);
    drawPixelXY(CENTER_X_MINOR + 1, CENTER_Y_MINOR + 1, 0xFF7F00);
  } else {
    drawPixelXY(CENTER_X_MAJOR, CENTER_Y_MINOR + 1, 0xFF7F00);
  }

  FastLED.delay(1);

#ifdef JAVELIN_VII
  showMsg(ico_debug16, 5, 16, 16, FPSTR(sd->s0), FPSTR(sd->s2), 1, 2);
#endif
  delay(5000);

  /* test color */
  fillAll(CRGB::Red);
  FastLED.delay(1);

#ifdef JAVELIN_VII
  showMsg(ico_debug16, 5, 16, 16, FPSTR(sd->s1), FPSTR(sd->s3), 1, 2);
#endif

  delay(3000);

  fillAll(CRGB::Green);
  FastLED.delay(1);
#ifdef JAVELIN_VII
  showMsg(ico_debug16, 5, 16, 16, FPSTR(sd->s1), FPSTR(sd->s4), 1, 2);
#endif
  delay(3000);

  fillAll(CRGB::Blue);
  FastLED.delay(1);
#ifdef JAVELIN_VII
  showMsg(ico_debug16, 5, 16, 16, FPSTR(sd->s1), FPSTR(sd->s5), 1, 2);
#endif
  delay(3000);
  /* x | y | x1 y1 | start_color | end_color | start_br | end_br | saturate | angleDeg */
  gradientRect(0, 0, WIDTH, HEIGHT, 0, 255, 255, 255, 255U, 90);
  FastLED.delay(1);
#ifdef JAVELIN_VII
  showMsg(ico_debug16, 5, 16, 16, FPSTR(sd->s1), FPSTR(sd->s6), 1, 2);
#endif
  delay(3000);

#ifdef JAVELIN_VII
  showMsg(ico_debug16, 5, 16, 16, FPSTR(sd->s1), FPSTR(sd->s7), 1, 2);
#endif
  for (uint8_t y = 0U; y < HEIGHT; y++) {
    for (uint8_t x = 0U; x < WIDTH; x++) {
      leds[XY(x, y)] = CRGB::White;
    }
    FastLED.delay(2);
  }
#ifdef JAVELIN_VII
  showMsg(ico_debug16, 5, 16, 16, FPSTR(sd->s0), FPSTR(sd->s8), 1, 2);
#endif
  delay(2000);
  for (uint8_t y = HEIGHT - 1; y > 0U; y--) {
    for (uint8_t x = 0U; x < WIDTH; x++) {
      leds[XY(x, y)] = CRGB::Black;
    }
    FastLED.delay(1);
  }
  delay(50);
  FastLED.setBrightness(modes[currentMode].Brightness);
  ONflag = true;
}

// ======================================
void ResetDefaultEffects(bool warn) {
#ifdef JAVELIN_VII
  showMsg(icon_set24, 0, 24, 24, "effects", "default settings", 1, 2);
#endif
  restoreSettings();
  updateSets();
  if (warn) showWarning(CRGB::Blue, 2000U, 500U);      // мигание синим цветом 2 секунды
}

// ======================================
String runCommand(byte cmd, uint8_t val, String valStr) {
  String body = "";
#ifdef GENERAL_DEBUG
  //  LOG.printf_P(PSTR("RUN COMMAND • %02d | val • %03d | "), cmd, val);
  //  LOG.println(valStr);
#endif
  /* ------------------------------------ */
  switch (cmd ) {
    case CMD_POWER:
      if (val == 3) {               // toggle
        ONflag = !ONflag;
      } else {                      // on or off
        ONflag = val;
      }

      changePower();
#ifdef USE_MULTIPLE_LAMPS_CONTROL
      multipleLampControl();
#endif  //USE_MULTIPLE_LAMPS_CONTROL  
      break;

    case CMD_PREV_EFF:
      selectedSettings = false;
      prevEffect();
      break;

    case CMD_NEXT_EFF:
      selectedSettings = false;
      nextEffect();
      break;
    // case CMD_FAVORITES:
    // FavoritesManager::FavoritesRunning = val;
    // break;
    //case CMD_PC_STATE:
    //  InfoPC(valStr);
    //  break;

    case CMD_BRIGHT_UP:
      changeBrightness(true);
      break;

    case CMD_BRIGHT_DW:
      changeBrightness(false);
      break;

    case CMD_SPEED:
      modes[currentMode].Speed = val;
      loadingFlag = true;
      updateSets();
#ifdef USE_MULTIPLE_LAMPS_CONTROL
      multipleLampControl ();
#endif  //USE_MULTIPLE_LAMPS_CONTROL
#ifdef USE_OLED
      showSetsParameter(icon_set24, 1, val, "");
#endif
      break;

    case CMD_SCALE:
      modes[currentMode].Scale = val;
      loadingFlag = true;
      updateSets();
#ifdef USE_MULTIPLE_LAMPS_CONTROL
      multipleLampControl();
#endif  //USE_MULTIPLE_LAMPS_CONTROL
#ifdef USE_OLED
      showSetsParameter(icon_set24, 2, val, "%");
#endif
      break;
    case CMD_WHITE:
      // javelinConnect();
      currentMode = EFF_WHITE_COLOR;
      runEffect();
      break;

    case CMD_TEXT: {
        if (valStr == "dev") {
#ifdef JAVELIN_VII
          showDevelopMode();
#endif
        } else {
          String tempStr = TextTicker;
          if (valStr == "") {
            tempStr = getNameIOT(IOT_TYPE);
          } else {
            tempStr = (eff_valid < 2) ? valStr : jsonRead(configSetup, "run_text");
          }
          LOG.println("• Print Text • " + tempStr);
          printMSG(tempStr, false);
        }
      }
      break;
    case CMD_SCAN:
      //      body += "\"id\":" + String(cmd) + ",";
      //      body += getInfo();
      // showWarning(CRGB::Blue, 1000U, 500U);
      break;

    // effect ----------------
    case CMD_DEFAULT:
      setDefState(true, currentMode);
      runEffect();
      break;
    case CMD_RESET_EFF:
      ResetDefaultEffects(true);
      break;
    case CMD_AUTO:
      cycleEffect();
      break;
    case CMD_INTIM:
      currentMode = MODE_AMOUNT - 5;
      runEffect();
      break;
    case CMD_FAV:
      currentMode = EFF_FAV;
      runEffect();
      break;
    case CMD_RANDOM:
      selectedSettings = true;
      updateSets();
      break;

    case CMD_LIST:
      loadingFlag = false;
      // path -----------------
      // IPAddress ip = WiFi.localIP();
#ifdef GENERAL_DEBUG
      LOG.printf(" • Loading package effects • %2d | Heap • %d bytes\n\r", val, system_get_free_heap_size());
#endif
      body = "\"status\":\"OK\",";
      body += getLampID() + ",";
      //      body += "\"ip\":\"" + ipToString(ip) + "\",";
      //      //  body += "\"e_ip\":\"" + e_ip + "\",";
      body += "\"max_eff\":" + String(MODE_AMOUNT) + ",";
      //      // body += getLampID() + ",";
      body += "\"list\":" + createListPackage(val);
      // LOG.println("body: " + body);
      sendResponse(cmd, body);
      body = "";

#ifdef JAVELIN_VII
      {
        //        String load_progres;
        //        load_progres = "";
        //        for (int8_t i = 0; i < (MODE_AMOUNT / 10); i++) load_progres += (val / 10 >= i) ? "|" : ".";
        //        showMsg(icon_lamp32, 1, 32, 32, "load list effects", load_progres, 1, 1);

#define LOAD_BAR_MAX 15   // підбери під MODE_AMOUNT / 10
        char load_progress[LOAD_BAR_MAX + 1];
        uint8_t bars = MODE_AMOUNT / 10;
        uint8_t filled = val / 10;
        for (uint8_t i = 0; i < bars && i < LOAD_BAR_MAX; i++) {
          load_progress[i] = (filled >= i) ? '|' : '.';
        }
        load_progress[bars] = '\0';
        showMsg(icon_lamp32, 1, 32, 32, "load list effects", load_progress, 1, 1);
        oledON(SHOW_DEF);
      }
#endif
      return "";

    case CMD_SHOW_EFF:
      if (eff_auto == 1) eff_auto = 0; // off cycle effects
      currentMode = val;
      runEffect();
      break;

      // group lamps ----------
#ifdef USE_MULTIPLE_LAMPS_CONTROL
    case CMD_GROUP_INIT:
      initWorkGroup(valStr);
      warnDinamicColor(7);
      break;
    case CMD_GROUP_DESTROY:
      resetWorkGroup();
      warnDinamicColor(val);
      break;
#endif //USE_MULTIPLE_LAMPS_CONTROL
    // ----------------------

    // configure commands ---
    case CMD_CONFIG:
      loadingFlag = false;
      // warnDinamicColor(2);
      // LOG.println("config Setup:" + configSetup);
      body += "\"cfg\":" + configSetup + ",";
      body += "\"alarms\":" + readFile("alarm_config.json", 1024) + ",";
      loadingFlag = true;
      break;
    case CMD_SAVE_CFG :
      configSetup = valStr;
      getNameIOT(IOT_TYPE);
      LOG.println("config save:" + configSetup);
      body += "\"cfg_save\":\"OK\",";
      saveConfig();
      valStr = "";
      warnDinamicColor(0);
      initConfigure();
      break;
    case CMD_SAVE_ALARMS :
      // configSetup = valStr;
      body += "\"cfg_save\":\"OK\",";
      saveAlarm(valStr);
      valStr = "";
      break;

    case CMD_GLOBAL_BRI:
      //#ifdef JAVELIN_VII
      //      develop = val;
      //      fps = 0;
      //#endif

      gb = val;
      jsonWrite(configSetup, "gb", val);
      saveConfig();
      loadingFlag = false;
      runEffect();
      // setGlobalBrightness(modes[currentMode].Brightness, true);
      break;
    // fs commands ----------
    case CMD_FS_DIR:
      loadingFlag = false;
      body += getLampID() + ",";
      body += getFS();
      // LOG.println(body);
      sendResponse(cmd, body);
      return "";

    // develop commands -----
    case CMD_TEST_MATRIX:
      testMatrix(val);
      return "";
    case CMD_CUSTOM_EFF:

      if (val == 2) {               // restore
        custom_eff = jsonReadtoInt(configSetup, "custom_eff");
        FastLED.clear();
      } else {
        custom_eff = val;           // toggle
      }
      LOG.printf("CUSTOM_EFF • %d | val • %d | \n\r", custom_eff, val);
#ifdef JAVELIN_VII
      motion_control = custom_eff;
      showMotionControl(motion_control);
#endif
      break;
    case CMD_FW_INFO:
    case CMD_INFO:
      body += getLampID() + ",";
      body += getInfo();
      sendResponse(cmd, body);
#ifdef JAVELIN_VII
      if (cmd == CMD_INFO) outIP(WiFi.localIP());
#endif
      return "";
    case CMD_ECHO:
      warnDinamicColor(val);
      break;
    case CMD_DEL_FILE:
      // if (valStr == "") body += "\"status\":\"Error BAD ARGS\",";
      if (valStr == "/") body += "\"status\":\"Error BAD PATH\",";
      if (SPIFFS.exists(valStr)) {
        SPIFFS.remove(valStr);
        cmd = CMD_FS_DIR;
        body += "\"status\":\"OK\",";
        // send new list files ----
        body += getFS() + ",";
      } else {
        if (valStr.lastIndexOf(".") == -1) {
          SPIFFS.remove(valStr + ".");
          body += "\"status\":\"Remove Directory " + valStr + "\",";
        } else {
          body += "\"status\":\"Error File Not Found\",";
        }
      }
      break;

    case CMD_RESET:
#ifdef JAVELIN_VII
      fillLedInfo(CRGB::Red);
      showMsg(icon_cpu32, 0, 32, 32, "RESET LAMP", "", 1, 2);
#endif
      bitSet(notifications, 4);
      jsonWrite(configSetup, "notifications", notifications);
      saveConfig();
      showWarning(CRGB::MediumSeaGreen, 2000U, 500U);
      ESP.restart();
      break;
    case CMD_ACTIVATE:
      eff_valid = val;
      jsonWrite(configSetup, "eff_valid", eff_valid);
#if defined(JAVELIN_VII)
      if (val == 1) showMsg(icon_lamp32, 1, 32, 32, "L A M P", "activated", 1, 1);
#endif
      break;
    case CMD_CONNECT:
#if defined(JAVELIN_VII)
      showMsg(icon_lamp32, 1, 32, 32, "application", "controls", 1, 1);
#endif
#ifdef GENERAL_DEBUG
      LOG.println(" ======== APP CONNECTED TO LAMP ========");
#endif
      break;
    case CMD_OTA:
      eff_auto = 0;
      runOTA();
      break;


#if defined(JAVELIN_VII) || defined(JAVELIN)
    // javelin --------------
    case CMD_EFF_JAVELIN:
      // progress = 100;
      JavelinStatic(val);
      return "";
    case CMD_DIAGNOSTIC:
      step = 0;
      if (val == 0) {
        progress = 0;
        diagnostic = true;
      }
      body += getDiagnosticProgress();
      sendResponse(cmd, body);
      return body;
#endif

    default:
      break;
  }

  return body;
}

// ======================================
// Http Remote Control Command ----------
void handle_cmd() {
  uint8_t cmd = HTTP.arg("cmd").toInt();
  String valStr = HTTP.arg("valStr");
  uint8_t val = HTTP.arg("val") ? HTTP.arg("val").toInt() : 0;
  String body = runCommand(cmd, val, valStr);
  if (cmd == CMD_LIST) {
    return;
  }
  body += getCurState();
  sendResponse(cmd, body);
}

// ======================================
#if (defined(JAVELIN_VII) || defined(JAVELIN))
String getDiagnosticProgress() {
  String lamp_state = "";
  lamp_state += getLampID() + ",";
  lamp_state += "\"power\":" + String(ONflag) + ",";
  lamp_state += "\"javelin\":1,";
  lamp_state += "\"iot\":1,";
  lamp_state += "\"progress\":" + String(progress);
  return lamp_state;
}
#endif

// ======================================
String getInfo() {
  byte mac[6];
  WiFi.macAddress(mac);
  IPAddress ip = WiFi.localIP();
  String ssid = jsonRead(configSetup, "ssid");
  String lamp_info = "";
  lamp_info += "\"ssid\":\"" + ssid + "\",";
  lamp_info += "\"mac\":\"" + (String(mac[0], HEX) + ":" + String(mac[1], HEX) + ":" + String(mac[2], HEX) + ":" + String(mac[3], HEX) + ":" + String(mac[4], HEX) + ":" + String(mac[5], HEX)) + "\",";
  lamp_info += "\"free_heap\":" + String(system_get_free_heap_size()) + ",";
  lamp_info += "\"rssi\":" + String(WiFi.RSSI()) + ",";
  //  WIDTH | HEIGHT | MATRIX_TYPE | CONNECTION_ANGLE | STRIP_DIRECTION | COLOR_ORDER
  lamp_info += "\"matrix\":[" + String(WIDTH) + "," + String( HEIGHT) + "," + String(MATRIX_TYPE) + "," + String(CONNECTION_ANGLE) + "," + String(STRIP_DIRECTION) + "],";
  lamp_info += "\"chip_id\":\"" + String(ESP.getChipId(), HEX) + "\",";

#ifdef  JAVELIN
  lamp_info += "\"javelin\":1,";
#else
  lamp_info += "\"javelin\":0,";
#endif
  lamp_info += "\"ver\":\"" + VERSION + "\"";
  return lamp_info;
}

// ======================================
String getCurState() {
  // uint8_t bright = gb ? global_br * 2 : modes[currentMode].Brightness;
  uint8_t bright = gb ? 100 : global_br;
  String lamp_state = "";
  lamp_state += getLampID() + ",";
  lamp_state += "\"pass\":\"" + AP_PASS + "\",";
  lamp_state += "\"ver\":\"" + VERSION + "\",";
  lamp_state += "\"valid\":" + String(eff_valid) + ",";
  lamp_state += "\"power\":" + String(ONflag) + ",";
  lamp_state += "\"workgroup\":" + String(WORKGROUP) + ",";
  lamp_state += "\"custom_eff\":" + String(custom_eff) + ",";
  lamp_state += "\"eff_auto\":" + String(eff_auto) + ",";
  lamp_state += "\"cycle\":" + String(eff_auto) + ",";
  //  lamp_state += "\"cycle\":" + String(FavoritesManager::FavoritesRunning) + ",";
  lamp_state += "\"list_idx\":" + String(currentMode) + ",";
  lamp_state += "\"max_eff\":" + String(MODE_AMOUNT) + ",";
  lamp_state += "\"gb\":" + String(gb) + ",";
  lamp_state += "\"bright\":" + String(bright) + ",";
  lamp_state += "\"speed\":" + String(modes[currentMode].Speed) + ",";
  lamp_state += "\"free_heap\":" + String(system_get_free_heap_size()) + ",";
  lamp_state += "\"javelin\":" + String(IOT_TYPE) + ",";
  lamp_state += "\"scale\":" + String(modes[currentMode].Scale);
  return lamp_state;
}

// ======================================
String getLampID() {
  IPAddress ip = WiFi.localIP();
  String id = "\"name\":\"" + LAMP_NAME + "\",";
  id += "\"ip\":\"" + ipToString(ip) + "\"";
  return id;
}

// ======================================
String ipToString(IPAddress ip) {
  String s = "";
  for (int i = 0; i < 4; i++)
    s += i  ? "." + String(ip[i]) : String(ip[i]);
  return s;
}

// ======================================
String getFS() {
  //  if (!HTTP.hasArg("dir")) {
  //    HTTP.send(500, "text/plain", "BAD ARGS");
  //    return;
  //  }
  String path = ""; //HTTP.arg("dir");
  Dir dir = SPIFFS.openDir(path);
  path = String();
  String output = "[";
  while (dir.next()) {
    File entry = dir.openFile("r");  // Открываем файл только для чтения
    String fileName = entry.name();  // Имя файла
    String extension = "";           // Расширение файла

    // Извлекаем расширение вручную
    int dotIndex = fileName.lastIndexOf('.');
    if (dotIndex >= 0) {
      extension = fileName.substring(dotIndex + 1);
    }

    // Логируем расширение (для отладки)
    if (extension == "json" || extension == "img") {
      // LOG.println("Расширение файла: " + extension);
      // Формируем JSON ---------------
      if (output != "[") output += ',';
      output += "{";  /* { start } */
      //      bool isDir = false;
      //      output += "\"t\":\"";
      //      output += (isDir) ? "dir" : "f";
      //      output += "\",";
#if defined (USE_LittleFS)
      output += "\"n\":\"" + String(entry.name()).substring(0) + "\",";
#else
      output += "\"n\":\"" + String(entry.name()).substring(1) + "\",";
#endif
      output += "\"s\":" + String(entry.size());
      output += "}"; /* { end } */
    }             /* endif extension */
    entry.close();  // Закрываем файл
  }
  output += "]";
  return "\"fs\":" + output;
}

// ======================================
void sendResponse(uint8_t cmd, String json) {
  String body = "\"id\":" + String(cmd) + ",";
  body += "\"json\":{";
  body += json;
  body += "}";
  sendHTML(body);
}

// ======================================
void sendHTML(String body) {
  char buffer[1024]; // Буфер для копирования из PROGMEM
  strcpy_P(buffer, htmlHeader);
  String output = buffer + body; // + "}</body></html>\r\n";
  strcpy_P(buffer, htmlHeaderEnd);
  output += buffer;
  // LOG.println(output);
  HTTP.send(200, "text/html", output);
}
