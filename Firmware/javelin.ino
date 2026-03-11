// javelin.ino  ------

uint32_t level_timeout;  /* використовується і як tick, і як таймер (економія RAM) */

// ======================================
#ifdef  JAVELIN
uint8_t br_led;
uint8_t sp_led;
uint8_t sc_led;

// ======================================
void DrawLevel(uint8_t startPos, uint8_t val, uint8_t maxVal, CHSV color) {
  uint16_t index;
  index = NUM_LEDS + startPos;
  for (int x = 0; x < maxVal; x++) {
    leds[index + x] = val <= x ? CHSV{0, 255, 0} : color;
  }
}

// ======================================
void ClearLevelIndicator() {
  for (int x = 0; x < ROUND_MATRIX; x++) {
    leds[NUM_LEDS + x] = CHSV{0, 255, 0};
  }
}

// ======================================
//void VerticalScroll(byte direct) {
//  if (direct == 0U) {
//    return;
//  }
//  for (uint8_t x = 0U; x < WIDTH; x++) {
//    for (uint8_t y = HEIGHT; y > 0U; y--) {
//      if (direct == 1) {
//        // scroll up --------------
//        leds[XY( x, y)] = getPixColorXY(x, y - 1U);
//      }
//      if (direct == 2) {
//        // scroll down  -----------
//        leds[XY( x, HEIGHT - y)] = getPixColorXY(x, HEIGHT - y + 1U);
//      }
//    }
//  }
//}



// =====================================
void buttonJavelinTick() {
  touchJavelin.tick();
  uint8_t clickCount = touchJavelin.hasClicks() ? touchJavelin.getClicks() : 0U;

  // кнопка только начала удерживаться
  if (touchJavelin.isHolded()) {
    progress = 0;
    // LOG.printf_P("touchJavelin | isHolded");
  }

  // кнопка нажата и удерживается
  if (touchJavelin.isStep() || diagnostic) {
    int8_t but = touchJavelin.getHoldClicks();
#ifdef GENERAL_DEBUG
    // LOG.printf_P(PSTR("• Progress : %03d | %02d\n\r"), progress, but);
#endif
    // 1 click | start diagnostic =======
    JavelinDiagnostic(progress);
    if (progress == 110) {
      diagnostic = false;
      JavelinStatic(0);
    } else {
      delay((diagnostic ? 150 : 10));
      progress++;
    }
  }

  if (clickCount >= 1U) {
#ifdef GENERAL_DEBUG
    LOG.printf_P(PSTR("Button Javelin | click Count: %d\n\r"), clickCount);
#endif
  }

  // 1 click | fire ===================
  if (clickCount == 1U) {
    JavelinStatic(1);
    delay(200);
    JavelinStatic(2);
  }
  // 2 click | stop diagnostic ========
  if (clickCount == 2U) {
    JavelinDiagnostic(100);
    progress = 0;
    JavelinStatic(0);
  }

  // 3 click | work group =============
  if (clickCount == 3U) {
    if (ONflag) {
      WORKGROUP = (WORKGROUP == 1) ? 0 : 1;
      CRGB val = (WORKGROUP == 1) ? 0xFF00FF : 0x00FFFF;
      JavelinLight(val, val, val);
      DrawLevel(0, ROUND_MATRIX, ROUND_MATRIX, CHSV((WORKGROUP ? 192U : 128U), 255U, 255U));
      FastLED.show();
      delay(250U);
    }
  }
  // 4 click | set default camouflage =
  if (clickCount == 4U) {
    camouflage++;
    if (camouflage > 2) {
      camouflage = 0;
    }
    JavelinStatic(1);
    jsonWrite(configSetup, "camouflage", camouflage);
    delay(1000U);
    JavelinStatic(0);
  }
  // 5 click | reset default effects ==
  if (clickCount == 5U) {
    ResetDefaultEffects(true);
  }
}


//#ifdef JAVELIN
// ======================================
void JavelinLight(CRGB val1, CRGB val2, CRGB val3) {
  /* left */
  leds[ NUM_LEDS + ROUND_MATRIX] = val1;
  /* center */
  leds[ NUM_LEDS + ROUND_MATRIX + 1] = val2;
  /* right */
  leds[ NUM_LEDS + ROUND_MATRIX + 2] = val3;
}

// ======================================
uint8_t NormalizeBrightness() {
  int bri = constrain(200 - FastLED.getBrightness(), 48, 200);
  //  LOG.printf_P(PSTR("Brightness : %03d | REAL • %03d | BR • %d\n\r"), modes[currentMode].Brightness, FastLED.getBrightness(), bri);
  return bri;
}

// ======================================
void ShowStateIndicator(uint8_t br_level) {
  uint8_t bar_size = 10;
  static bool flag;


  if ((millis() - level_timeout >= 500U)) {
    level_timeout = millis();
    flag = !flag;
  }

  // ROUND_MATRIX -----
  if (eff_auto) {
    leds[NUM_LEDS] = flag ? 0x000000 : 0x7F007F;;
    leds[NUM_LEDS + 2] = !flag ? 0x000000 : 0x7F007F;;
  } else {
    leds[NUM_LEDS] = CHSV{0, 0, 0};
    leds[NUM_LEDS + 2] = CHSV{0, 0, 0};
  }

  // MARCKERS ---------
  if ((flag) & (WORKGROUP == 1U)) {
    leds[NUM_LEDS + 1] = CHSV{0, 205U, br_level * 0.7};
    leds[NUM_LEDS + bar_size + 3] = CHSV{172, 205U, br_level * 0.7};
    leds[NUM_LEDS + bar_size * 2 + 4] = CHSV{96, 205U, br_level * 0.7};
  } else {
    leds[NUM_LEDS + 1] = CHSV{0, 0, br_level * 0.7};
    leds[NUM_LEDS + bar_size + 3] = CHSV{0, 0, br_level * 0.7};
    leds[NUM_LEDS + bar_size * 2 + 4] = CHSV{0, 0, br_level * 0.7};
  }
  JavelinLight(0xB0A000, 0x000000, 0x001FFF);
}

// ======================================
void StateLampIndicator() {
  if (lendLease) {
    return;
  };
  uint8_t DELAY;
  uint8_t val;
  uint8_t bar_size = 10;  // max levels for speed | brightness | scale
  uint8_t br_level = NormalizeBrightness();
  //---------------------------------------

  DELAY = map(modes[currentMode].Speed, 30, 254, 1U, 10U);
  // speed ------
  val = floor((256 - FPSdelay) * bar_size / 255);
  if (((level_timeout % DELAY) == 0U) && (sp_led != val)) {
    if (sp_led < val) {
      sp_led++;
    } else {
      sp_led--;
    }
    //  LOG.printf_P(PSTR("     Speed : %03d | %02d\n\r"), modes[currentMode].Speed, val);
  }
  DrawLevel(3, sp_led, bar_size, CHSV{90, 255, br_level});
  // brightness --
  if (gb) {
    val = global_br * bar_size / 127;
  } else {
    val = floor(FastLED.getBrightness() * bar_size / 255);
  }

  if (((level_timeout % DELAY) == 0U) & (br_led != val)) {
    if (br_led < val) {
      br_led++;
    } else {
      br_led--;
    }
  }
  DrawLevel(bar_size + 4, br_led + 1, bar_size, CHSV{60, 200, br_level});
  // scale -------
  val = floor(modes[currentMode].Scale * bar_size / 100);
  if (((level_timeout % DELAY) == 0U) & (sc_led != val)) {
    if (sc_led < val) {
      sc_led++;
    } else {
      sc_led--;
    }
    //  LOG.printf_P(PSTR("     Scale : %03d | %02d\n\r"), modes[currentMode].Scale, val);
  }

  DrawLevel(bar_size * 2 + 5, sc_led, bar_size, CHSV{160, 255, br_level});
  ShowStateIndicator(br_level);
  level_timeout ++;
}

#endif
