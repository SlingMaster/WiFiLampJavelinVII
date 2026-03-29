/* action.ino */
// ========================================
// ========================================
#define WARNING_BRIGHTNESS    (10U)                         // яркость вспышки
void showWarning(
  // мигающий цвет
  // используется для отображения краткосрочного предупреждения; блокирующий код!)
  CRGB color,                                               /* цвет вспышки                                                 */
  uint32_t duration,                                        /* продолжительность отображения предупреждения (общее время)   */
  uint16_t blinkHalfPeriod)                                 /* продолжительность одной вспышки в миллисекундах (полупериод) */
{
#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)      // установка сигнала в пин, управляющий MOSFET транзистором, матрица должна быть включена на время вывода текста
  digitalWrite(MOSFET_PIN, MOSFET_LEVEL);
#endif
  uint32_t blinkTimer = millis();
  enum BlinkState { OFF = 0, ON = 1 } blinkState = BlinkState::OFF;

  FastLED.setBrightness(WARNING_BRIGHTNESS);                // установка яркости для предупреждения
  FastLED.clear();
  FastLED.delay(2);
  fillAll(color);

  uint32_t startTime = millis();
  while (millis() - startTime <= (duration + 5)) {          // блокировка дальнейшего выполнения циклом на время отображения предупреждения
    if (millis() - blinkTimer >= blinkHalfPeriod) {         // переключение вспышка/темнота
      blinkTimer = millis();
      blinkState = (BlinkState)!blinkState;
      FastLED.setBrightness(blinkState == BlinkState::OFF ? 0 : WARNING_BRIGHTNESS);
      FastLED.delay(1);
    }
    delay(50);
  }

  FastLED.clear();
  FastLED.setBrightness(ONflag ? modes[currentMode].Brightness : 0);  // установка яркости, которая была выставлена до вызова предупреждения
  FastLED.delay(1);

#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)      // установка сигнала в пин, управляющий MOSFET транзистором, соответственно состоянию вкл/выкл матрицы или будильника
  digitalWrite(MOSFET_PIN, ONflag || (dawnFlag && !manualOff) ? MOSFET_LEVEL : !MOSFET_LEVEL);
#endif
  loadingFlag = true;                                       // принудительное отображение текущего эффекта (того, что был активен перед предупреждением)
}

// ========================================
void startClick() {
  if (dawnFlag) {
    manualOff = true;
    dawnFlag = false;
    // FastLED.setBrightness(modes[currentMode].Brightness);
    changePower();
  } else {
    ONflag = !ONflag;
    changePower();
    if (ONflag) {
      runEffect();
#if (defined(JAVELIN_VII) | defined(JAVELIN))
      StateLampIndicator();
#endif
    } else {
#ifdef USE_OLED
      // showIcon(255, 0);
      showMsg(icon_sleep32, 0, 32, 32, "SLEEP", "", 1, 0);
      oledON(5);
#endif
    }
  }

  //updateSets();

  if (eff_valid > CMD_POWER) return;
#if (USE_MQTT)
  if (espMode == 1U) {
    MqttManager::needToPublish = true;
  }
#endif

#ifdef USE_MULTIPLE_LAMPS_CONTROL
  multipleLampControl();
#endif  //USE_MULTIPLE_LAMPS_CONTROL
}

// ========================================
void setGlobalBrightness(uint8_t brightness, bool is_show) {
  uint8_t newBR = constrain( brightness  * global_br / 50, 1, 50);
  /*  if (gb) set max brightness */
  FastLED.setBrightness(gb ?  55 : newBR);
  // FastLED.show();
#ifdef JAVELIN_VII
  if (is_show) showSetsParameter(icon_set24, 0, global_br, "%");
#endif
}


// ========================================
void runEffect() {
  setGlobalBrightness(modes[currentMode].Brightness, false);
  updateSets();
  if (random_on) {
    selectedSettings = true;
  }
#if (USE_MQTT)
  if (espMode == 1U) MqttManager::needToPublish = true;
#endif
#ifdef USE_MULTIPLE_LAMPS_CONTROL
  multipleLampControl();
#endif  //USE_MULTIPLE_LAMPS_CONTROL 
  lendLease = false;
#ifdef USE_OLED
  showEffectNum();
#endif
}



// ========================================
void prevEffect() {
  if (++currentMode >= MODE_AMOUNT) currentMode = 0U;
  runEffect();
}

// ========================================
void nextEffect() {
  if (--currentMode >= MODE_AMOUNT) currentMode = MODE_AMOUNT - 1;
  runEffect();
}

// ========================================
void cycleEffect() {
  eff_auto = (eff_auto == 1) ? 0 : 1;
  warnDinamicColor(eff_auto);
}

// ========================================
void autoSwapEff() {
  //  if (ONflag && eff_auto && (FavoritesManager::FavoritesRunning == 0)) {
  if (ONflag && eff_auto) {
    if (eff_rnd) {
      currentMode = random8(EFF_MATRIX + 1);
      runEffect();
    } else {
      if (currentMode > (EFF_MATRIX)) {
        currentMode = 0;
      }
      prevEffect();
    }
#ifdef GENERAL_DEBUG
    LOG.print (LAMP_NAME);
    LOG.printf_P(PSTR(" | Auto Swap Effects %02d |\n"), currentMode);
#endif
  }
}

// ========================================
void changeBrightness(bool Direction) {
  global_br = constrain(Direction ? global_br + 5 : global_br - 5, 10, 100);
  //#ifdef USE_OLED#endif
  setGlobalBrightness(modes[currentMode].Brightness, true);

#ifdef USE_MULTIPLE_LAMPS_CONTROL
  multipleLampControl();
#endif  //USE_MULTIPLE_LAMPS_CONTROL

#ifdef GENERAL_DEBUG
  LOG.printf_P(PSTR("Нове значення глобальної яскравості: %d | br_eff • %d\n\r"), global_br, modes[currentMode].Brightness);
#endif
}

// ========================================
void runOTA() {
#ifdef OTA
  if (otaManager.RequestOtaUpdate()) {
    ONflag = true;
    currentMode = EFF_MATRIX;                             // принудительное включение режима "Матрица" для индикации перехода в режим обновления по воздуху
    changePower();
    bitSet(notifications, 4);

    jsonWrite(configSetup, "notifications", notifications);
    saveConfig();
#ifdef  JAVELIN_VII
    showOtaUpdate();
#endif
#ifdef  JAVELIN
    digitalWrite(OTA_PIN, LOW);
    leds[NUM_LEDS + ROUND_MATRIX] = CHSV{96U, 255U, 255U};
    leds[NUM_LEDS + ROUND_MATRIX + 2] = CHSV{96U, 255U, 255U};
#ifdef BACKLIGHT_PIN
    digitalWrite(BACKLIGHT_PIN, LOW);
#endif
#endif
  }
#endif
}


// ========================================
void smartLampOff(uint8_t timeout ) {
#ifdef BUTTON_CAN_SET_SLEEP_TIMER
  ONflag = true;
  changePower();
  settChanged = true;
  eepromTimeout = millis();
  TimerManager::TimeToFire = millis() + timeout * 60UL * 1000UL;
  TimerManager::TimerRunning = true;

#if (defined(JAVELIN_VII))
  showWait();
#endif
  showWarning(CRGB::Red, 1000, 250U);
#endif //BUTTON_CAN_SET_SLEEP_TIMER
}


// JAVELIN MODE ========================
#if (defined(JAVELIN_VII) | defined(JAVELIN))

// ======================================
void javelinConnect() {
  CRGB color = (espMode == 0U) ? CRGB::Blue : CRGB::Green;
  // FastLED.setBrightness(20);
  const byte DELAY = 50;
  progress = 0;
#ifdef USE_OLED
  display.clearDisplay();
  if (espMode) {
    display.drawBitmap(48, 8, logo_wifi16, 32, 16, 1);
  } else {
    display.drawBitmap(60, 7, logo_ap, 8, 18, 1);
  }
  display.setCursor(0, 36);
  printlnTextCenter(espMode ? "CONNECT" : "AP MODE", 1, SH110X_WHITE);
  display.display();
#endif

  for (int8_t i = 0; i < 16; i++) {
#ifdef JAVELIN_VII
    fadeToBlackBy (leds_info, ROUND_MATRIX, 160);
#else
    fadeToBlackBy (leds, NUM_LEDS + ROUND_MATRIX, 160);
#endif

    if  (progress % 4U == 0) {
      progress = 0;
      // fillLedInfo(CRGB::Black);
      FastLED.delay(1);
      for (int8_t j = 5; j < 20; j += 2) {
#ifdef USE_OLED
        display.drawCircle(display.width() / 2, 16, j - 4, SH110X_BLACK);
        if (j < 17) display.drawCircle(display.width() / 2, 16, j, SH110X_WHITE);
        if (espMode) {
          display.fillRect(48, 7, 32, 18, SH110X_BLACK);
          display.drawBitmap(48, 8, logo_wifi16, 32, 16, 1);
        } else {
          display.fillRect(59, 6, 10, 20, SH110X_BLACK);
          display.drawBitmap(60, 7, logo_ap, 8, 18, 1);
        }
        display.display();
#endif
        delay(40);

#ifdef JAVELIN_VII
        fadeToBlackBy (leds_info, ROUND_MATRIX, 20);
#else
        fadeToBlackBy (leds, ROUND_MATRIX, 20);
#endif
      }
    }

    if (i < 12) {
#ifdef JAVELIN_VII
      leds_info[3 - progress] = color;
      leds_info[4 + progress] = color;
      leds_info[11 - progress] = color;
      leds_info[12 + progress] = color;
      leds_info[19 - progress] = color;
      leds_info[20 + progress] = color;
#else
      // uint8_t br = 64U;
      leds[NUM_LEDS + 7 + progress] = color;
      leds[NUM_LEDS + 8 - progress] = color;
      leds[NUM_LEDS + 18 + progress] = color;
      leds[NUM_LEDS + 19 - progress] = color;
      leds[NUM_LEDS + 29 + progress] = color;
      leds[NUM_LEDS + 30 - progress] = color;
#endif
    }
    progress++;
    FastLED.delay(1);
    delay(DELAY);
  }
}

// =====================================
void drawLogo( uint32_t ink) {
  // expects HEIGHT >= 28 && WIDTH >= 10
  static const uint8_t data[28][7] PROGMEM = {
    // N
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 1, 1, 1, 1, 0 },
    { 0, 1, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 1, 1, 1, 0 },
    // I
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 1, 1, 1, 0 },
    // L
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 1, 0 },
    { 0, 1, 1, 1, 1, 1, 0 },
    // E
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 0, 0, 0, 1, 0 },
    { 0, 1, 0, 1, 0, 1, 0 },
    { 0, 1, 1, 1, 1, 1, 0 },
    // V
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 1, 1, 0, 0 },
    { 0, 0, 0, 0, 0, 1, 0 },
    { 0, 1, 1, 1, 1, 0, 0 },
    // A
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 1, 1, 1, 0 },
    { 0, 1, 0, 1, 0, 0, 0 },
    { 0, 0, 1, 1, 1, 1, 0 },
    // J
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 1, 1, 1, 0 },
    { 0, 0, 0, 0, 0, 1, 0 },
    { 0, 0, 0, 1, 1, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 }
  };
  uint8_t posX = CENTER_Y_MINOR - 10;
  uint8_t posY = floor(HEIGHT * 0.5) + ((HEIGHT > 32) ? 14 : 13);
  uint32_t color;
  for (uint8_t y = 0; y < 28; y++) {
    for (uint8_t x = 0; x < 7; x++) {
      color = (pgm_read_byte(&(data[y][x]))) ? ink : 0x000000;
      leds[XY(posX + x, posY - y)] = color;
    }
  }
}

// =====================================
void Camouflage(uint8_t theme) {
  const byte OFFSET_Y = (HEIGHT > 32) ? 2 : 1;
  const uint8_t STEP = 2;
  CRGB color;

  if (theme > 2) theme = 1;

  drawRec(0, 0, WIDTH, HEIGHT, CRGB::Black);
  byte maxPos = HEIGHT / STEP;

  for (uint8_t x = 0; x < maxPos; x++) {
    for (uint8_t y = 0; y < maxPos; y++) {

      color = CRGB(pgm_read_dword(&(camouflageColors[theme][random8(5)])));
      keepColor(color, 2); /* нормалізуємо кольори з низькою яскравістю */

      for (uint8_t xx = 0; xx < STEP; xx++) {
        for (uint8_t yy = 0; yy < STEP; yy++) {
          drawPixelXY(x * STEP + xx, y * STEP + yy + OFFSET_Y, color);
        }
      }
    }
  }

  if ((HEIGHT >= 28) && (WIDTH >= 10)) {
    drawLogo(0x101015);
  }
}




// ======================================
void JavelinStatic(uint8_t val) {
  const byte BR = 128;
#ifdef GENERAL_DEBUG
  // LOG.printf_P(PSTR("JavelinStatic : %02d |\n\r"), val);
#endif
  //#ifdef JAVELIN_VII
  //  if (val > 0) {
  //    showTarget(255, (val == 2));
  //  }
  //#endif

  switch (val) {
    case 0: // close ----
#ifdef JAVELIN
      JavelinLight(0x000000, 0x000000, 0x000000);
      digitalWrite(MB_LED_PIN, HIGH);
#endif
      lendLease = false;
      runEffect();
      ONflag = true;
      break;
    case 1: // open -----
      lendLease = true;
      FPSdelay = 250U;
      ONflag = true;
      step = 0;
      FastLED.setBrightness(BR);
      Camouflage(camouflage);
      FastLED.show();
#ifdef JAVELIN
      DrawLevel(0, ROUND_MATRIX, ROUND_MATRIX, CHSV {0, 0, 0});
#endif

#ifdef JAVELIN_VII
      showMsg(icon_rocket24, 5, 24, 24, "BIT IN PROGRESS", "", 1, 2);
      delay(2000);
      showMsg(icon_rocket24, 5, 24, 24, "SEEKER COOLING", "", 1, 2);
      delay(5000);
      showMsg(icon_rocket24, 5, 24, 24, "READY", "", 1, 2);
      oledON(20);
      progress = 0;
#endif
      break;
    case 2: // fire -----
      if (step == 0) {
        // restore lamp state ---
        if (lendLease == false) {
          ONflag = false;
          FastLED.setBrightness(BR);
          Camouflage(camouflage);
        }
#ifdef JAVELIN_VII
        showTarget(255, true);
#endif
#ifdef JAVELIN
        digitalWrite(MB_LED_PIN, LOW);
#endif
        lendLease = true;
        FPSdelay = 5U;
        ONflag = true;
        step = 4;
      }
      break;

    case 3: // done diagnostic -----
#ifdef JAVELIN
      digitalWrite(MB_LED_PIN, HIGH);
#endif
      diagnostic = false;
      // ONflag = false;
      lendLease = true;
      // FastLED.setBrightness(BR);
      Camouflage(camouflage);
      FPSdelay = 5U;
      step = 0;
      FastLED.show();
      break;

    case 4: // fire guard -----
#ifdef JAVELIN_VII
      showMsg(icon_rocket24, 5, 24, 24, "FIRING INHIBIT", "", 1, 2);
#endif
      break;
    case 5: // show target -----
#ifdef JAVELIN_VII
      showTarget(255, false);
#endif
      break;
  }
}

// ======================================
void Javelin() {
  if (diagnostic) {
    FPSdelay = 50U;
    if (progress < 100) progress++;
    byte led_max = floor(progress / 4);
    if (progress > 97) led_max = 24;
#ifdef JAVELIN_VII
    for (uint8_t x = 0; x < led_max; x++) {
      leds_info[x] = CHSV(210, 255, 255);
    }
#endif
    JavelinDiagnostic(progress);
    return;
  }
  CRGB next_color;
  if (step > 3) {
    // LOG.printf("Javelin | step %3d\n\r", step);
#ifdef JAVELIN_VII
    //    if (step == 4) {
    //      fillLedInfo(CHSV(160, 255, 200)); /* clear to black */
    //    }
    nscale8(leds_info, ROUND_MATRIX, 250);
    if (step < 128) {
      for (uint8_t x = 0; x < WIDTH; (x += 2)) {
        leds[XY(x, 0)] = CHSV(random8(10), 255, 255 - step * 2);
        leds[XY(x + 1, 1)] = CHSV(random8(10), 255, 255 - step * 2);
        leds[XY(x, 1)] = CHSV(random8(10), 255, 255 - step * 2);
      }
      //      if (step == 4)  fillLedInfo(CHSV(250, 255, 255));
      if (step == 4)  fillLedInfo(CHSV(160, 255, 0));
      if (step == 20)  {
        //fillLedInfo(CHSV(0, 255, 128));
        fillLedInfo(CHSV(255, 255, 180));
        /* restore oled display */
        showTarget(255, false);
      }
    }
    if (step > 48) {
      next_color = CHSV(255, 255, ((step >= 127) ? 255 - step * 2 : step * 2));
      fillLedInfo(next_color);

    }
#endif

#ifdef JAVELIN
    if (step < 128) {
      for (uint8_t x = 0; x < WIDTH; (x += 2)) {
        leds[XY(x, 0)] = CHSV(random8(10), 255, 255 - step * 2);
      }
      if (step == 4)   JavelinLight(0xFF0000, 0xFF0000, 0xFF0000);
      if (step == 20)  JavelinLight(0x000000, 0x7F5F00, 0x000000);
    }
    if (step > 48) {
      next_color = CHSV(255, 255, ((step >= 127) ? 255 - step * 2 : step * 2));
      for (uint8_t x = 0; x < ROUND_MATRIX; (x++)) {
        leds[NUM_LEDS + x] =  next_color;
      }
    }
#endif

    step += 4;
    if (step >= 250) {
      // JavelinStatic(1);
      step = 0;
#ifdef JAVELIN_II
      nscale8(leds_info, ROUND_MATRIX, ROUND_MATRIX_BRIGHTNESS);
      showTarget(255, false);
#else
      digitalWrite(MB_LED_PIN, HIGH);
#endif
      FPSdelay = 254U;
    }
  }

}

// ======================================
void JavelinDiagnostic(uint8_t val) {
  //  if (val > 101) {
  //    diagnostic = false;
  //    // delay(1000);
  //    JavelinStatic(1);
  //    progress = 255;
  //    showMsg(icon_set24, 0, 24, 24, "diagnostic", "passed", 2, 2);
  //    return;
  //  } else { }

#ifdef JAVELIN_VII
  if (val < 101) showMsg(icon_set24, 0, 24, 24, "diagnostic", String(val) + "%", 2, 1);
#endif
#ifdef GENERAL_DEBUG
  LOG.printf_P(PSTR("JavelinDiagnostic : %02d |\n\r"), val);
#endif
  CHSV color = CHSV(2.5 * (val + 1), 255, 255 );
  switch (val) {
    case 0:
#ifdef JAVELIN_VII
      fillLedInfo(CRGB::Black);
#endif
      FastLED.setBrightness(200);
      JavelinStatic(1);
#ifdef JAVELIN
      digitalWrite(MB_LED_PIN, LOW);
#endif
      break;
    case 1:
#ifdef JAVELIN_VII
      fillLedInfo(CRGB::Black);
#endif
      break;
    case 10:
#ifdef JAVELIN
      digitalWrite(OTA_PIN, LOW);
#endif
      break;
    case 20:
#ifdef JAVELIN
      digitalWrite(OTA_PIN, HIGH);
#endif
      Camouflage(0);
      break;
    case 30:
      Camouflage(1);
      break;
    case 40:
      Camouflage(2);
      break;
    case 50:
      drawRec(0, 0, WIDTH, HEIGHT, CRGB::Black);
      /* x | y | x1 y1 | start_color | end_color | start_br | end_br | saturate | angleDeg */
      gradientRect(CENTER_X_MINOR - 1, HEIGHT - 8, CENTER_X_MAJOR + 1, HEIGHT - 8, 150U, 150U, 100U, 28U, 200U, 0);
      gradientRect(CENTER_X_MINOR - 1, HEIGHT - 9, CENTER_X_MAJOR + 1, HEIGHT - 9, 50U, 50U, 28U, 100U, 200U, 0);
      hue = 0;
      break;
    case 60:
      break;
    case 70:
      break;
    case 80:
      break;
    case 90:
      break;
    case 95:
      break;
    case 100:
      diagnostic = false;
#ifdef JAVELIN
      digitalWrite(MB_LED_PIN, HIGH);
      DrawLevel(0, 35, 35, CHSV{180, 255, MATRIX_LEVEL});
#endif
      FastLED.show();
      delay(2000);
      // clear indicator ---
      // ONflag = false;
      Camouflage(camouflage);
#ifdef JAVELIN
      JavelinLight(0x000000, 0x000000, 0x000000);
      DrawLevel(0, 35, 35, CHSV{0, 255, 0});
#endif
#ifdef JAVELIN_VII
      fillLedInfo(CRGB::Black);
      //    JavelinStatic(1);
      progress = 101;
      showMsg(icon_set24, 0, 24, 24, "diagnostic", "passed", 1, 2);
#endif
      FastLED.show();
      delay(1000);
      JavelinStatic(3);
      // progress = 0;
      return;
  }

#ifdef JAVELIN
  JavelinLight(color, color, color);
  DrawLevel(0, floor(val / 2.85), 35, CHSV{180, 255, MATRIX_LEVEL});
#endif
  ColoritShow(val);
  FastLED.show();
}

// ======================================
void ColoritShow(uint8_t val) {
  CHSV color1 = CHSV(250, 255, 100);
  CHSV color2 = CHSV(250, 255, 32);
  const byte OFFSET = (HEIGHT > 32) ? 2 : 0;
  if (val < 50) {
    return;
  }
  uint8_t y = val - 49;
  if (y >= HEIGHT) {
    if (val < 95) {
      //  nscale8(leds, NUM_LEDS, 128);
    }
    return;
  }
  if ((y < HEIGHT - 11) || (y > HEIGHT - 6)) {
    if (y > OFFSET) {
      if (val % 2) {
        leds[XY(CENTER_X_MINOR, y)] = color2;
        leds[XY(CENTER_X_MAJOR, y)] = color2;
        leds[XY(CENTER_X_MINOR - 1, y)] = color2;
        leds[XY(CENTER_X_MAJOR + 1, y)] = color2;
      } else {
        leds[XY(CENTER_X_MINOR, y)] = color1;
        leds[XY(CENTER_X_MAJOR, y)] = color1;
        leds[XY(CENTER_X_MINOR - 2, y)] = color2;
        leds[XY(CENTER_X_MAJOR + 2, y)] = color2;
      }
    }
  } else {
    hue += 16U;
    /* x | y | x1 y1 | start_color | end_color | start_br | end_br | saturate | angleDeg */
    gradientRect(CENTER_X_MINOR - 2, HEIGHT - 7, CENTER_X_MAJOR + 2, HEIGHT - 8, 150U, 150U, 100 + hue, 28 + hue, 200U, 15);
    gradientRect(CENTER_X_MINOR - 2, HEIGHT - 10, CENTER_X_MAJOR + 2, HEIGHT - 9, 50U, 50U, 28 + hue, 100 + hue, 200U, 15);
  }
  if (val == 100) Camouflage(camouflage);
}

#else
/* ==== STANDART LAMP CONSTRUCTION ==== */
// ======================================
// espModeStat default lamp start effect
// ======================================
void  espModeState() {
  CRGB color = (espMode == 0U) ? CRGB::Blue : CRGB::Green;
  if (loadingFlag) {
    loadingFlag = false;
    step = deltaValue;
    deltaValue = 1;
    hue2 = 0;
    deltaHue2 = 1;
    DrawLine(CENTER_X_MINOR, CENTER_Y_MINOR, CENTER_X_MAJOR + 1, CENTER_Y_MINOR, CHSV(color, 255, 210));
    DrawLine(CENTER_X_MINOR, CENTER_Y_MINOR - 1, CENTER_X_MAJOR + 1, CENTER_Y_MINOR - 1, CHSV(color, 255, 210));
    // setModeSettings(128U, 128U);
    pcnt = 1;
    FastLED.clear();
  }
  if (pcnt > 0 & pcnt < 200) {
    if (pcnt != 0) {
      pcnt++;
    }

    // animation esp state ===========
    //    dimAll(108);
    nscale8(leds, NUM_LEDS, 108);

    //    if (step % 2 == 0) {
    uint8_t w = validMinMax(hue2, 0, WIDTH / 2 - 1);
    uint8_t posY = validMinMax(CENTER_Y_MINOR + deltaHue2, 0, HEIGHT - 1);
    DrawLine(CENTER_X_MINOR - w, posY, CENTER_X_MAJOR + w, posY, CHSV(color, 255, (210 - deltaHue2)));
    posY = validMinMax(CENTER_Y_MINOR - 1 - deltaHue2, 1, HEIGHT - 1);
    DrawLine(CENTER_X_MINOR - w, posY, CENTER_X_MAJOR + w, posY, CHSV(color, 255, (210 - deltaHue2)));

    if (deltaHue2 == 0) {
      deltaHue2 = 1;
    }
    hue2++;
    deltaHue2 = deltaHue2 << 1;
    if (deltaHue2 == 2) {
      deltaHue2 = deltaHue2 << 1;
    }
    if (CENTER_Y_MINOR + deltaHue2 > HEIGHT) {
      deltaHue2 = 0;
      hue2 = 0;
    }
    // LOG.printf_P(PSTR("espModeState | pcnt = %05d | deltaHue2 = %03d | step %03d | ONflag • %s\n"), pcnt, deltaHue2, step, (ONflag ? "TRUE" : "FALSE"));
  } else {
    color = 255;         // если время не получено, будем красным цветом мигать
    // color = 176U; иначе скромно синим - нормальная ситуация при отсутствии NTP
    // animtion no time -----------
    leds[XY(CENTER_X_MINOR , 0U)] = CHSV( color, 255, (step % 4 == 0) ? 200 : 128);

  }
  // clear led lamp ---------------
  if ( pcnt >= 100) {
    pcnt = 0;
    //    FastLED.clear();
    //    FastLED.delay(2);
    FastLED.clear();
    delay(2);
    FastLED.show();
    loadingFlag = false;
  }
  step++;
}

#endif
