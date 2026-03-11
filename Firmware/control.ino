/* control.ino */
// https://github.com/tobozo/ghetto_blaster
// ======================================
#ifdef  JAVELIN_VII

//static byte clicks;
//static byte time_out;
//static int val;
#endif

void initPinIOT() {
  // INPUT | INPUT_PULLUP | OUTPUT
#ifdef  JAVELIN_VII
#ifdef GENERAL_DEBUG
  LOG.println("Init Pin IOT | WiFi Lamp JAVELIN VII");
#endif
  pinMode(BTN_PIN, INPUT);
  pinMode(MOSFET_PIN, OUTPUT);
  digitalWrite(MOSFET_PIN, LOW);
  delay(1000);
  digitalWrite(MOSFET_PIN, !MOSFET_LEVEL);

  // enc.setEncType(EB_STEP4_LOW);
  menu = 0;
  enc.counter = 0;
#endif

#ifdef JAVELIN
#ifdef GENERAL_DEBUG
  LOG.println("Init Pin IOT | WiFi Lamp JAVELIN");
#endif
  pinMode(BACKLIGHT_PIN, OUTPUT);
  digitalWrite(BACKLIGHT_PIN, HIGH);
  pinMode(MB_LED_PIN, OUTPUT);
  digitalWrite(MB_LED_PIN, HIGH);
  pinMode(OTA_PIN, OUTPUT);
  digitalWrite(OTA_PIN, HIGH);
  pinMode(MOSFET_PIN, OUTPUT);
  delay(1000);
  digitalWrite(MOSFET_PIN, MOSFET_LEVEL);
#endif

#ifdef ORG_LAMP
#ifdef GENERAL_DEBUG
  LOG.println("Init Pin IOT | WiFi Lamp");
  pinMode(MB_LED_PIN, OUTPUT);
  digitalWrite(MB_LED_PIN, HIGH);
  pinMode(BTN_PIN, INPUT);
#endif

#endif
}

// =====================================
bool controlActionTick() {
  //  static byte last_eff;
#ifdef  JAVELIN_VII
  encoderTick();
  touchButtonTick();
  oledTick();
  motionControls();
#endif

#ifdef JAVELIN
  buttonTick();
  buttonJavelinTick();
#endif

#ifdef ORG_LAMP
  buttonTick();
#endif

  return true;
}

#ifdef  JAVELIN_VII
// =====================================
void touchButtonTick() {

  if (!buttonEnabled) {                                     // события кнопки не обрабатываются, если она заблокирована
    return;
  }
  static bool startButtonHolding = false;                     // флаг: кнопка удерживается для изменения яркости/скорости/масштаба лампы кнопкой
  static bool Button_Holding = false;
  touch.tick();
  uint8_t clickCount = touch.hasClicks() ? touch.getClicks() : 0U;
  if (clickCount >= 1U) {
    LOG.printf_P(PSTR("Click : %d | Speed : %d | Scale : %d \n\r"), clickCount, modes[currentMode].Speed, modes[currentMode].Scale);
  }

  // 1 click | stop diagnostic ========
  if (clickCount == 1U) {

    if (lendLease) {
      JavelinDiagnostic(100);
      progress = 0;
      JavelinStatic(0);
    } else {
      smartLampOff(BUTTON_SET_SLEEP_TIMER1);                    // включение дампы на 5 минут (не зависимо от того включена она или выключена)
    }
  }
  // 2 click | fire ===================
  if (clickCount == 2U) {
    JavelinStatic(1);
    delay(200);
    JavelinStatic(2);
  }

  // трёхкратное нажатие =======
  if (clickCount == 3U) {
    // motion control
    motion_control = !motion_control;
    jsonWrite(configSetup, "motion_control", (int)motion_control);
    showMotionControl(motion_control);
  }

  // четырёхкратное нажатие =======
  if (clickCount == 4U) {
    //                                                        // нa выбор -------------
    // runOTA();                                              // редко используемый режим проще и удобней включить из приложения заменен на любимый эффект
    cycleEffect();                                            // или включение показа эффектов в цикле
  }

  //  пятикратное нажатие =======
  //  включить эффект огонь
  if (clickCount == 5U) {                                     // нa выбор -------------
    currentMode = EFF_MATRIX - 1; runEffect();                // включить эффект огонь

  }

  // шестикратное нажатие =======
  if (clickCount == 6U) {                                     // нa выбор -------------
    // printTime(thisTime, true, ONflag);                     // вывод текущего времени бегущей строкой
    // currentMode = EFF_FAV; runEffect();                    // или любимый эффект
    showDevelopMode();
  }

  // семикратное нажатие =======
  if (clickCount == 7U) {                                     // смена рабочего режима лампы: с WiFi точки доступа на WiFi клиент или наоборот

#ifdef RESET_WIFI_ON_ESP_MODE_CHANGE
    if (espMode) wifiManager.resetSettings();                 // сброс сохранённых SSID и пароля (сброс настроек подключения к роутеру)
#endif
    espMode = (espMode == 0U) ? 1U : 0U;
    jsonWrite(configSetup, "ESP_mode", (int)espMode);
    saveConfig();

#ifdef GENERAL_DEBUG
    LOG.printf_P(PSTR("Рабочий режим лампы изменён и сохранён в энергонезависимую память\n\rНовый рабочий режим: ESP_MODE = %d, %s\n\rРестарт...\n\r"),
                 espMode, espMode == 0U ? F("WiFi точка доступа") : F("WiFi клиент (подключение к роутеру)"));
    delay(1000);
#endif

    showWarning(CRGB::Red, 3000U, 500U);                      // мигание красным цветом 3 секунды - смена рабочего режима лампы, перезагрузка
    ESP.restart();
  }
#ifdef GENERAL_DEBUG
  if (clickCount > 0U) {
    LOG.printf_P(PSTR("Button Click Count: %d | %d ms\n\r"), clickCount, BUTTON_SET_DEBOUNCE);
  }
#endif

  if (touch.isStep()) {
    if (ONflag && !Button_Holding) {
      int8_t but = touch.getHoldClicks();
      switch (but ) {
        case 0U: {                                               // просто удержание (до удержания кнопки кликов не было) - изменение яркости
            // changeBrightness(brightDirection);
            break;
          }
        default:
          break;
      }
    } else {
      if (!Button_Holding) {
        Button_Holding = true;
        currentMode = EFF_WHITE_COLOR;
        ONflag = true;
        changePower();
        settChanged = true;
        eepromTimeout = millis();
      }
    }
  }
  // кнопка отпущена после удерживания
  if (ONflag && !touch.isHold() && startButtonHolding) {     // кнопка отпущена после удерживания, нужно отправить MQTT сообщение об изменении яркости лампы
    startButtonHolding = false;
    Button_Holding = false;
    loadingFlag = true;

#if (USE_MQTT)
    if (espMode == 1U) MqttManager::needToPublish = true;
#endif
  }
}
#endif  JAVELIN_VII



#if defined(ORG_LAMP) || defined(JAVELIN) // || defined(ORG_LAMP)
// =====================================
#ifdef ESP_USE_BUTTON
bool brightDirection;
static bool startButtonHolding = false;                     // флаг: кнопка удерживается для изменения яркости/скорости/масштаба лампы кнопкой
static bool Button_Holding = false;
// =====================================
void buttonTick() {
  if (!buttonEnabled) {                                     // события кнопки не обрабатываются, если она заблокирована
    return;
  }

  touch.tick();
  uint8_t clickCount = touch.hasClicks() ? touch.getClicks() : 0U;
  if (clickCount >= 1U) {
    LOG.printf_P(PSTR("Click : %d | Speed : %d | Scale : %d \n\r"), clickCount, modes[currentMode].Speed, modes[currentMode].Scale);
  }
  // однократное нажатие =======
  if (clickCount == 1U) {
    startClick();
  }

  // двухкратное нажатие =======
  if (ONflag && clickCount == 2U) {
    prevEffect();
  }

  // трёхкратное нажатие =======
  if (ONflag && clickCount == 3U) {
    nextEffect();
  }

  // четырёхкратное нажатие =======
  if (clickCount == 4U) {
    //                                                        // нa выбор -------------
    // runOTA();                                              // редко используемый режим проще и удобней включить из приложения заменен на любимый эффект
    cycleEffect();                                            // или включение показа эффектов в цикле
  }

  //  пятикратное нажатие =======
  //  включить эффект огонь
  if (clickCount == 5U) {                                     // нa выбор -------------
    currentMode =  (MODE_AMOUNT - 5); runEffect();            // включить эффект огонь

  }

  // шестикратное нажатие =======
  if (clickCount == 6U) {                                     // нa выбор -------------
    // printTime(thisTime, true, ONflag);                     // вывод текущего времени бегущей строкой
    // currentMode = EFF_FAV; runEffect();                    // или любимый эффект
    smartLampOff(BUTTON_SET_SLEEP_TIMER1);                    // включение дампы на 5 минут (не зависимо от того включена она или выключена)
  }

  // семикратное нажатие =======
  if (clickCount == 7U) {                                     // смена рабочего режима лампы: с WiFi точки доступа на WiFi клиент или наоборот

#ifdef RESET_WIFI_ON_ESP_MODE_CHANGE
    if (espMode) wifiManager.resetSettings();                 // сброс сохранённых SSID и пароля (сброс настроек подключения к роутеру)
#endif
    espMode = (espMode == 0U) ? 1U : 0U;
    /* control blackout */
    if (espMode == 0) bitSet(notifications, 5); else bitClear(notifications, 5);
    jsonWrite(configSetup, "ESP_mode", (int)espMode);
    saveConfig();

#ifdef GENERAL_DEBUG
    LOG.printf_P(PSTR("Рабочий режим лампы изменён и сохранён в энергонезависимую память\n\rНовый рабочий режим: ESP_MODE = %d, %s\n\rРестарт...\n\r"),
                 espMode, espMode == 0U ? F("WiFi точка доступа") : F("WiFi клиент (подключение к роутеру)"));
    delay(1000);
#endif

    showWarning(CRGB::Red, 3000U, 500U);                      // мигание красным цветом 3 секунды - смена рабочего режима лампы, перезагрузка
    ESP.restart();
  }
#ifdef GENERAL_DEBUG
  if (clickCount > 0U) {
    LOG.printf_P(PSTR("Button Click Count: %d | %d ms\n\r"), clickCount, BUTTON_SET_DEBOUNCE);
  }
#endif

  // кнопка только начала удерживаться
  //  if (ONflag && touch.isHolded())
 if (touch.isHolded()) {                                     // пускай для выключенной лампы удержание кнопки включает белую лампу
    brightDirection = !brightDirection;
    startButtonHolding = true;
  }

  // кнопка нажата и удерживается
  if (touch.isStep()) {
    if (ONflag && !Button_Holding) {
      int8_t but = touch.getHoldClicks();

      switch (but ) {
        case 0U: {                                               // просто удержание (до удержания кнопки кликов не было) - изменение яркости
            changeBrightness(brightDirection);
            break;
          }

        case 1U: {                                               // удержание после одного клика - изменение скорости
            modes[currentMode].Speed = constrain(brightDirection ? modes[currentMode].Speed + 1 : modes[currentMode].Speed - 1, 1, 255);
#ifdef USE_MULTIPLE_LAMPS_CONTROL
            multipleLampControl();
#endif  //USE_MULTIPLE_LAMPS_CONTROL
#ifdef GENERAL_DEBUG
            LOG.printf_P(PSTR("Новое значение скорости: %d\n\r"), modes[currentMode].Speed);
#endif
            updateSets();
            break;
          }

        case 2U: {                                             // удержание после двух кликов - изменение масштаба
            modes[currentMode].Scale = constrain(brightDirection ? modes[currentMode].Scale + 1 : modes[currentMode].Scale - 1, 1, 100);

#ifdef USE_MULTIPLE_LAMPS_CONTROL
            multipleLampControl();
#endif  //USE_MULTIPLE_LAMPS_CONTROL

#ifdef GENERAL_DEBUG
            LOG.printf_P(PSTR("Новое значение масштаба: %d\n\r"), modes[currentMode].Scale);
#endif
            updateSets();
            break;
          }

        default:
          break;
      }

      settChanged = true;
      eepromTimeout = millis();
    } else {
      if (!Button_Holding) {
        Button_Holding = true;
        currentMode = EFF_WHITE_COLOR;
        ONflag = true;
        changePower();
        settChanged = true;
        eepromTimeout = millis();
      }
    }
  }
  // кнопка отпущена после удерживания
  if (ONflag && !touch.isHold() && startButtonHolding) {     // кнопка отпущена после удерживания, нужно отправить MQTT сообщение об изменении яркости лампы
    startButtonHolding = false;
    Button_Holding = false;
    loadingFlag = true;

#if (USE_MQTT)
    if (espMode == 1U) MqttManager::needToPublish = true;
#endif
  }
}
#endif
#endif



#ifdef  JAVELIN_VII

// ======================================
void motionControls() {
  if (menu == 0) {
    if (sleep) {
      EVERY_N_SECONDS(2) {
        uint16_t analog_val = analogRead( SENSOR_MOTION );
        if (analog_val > 512U) {
          sleep = false;
          // LOG.printf("Analog Val • %d | sleep • %d\n\r", analog_val, sleep);
          if (motion_control & ONflag) {
            if (currentMode >= MODE_AMOUNT - 3) currentMode = 0U;
            prevEffect();
          } else {
            showTime();
          }
        }
      }
    }
  }
}
#endif


// ======================================
//                ENCODER
// ======================================
#ifdef USE_ENCODER

IRAM_ATTR void isr() {
  enc.tickISR();
}

// ======================================
void encoderTick() {
  // static byte time_out;
  static byte clicks;
  int val;
  byte valMin, valMax;
  EffData effectsSRAM;
  enc.tick();

  //  EVERY_N_SECONDS(5) {
  //    LOG.printf("encoderTick | menu • %d | currentMode • %03d\n\r", menu, currentMode);
  //  }

  /* ===================
    click encoder button
    ==================== */
  /* on/off ------------ */
  if (enc.hasClicks(1)) startClick();

  /* select menu ------- */
  if (enc.hasClicks(2)) {
    LOG.println("has 2 clicks");
    menu++;
    if (menu > 2) {
      menu = 0;
    }
    if (menu == 0) showEffectNum(); else showMenu(menu);
  }

  /* def reset effects */
  if (enc.hasClicks(3)) ResetDefaultEffects(true);
  if (enc.hasClicks(4)) cycleEffect();
  /* ================== */

  if (enc.turn()) {

    //    LOG.print("\n\rturn: dir ");
    //    LOG.print(enc.dir());
    //    LOG.print(", fast ");
    //    LOG.print(enc.fast());
    //    LOG.print(", hold ");
    //    LOG.print(enc.pressing());
    //    LOG.print(", counter ");
    //    LOG.print(enc.counter);
    //    LOG.print(", clicks ");
    //    LOG.println(enc.getClicks());
    //    LOG.println("");

    // time_out = 0;

    if (enc.pressing()) {
      menu = 0;
      /* Set Led Brigthness */
      global_br += (enc.fast() ? 20 : 5) * enc.dir();
      if (global_br > 100) global_br = 100;  /* max */
      if (global_br < 10) global_br = 10;    /* min */
      setGlobalBrightness(modes[currentMode].Brightness, true);
    } else {
      /* prev • next effect */
      // LOG.printf("\n\r• enc.turn | Effect %3d | Clicks %1d | time_out %2d | MENU • %1d |\n\r", currentMode, clicks, time_out, menu);
      if ( ONflag) {
        switch (menu) {
          case 0: /* def mode | prev • next effect */
            currentMode += (enc.fast() ? 10 : 1) * enc.dir();
            if (currentMode >= MODE_AMOUNT) currentMode = 0;
            if (currentMode < 0) currentMode = MODE_AMOUNT - 1;
            runEffect();
            // LOG.printf("• turn | Effect % 3d | Brightness % 3d | ", currentMode, FastLED.getBrightness());
            showEffectNum();
            break;

          case 1: /* speed | < • > */

            memcpy_P( &effectsSRAM, &effectsPROGMEM[currentMode], sizeof(EffData));
            valMin  = effectsSRAM.min_spd;
            valMax  = effectsSRAM.max_spd;

            val = modes[currentMode].Speed;
            val += (enc.fast() ? 10 : 1) * enc.dir();
            if ( val >= valMax) val = valMax;
            if ( val <= valMin) val = valMin;
            modes[currentMode].Speed = val;
            loadingFlag = true;
            updateSets();

            showSetsParameter(icon_set24, 1, modes[currentMode].Speed, "");
            //LOG.printf("• turn | Effect % 3d | Speed [ %3d . %3d . %3d ] ", currentMode, valMin, modes[currentMode].Speed, valMax);
            break;

          case 2:  /* scale | < • > */

            memcpy_P( &effectsSRAM, &effectsPROGMEM[currentMode], sizeof(EffData));
            valMin  = effectsSRAM.min_scl;
            valMax  = effectsSRAM.max_scl;
            // LOG.printf("• turn | Effect % 3d | Scale [ %3d . %3d . %3d ] ", currentMode, valMin, modes[currentMode].Scale, valMax);

            val = modes[currentMode].Scale;
            val += (enc.fast() ? 10 : 1) * enc.dir();
            if ( val >= valMax) val = valMax;
            if ( val <= valMin) val = valMin;
            modes[currentMode].Scale = val;
            loadingFlag = true;
            updateSets();
            showSetsParameter(icon_set24, 2, modes[currentMode].Scale, "%");
            // LOG.printf("• turn | Effect % 3d | Scale % 3d | ", currentMode, modes[currentMode].Scale);
            break;
          default:
            break;
        }
      }
    }
    sleep = false;
  }
}

#endif
