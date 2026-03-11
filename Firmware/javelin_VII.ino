/* javelin_VII.ino  ----- */
#ifdef  JAVELIN_VII
uint8_t br_led;
uint8_t sp_led;
uint8_t sc_led;

// ======================================
// ======================================
void DrawLevel(uint8_t startPos, uint8_t val, uint8_t maxVal, uint8_t color) {
  uint16_t index;
  index = startPos;
  for (int x = 0; x < maxVal; x++) {
    leds_info[index + x] = val <= x ? CHSV{0, 255, 0} : CHSV{color, (color == 64) ? 128 : 255, 255};
  }
}

// ======================================
void StateLampIndicator() {
  uint8_t DELAY;
  uint8_t val;
  uint8_t bar_size = 8;     // max levels for speed | brightness | scale
  uint8_t br_level = 128;   // NormalizeBrightness();
  //  nscale8(leds_info, ROUND_MATRIX, ROUND_MATRIX_BRIGHTNESS);
  //---------------------------------------
  // LOG.println("• StateLampIndicator •");
  DELAY = map(modes[currentMode].Speed, 24, 254, 1U, 10U);
  // speed ------
  val = floor((256 - FPSdelay) * bar_size / 255);
  if (((level_timeout % DELAY) == 0U) & (sp_led != val)) {
    if (sp_led < val) {
      sp_led++;
    } else {
      sp_led--;
    }
    // LOG.printf_P(PSTR("     Speed : %03d | %02d\n\r"), modes[currentMode].Speed, val);
  }
  DrawLevel(0, sp_led, bar_size, 96);

  // brightness --
  val = gb ? bar_size : global_br * bar_size / 100;

  if (((level_timeout % DELAY) == 0U) & (br_led != val)) {
    if (br_led < val) {
      br_led++;
    } else {
      br_led--;
    }
  }
  DrawLevel(bar_size, br_led, bar_size, gb ? 240 : 64);

  // scale -------
  val = floor(modes[currentMode].Scale * bar_size / 100);
  if (((level_timeout % DELAY) == 0U) & (sc_led != val)) {
    if (sc_led < val) {
      sc_led++;
    } else {
      sc_led--;
    }
    // LOG.printf_P(PSTR("     Scale : %03d | %02d\n\r"), modes[currentMode].Scale, val);
  }
  DrawLevel(bar_size * 2, sc_led, bar_size, 160);
  nscale8(leds_info, ROUND_MATRIX, ROUND_MATRIX_BRIGHTNESS);
  level_timeout ++;
}

#endif
