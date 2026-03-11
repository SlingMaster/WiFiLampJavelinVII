// usage_cpu.ino
#ifdef USE_CPU_USAGE

// ======================================
void CPUUsageVisualiser(char *packetBuffer) {
  static const uint8_t colorMap[4] PROGMEM =  {140, 92, 20, 1};
  static byte lastColor;
  static byte colorDisplay = 0;
  const byte TRANSFER_START = 0x0E;
  const byte TRANSFER_END = 0x0F;

  byte cmd = packetBuffer[3];
  byte visID = packetBuffer[4];
  const byte OFFSET = (HEIGHT > 32 && visID == 2) ? 2 : 0;
  // percent cpu usage ---------
  byte level = packetBuffer[5];

  // bar height ----------------
  byte bar_h = ((HEIGHT - OFFSET) * level / 100);
  // ai ------------------------
  byte ai = packetBuffer[6];
  // ---------------------------

  //  EVERY_N_SECONDS(1) {
  //    LOG.printf("cpu usage • %d%% | bar_h • %d | visID • %d\n\r", level, bar_h, visID);
  //  }

  switch (cmd) {
    case TRANSFER_START:
      ff_z = 200;
      ff_y = 4;
      ff_x = 0;
      FPSdelay = LOW_DELAY;
      FastLED.setBrightness(20U);
      deltaHue2 = 1;
      break;
    case TRANSFER_END:
      stopTransfer();
      return;
    default:
      // reset delay ---
      ff_y = 4;
      byte idx = level / 25;
      if (idx > 3) idx = 3;
      lastColor = pgm_read_byte(&colorMap[idx]);
      if (FastLED.getBrightness() != 20U) FastLED.setBrightness(20U);
      break;
  }

  if (colorDisplay < lastColor) {
    colorDisplay ++;
  }

  if (colorDisplay > lastColor) {
    colorDisplay --;
  }

  // visualization type --------
  switch (visID) {
    /* BARS */
    case 0:
      usageLevel(cmd, level, bar_h, colorDisplay);
      break;
    /* SPRING */
    case 1:
      usageSpringLevel(cmd, level, bar_h, colorDisplay);
      break;
    /* USAGE RECORD */
    case 2:
      usageLevelRec(cmd, level, bar_h, colorDisplay, ai);
      break;
    /* FIRE */
    case 3:
      visFire(cmd, level, colorDisplay);
      break;
    default:
      break;
  }

#ifdef JAVELIN
  DrawLevel(0, 35, 35, CHSV {colorDisplay, 255, 255});
#endif

#ifdef JAVELIN_VII
  showUsageCPU(level);
#endif

  // ---------------------------
  FastLED.show();
#ifdef GENERAL_DEBUG
  //  EVERY_N_SECONDS(1) {
  //    LOG.printf_P(PSTR("CPUUsage • | vis %2d | usage %3d | bar_h %2d |\n\r"), visID, level, bar_h);
  //  }
#endif
}

// ======================================
void showValue(byte cmd, byte y, byte level, CHSV color) {
  const byte TRANSFER_DIG = 0x01;
  if (cmd == TRANSFER_DIG) {
    drawRec(CENTER_X_MINOR - 3, y - 1, CENTER_X_MINOR + 6, y + 6, CHSV(255, 255, 0));
    drawDig(CENTER_X_MINOR - 2, y, level / 10U % 10U, color);
    drawDig(CENTER_X_MINOR + 2, y, level % 10U, color);
  }
}

// ======================================
void stopTransfer() {
  /* reset extCtrl if not used */
  ff_z = 1;
  ff_y = 4; 
  extCtrl = 0;
  FastLED.clear();
#ifdef JAVELIN
  DrawLevel(0, 35, 35, CHSV {0, 255, 0});
#endif
  FastLED.show();
  runEffect();
  // LOG.printf_P(PSTR("stopTransfer • ff_z %3d | ff_y %3d \n\r"), ff_z, ff_y);
}

// ======================================
void usageSpringLevel(byte cmd, byte level, byte bar_h, byte colorDisplay) {
  static const uint8_t colorMap[5] PROGMEM =  {160, 144, 32, 240, 0};
  const byte colorStep = 128 / HEIGHT;
  const byte pixStep = max<uint8_t>(1, (NUM_LEDS + 159) / 160);
  static uint32_t lvl;
  bool low;
  // ------------------------------------

  low = abs(lvl - ff_x) < pixStep + 1;
  if (lvl > ff_x) {
    ff_x += (low ? 1 : pixStep);
  } else {
    if (ff_x > 0U)
      if (lvl < ff_x) ff_x -= (low ? 1 : pixStep);
  }
  lvl = (level * NUM_LEDS / 100);
  // LOG.printf_P(PSTR(" usageLineLevel | level • %2d | lvl %6d | ff_x %6d \n\r"), level, lvl, ff_x);

  for (uint16_t i = 0U; i < NUM_LEDS; i++) {
    byte br = (i > ff_x) ? 0 : 255U;
    hue = 128;
    byte x = i % WIDTH;
    byte y = i / WIDTH;
    byte delta = y * colorStep;
    drawPixelXY(x, y, CHSV(hue + delta, 255U - (64 - delta / 2), br));
  }
  showValue(cmd, HEIGHT - 9, level, CHSV(colorDisplay, 255, 200));
}

// ======================================
void usageLevel(byte cmd, byte level, byte bar_h, byte colorDisplay) {
  static const uint8_t colorMap[5] PROGMEM =  {160, 144, 32, 240, 0};
  const byte colorStep = 96 / HEIGHT;
  if (bar_h > deltaHue2) {
    deltaHue2++;
  } else {
    if (deltaHue2 > 0U)
      if (bar_h < deltaHue2) deltaHue2--;
  }
  byte cur_val = deltaHue2;

  for (uint8_t x = 0U; x < WIDTH; x++) {
    for (uint8_t y = 0U; y < HEIGHT; y++) {
      byte br = 64 + y * 160 / HEIGHT;
      hue = 144;
      byte delta = (y == cur_val) ? 0 : y * colorStep;
      if (y == cur_val) {
        drawPixelXY(x, y, CHSV(0U, 0U, 255U));
      } else {
        if (cur_val == HEIGHT - 1) {
          drawPixelXY(x, y, CHSV(hue + delta, 255U, 255U));
        } else {
          drawPixelXY(x, y, CHSV(hue + delta, 255U, (y > cur_val - 2) ? 0U : 255U));
        }
      }
    }
  }
  showValue(cmd, HEIGHT - 9, level, CHSV(colorDisplay, 255, 200));
}

// ======================================
void usageLevelRec(byte cmd, byte level, byte bar_h, byte colorDisplay, byte ai) {
  static const uint32_t colorPaper = 0x00042f;
  static const uint32_t colorPaper2 = 0x000800;
  static byte posX = WIDTH > 8 ? WIDTH - WIDTH / 4 : WIDTH - 1;
  static byte posY;
  const byte OFFSET = HEIGHT > 32 ? 2 : 0;

  EVERY_N_MILLIS(150) {
    if (bar_h > deltaHue2) deltaHue2++;
    if (bar_h < deltaHue2) deltaHue2--;

    byte cur_val = deltaHue2;

    // create paper ------
    for (uint8_t y = 0; y < HEIGHT; y++) {
      for (uint8_t x = 0U; x < WIDTH; x++) {
        if (step % 2) {
          drawPixelXY(x, y + OFFSET / 2,  (x % 2) ? colorPaper2 : colorPaper);
        } else {
          drawPixelXY(x, y + OFFSET / 2,  (x % 2) ? colorPaper : colorPaper2);
        }
        if (x == posX && y == cur_val) {
          drawPixelXY(x + 1, y + OFFSET, 0xFFFFFF);
          if (y > OFFSET) drawPixelXY(x, (y - 1), 0x000000);
          line[posX] = cur_val;
        }
      }
    }

    step++;

    // draw grid and emuletion scroll to left ----
    for (uint8_t x = 0U; x < posX; x++) {
      drawPixelXY(x, (line[x] + OFFSET), 0xFF003F);
      drawPixelXY(x, (line[x] + OFFSET - 1), 0x000000);
      line[x] = line[x + 1];
    }
  }

  if (ai == 1) {
    posY = abs(CENTER_Y_MAJOR - deltaHue2 * 0.25) + 7;
    showValue(cmd, posY, level, CHSV(colorDisplay, 205, 255));
  } else {
    showValue(cmd, HEIGHT - 9, level, CHSV(colorDisplay, 205, 255));
  }
  if (OFFSET > 0) {
    DrawLine(0, 0, WIDTH - 1, 0, CHSV(colorDisplay, 205, 255));
  }
}

// ======================================
void visFire(byte cmd, byte level, byte colorDisplay) {
  pcnt = 32; // control from vis
  fireRoutine(colorDisplay);
  showValue(cmd, HEIGHT - 7, level, CHSV(255, 0, 160));
}

#endif
