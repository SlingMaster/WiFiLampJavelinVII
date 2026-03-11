/* display_oled.ino
  SH110X | i2c | 128x64
*/
#ifdef USE_OLED

struct StrData {
  const char* s0;
  const char* s1;
  const char* s2;
  const char* s3;
  const char* s4;
  const char* s5;
  const char* s6;
  const char* s7;
  const char* s8;
  const char* s9;
};

const char s_test_0[] PROGMEM = "test led matrix";
const char s_test_1[] PROGMEM = "test led color";
const char s_test_2[] PROGMEM = "COORDINATES";
const char s_test_3[] PROGMEM = "RED";
const char s_test_4[] PROGMEM = "GREEN";
const char s_test_5[] PROGMEM = "BLUE";
const char s_test_6[] PROGMEM = "RAINBOW";
const char s_test_7[] PROGMEM = "WHITE";
const char s_test_8[] PROGMEM = "PASSED";
const char s_test_9[] PROGMEM = "s9";

const char s_ver_0[] PROGMEM = "VERSION";
const char s_ver_1[] PROGMEM = "Made in Ukraine";
const char s_ver_2[] PROGMEM = "HOT WATER";
const char s_ver_3[] PROGMEM = "CPU USAGE";
const char s_ver_4[] PROGMEM = "UPDATE OTA";
const char s_ver_5[] PROGMEM = "motion control";
const char s_ver_6[] PROGMEM = "current time";
const char s_ver_7[] PROGMEM = "free memory";
const char s_ver_8[] PROGMEM = "s8";
const char s_ver_9[] PROGMEM = "s9";

const char s_set_0[] PROGMEM = "set brightness";
const char s_set_1[] PROGMEM = "set speed";
const char s_set_2[] PROGMEM = "set scale";
const char s_set_3[] PROGMEM = "s3";
const char s_set_4[] PROGMEM = "s4";
const char s_set_5[] PROGMEM = "s5";
const char s_set_6[] PROGMEM = "s6";
const char s_set_7[] PROGMEM = "s7";
const char s_set_8[] PROGMEM = "s8";
const char s_set_9[] PROGMEM = "s9";

const StrData strdataPROGMEM[] PROGMEM = {
  { s_test_0, s_test_1, s_test_2, s_test_3, s_test_4,
    s_test_5, s_test_6, s_test_7, s_test_8, s_test_9
  },

  { s_ver_0, s_ver_1, s_ver_2, s_ver_3, s_ver_4,
    s_ver_5, s_ver_6, s_ver_7, s_ver_8, s_ver_9
  },

  { s_set_0, s_set_1, s_set_2, s_set_3, s_set_4,
    s_set_5, s_set_6, s_set_7, s_set_8, s_set_9
  }
};

#define SHOW_DEF   ( 10U )
uint32_t oled_timer;
uint32_t oled_delay;
bool display_off = false;

// ======================================
inline void oledTick() {
  // LOG.printf("oledTick %d | %d | %d\n\r", millis(), oled_timer, oled_delay);
  if (millis() - oled_timer >= oled_delay) {
    // LOG.printf("\n\r• oledTick %d | \n\r\n\r", (millis() - oled_timer));
    oled_timer = millis();
    display.clearDisplay();
    display_off = true;
    display.display();
    fps = 0;
  }
}

// ======================================
void oledON(uint32_t interval) {
  oled_timer = millis();
  oled_delay = interval * 1000;
  sleep = true;
  display_off = false;
  display.display();
}

// ======================================
// FIX: function must not return byte (was UB), safe centering, no negative X
void printlnTextCenter(const String &txt, byte font_size, byte color) {
  display.setTextSize(font_size);
  display.setTextColor(color);

  // approximate text width: 6 px per char for Adafruit default font
  int16_t textWidth = txt.length() * 6 * font_size;
  int16_t x = (128 - textWidth) / 2;

  if (x < 0) x = 0; // prevent negative cursor

  display.setCursor(x, display.getCursorY() + 5);
  display.println(txt);
}

// ======================================
void showMenu(byte menu) {
  String title;
  const StrData* sd = &strdataPROGMEM[2];
  switch (menu) {
    case 0: title = FPSTR(sd->s0); break;
    case 1: title = FPSTR(sd->s1); break;
    case 2: title = FPSTR(sd->s2); break;
    default: title = "Unknown"; break;
  }
  String strTitle = title;
  strTitle.toUpperCase();
  showMsg(icon_set24, 1, 24, 24, "current menu", strTitle, 1, 2);
  oledON(SHOW_DEF);
}

// ======================================
void showMsg(const uint8_t *icon, uint8_t posY, uint8_t w_ico, uint8_t h_ico, String msg1, String msg2, uint8_t msg2_size, uint8_t lines) {
  const byte PADDING = 4;
  const byte DW = 128;
  display.clearDisplay();
  uint8_t posX = (DW - w_ico) * 0.5 - 1;
  uint8_t wl = (DW - w_ico) * 0.5 - PADDING * 2;
  if (lines > 0) {
    display.drawFastHLine(0, posY + h_ico * 0.5, wl, SH110X_WHITE );
    display.drawFastHLine((DW + w_ico) * 0.5 + PADDING + 2, posY + h_ico * 0.5, wl, SH110X_WHITE );
  }
  if (lines > 1) display.drawFastHLine(0, 63, 128, SH110X_WHITE );
  display.drawBitmap(posX, posY, icon, w_ico, h_ico, 1);
  display.setCursor(0, posY + h_ico + PADDING);
  if (msg1 != "") printlnTextCenter(msg1, 1, SH110X_WHITE);
  printlnTextCenter(msg2, msg2_size, SH110X_WHITE);
  display.display();
}

// ======================================
void showDevelopMode() {
  develop = !develop;
  String t_str = develop ? "ON" : "OFF";
  showMsg(ico_debug24, 0, 24, 24, "develop mode", t_str, 1, 2);
  oledON(SHOW_DEF);
}

// ======================================
// FIX: remove double clearDisplay, safe int math, no heap copy of String
void showCustomParameter(const uint8_t *icon, const String &title, const String &val) {
  const byte PADDING = 4;
  const byte DW = 128;
  const byte w_ico = 32;
  int16_t posX = (DW - w_ico) / 2;
  int16_t wl   = (DW - w_ico) / 2 - PADDING * 2;
  display.clearDisplay();
  if (wl < 0) wl = 0;
  display.drawBitmap(posX, 0, icon, w_ico, w_ico, 1);
  display.drawFastHLine(0, w_ico / 2, wl, SH110X_WHITE);
  display.drawFastHLine(posX + w_ico + PADDING, w_ico / 2, wl, SH110X_WHITE);
  display.setCursor(0, w_ico);
  printlnTextCenter(title, 1, SH110X_WHITE);
  printlnTextCenter(val, 2, SH110X_WHITE);
  display.display();
}

// ======================================
void showUsageCPU(byte val) {
  const StrData* sd = &strdataPROGMEM[1];
  String strVal = String(val) + "%";
  showCustomParameter(icon_cpu32, FPSTR(sd->s3), strVal);
}

// ======================================
void showTemperature(String temp) {
  const StrData* sd = &strdataPROGMEM[1];
  String strVal = temp + "\"C";
  // "hot water"
  showCustomParameter(icon_temp32, FPSTR(sd->s2), strVal);
  oledON(SHOW_DEF);
}

// ======================================
void showLogoDevice() {
  display.setTextColor(SH110X_WHITE);
  showMsg(logo_device, 0, 84, 64, "", "", 1, 0);
  oledON(SHOW_DEF);
}

// ======================================
void showTarget(uint16_t delay_interval, bool invert_img) {
  display.clearDisplay();
  display.invertDisplay(invert_img);
  display.drawBitmap(6, 4, bmp_target, 114, 54, 1);
  display.display();
  oledON(delay_interval);
}

// ======================================
void showOtaUpdate() {
  const StrData* sd = &strdataPROGMEM[1];
  // "Update OTA"
  showMsg(logo_brain48, 0, 48, 48, FPSTR(sd->s4), "", 1, 0);
  oledON(120);
}

// ======================================
void showMotionControl(byte flag) {
  String t_str =  flag ? "ON" : "OFF";
  const StrData* sd = &strdataPROGMEM[1];
  // "motion control"
  showMsg(ico_debug24, 0, 24, 24,  FPSTR(sd->s5), t_str, 1, 2);
  oledON(SHOW_DEF);
}

// ======================================
void showTime() {
  const StrData* sd = &strdataPROGMEM[1];
  if (TimerManager::TimerRunning || (notifications > 5) || lendLease)  return;
  time_t currentLocalTime = getCurrentLocalTime();
  String str_time = FomatXX(hour(currentLocalTime)) + ":" + FomatXX(minute(currentLocalTime));
  /* current time */
  showMsg(ico_null, 1, 8, 1, FPSTR(sd->s6) , str_time, 3, 0);
  oledON(SHOW_DEF);
}

// ======================================
void showWait() {
  if (extCtrl == 0U ) {
    showMsg(icon_wait32, 0, 32, 32, "SMART OFF", "", 1, 2);
    oledON(3600);
  }
}

// ======================================
void showEffectNum() {
  const uint8_t *icon = ico_eff32;
  if ((extCtrl == 0U) && ONflag) {
    if (eff_auto == 1) {
      icon = icon_cycle32;
    } else {
      icon = (custom_eff == 0) ? ico_eff32 : ico_motion32;
    }
    String txt = "Effect " + String(currentMode);
    String eff_name = getEffectName(currentMode);
    eff_name.toUpperCase();
    showMsg(icon, 0, 32, 32, txt, eff_name, 1, 1);
    oledON(SHOW_DEF);
    StateLampIndicator();
  }
}

// ======================================
void outIP(IPAddress ip) {
  if (espMode) {
    showMsg(logo_wifi16, 5, 32, 16, "LAMP IP", ip.toString(), 1, 2);
  } else {
    showMsg(logo_ap, 3, 8, 18, "LAMP IP", ip.toString(), 1, 2);
  }
  oledON(SHOW_DEF);
}

// ======================================
void showSetsParameter(const uint8_t *icon, byte title_id, uint8_t val, String unit) {
  const StrData* sd = &strdataPROGMEM[2];
  String title;
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 10);
  switch (title_id) {
    case 0: title = FPSTR(sd->s0); break;
    case 1: title = FPSTR(sd->s1); break;
    case 2: title = FPSTR(sd->s2); break;
    default: title = "Unknown"; break;
  }
  String strVal = String(val) + unit;
  showMsg(icon, 0, 24, 24, title, strVal, 2, 1);
  oledON(SHOW_DEF);
}



// ======================================
void showVersion() {
#ifdef JAVELIN_VII
  const StrData* sd = &strdataPROGMEM[1];
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 10);
  printlnTextCenter(FPSTR(sd->s0), 1, SH110X_WHITE); // VERSION
  printlnTextCenter(VERSION, 1, SH110X_WHITE);
  display.setCursor(0, 44);
  printlnTextCenter(FPSTR(sd->s1), 1, SH110X_WHITE); // Made in Ukraine
  display.display();
  delay(5);
#endif
}


// ======================================
String FomatXX(int val) {
  return (val < 10) ? "0" + String(val) : String(val);
}

// ======================================
void showSoundSpectr(char *packetBuffer) {
  display.clearDisplay();
  display.setCursor(0, 0);
  printlnTextCenter("waveform audio", 1, SH110X_WHITE);
  for (uint8_t x = 6; x < 48; x++) {
    display.drawPixel(10 + x * 2, 41, SH110X_WHITE);
    if (x > 10 && x < 37) {
      byte val = (48 * packetBuffer[x - 5]) / 100;
      display.drawFastVLine(18 + x * 2, 41 - val / 2, val, SH110X_WHITE);
    }
  }
  display.display();
}

// ======================================
void showFPS() {
  if (develop) {
    EVERY_N_SECONDS(1) {
      if (ONflag && fps < 60) {
        const byte PADDING = 4;
        const byte DW = 128;
        const byte w_ico = 24;
        display.clearDisplay();
        uint8_t posX = (DW - w_ico) / 2 - 1;
        uint8_t wl = (DW - w_ico) / 2 - PADDING * 2;
        display.drawBitmap(posX, 0, ico_debug24, w_ico, w_ico, 1);
        display.drawFastHLine(0, w_ico / 2, wl, SH110X_WHITE);
        display.drawFastHLine((DW + w_ico) / 2 + PADDING + 1, w_ico / 2, wl, SH110X_WHITE);
        display.setCursor(0, w_ico + PADDING);
        printlnTextCenter("free memory", 1, SH110X_WHITE);
        printlnTextCenter(String(system_get_free_heap_size()), 2, SH110X_WHITE);
        display.drawBitmap(100, 0, ico_fps, 24, 9, 1);
        display.fillRect(100, 14, 24, 16, SH110X_BLACK);
        display.setTextColor(SH110X_WHITE);
        if (custom_eff) {
          display.setCursor(106, 16);
          display.setTextSize(1);
        } else {
          display.setCursor(101, 15);
          display.setTextSize(2);
        }
        display.println(FomatXX(fps));
        display.display();
        fps = 0;
      }
    }
  }
}

// ======================================
void outHeap() {
  showMsg(ico_debug16, 1, 16, 16, "free memory ", String(system_get_free_heap_size()), 2, 2);
  oledON(SHOW_DEF);
}
#endif



#ifdef USE_ROBOT_007
// ======================================
void showSoundSpectr(char *packetBuffer) {
  display.clearDisplay();
  for (uint8_t x = 0U; x < 30; x++) {
    display.drawPixel(2 + x * 2, 24, SH110X_WHITE);
    if (x > 7 & x < 24) {
      byte val = (46 / 100.0) * packetBuffer[(x - 2)];
      display.drawFastVLine(x * 2, 24 - val / 2, val, SH110X_WHITE);
    }
  }
  display.display();
}

// ======================================
//void showPlayerCommand(String cmd) {
//  showIcon(5, 1);
//  display.setCursor(0, 34);
//  printlnTextCenter(cmd, 1, SH110X_WHITE);
//  delay(1000);
//  //
//  //  sleep_mode = false;
//  //  action_timeout = 95;
//  // activity = SHOW_DELAY;
//}
#endif
