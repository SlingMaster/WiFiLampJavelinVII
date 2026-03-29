/* выделяем в памяти масив для загружаемых бинарных изображений
  header= 16 | заголовок
  w=16, h=16 | ширина высота
  color  = 2 | байт на цвет
  frames = 5 | количество кадров
  масив на 5 кадров 16x16 | размером w * h * frames * color + header = 2 576
  размер можно увеличивать по мере надобности, постоянно занимает место в памятиdesigner

  возможно в будущем будет сделано динамическим
*/

// ======================================
// New Effects
// ======================================

// =============== Wine ================
//    © SlingMaster | by Alex Dovby
//    Головний консультант ChatGPT4
//                 2026
// =====================================
void drawPerlage(uint8_t level) {

  /* бульбашки з'являються з інтервалом коли достатньо вина в бокалі*/
  if (level < CENTER_Y_MINOR) return;
  // -------------------

  uint8_t posX = random8(WIDTH);

  for (uint8_t x = 0; x < WIDTH; x++) {
    /* старт бульбашки знизу */
    if ((noise3d[0][x][1] == 0U) && (x == posX) && (random8() & 1)) {
      noise3d[0][x][1] = 1;
    }
    /* рух вгору з можливим плавним дрейфом ±1 для нової бульбашки */
    for (int y = HEIGHT - 2; y > 0; y--) {
      uint8_t v = noise3d[0][x][y - 1];
      noise3d[0][x][y - 1] = 0;
      if (v) {
        /* з невеликим шансом ±1, але зберігаємо більшість колонок прямо */
        int newX = x;
        if (random8() < 20) { // 20% шанс зсунути
          newX = x + ((random8() & 1) ? 1 : -1);
          if (newX < 0) newX = 0;
          if (newX >= WIDTH) newX = WIDTH - 1;
        }
        noise3d[0][newX][y] = v;
        if (y < level) {
          leds[XY(newX, y)] = CHSV(40, 5U, random8(220U, 255U));
        }
      }
    }
  }
}

// ======================================
void drawWineBody(CRGBPalette16 palette, uint8_t level, bool filling) {
  const uint8_t OFFSET = CENTER_Y_MINOR / 2;
  const float goldenAngle = 2.39996323f;
  const uint8_t SVH = CENTER_X_MINOR;
  float t = millis() * 0.0012f;

  for (uint8_t x = 0; x < WIDTH; x++) {

    float surfaceWave =
      emitterX *
      (sinf(t * 0.8f + x * 0.35f) * 1.8f +
       sinf(t * 0.45f + x * 0.12f) * 0.8f);

    int effectiveLevel = (int)level + (int)surfaceWave;
    effectiveLevel = constrain(effectiveLevel, 0, HEIGHT - 1);

    for (uint8_t y = 0; y < HEIGHT; y++) {

      if (y > effectiveLevel) {
        drawPixelXY(x, y, CRGB::Black);
        continue;
      }

      float dx = (float)x - CENTER_X_MINOR;
      float dy = (float)y - CENTER_Y_MINOR - OFFSET;
      float dist = sqrtf(dx * dx * SVH + dy * dy);

      uint8_t idx = (uint8_t)(dist * 6.0f);
      CRGB col = ColorFromPalette(palette, idx);

      // мʼякий край поверхні поки заспокоюється хвиля
      if (y == effectiveLevel && emitterX > 0.1f) col.nscale8(150 + random8(20));

      keepColor(col, 2);
      drawPixelXY(x, y, col);
    }
  }
}

// ======================================
void renderWine(CRGBPalette16 palette, uint8_t level, uint8_t wine_type) {
  bool filling = (deltaValue < deltaHue);
  drawWineBody(palette, level, filling);
  if (wine_type < 2) drawPerlage(level); /* sparcling only */
}

// ======================================
void Wine() {
  const char* eff_name;
  const uint8_t DOWN_STEEP = HEIGHT / 8;

  if (loadingFlag) {
    IsRandomMode();
    FastLED.clear();
    ff_y = 0;
    pcnt = 0;
    emitterX = 0.0f;

    if (modes[currentMode].Scale > 90) hue2 = 0;
    else hue2 = modes[currentMode].Scale / 15;
  }

  switch (hue2) {
    case 0: currentPalette = WINE_Rose_gp;   eff_name = "Rose Sparkling"; break;
    case 1: currentPalette = WINE_WhiteS_gp; eff_name = "White Sparkling"; break;
    case 2: currentPalette = WINE_White_gp;  eff_name = "White Wine"; break;
    case 3: currentPalette = WINE_Orange_gp; eff_name = "Orange Wine"; break;
    case 4: currentPalette = WINE_Rose_gp;   eff_name = "Rose Wine"; break;
    case 5: currentPalette = WINE_Red_gp;    eff_name = "Red Wine"; break;
    case 6: currentPalette = WINE_RedOld_gp; eff_name = "Old Red Wine"; break;
    default: break;
  }

  EVERY_N_MILLISECONDS(50) {

    // ---------- ІМПУЛЬС ХВИЛІ ----------
    if (deltaValue < deltaHue) {          /*/ наповнення сильна хвиля */
      deltaValue++;
      emitterX += 0.10f;
    }
    else if (deltaValue > deltaHue) {     /* спад слабша хвиля */
      deltaValue--;
      emitterX += 0.06f;
    }

    // ---------- ОБМЕЖЕННЯ ----------
    if (emitterX > 1.0f) emitterX = 1.0f;

    // ---------- ЗАТУХАННЯ (РІЗНЕ) ----------
    /*              повільніше — хвиля тримається | повільне — видно при спаді */
    if (deltaValue < deltaHue) emitterX *= 0.94f; else emitterX *= 0.985f;
    if (emitterX < 0.001f) emitterX = 0.0f;

    // ---------- РЕНДЕР ----------
    renderWine(currentPalette, deltaValue, hue2);

    // ---------- ЛОГІКА РІВНЯ ----------
    if (ff_y == 1) {
      deltaHue = (hue2 < 2) ? HEIGHT - 3 : (HEIGHT * 0.75) + random8(HEIGHT / 8);
      ff_y++;
    }

    if (ff_y == 350) deltaHue = 0;

    if (ff_y > (350 + deltaValue)) {
      ff_y = 0;
      if (modes[currentMode].Scale > 90) hue2++;
      if (hue2 > 6) hue2 = 0;
    }

    if (ff_y > 17 && ff_y % 50 == 0 && deltaHue > DOWN_STEEP * 2) {
      deltaHue -= DOWN_STEEP;
      ff_y++;
    }
  }

  if (deltaValue == deltaHue) ff_y++;
  step++;
}

// ============== Swirl ================
//    © SlingMaster | by Alex Dovby
//              EFF_SWIRL
//--------------------------------------
void Swirl() {
  static uint8_t divider;
  static uint8_t lastHue;
  static const uint32_t colors[6][6] PROGMEM = {
    {CRGB::Blue, CRGB::DarkRed, CRGB::Aqua, CRGB::Magenta, CRGB::Gold, CRGB::Green },
    {CRGB::Yellow, CRGB::LemonChiffon, CRGB::LightYellow, CRGB::Gold, CRGB::Chocolate, CRGB::Goldenrod},
    {CRGB::Green, CRGB::DarkGreen, CRGB::LawnGreen, CRGB::SpringGreen, CRGB::Cyan, CRGB::Black },
    {CRGB::Blue, CRGB::DarkBlue, CRGB::MidnightBlue, CRGB::MediumSeaGreen, CRGB::MediumBlue, CRGB:: DeepSkyBlue },
    {CRGB::Magenta, CRGB::Red, CRGB::DarkMagenta, CRGB::IndianRed, CRGB::Gold, CRGB::MediumVioletRed },
    {CRGB::Blue, CRGB::DarkRed, CRGB::Aqua, CRGB::Magenta, CRGB::Gold, CRGB::Green }
  };
  uint32_t color;

  if (loadingFlag) {
    IsRandomMode();
    deltaValue = 255U - modes[currentMode].Speed + 1U;
    divider = constrain( (modes[currentMode].Scale - 1) / 20, 0, 5);
    deltaHue2 = 0U;                         // count для замедления смены цвета
    deltaHue = 0U;                          // direction | 0 hue-- | 1 hue++ |
    hue2 = 0U;                              // x
    hue = 0;
    FastLED.clear();
  }

  // задаем цвет и рисуем завиток --------
  color = pgm_read_byte(&(colors[divider][hue]));
  if (deltaHue2 < HEIGHT - 3) {
    drawPixelXY(hue2, deltaHue2 + 2, color);
  }
  drawPixelXY(hue2, deltaHue2, color);
  drawPixelXY(WIDTH - hue2, HEIGHT - deltaHue2, colors[divider + 1][hue]);
  // -------------------------------------

  hue2++;                     // x
  // два варианта custom_eff задается в сетапе лампы ----
  if (custom_eff == 1) {
    if (hue2 % 2 == 0) deltaHue2++;
  } else {
    deltaHue2++;
  }
  // -------------------------------------

  if  (hue2 > WIDTH)  hue2 = 0U; {
    if (deltaHue2 >= HEIGHT) {
      deltaHue2 = 0U;
      // new swirl ------------
      hue2 = random8(WIDTH - 2);
      // hue2 = hue2 + 2;
      // select new color -----
      hue = random8(6);

      if (lastHue == hue) {
        hue++;
        if (hue >= 6) hue = 0;
      }
      lastHue = hue;
    }
  }
  blurScreen(5U + random8(5));
  step++;
}

// -------------------------------------------
// for effect Ukraine
// -------------------------------------------
void drawCrest() {

  static const uint8_t crest_ua[9][5] PROGMEM = {
    {0, 0, 1, 0, 0 },
    {1, 0, 1, 0, 1 },
    {1, 0, 1, 0, 1 },
    {1, 0, 1, 0, 1 },
    {1, 0, 1, 0, 1 },
    {1, 1, 1, 1, 1 },
    {1, 0, 1, 0, 1 },
    {0, 1, 1, 1, 0 },
    {0, 0, 1, 0, 0 }
  };

  //  uint8_t posX = floor(WIDTH * 0.5) - 3;
  //  uint8_t posY = constrain(floor(HEIGHT * 0.4) + 5, 9, HEIGHT);

  uint8_t posX = CENTER_X_MAJOR - 3;
  uint8_t posY = constrain((HEIGHT * 2) / 5 + 5, 9, HEIGHT - 1);


  uint32_t color;
  FastLED.clear();
  for (int y = 0U; y < 9; y++) {
    for (int x = 0U; x < 5; x++) {
      drawPixelXY(posX + x, posY - y, pgm_read_byte(&(crest_ua[y][x])) ? CRGB(255, 0xD7, 0) : CRGB(0, 0, 0));
    }
  }
}

// ============== Ukraine ==============
//      © SlingMaster | by Alex Dovby
//              EFF_UKRAINE
//--------------------------------------
void Ukraine() {
  uint8_t divider;
  uint32_t color;
  static const uint16_t MAX_TIME = 500;
  uint16_t tMAX = 100;
  static const uint8_t timeout = 100;
  static const uint8_t blur_step = CENTER_Y_MAJOR;
  static const uint32_t colors[2][5] = {
    {CRGB::Blue, CRGB::MediumBlue, 0x0F004F, 0x02002F, 0x1F2FFF },
    {CRGB::Yellow, CRGB::Gold, 0x4E4000, 0xFF6F00, 0xFFFF2F }
  };

  // Initialization =========================
  if (loadingFlag) {
    IsRandomMode();
    drawCrest();
    // minspeed 200 maxspeed 250 ============
    // minscale   0 maxscale 100 ============
    deltaValue = 255U - modes[currentMode].Speed + 1U;
    step = deltaValue;                        // чтообы при старте эффекта сразу покрасить лампу
    deltaHue2 = 0U;                           // count для замедления смены цвета
    deltaHue = 0U;                            // direction | 0 hue-- | 1 hue++ |
    hue2 = 0U;                                // Brightness
    ff_x = 1U;                                // counter
    tMAX = 100U;                              // timeout
  }
  // divider = floor((modes[currentMode].Scale - 1) / 10);
  divider = modes[currentMode].Scale / 10;    // маштаб задает режим рестарта
  if (divider > 10) divider = 10;


  tMAX = timeout + 100 * divider;

  if ((ff_x > timeout - 10) && (ff_x < timeout)) { // таймаут блокировки отрисовки флага
    if (ff_x < timeout - 5) {                  // размытие тризуба
      blurScreen(beatsin8(5U, 60U, 5U));
    } else {
      blurScreen(230U - ff_x);
    }
  }

  if (ff_x > tMAX) {
    if (divider == 0U) {                       // отрисовка тризуба только раз
      ff_x = 0U;
      tMAX += 20;
    } else {
      if (ff_x > tMAX + 100U * divider) {      // рестар эффект
        drawCrest();
        ff_x = 1U;
      }
    }
  }
  if ((ff_x != 0U) || (divider > 0)) {
    ff_x++;
  }

  // Flag Draw =============================
  if ((ff_x > timeout) || (ff_x == 0U))  {     // отрисовка флага
    if (step >= deltaValue) {
      step = 0U;
      hue2 = random8(WIDTH - 2);
      hue = random8(5);                        // flag color
    }
    if (step % blur_step == 0 && modes[currentMode].Speed > 230) {
      blurScreen(beatsin8(5U, 5U, 72U));
    }
    hue2++;                                    // x
    deltaHue2++;                               // y

    if (hue2 >= WIDTH) {
      if (deltaHue2 > HEIGHT - 2 ) {           // если матрица высокая дорисовываем остальные мазки
        deltaHue2 = random8(5);                // изменяем положение по Y только отрисовав весь флаг
      }
      if (step % 2 == 0) {
        hue2 = 0U;
      } else {
        hue2 = random8(WIDTH);                 // смещение первого мазка по оси X
      }
    }

    if (deltaHue2 >= HEIGHT) {
      deltaHue2 = 0U;
      if (deltaValue > 200U) {
        hue = random8(5);                      // если низкая скорость меняем цвет после каждого витка
      }
    }

    if (deltaHue2 > CENTER_Y_MAJOR - random8(2)) {    // меняем цвет для разных частей флага
      color = colors[0][hue];
    } else {
      color = colors[1][hue];
    }

    // LOG.printf_P(PSTR("color = %08d | hue2 = %d | speed = %03d | custom_eff = %d\n"), color, hue2, deltaValue, custom_eff);
    drawPixelXY(hue2, deltaHue2, color);
    // ----------------------------------
    step++;
  }
}

// ============ Oil Paints ==============
//      © SlingMaster | by Alex Dovby
//           Масляные Краски
//---------------------------------------
void OilPaints() {
  const byte BRI_STEP = HEIGHT / 3;
  const byte FIRST_STEP = BRI_STEP * 8;
  uint32_t value;
  static uint32_t max_val;

  if (loadingFlag) {
    IsRandomMode();
    FastLED.clear();
    pcnt =  modes[currentMode].Speed < 180 ? 16 : 0; /* min brightness */
    hue = 0;
    max_val = 1UL << WIDTH; // pow(2, WIDTH);
    deltaHue = (modes[currentMode].Scale > 95);
    /* масштаб задає діапазон зміни розміру потоку */
    float percent = modes[currentMode].Scale / 100.0f;
    deltaValue = CENTER_Y_MINOR * percent + 2;
  }

  // Create Oil Paints --------------
  // вибираємо фарбу ----------------
  if (step % deltaValue == 0) {
    hue += 64;
    if (step == 0) hue += 17;
    uint8_t entry_point = random(3, WIDTH - 3);        /* start X position */
    /*       |             |0 hue |1 bright */
    noise3d[0][entry_point][0] = hue; // колір
    noise3d[0][entry_point][1] = 255; // мах яскравість
  }
  /*
    формуємо форму фарби, плавно розширюючи струмінь
    form the paint shape by smoothly widening the stream */
  if (random8(3) == 1) {
    // LOG.println("<--");
    for (uint8_t x = 1U; x < WIDTH; x++) {
      if (noise3d[0][x][0] == hue) {
        noise3d[0][x - 1][0] = hue; // колір
        noise3d[0][x - 1][1] = 255; // мах яскравість
        break;
      }
    }
  } else {
    // LOG.println("-->");
    for (uint8_t x = WIDTH - 1; x > 0U ; x--) {
      if (noise3d[0][x][0] == hue && x < WIDTH - 1) {
        noise3d[0][x + 1][0] = hue; // колір
        noise3d[0][x + 1][1] = 255; // мах яскравість
        break;
      }
    }
  }

  /* виводимо сформований рядок з максимальною яскравістю в момент зміни кольору */
  for (uint8_t x = 0U; x < WIDTH; x++) {
    uint8_t curHue = noise3d[0][x][0];
    CRGB col = CHSV(curHue, 250, 255);
    /* створюємо тінь від струменя */
    if (noise3d[0][x][1] == 255) {
      drawPixelXY(x, HEIGHT - 2, CRGB::Black);
    }
    if (noise3d[0][x][1] < 253) {
      col = CHSV(curHue, 255, 255);
      col.nscale8_video(noise3d[0][x][1]);
      keepColor(col, 5); /* нормалізуєм сольори з низькою яскравістю */
    }
    drawPixelXY(x,  HEIGHT - 1, col);
    /*
      зменшуємо яскравість для наступних рядків
      decrease brightness for the following rows */
    if ( noise3d[0][x][1] > (pcnt + BRI_STEP)) {
      /* різко зменшуємо яскравість щоб сформувати каплі, далі рівномірно */
      if (noise3d[0][x][1] == 255) {
        noise3d[0][x][1] = 254;
      } else {
        noise3d[0][x][1] -= (noise3d[0][x][1] == 254) ? FIRST_STEP : BRI_STEP;
      }
    } else {
      noise3d[0][x][1] = pcnt;
    }

    /* нерівномірно зсуваємо потік вниз | shift the flow downward unevenly */
    value = random(max_val);
    if ( bitRead(value, x ) == 0) {
      for (uint8_t y = 0U; y < HEIGHT - 1; y++) {
        drawPixelXY(x, y, getPixColorXY(x, y + 1U));
      }
    }
  }
  step++;
}

// ========== Botswana Rivers ===========
//      © SlingMaster | by Alex Dovby
//              EFF_RIVERS
//            Реки Ботсваны

//---------------------------------------
void flora() {
  static const uint8_t data[5][5] PROGMEM = {
    { 0, 0, 0, 0, 1 },
    { 0, 0, 1, 1, 1 },
    { 0, 1, 1, 1, 1 },
    { 0, 1, 1, 1, 0 },
    { 1, 0, 0, 0, 0 }
  };

  if (WIDTH < 10) return;
  const uint8_t POS_X = (CENTER_X_MINOR - 6);
  const uint32_t LEAF_COLOR = 0x1F2F00;
  const uint8_t posX = 3;
  uint8_t h =  random8(10U, 15U);
  byte deltaY = random8(2U);

  gradientRect(POS_X - 1, 0, POS_X - 1, 9U, 70, 75, 65U, 255U, 255U, 90);
  gradientRect(POS_X + 1, 0, POS_X + 1, 15, 70, 75, 65U, 255U, 255U, 90);

  drawPixelXY(POS_X + 2, h - random8(h / 2), random8(2U) == 1 ? 0xFF00E0 :  random8(2U) == 1 ? 0xFFFF00 : 0x00FF00);
  drawPixelXY(POS_X, h - random8(h / 4), random8(2U) == 1 ? 0xFF00E0 : 0xFFFF00);
  if (random8(2U) == 1) {
    drawPixelXY(posX + 1, 5U, random8(2U) == 1 ? 0xEF001F :  0x9FFF00);
  }
  h =  floor(h * 0.65);
  drawPixelXY(POS_X, h - random8(5, h - 2), random8(2U) == 1 ? 0xFF00E0 : 0xFFFF00);

  // draw leafs -------------------
  for (uint8_t y = 0; y < 5; y++) {
    for (uint8_t x = 0; x < 5; x++) {
      if (pgm_read_byte(&(data[y][x]))) {
        leds[XY(POS_X + x, 7 + deltaY - y)] = LEAF_COLOR;
        if (WIDTH > 16) {
          leds[XY(POS_X - x, 15 - deltaY - y)] = LEAF_COLOR;
        }
      }
    }
  }
}

//---------------------------------------
void animeBobbles() {
  const uint32_t color = 0xF0F7FF;
  const byte PADDING = HEIGHT > 32 ? 2 : 0;
  // scroll bobbles up ----
  for (uint8_t x = CENTER_X_MINOR; x < WIDTH - 1; x++) {
    for (uint8_t y = HEIGHT; y > 0; y--) {
      if (getPixColorXY(x, y - 1) == color) {
        drawPixelXY(x, y - 1, getPixColorXY(WIDTH - 1, y - 1));
        drawPixelXY(x, y, color);
      }
    }
  }
  // ----------------------
  if (step % 7 == 0) { /* формуємо бульбашки */
    drawPixelXY(random8(CENTER_X_MINOR, WIDTH - 1), PADDING, color);
  }
  if (step % 4 == 0) {
    drawPixelXY(random8(CENTER_X_MINOR,  WIDTH - 1), PADDING + 1, color);
  }
}

//---------------------------------------
void createScene(uint8_t idx) {
  const byte PADDING = HEIGHT > 32 ? 2 : 0;
  if (PADDING == 2) gradientRect(0, 0, WIDTH, 2, 56, 75, 120, 250, 128U, 90);
  switch (idx) {
    case 0:     // blue green ------
      /* x | y | x1 y1 | start_color | end_color | start_br | end_br | saturate | angleDeg */
      gradientRect(0, CENTER_Y_MINOR - 4, WIDTH, HEIGHT, 96, 160, 80, 255, 255U, 90);
      gradientRect(0, PADDING, WIDTH, CENTER_Y_MINOR - 4, 96, 108, 80, 255, 255U, 270);
      break;
    case 1:     // aquamarine green
      gradientRect(0, PADDING,  WIDTH, HEIGHT, 96, 130, 64, 255, 255U, 90);
      break;
    case 2:     // blue aquamarine -
      gradientRect(0, PADDING,  WIDTH, HEIGHT, 100, 170, 255, 100, 255U, 90);
      break;
    case 3:     // blue green yellow----
      gradientRect(0, PADDING, WIDTH, HEIGHT, 152, 48, 200, 240, 255U, 90);
      break;
    default:     // green sea -------
      gradientRect(0, PADDING, WIDTH, HEIGHT, 128, 152, 128, 200, 255U, 90);
      break;
  }
  flora();
}

//---------------------------------------
void BotswanaRivers() {
  static uint8_t divider;
  if (loadingFlag) {
    IsRandomMode();
    step = 0U;
    divider = floor((modes[currentMode].Scale - 1) / 20);       // маштаб задает смену палитры воды
    createScene(divider);
  }

  // restore scene after power on ---------
  if (getPixColorXY(0U, HEIGHT - 2) == CRGB::Black) {
    createScene(divider);
  }

  // LOG.printf_P(PSTR("%02d | hue2 = %03d | min = %03d \n\r"), step, hue2, deltaHue2);
  // -------------------------------------
  animeBobbles();
  step++;
}

// ============== Candle ================
//         адаптация © SottNick
//    github.com/mnemocron/FeatherCandle
//      modify & design © SlingMaster
//                Свічка
//---------------------------------------
void FeatherCandleRoutine() {
  uint8_t posY = (HEIGHT < 18) ? 0 : constrain(HEIGHT / 5, 3, HEIGHT);
  uint8_t deltaX = CENTER_X_MAJOR - 4;     // position img
  const uint8_t  level = 180;
  const uint8_t  low_level = 110;
  const uint8_t  w    = 7;                        // image width
  const uint8_t  h    = 15;                       // image height
  static uint8_t        img[w * h];               // Buffer for rendering image
  static const uint8_t *ptr = anim;               // Current pointer into animation data

  if (loadingFlag) {
    IsRandomMode();
    FastLED.clear();
    hue = 0;
    trackingObjectState[0] = low_level;
    trackingObjectState[1] = low_level;
    trackingObjectState[2] = low_level;
    trackingObjectState[4] = CENTER_X_MINOR;
  }

  uint8_t a = pgm_read_byte(ptr++);     // New frame X1/Y1
  if (a >= 0x90) {                      // EOD marker? (valid X1 never exceeds 8)
    ptr = anim;                         // Reset animation data pointer to start
    a   = pgm_read_byte(ptr++);         // and take first value
  }
  uint8_t x1 = a >> 4;                  // X1 = high 4 bits
  uint8_t y1 = a & 0x0F;                // Y1 = low 4 bits
  a  = pgm_read_byte(ptr++);            // New frame X2/Y2
  uint8_t x2 = a >> 4;                  // X2 = high 4 bits
  uint8_t y2 = a & 0x0F;                // Y2 = low 4 bits

  // Read rectangle of data from anim[] into portion of img[] buffer
  for (uint8_t y = y1; y <= y2; y++)
    for (uint8_t x = x1; x <= x2; x++) {
      img[y * w + x] = pgm_read_byte(ptr++);
    }
  int i = 0;
  uint8_t color = (modes[currentMode].Scale - 1U) * 2.57;



  // draw flame -------------------
  for (uint8_t y = 1; y < h; y++) {
    if ((HEIGHT < 15) || (WIDTH < 9)) {
      // for small matrix -----
      if (y % 2 == 0) {
        leds[XY(CENTER_X_MINOR - 1, 7 + posY)] = CHSV(color, 255U, 55 + random8(200));
        leds[XY(CENTER_X_MINOR, 6 + posY)] = CHSV(color, 255U, 160 + random8(90));
        leds[XY(CENTER_X_MINOR + 1, 6 + posY)] = CHSV(color, 255U, 205 + random8(50));
        leds[XY(CENTER_X_MINOR - 1, 5 + posY)] = CHSV(color, 255U, 155 + random8(100));
        leds[XY(CENTER_X_MINOR, 5 + posY)] = CHSV(color - 10U , 255U, 120 + random8(130));
        leds[XY(CENTER_X_MINOR, 4 + posY)] = CHSV(color - 10U , 255U, 100 + random8(120));
      }
    } else {
      for (uint8_t x = 0; x < w; x++) {
        uint8_t brightness = img[i];
        drawPixelXY( deltaX + x, y + posY,  CHSV(brightness > 240 ? color : color - 10U , 255U, brightness) );
        i++;
      }
    }

    // draw body FeatherCandle ------
    if (y <= posY) {
      if (y % 2 == 0) {
        /* x | y | x1 y1 | start_color | end_color | start_br | end_br | saturate | angleDeg */
        gradientRect(0, 0, WIDTH, 2 + posY, color, color, 56, 96, 10U, 90);
      }
    }

    // drops of wax move -------------
    switch (hue ) {
      case 0:
        if (trackingObjectState[0] < level) {
          trackingObjectState[0]++;
        }
        break;
      case 1:
        if (trackingObjectState[0] > low_level) {
          trackingObjectState[0] --;
        }
        if (trackingObjectState[1] < level) {
          trackingObjectState[1] ++;
        }
        break;
      case 2:
        if (trackingObjectState[1] > low_level) {
          trackingObjectState[1] --;
        }
        if (trackingObjectState[2] < level) {
          trackingObjectState[2] ++;
        }
        break;
      case 3:
        if (trackingObjectState[2] > low_level) {
          trackingObjectState[2] --;
        } else {
          hue++;
          // set random position drop of wax
          trackingObjectState[4] = CENTER_X_MINOR - 3 + random8(6);
        }
        break;
    }

    if (hue > 3) {
      hue++;
    } else {
      // LOG.printf_P(PSTR("[0] = %03d | [1] = %03d | [2] = %03d \n\r"), trackingObjectState[0], trackingObjectState[1], trackingObjectState[2]);
      if (hue < 2) {
        drawPixelXY( trackingObjectState[4], posY + 1, CHSV(50U, 30U, trackingObjectState[0]) );
      }
      if ((hue == 1) || (hue == 2)) {
        drawPixelXY(trackingObjectState[4], posY, CHSV(50U, 15U, trackingObjectState[1]) ); // - 10;
      }
      if (hue > 1) {
        drawPixelXY( trackingObjectState[4], posY - 1, CHSV(50U, 5U, trackingObjectState[2]) ); // - 20;
      }
    }
  }

  // next -----------------
  if ((trackingObjectState[0] == level) || (trackingObjectState[1] == level) || (trackingObjectState[2] == level)) {
    hue++;
  }
}

// ============= Hourglass ==============
//             © SlingMaster
//             EFF_HOURGLASS
//           Пісочний годинник
//---------------------------------------
void Hourglass() {
  const uint8_t h = (HEIGHT * 2) / 5;
  uint8_t posX = 0;
  const uint8_t NECK_TOP = HEIGHT - h - 4;
  const uint8_t route = (HEIGHT > h + 1) ? (HEIGHT - h - 1) : 0;
  const uint8_t STEP = WIDTH / 3 * 2;

  if (loadingFlag) {
    IsRandomMode();
    pcnt = 0;
    deltaHue2 = 0;
    hue2 = 0;

    FastLED.clear();
    hue = modes[currentMode].Scale * 2.55;

    // верхня форма піску ----
    for (uint8_t x = 0U; x < ((WIDTH / 2)); x++) {
      for (uint8_t y = 0U; y < h; y++) {
        drawPixelXY(CENTER_X_MINOR - x, HEIGHT - y - 2, CHSV(hue, 255, 255 - x * STEP));
        drawPixelXY(CENTER_X_MAJOR + x, HEIGHT - y - 2, CHSV(hue, 255, 255 - x * STEP));
      }
    }
    drawPixelXY(CENTER_X_MINOR, HEIGHT - h - 1, CHSV(hue, 255, 255));
  }

  if (hue2 == 0) {
    posX = min<uint8_t>(pcnt >> 1, WIDTH / 2);

    /* draw funnel */
    if (posX == CENTER_X_MINOR - 1 && deltaHue2 < h - 2) {
      DrawLine(CENTER_X_MINOR - 1, HEIGHT - deltaHue2 - 2, CENTER_X_MINOR + 1, HEIGHT - deltaHue2 - 2, CRGB::Black);
    }
    if (posX == 1 && deltaHue2 < h ) drawPixelXY(CENTER_X_MINOR, HEIGHT - deltaHue2 - 2, CRGB::Black);
    /* static sand bottom --- */
    drawPixelXY(CENTER_X_MINOR, deltaHue2 + 1, CHSV(hue, 255, 255));
    // LOG.printf_P(PSTR("• [%03d] | posX %03d | deltaHue2 %03d | \n"), step, posX, deltaHue2);

    /* draw body hourglass */
    /* left side */
    if (deltaHue2 == h) drawPixelXY(posX, HEIGHT - deltaHue2 - 1, CHSV(hue, 255, 0));
    else drawPixelXY(CENTER_X_MAJOR - posX, HEIGHT - deltaHue2 - 1, CHSV(hue, 255, 0));
    drawPixelXY(CENTER_X_MAJOR - posX, deltaHue2, CHSV(hue, 255, 255 - posX * STEP));

    /* move sand -------- */
    for (int8_t y = NECK_TOP; y > deltaHue2 + 2; y--) {
      uint8_t fade = 240 - (y + step / 2) * 48;
      uint8_t hue_step = (NECK_TOP - deltaHue2 - 2) / 4;
      if (fade < 100) fade = 0;
      if (fade > 240) fade = 250;
      drawPixelXY(CENTER_X_MINOR, y, CHSV(hue - (NECK_TOP - deltaHue2 - y) * hue_step, 255, fade));
    }
    step++;

    /* right side */
    if (deltaHue2 == h) drawPixelXY(WIDTH - posX - 1, HEIGHT - deltaHue2 - 1, CHSV(hue, 255, 0));
    else drawPixelXY(CENTER_X_MAJOR + posX, HEIGHT - deltaHue2 - 1, CHSV(hue, 255, 0));
    drawPixelXY(CENTER_X_MAJOR + posX, deltaHue2, CHSV(hue, 255, 255 - posX * STEP));
    if (posX == 3) {
      drawPixelXY(CENTER_X_MINOR - 1, deltaHue2 + 1, CHSV(hue, 255, 255 - posX * STEP));
      drawPixelXY(CENTER_X_MINOR + 1, deltaHue2 + 1, CHSV(hue, 255, 255 - posX * STEP));
    }

    if (pcnt > WIDTH) {
      if (modes[currentMode].Scale > 95) hue += 3U;
      deltaHue2++;
      pcnt = 0;
    }
    pcnt++;

    if (deltaHue2 > h) {
      // clear sand ---
      DrawLine(CENTER_X_MINOR, deltaHue2 + 1, CENTER_X_MINOR, NECK_TOP, CRGB::Black);
      deltaHue2 = 0;
      hue2 = 1;
    }
  }

  // імітація перевороту пісочного годинника
  if (hue2 == 0) drawPixelXY(CENTER_X_MINOR, HEIGHT - h - 2, getPixColorXY(CENTER_X_MINOR - 1, HEIGHT - h - 1));

  if (hue2 > 0) {
    for (uint8_t x = 0U; x < WIDTH; x++) {
      for (uint8_t y = HEIGHT; y > 0U; y--) {
        drawPixelXY(x, y, getPixColorXY(x, y - 1U));
        drawPixelXY(x, y - 1, 0x000000);
      }
    }
    hue2++;
    if (hue2 > route) {
      hue2 = 0;
    }
  }
}

// ============== Spectrum ==============
//             © SlingMaster
//         source code © kostyamat
//                Spectrum
//---------------------------------------
void Spectrum() {
  static const byte COLOR_RANGE = 32;
  static uint8_t customHue;
  if (loadingFlag) {
    IsRandomMode();
    ff_y = map(WIDTH, 8, 64, 310, 63);
    ff_z = ff_y;
    speedfactor = map(modes[currentMode].Speed, 1, 255, 32, 4); // _speed = map(speed, 1, 255, 128, 16);
    customHue = floor( modes[currentMode].Scale - 1U) * 2.55;
    FastLED.clear();
  }
  uint8_t color = customHue + hue;
  if (modes[currentMode].Scale == 100) {
    if (hue2++ & 0x01 && deltaHue++ & 0x01 && deltaHue2++ & 0x01) hue += 8;
    fillMyPal16_2(customHue + hue, modes[currentMode].Scale & 0x01);
  } else {
    color = customHue;
    fillMyPal16_2(customHue + AURORA_COLOR_RANGE - beatsin8(AURORA_COLOR_PERIOD, 0U, AURORA_COLOR_RANGE * 2), modes[currentMode].Scale & 0x01);
  }

  for (byte x = 0; x < WIDTH; x++) {
    if (x % 2 == 0) {
      leds[XY(x, 0)] = CHSV( color, 255U, 128U);
    }

    emitterX = ((random8(2) == 0U) ? 545. : 390.) / HEIGHT;
    for (byte y = 2; y < HEIGHT - 1; y++) {
      polarTimer++;
      leds[XY(x, y)] =
        ColorFromPalette(myPal,
                         qsub8(
                           inoise8(polarTimer % 2 + x * ff_z,
                                   y * 16 + polarTimer % 16,
                                   polarTimer / speedfactor
                                  ),
                           fabs((float)HEIGHT / 2 - (float)y) * emitterX
                         )
                        ) ;
    }
  }
}

// =========== Christmas Tree ===========
//             © SlingMaster
//           EFF_CHRISTMAS_TREE
//            Новорічна ялинка
// ======================================
void VirtualSnow(byte snow_type) {
  uint8_t posX = random8(WIDTH - 1);
  const uint8_t maxX = WIDTH - 1;
  int deltaPos;
  byte delta = (snow_type == 3) ? 0 : 1;
  for (uint8_t x = delta; x < WIDTH - delta; x++) {

    // заполняем случайно верхнюю строку
    if ((noise3d[0][x][HEIGHT - 2] == 0U) &&  (posX == x) && (random8(0, 2) == 0U)) {
      noise3d[0][x][HEIGHT - 1] = 1;
    } else {
      noise3d[0][x][HEIGHT - 1] = 0;
    }

    for (uint8_t y = 0U; y < HEIGHT; y++) {
      switch (snow_type) {
        case 0:
          noise3d[0][x][y] = noise3d[0][x][y + 1];
          deltaPos = 0;
          break;
        case 1:
        case 2:
          noise3d[0][x][y] = noise3d[0][x][y + 1];
          deltaPos = 1 - random8(2);
          break;
        default:
          deltaPos = -1;
          if ((x == 0 ) && (y == 0 ) && (random8(2) == 0U)) {
            noise3d[0][WIDTH - 1][random8(CENTER_Y_MAJOR / 2, HEIGHT - CENTER_Y_MAJOR / 4)] = 1;
          }
          if (x > WIDTH - 2) {
            noise3d[0][WIDTH - 1][y] = 0;
          }
          if (x < 1)  {
            noise3d[0][x][y] = noise3d[0][x][y + 1];
          } else {
            noise3d[0][x - 1][y] = noise3d[0][x][y + 1];
          }
          break;
      }

      if (noise3d[0][x][y] > 0) {
        if (snow_type < 3) {
          if (y % 2 == 0U) {
            leds[XY(x - ((x > 0) ? deltaPos : 0), y)] = CHSV(160, 5U, random8(200U, 240U));
          } else {
            leds[XY(x + deltaPos, y)] = CHSV(160, 5U,  random8(200U, 240U));
          }
        } else {
          leds[XY(x, y)] = CHSV(160, 5U,  random8(200U, 240U));
        }
      }
    }
  }
}

// ======================================
void GreenTree(uint8_t tree_h) {
  hue = floor(step / 32) * 32U;
  const byte posY = HEIGHT > 32 ? 2 : 0;
  for (uint8_t x = 0U; x < WIDTH + 1 ; x++) {
    if (x % 8 == 0) {
      // if (modes[currentMode].Scale < 60) {
      // nature -----
      DrawLine(x - 1U - deltaValue, posY + floor(tree_h * 0.70), x + 1U - deltaValue, posY + floor(tree_h * 0.70), 0x002F00);
      DrawLine(x - 1U - deltaValue, posY + floor(tree_h * 0.55), x + 1U - deltaValue, posY + floor(tree_h * 0.55), 0x004F00);
      DrawLine(x - 2U - deltaValue, posY + floor(tree_h * 0.35), x + 2U - deltaValue, posY + floor(tree_h * 0.35), 0x005F00);
      DrawLine(x - 2U - deltaValue, posY + floor(tree_h * 0.15), x + 2U - deltaValue, posY + floor(tree_h * 0.15), 0x007F00);

      drawPixelXY(x - 3U - deltaValue, posY + floor(tree_h * 0.15), 0x001F00);
      drawPixelXY(x + 3U - deltaValue, posY + floor(tree_h * 0.15), 0x001F00);
      if ((x - deltaValue ) >= 0) {
        /* x | y | x1 y1 | start_color | end_color | start_br | end_br | saturate | angleDeg */
        gradientRect(x - deltaValue, posY, x - deltaValue, posY + tree_h, 90U, 90U, 190U, 80U, 255U, 90);

      }

      if (modes[currentMode].Scale > 60) {
        // holiday -----
        drawPixelXY(x - 1 - deltaValue, posY + floor(tree_h * 0.6), CHSV(step + 32, 128U, 128 + random8(128)));
        drawPixelXY(x + 1 - deltaValue, posY + floor(tree_h * 0.6), CHSV(step, 128U, 128 + random8(128)));

        drawPixelXY(x - deltaValue, posY + floor(tree_h * 0.4), CHSV(step - 32, 128U, 200U));

        drawPixelXY(x - deltaValue, posY + floor(tree_h * 0.2), CHSV(step + 64, 128U, 190 + random8(65)));
        drawPixelXY(x - 2 - deltaValue, posY + floor(tree_h * 0.25), CHSV(step / 2, 128U, 96 + random8(128)));
        drawPixelXY(x + 2 - deltaValue, posY + floor(tree_h * 0.25), CHSV(step, 128U, 96 + random8(128)));

        drawPixelXY(x - 2 - deltaValue, posY + 1U, CHSV(step + 128, 128U, 200U));
        drawPixelXY(x - deltaValue, posY, CHSV(step, 128U, 250U));
        drawPixelXY(x + 2 - deltaValue, posY + 1U, CHSV(step + 128, 128U, 200U));

        if (HEIGHT > 32) {
          drawPixelXY(x - deltaValue, 1U, CHSV(0, 255U, 80U));
          drawPixelXY(x - 1 - deltaValue, 0U, CHSV(0, 255U, 80U));
          drawPixelXY(4 + x - deltaValue, 1U, CHSV(0, 255U, 80U));
          drawPixelXY(3 + x - deltaValue, 0U, CHSV(0, 255U, 80U));
        }
        if ((x - deltaValue) >= 0) { /*head tree */
          /* x | y | x1 y1 | start_color | end_color | start_br | end_br | saturate | angleDeg */
          gradientRect( x - deltaValue, posY + tree_h - 3, x - deltaValue, posY + tree_h,  hue, hue, 250U, 40U, 128U, 90);
        }
      }
    }
  }
}

// ======================================
void ChristmasTree() {
  const byte posY = HEIGHT > 32 ? 2 : 0;
  static uint8_t tree_h = HEIGHT;
  if (loadingFlag) {
    IsRandomMode();
    clearNoiseArr();
    deltaValue = 96;
    step = deltaValue;
    FastLED.clear();

    if (HEIGHT > 16) {
      tree_h = 16;
      if (modes[currentMode].Scale < 60) gradientRect(0, 0, WIDTH, HEIGHT, 160, 160, 64, 128, 255U, 90);
    }
  }

  if (HEIGHT > 16) {
    if (modes[currentMode].Scale < 60) {
      /* x | y | x1 y1 | start_color | end_color | start_br | end_br | saturate | angleDeg */
      gradientRect( 0, posY, WIDTH, HEIGHT, 160, 168, 64, 128, 255U, 90);
    } else {
      FastLED.clear();
    }
  } else {
    FastLED.clear();
  }
  GreenTree(tree_h);

  if (modes[currentMode].Scale < 60) {
    VirtualSnow(1);
  }
  if (modes[currentMode].Scale > 30) {
    deltaValue++;
  }
  if (deltaValue >= 8) {
    deltaValue = 0;
  }
  step++;
}

// ============== ByEffect ==============
//             © SlingMaster
//             EFF_BY_EFFECT
//            Побочный Эффект
// --------------------------------------
void ByEffect() {
  uint8_t saturation;
  uint8_t delta;
  if (loadingFlag) {
    IsRandomMode();
    deltaValue = 0;
    step = deltaValue;
    FastLED.clear();
  }

  hue = floor(step / 32) * 32U;
  dimAll(180);
  // ------
  saturation = 255U;
  delta = 0;
  for (uint8_t x = 0U; x < WIDTH + 1; x++) {
    if (x % 8 == 0) {

      /* x | y | x1 y1 | start_color | end_color | start_br | end_br | saturate | angleDeg */
      gradientRect(x - deltaValue, (HEIGHT * 0.75), x + 1U - deltaValue, HEIGHT,  hue, hue + 2, 250U, 20U, 255U, 90);
      if (modes[currentMode].Scale > 50) {
        delta = random8(200U);
      }
      drawPixelXY(x - 2 - deltaValue, (HEIGHT * 0.7), CHSV(step, saturation - delta, 128 + random8(128)));
      drawPixelXY(x + 2 - deltaValue, (HEIGHT * 0.7), CHSV(step, saturation, 128 + random8(128)));

      drawPixelXY(x - deltaValue, (HEIGHT * 0.6), CHSV(hue, 255U, 190 + random8(65)));
      if (modes[currentMode].Scale > 50) {
        delta = random8(200U);
      }
      drawPixelXY(x - 1 - deltaValue, CENTER_Y_MINOR, CHSV(step, saturation, 128 + random8(128)));
      drawPixelXY(x + 1 - deltaValue, CENTER_Y_MINOR, CHSV(step, saturation - delta, 128 + random8(128)));

      drawPixelXY(x - deltaValue, (HEIGHT * 0.4), CHSV(hue, 255U, 200U));
      if (modes[currentMode].Scale > 50) {
        delta = random8(200U);
      }
      drawPixelXY(x - 2 - deltaValue, (HEIGHT * 0.3), CHSV(step, saturation - delta, 96 + random8(128)));
      drawPixelXY(x + 2 - deltaValue, (HEIGHT * 0.3), CHSV(step, saturation, 96 + random8(128)));

      /* x | y | x1 y1 | start_color | end_color | start_br | end_br | saturate | angleDeg */
      gradientRect(x - deltaValue, 0U, x + 1U - deltaValue, floor(HEIGHT * 0.25),  hue + 2, hue, 20U, 250U, 255U, 90);
      if (modes[currentMode].Scale > 50) {
        drawPixelXY(x + 3 - deltaValue, HEIGHT - 3U, CHSV(step, 255U, 255U));
        drawPixelXY(x - 3 - deltaValue, CENTER_Y_MINOR, CHSV(step, 255U, 255U));
        drawPixelXY(x + 3 - deltaValue, 2U, CHSV(step, 255U, 255U));
      }
    }
  }
  // ------
  deltaValue++;
  if (deltaValue >= 8) {
    deltaValue = 0;
  }
  step++;
}


// =====================================
//            Строб Хаос Дифузия
//          Strobe Haos Diffusion
//             © SlingMaster
// =====================================
void StrobeAndDiffusion() {
  const uint8_t SIZE = 3U - custom_eff;
  const uint8_t DELTA = 1U;         // центровка по вертикали
  uint8_t STEP = 2U;
  if (loadingFlag) {
    IsRandomMode();
    FPSdelay = 25U; // LOW_DELAY;
    hue2 = 1;
    clearNoiseArr();
    FastLED.clear();
  }

  STEP = floor((255 - modes[currentMode].Speed) / 64) + 1U; // for strob
  if (modes[currentMode].Scale > 50) {
    // diffusion ---
    blurScreen(beatsin8(3, 64, 80));
    FPSdelay = LOW_DELAY;
    STEP = 1U;
    if (modes[currentMode].Scale < 75) {
      // chaos ---
      FPSdelay = 30;
      VirtualSnow(0);
    }

  } else {
    // strob -------
    if (modes[currentMode].Scale > 25) {
      dimAll(200);
      FPSdelay = 30;
    } else {
      dimAll(240);
      FPSdelay = 40;
    }
  }

  const uint8_t rows = (HEIGHT + 1) / SIZE;
  deltaHue = (modes[currentMode].Speed / 64) * 64;
  bool dir = false;
  for (uint8_t y = 0; y < rows; y++) {
    if (dir) {
      if ((step % STEP) == 0) {   // small layers
        drawPixelXY(WIDTH - 1, y * SIZE + DELTA, CHSV(step, 255U, 255U ));
      } else {
        drawPixelXY(WIDTH - 1, y * SIZE + DELTA, CHSV(170U, 255U, 1U));
      }
    } else {
      if ((step % STEP) == 0) {   // big layers
        drawPixelXY(0, y * SIZE + DELTA, CHSV((step + deltaHue), 255U, 255U));
      } else {
        drawPixelXY(0, y * SIZE + DELTA, CHSV(0U, 255U, 0U));
      }
    }

    // сдвигаем слои  ------------------
    for (uint8_t x = 0U ; x < WIDTH; x++) {
      if (dir) {  // <==
        drawPixelXY(x - 1, y * SIZE + DELTA, getPixColorXY(x, y * SIZE + DELTA));
      } else {    // ==>
        drawPixelXY(WIDTH - x, y * SIZE + DELTA, getPixColorXY(WIDTH - x - 1, y * SIZE + DELTA));
      }
    }
    dir = !dir;
  }

  if (hue2 == 1) {
    step ++;
    if (step >= 254) hue2 = 0;
  } else {
    step --;
    if (step < 1) hue2 = 1;
  }
}

// =====================================
//               Фейерверк
//                Firework
//             © SlingMaster
// =====================================
void VirtualExplosion(uint8_t f_type, int8_t timeline) {
  const uint8_t DELAY_SECOND_EXPLOSION = HEIGHT * 0.25;
  uint8_t horizont = 1U; // HEIGHT * 0.2;
  const int8_t STEP = 255 / HEIGHT;
  uint8_t firstColor = random8(255);
  uint8_t secondColor = 0;
  uint8_t saturation = 255U;
  switch (f_type) {
    case 0:
      secondColor =  random(50U, 255U);
      saturation = random(245U, 255U);
      break;
    case 1: /* сакура */
      firstColor = random(210U, 230U);
      secondColor = random(65U, 85U);
      saturation = 255U;
      break;
    case 2: /* день Независимости */
      firstColor = random(160U, 170U);
      secondColor = random(25U, 50U);
      saturation = 255U;
      break;
    default: /* фризантемы */
      firstColor = random(30U, 40U);
      secondColor = random(25U, 50U);
      saturation = random(128U, 255U);
      break;
  }
  if ((timeline > HEIGHT - 1 ) & (timeline < HEIGHT * 1.75)) {
    for (uint8_t x = 0U; x < WIDTH; x++) {
      for (uint8_t y =  horizont; y < HEIGHT - 1; y++) {
        noise3d[0][x][y] = noise3d[0][x][y + 1];
        uint8_t bri = y * STEP;
        if (noise3d[0][x][y] > 0) {
          if (timeline > (HEIGHT + DELAY_SECOND_EXPLOSION) ) {
            /* second explosion */
            drawPixelXY((x - 2 + random8(4)), y - 1, CHSV(secondColor + random8(16), saturation, bri));
          }
          if (timeline < ((HEIGHT - DELAY_SECOND_EXPLOSION) * 1.75) ) {
            /* first explosion */
            drawPixelXY(x, y, CHSV(firstColor, 255U, bri));
          }
        } else {
          // drawPixelXY(x, y, CHSV(175, 255U, floor((255 - bri) / 4)));
        }
      }
    }
    uint8_t posX = random8(WIDTH);
    noise3d[0][CENTER_X_MAJOR][HEIGHT - 1] = 1;
    for (uint8_t x = 0U; x < WIDTH; x++) {
      // заполняем случайно верхнюю строку
      if (posX == x || step == 3) {

        if (step % 2 == 0 || step % 7 == 0) {
          noise3d[0][x][HEIGHT - 1U] = 1;

        } else {
          noise3d[0][x][HEIGHT - 1U]  = 0;
        }
      } else {
        noise3d[0][x][HEIGHT - 1U]  = 0;
      }
    }
  }
}

// --------------------------------------
void Firework() {
  const uint8_t MAX_BRIGHTNESS = 40U;            /* sky brightness */
  const uint8_t DOT_EXPLOSION = HEIGHT * 0.95;
  const uint8_t HORIZONT = HEIGHT * 0.25;
  const uint8_t DELTA = 1U;                      /* центровка по вертикали */
  const float stepH = HEIGHT / 128.0;
  const uint8_t FPS_DELAY = 20U;
  const uint8_t STEP = 3U;
  const uint8_t skyColor = 156U;
  uint8_t sizeH;

  if (loadingFlag) {
    IsRandomMode();
    deltaHue2 = 0;
    FPSdelay = 255U;
    clearNoiseArr();
    FastLED.clear();
    step = 0U;
    deltaHue2 = floor(modes[currentMode].Scale / 26);
    hue = 48U;            // skyBright
    sizeH = HEIGHT;
    if (modes[currentMode].Speed > 85U) {
      sizeH = HORIZONT;
      FPSdelay = FPS_DELAY;
    }
    if (modes[currentMode].Speed <= 85U) {
      /* x | y | x1 y1 | start_color | end_color | start_br | end_br | saturate | angleDeg */
      gradientRect(0, 0, WIDTH, HEIGHT, skyColor,  skyColor, 96U, 0U, 255U, 90);
    }
  }
  if (FPSdelay == 240U) {
    FPSdelay = FPS_DELAY;
  }
  if (FPSdelay > 230U) {
    //  if (FPSdelay > 128U) {
    /* вечерело */
    FPSdelay--;
    sizeH = (FPSdelay - 128U) * stepH;

    if (modes[currentMode].Speed <= 85U) {
      dimAll(225U);
      return;
    }
    if (sizeH > HORIZONT)  {
      dimAll(200);
      return;
    }
    if (sizeH == HORIZONT )  FPSdelay = FPS_DELAY;
  }

  if (step > DOT_EXPLOSION ) {
    blurScreen(beatsin8(3, 64, 80));
    //    FastLED.setBrightness(250);
  }
  if (step == DOT_EXPLOSION - 1) {
    /* включаем фазу затухания */
    FPSdelay = 70;
  }
  if (step > CENTER_Y_MAJOR) {
    dimAll(140);
  } else {
    dimAll(100);
  }


  /* ============ draw sky =========== */
  if ((modes[currentMode].Speed > 85U) && (modes[currentMode].Speed < 180U)) {
    /* x | y | x1 y1 | start_color | end_color | start_br | end_br | saturate | angleDeg */
    gradientRect(0, 0, WIDTH, HORIZONT, skyColor, skyColor, 48U, 0U, 255U, 90);
  }


  /* deltaHue2 - Firework type */
  VirtualExplosion(deltaHue2, step);

  if ((step > DOT_EXPLOSION ) && (step < HEIGHT * 1.5)) {
    /* фаза взрыва */
    FPSdelay += 5U;
  }
  const uint8_t rows = (HEIGHT + 1) / 3U;
  deltaHue = floor(modes[currentMode].Speed / 64) * 64;
  if (step > CENTER_Y_MAJOR) {
    bool dir = false;
    for (uint8_t y = 0; y < rows; y++) {
      /* сдвигаем слои / эмитация разлета */
      for (uint8_t x = 0U ; x < WIDTH; x++) {
        if (dir) {  // <==
          drawPixelXY(x - 1, y * 3 + DELTA, getPixColorXY(x, y * 3 + DELTA));
        } else {    // ==>
          drawPixelXY(WIDTH - x, y * 3 + DELTA, getPixColorXY(WIDTH - x - 1, y * 3 + DELTA));
        }
      }
      dir = !dir;
      /* --------------------------------- */
    }
  }

  /* закоментируйте следующие две строки если плоская лампа
    подсветка заднего фона */
  // if (custom_eff == 1) {}
  //  DrawLine(0U, 0U, 0U, HEIGHT - step, CHSV(skyColor, 255U, 64U));
  //  DrawLine(WIDTH - 1, 0U, WIDTH - 1U, HEIGHT - step, CHSV(skyColor, 255U, 64U));
  /* ------------------------------------------------------ */

  /* ========== фаза полета ========== */
  if (step < DOT_EXPLOSION ) {
    FPSdelay ++;
    if (HEIGHT < 20) {
      FPSdelay ++;
    }


    uint8_t saturation = (step > (DOT_EXPLOSION - 2U)) ? 192U : 20U;
    uint8_t rndPos = 3U * deltaHue2 * 0.5;
    drawPixelXY(CENTER_X_MINOR + rndPos, step,  CHSV(50U, saturation, 80U));                 // first
    drawPixelXY(CENTER_X_MAJOR + 1 - rndPos, step - HORIZONT,  CHSV(50U, saturation, 80U));  // second
    if (rndPos > 1) {
      drawPixelXY(CENTER_X_MAJOR + 4 - rndPos, step - HORIZONT + 2,  CHSV(50U, saturation, 80U));// three
    }
    /* sky brightness */
    if (hue > 2U) {
      hue -= 1U;
    }
  }
  if (step > HEIGHT * 1.25) {
    /* sky brightness */
    if (hue < MAX_BRIGHTNESS) {
      hue += 2U;
    }
  }

  if (step >= (HEIGHT * 2.0)) {
    step = 0U;
    // LOG.printf_P(PSTR("• Bright • [%03d]\n"), FastLED.getBrightness());
    FPSdelay = FPS_DELAY;
    if (modes[currentMode].Scale < 5) {
      deltaHue2++;
    }
    if (deltaHue2 >= 4U) deltaHue2 = 0U;  // next Firework type
  }
  // LOG.printf_P(PSTR("• [%03d] | %03d | sky Bright • [%03d]\n"), step, FPSdelay, hue);
  step ++;
}

// =====================================
//             Планета Земля
//              PlanetEarth
//             © SlingMaster
// =====================================
void PlanetEarth() {
  static uint16_t imgW = 0;
  static uint16_t imgH = 0;
  const char* file_name;

  if (HEIGHT < 16U) {
    return;
  }
  if (loadingFlag) {
    IsRandomMode();
    if ( modes[currentMode].Speed > 128) hue = 0;
    FPSdelay = 128U;
    FastLED.clear();
    if (hue > 10) {
      file_name = (modes[currentMode].Scale < 50) ? "globe0" : (HEIGHT >= 24U) ? "globe_big404" : "globe1";
      ff_x = hue;
    } else {
      file_name = (modes[currentMode].Scale < 50) ? "globe0" : (HEIGHT >= 24U) ? "globe_big" : "globe1";
    }

    char path[32];
    snprintf(path, sizeof(path), "/bin/%s.img", file_name);
    readBinFile(path, 4640);

    // Используем binImage.data() для передачи указателя
    if (!binImage.empty()) {
      imgW = getSizeValue(binImage.data(), 8);
      imgH = getSizeValue(binImage.data(), 10);

#ifdef GENERAL_DEBUG
      LOG.printf_P(PSTR("Image • %03d x %02d px\n"), imgW, imgH);
#endif
      if (hue == 0) {
        scrollImage(imgW, imgH, 0U);
        ff_x = 1U;
      } else {
        scrollImage(imgW, imgH, 0U);
        scrollImage(imgW, imgH, hue);
        ff_x = hue + 1;
      }
    }
  }

  if (!binImage.empty()) {
    /* scrool index reverse --> */
    ff_x--;
    if (ff_x == 0) {
      scrollImage(imgW, imgH, 0U);
      ff_x = imgW;
    } else {
      scrollImage(imgW, imgH, ff_x);
    }

    /* <-- scrool index ------- */
    //  if (ff_x > (imgW - imgH)) ff_x = 1U;
    //  scrollImage(imgW, imgH, ff_x - 1);
    //  ff_x++;
  }

  if (hue == 0 && modes[currentMode].Speed < 128 && ff_x == floor(imgW * 0.5)) {
    uint8_t targetY = CENTER_Y_MAJOR + 5;
    uint8_t targetX = CENTER_X_MINOR;
    for (uint8_t i = 1; i < (WIDTH + 1); i++) {
      drawCircle(targetX, targetY, i, CRGB:: Red);
      if (i > 8)  drawCircle(targetX, targetY, i - 8, 0xff4f00);
      if (i > 14)  {
        drawCircleF(targetX, targetY + 0.5, i - 14, CRGB:: Black);
        blurScreen(5U);
      }
      FastLED.delay(30);
    }
    hue = ff_x;
    binImage.clear();
    binImage.shrink_to_fit();
    loadingFlag = true;
  }
}

int getRandomPos(uint8_t STEP, int prev) {
  uint8_t val = random(0, (STEP * 16 - WIDTH - 1)) / STEP * STEP;
  /* исключении небольшого поворота */
  if (abs(val - abs(prev)) > (STEP * 3)) {
    return - val;
  } else {
    return - (val + STEP * 3);
  }
}

/* --------------------------------- */
int getHue(uint8_t x, uint8_t y) {
  return ( x * 32 +  y * 24U );
}

/* --------------------------------- */
uint8_t getSaturationStep() {
  return (modes[currentMode].Speed > 170U) ? ((HEIGHT > 24) ? 12 : 24) : 0;
}

/* --------------------------------- */
uint8_t getBrightnessStep() {
  return (modes[currentMode].Speed < 85U) ? ((HEIGHT > 24) ? 16 : 24) : 0;
}

/* --------------------------------- */
void drawPalette(int posX, int startY, uint8_t STEP) {
  int PX, PY;
  const uint8_t SZ = STEP - 1;
  const uint8_t maxY = floor(HEIGHT / SZ);
  uint8_t sat = getSaturationStep();
  uint8_t br  = getBrightnessStep();

  for (uint8_t y = 0; y < maxY; y++) {
    for (uint8_t x = 0; x < 16; x++) {
      PY = startY + y * STEP;
      PX = posX + x * STEP;
      drawRec(PX, PY, PX + SZ + 1, PY + SZ + 1, CHSV( 0, 255, 0));
      if ((PX >= - STEP ) && (PY >= - STEP) && (PX < WIDTH) && (PY < HEIGHT)) {
        // LOG.printf_P(PSTR("y: %03d | br • %03d | sat • %03d\n"), y, (240U - br * y), sat);
        drawRec(PX, PY, PX + SZ, PY + SZ, CHSV( getHue(x, y), (255U - sat * y), (240U - br * y)));
      }
    }
  }
}

/* --------------------------------- */
void selectColor(uint8_t sc) {
  uint8_t offset = (WIDTH >= 16) ? WIDTH * 0.25 : 0;
  hue = getHue(random(offset, WIDTH - offset), random(HEIGHT));
  uint8_t sat = getSaturationStep();
  uint8_t br  = getBrightnessStep();

  for (uint8_t y = 0; y < HEIGHT; y++) {
    for (uint8_t x = offset; x < (WIDTH - offset); x++) {
      CHSV curColor = CHSV(hue, (255U - sat * y), (240U - br * y));
      if (curColor == getPixColorXY(x, y)) {
        /* show srlect color */
        drawRec(x, y, x + sc, y + sc, CHSV( hue, 64U, 255U));
        FastLED.show();
        delay(400);
        drawRec(x, y, x + sc, y + sc, CHSV( hue, 255U, 255U));
        y = HEIGHT;
        x = WIDTH;
      }
    }
  }
}

/* --------------------------------- */
void WebTools() {
  const uint8_t FPS_D = 10U;
  static uint8_t STEP = 3U;
  static int posX = -STEP;
  static int posY;
  static int nextX = -STEP * 2;
  static bool stop_moving = true;
  if (loadingFlag) {
    IsRandomMode();
    FPSdelay = 1U;
    step = 0;
    STEP = 2U + floor(modes[currentMode].Scale / 35);
    posX = 0;
    posY = (HEIGHT > 32) ? 2 : 0;
    FastLED.clear();
    drawPalette(posX, posY, STEP);
  }
  /* auto scenario */
  switch (step) {
    case 0:     /* restart ----------- */
      nextX = 0;
      FPSdelay = FPS_D;
      break;
    case 64:    /* start move -------- */
      nextX = getRandomPos(STEP, nextX);
      FPSdelay = FPS_D;
      break;
    case 100:    /* find -------------- */
      nextX = getRandomPos(STEP, nextX);
      FPSdelay = FPS_D;
      break;
    case 150:    /* find 2 ----------- */
      nextX = getRandomPos(STEP, nextX);
      FPSdelay = FPS_D;
      break;
    case 200:    /* find 3 ----------- */
      nextX = getRandomPos(STEP, nextX);
      FPSdelay = FPS_D;
      break;
    case 220:   /* select color ------ */
      FPSdelay = 200U;
      selectColor(STEP - 1);
      break;
    case 222:   /* show color -------- */
      FPSdelay = FPS_D;
      nextX = WIDTH;
      break;
  }
  if (posX < nextX) posX++;
  if (posX > nextX) posX--;

  if (stop_moving)   {
    FPSdelay = 80U;
    step++;
  } else {
    drawPalette(posX, posY, STEP);
    if ((nextX == WIDTH) || (nextX == 0)) {
      /* show select color bar gradient */
      // LOG.printf_P(PSTR("step: %03d | Next x: %03d • %03d | fps %03d\n"), step, nextX, posX, FPSdelay);
      if (posX > 1) {
#ifdef JAVELIN_VII
        /* x | y | x1 y1 | start_color | end_color | start_br | end_br | saturate | angleDeg */
        gradientRect(0, (posX > nextX) ? 2 : 0, (posX - 1), HEIGHT, hue, hue, 255U, 128U, 255U, 0);
#else
        /* x | y | x1 y1 | start_color | end_color | start_br | end_br | saturate | angleDeg */
        gradientRect(0, 0, (posX - 1), HEIGHT, hue, hue, 255U, 128U, 255U, 0);
#endif
      }
      if (posX > 3) DrawLine(posX - 3, CENTER_Y_MINOR, posX - 3, CENTER_Y_MAJOR, CHSV( hue, 192U, 255U));
    }
  }
  stop_moving = (posX == nextX);
}

// =====================================
//                Contacts
//             © Yaroslaw Turbin
//        Adaptation © SlingMaster
// =====================================

void Contacts() {
  if (loadingFlag) {
    IsRandomMode();
    FPSdelay = 80U;
    FastLED.clear();
  }

  int a = millis() / floor((255 - modes[currentMode].Speed) / 10);
  hue = floor(modes[currentMode].Scale / 17);
  for (int x = 0; x < WIDTH; x++) {
    for (int y = 0; y < HEIGHT; y++) {
      int index = XY(x, y);
      uint8_t color1 = pgm_read_byte(&(gamma_color[sin8((x - 8) * cos8((y + 20) * 4) / 4)]));
      uint8_t color2 = pgm_read_byte(&(gamma_color[(sin8(x * 16 + a / 3) + cos8(y * 8 + a / 2)) / 2]));
      uint8_t color3 = pgm_read_byte(&(gamma_color[sin8(cos8(x * 8 + a / 3) + sin8(y * 8 + a / 4) + a)]));
      if (hue == 0) {
        leds[index].b = color3 / 4;
        leds[index].g = color2;
        leds[index].r = 0;
      } else if (hue == 1) {
        leds[index].b = color1;
        leds[index].g = 0;
        leds[index].r = color3 / 4;
      } else if (hue == 2) {
        leds[index].b = 0;
        leds[index].g = color1 / 4;
        leds[index].r = color3;
      } else if (hue == 3) {
        leds[index].b = color1;
        leds[index].g = color2;
        leds[index].r = color3;
      } else if (hue == 4) {
        leds[index].b = color3;
        leds[index].g = color1;
        leds[index].r = color2;
      } else if (hue == 5) {
        leds[index].b = color2;
        leds[index].g = color3;
        leds[index].r = color1;
      }
    }
  }
}

// ============ Magic Lantern ===========
//             © SlingMaster
//            Чарівний Ліхтар
// --------------------------------------
void MagicLantern() {
  static uint8_t saturation;
  static uint8_t brightness;
  static uint8_t low_br;
  uint8_t delta;
  const uint8_t PADDING = HEIGHT * 0.25;
  const uint8_t WARM_LIGHT = 55U;
  const uint8_t STEP = 4U;
  if (loadingFlag) {
    IsRandomMode();
    deltaValue = 0;
    step = deltaValue;
    if (modes[currentMode].Speed > 52) {
      brightness = map(modes[currentMode].Speed, 1, 255, 50U, 250U);
      low_br = 64U;
    } else {
      brightness = 0U;
      low_br = 0U;
    }
    saturation = (modes[currentMode].Scale > 50U) ? 64U : 0U;
    if (abs (70 - modes[currentMode].Scale) <= 5) saturation = 170U;
    FastLED.clear();

  }
  dimAll(150);
  hue = (modes[currentMode].Scale > 95) ? floor(step / 32) * 32U : modes[currentMode].Scale * 2.55;

  // ------
  for (uint8_t x = 0U; x < WIDTH ; x++) {
    // light ---
    if (low_br > 0) {
      /* x | y | x1 y1 | start_color | end_color | start_br | end_br | saturate | angleDeg */
      gradientRect(x - deltaValue, CENTER_Y_MAJOR, x - deltaValue, HEIGHT - PADDING - 1,  WARM_LIGHT, WARM_LIGHT, brightness, low_br, saturation, 90);
      gradientRect(x - deltaValue, PADDING + 1, x - deltaValue, CENTER_Y_MAJOR, WARM_LIGHT, WARM_LIGHT, low_br, brightness, saturation, 90);
    } else {
      if (x % (STEP + 1) == 0) {
        leds[XY(random8(WIDTH), random8(PADDING + 2, HEIGHT - PADDING - 2))] = CHSV(step - 32U, random8(128U, 255U), 255U);
      }
      if ((modes[currentMode].Speed < 25) & (low_br == 0)) {
        deltaValue = 0;
        // body static --
        if (x % 2 != 0) {
          gradientRect(x - deltaValue, HEIGHT - PADDING, x - deltaValue, HEIGHT, hue, hue, 64U, 40U, 255U, 90);
          gradientRect( (WIDTH - x - deltaValue), 0U, (WIDTH - x - deltaValue), PADDING, hue, hue, 40U, 64U, 255U, 90);
        }
      }
    }

    if ((x % STEP == 0) || (x ==  (WIDTH - 1))) {
      // body --
      gradientRect(x - deltaValue, HEIGHT - PADDING, x - deltaValue, HEIGHT, hue, hue, 2554U, 32U, 255U, 90);
      gradientRect((WIDTH - x + deltaValue), 0U,  (WIDTH - x + deltaValue), PADDING, hue, hue, 32U, 255U, 255U, 90);

    }
  }
  // ------

  deltaValue++;
  if (deltaValue >= STEP) {
    deltaValue = 0;
  }

  step++;
}
// ============ Plasma Waves ============
//              © Руслан Ус
//        Adaptation © SlingMaster
//             Плазмові Хвилі
// --------------------------------------

void PlasmaWaves() {
  static int64_t frameCount = 0;
  if (loadingFlag) {
    IsRandomMode();
    hue = modes[currentMode].Scale;
  }
  EVERY_N_MILLISECONDS(1000 / 60) {
    frameCount++;
  }

  uint8_t t1 = cos8((42 * frameCount) / 30);
  uint8_t t2 = cos8((35 * frameCount) / 30);
  uint8_t t3 = cos8((38 * frameCount) / 30);

  for (uint16_t y = 0; y < HEIGHT; y++) {
    for (uint16_t x = 0; x < WIDTH; x++) {
      // Calculate 3 seperate plasma waves, one for each color channel
      uint8_t r = cos8((x << 3) + (t1 >> 1) + cos8(t2 + (y << 3)));
      uint8_t g = cos8((y << 3) + t1 + cos8((t3 >> 2) + (x << 3)));
      uint8_t b = cos8((y << 3) + t2 + cos8(t1 + x + (g >> 2)));

      // uncomment the following to enable gamma correction
      // r = pgm_read_byte_near(exp_gamma + r);
      r = pgm_read_byte(&(gamma_color[r]));
      g = pgm_read_byte(&(gamma_color[g]));
      b = pgm_read_byte(&(gamma_color[b]));

      // g = pgm_read_byte_near(exp_gamma + g);
      // b = pgm_read_byte_near(exp_gamma + b);

      leds[XY(x, y)] = CRGB(r, g, b);
    }
    hue++;
  }
  // blurScreen(beatsin8(3, 64, 80));
}

// ============== Hand Fan ==============
//           на основі коду від
//          © mastercat42@gmail.com
//             © SlingMaster
//                Опахало
// --------------------------------------

void HandFan() {
  const uint8_t V_STEP = 255 / (HEIGHT + 9);
  static uint8_t val_scale;
  if (loadingFlag) {
    IsRandomMode();
    hue = modes[currentMode].Scale * 2.55;
    val_scale = map(modes[currentMode].Speed, 1, 255, 200U, 255U);;
  }
  for (int index = 0; index < NUM_LEDS; index++) {
    leds[index].nscale8(val_scale);
  }

  for (int i = 0; i < HEIGHT; i++) {
    int tmp = sin8(i + (millis() >> 4));
    tmp = map8(tmp, 2, WIDTH - 2);

    leds[XY(WIDTH - tmp, i)] = CHSV(hue, V_STEP * i + 32, 205U);
    leds[XY(WIDTH - tmp - 1, i)] = CHSV(hue, 255U, 255 - V_STEP * i);
    leds[XY(WIDTH - tmp + 1, i)] = CHSV(hue, 255U, 255 - V_STEP * i);

    if ((i % 6 == 0) & (modes[currentMode].Scale > 95U)) {
      hue++;
    }
  }
}

// =============== Bamboo ===============
//             © SlingMaster
//                 Бамбук
// --------------------------------------
uint8_t nextColor(uint8_t posY, uint8_t base, uint8_t next ) {
  const byte posLine = (HEIGHT > 16) ? 4 : 3;
  if ((posY + 1 == posLine) || (posY == posLine)) {
    return next;
  } else {
    return base;
  }
}

// --------------------------------------
void Bamboo() {
  const uint8_t gamma[7] = {0, 32, 144, 160, 196, 208, 230};
  static float index;
  const byte DELTA = 4U;
  const uint8_t VG_STEP = 64U;
  const uint8_t V_STEP = 32U;
  const byte posLine = (HEIGHT > 16) ? 4 : 3;
  const uint8_t SX = 5;
  const uint8_t SY = 10;
  static float deltaX = 0;
  static bool direct = false;
  uint8_t posY;
  static uint8_t colLine;
  const float STP = 0.2;
  if (loadingFlag) {
    IsRandomMode();
    index = STP;
    uint8_t idx = map(modes[currentMode].Scale, 5, 95, 0U, 6U);;
    colLine = gamma[idx];
    step = 0U;
  }

  // *** ---
  for (int y = 0; y < HEIGHT + SY; y++) {
    if (modes[currentMode].Scale < 50U) {
      if (step % 128 == 0U) {
        deltaX += STP * ((direct) ? -1 : 1);
        if ((deltaX > 1) || (deltaX < -1)) direct = !direct;
      }
    } else {
      deltaX = 0;
    }
    posY = y;
    for (int x = 0; x < WIDTH + SX; x++) {
      if (y == posLine) {
        drawPixelXYF(x , y - 1, CHSV(colLine, 255U, 128U));
        drawPixelXYF(x, y, CHSV(colLine, 255U, 96U));
        if (HEIGHT > 16) {
          drawPixelXYF(x, y - 2, CHSV(colLine, 10U, 64U));
        }
      }
      if ((x % SX == 0U) & (y % SY == 0U)) {
        for (int i = 1; i < (SY - 3); i++) {
          if (i < 3) {
            posY = y - i + 1 - DELTA + index;
            drawPixelXYF(x - 3 + deltaX, posY, CHSV(nextColor(posY, 96, colLine), 255U, 255 - V_STEP * i));
            posY = y - i + index;
            drawPixelXYF(x + deltaX, posY, CHSV(nextColor(posY, 96, colLine), 255U, 255 - VG_STEP * i));
          }
          posY = y - i - DELTA + index;
          drawPixelXYF(x - 4 + deltaX, posY , CHSV(nextColor(posY, 96, colLine), 180U, 255 - V_STEP * i));
          posY = y - i + 1 + index;
          drawPixelXYF(x - 1 + deltaX, posY , CHSV(nextColor(posY, ((i == 1) ? 96 : 80), colLine), 255U, 255 - V_STEP * i));
        }
      }
    }
    step++;
  }
  if (index >= SY)  {
    index = 0;
  }
  fadeToBlackBy(leds, NUM_LEDS, 60);
  index += STP;
}

// ============ Light Filter ============
//             © SlingMaster
//              Cвітлофільтр
// --------------------------------------
void LightFilter() {
  static int64_t frameCount =  0;
  const byte END = WIDTH - 1;
  static byte dX;
  static bool direct;
  static byte divider;
  static byte deltaValue = 0;

  if (loadingFlag) {
    IsRandomMode();
    divider = floor(modes[currentMode].Scale / 25);
    direct = true;
    dX = 1;
    pcnt = 0;
    frameCount = 0;
    hue2 == 32;
    clearNoiseArr();
    FastLED.clear();
  }

  // EVERY_N_MILLISECONDS(1000 / 30) {
  frameCount++;
  pcnt++;
  // }

  uint8_t t1 = cos8((42 * frameCount) / 30);
  uint8_t t2 = cos8((35 * frameCount) / 30);
  uint8_t t3 = cos8((38 * frameCount) / 30);
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;

  if (direct) {
    if (dX < END) {
      dX++;
    }
  } else {
    if (dX > 0) {
      dX--;
    }
  }
  if (pcnt > 128) {
    pcnt = 0;
    direct = !direct;
    if (divider > 2) {
      if (dX == 0) {
        deltaValue++;
        if (deltaValue > 2) {
          deltaValue = 0;
        }
      }
    } else {
      deltaValue = divider;
    }

  }

  for (uint16_t y = 0; y < HEIGHT; y++) {
    for (uint16_t x = 0; x < WIDTH; x++) {
      if (x != END - dX) {
        r = cos8((y << 3) + (t1 >> 1) + cos8(t2 + (x << 3)));
        g = cos8((y << 3) + t1 + cos8((t3 >> 2) + (x << 3)));
        b = cos8((y << 3) + t2 + cos8(t1 + x + (g >> 2)));

      } else {
        // line gold -------
        r = 255U;
        g = 255U;
        b = 255U;
      }

      uint8_t val = dX * 8;
      switch (deltaValue) {
        case 0:
          if (r > val) {
            r = r - val;
          } else {
            r = 0;
          }
          if (g > val) {
            g = g - val / 2;
          } else {
            g = 0;
          }
          break;
        case 1:
          if (g > val) {
            g = g - val;
          } else {
            g = 0;
          }
          if (b > val) {
            b = b - val / 2;
          } else {
            b = 0;
          }
          break;
        case 2:
          if (b > val) {
            b = b - val;
          } else {
            b = 0;
          }
          if (r > val) {
            r = r - val / 2;
          } else {
            r = 0;
          }
          break;
      }

      r = pgm_read_byte(&(gamma_color[r]));
      g = pgm_read_byte(&(gamma_color[g]));
      b = pgm_read_byte(&(gamma_color[b]));

      leds[XY(x, y)] = CRGB(r, g, b);
    }
  }
  hue++;
}

// ========== New Year's Сard ===========
//             © SlingMaster
//           Новорічна листівка
// --------------------------------------

void NewYearsCard() {
  static const uint8_t gamma[3][30] PROGMEM = {
    {
      0x20, 0x20, 0x48, 0x41, 0x50, 0x50, 0x59, 0x20, 0x4E, 0x45,
      0x57, 0x20, 0x59, 0x45, 0x41, 0x52, 0x21, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x02
    },
    {
      0x20, 0x20, 0xD0, 0x97, 0x20, 0xD0, 0x9D, 0xD0, 0x9E, 0xD0,
      0x92, 0xD0, 0x98, 0xD0, 0x9C, 0x20, 0xD0, 0xA0, 0xD0, 0x9E,
      0xD0, 0x9A, 0xD0, 0x9E, 0xD0, 0x9C, 0x21, 0x20, 0x00, 0x02
    },
    {
      0x20, 0x20, 0x20, 0xD0, 0x9F, 0xD0, 0xA3, 0xD0, 0xA2, 0xD0,
      0x98, 0xD0, 0x9D, 0x20, 0xD0, 0xA5, 0xD0, 0xA3, 0xD0, 0x99,
      0xD0, 0x9B, 0xD0, 0x9E, 0x21, 0x21, 0x21, 0x20, 0x00, 0x02
    }
  };

  const byte GRID = WIDTH / 6U;
  const byte deltaY = (HEIGHT > 32) ? 2U : 0U;
  static int64_t frameCount =  0;
  const byte END = WIDTH - GRID;
  const byte tree_h = (HEIGHT > 20) ? 18 : HEIGHT;
  const float STEP = 16.0 / HEIGHT;
  const byte MAX = 3U;
  static byte dX;
  static bool direct;
  static byte divider;
  static byte index;
  static byte shadow = 2;


  if (loadingFlag) {
    IsRandomMode();

    divider = floor(modes[currentMode].Scale / 25);
    index = 0;
    if (eff_valid < 2) {
      if (divider >= 2)    {
        shadow = 1;
      } else {
        shadow = 0;
      }
    } else {
      shadow = eff_valid;
    }

    direct = true;
    dX = GRID;
    pcnt = 0;
    frameCount = 0;
    hue2 == 32;
    clearNoiseArr();
    FastLED.clear();
  }

  EVERY_N_MILLISECONDS(1000 / 60) {
    frameCount++;
    pcnt++;
  }

  uint8_t t1 = cos8((42 * frameCount) / 30);
  uint8_t t2 = cos8((35 * frameCount) / 30);
  uint8_t t3 = cos8((38 * frameCount) / 30);
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;

  if (direct) {
    if (dX < (END - GRID)) {
      dX++;
      index = 0;
    }
  } else {
    if (dX > 0) {
      dX--;
    }
  }

  if (pcnt > 120) {
    pcnt = 0;
    direct = dX < CENTER_X_MINOR;
  }

  for (uint16_t y = 0; y < HEIGHT; y++) {
    for (uint16_t x = 0; x < WIDTH; x++) {
      if (x >= END - dX) {
        r = sin8((x - 8) * cos8((y + 20) * 4) / 4);
        g = cos8((y << 3) + t1 + cos8((t3 >> 2) + (x << 3)));
        b = cos8((y << 3) + t2 + cos8(t1 + x + (g >> 2)));
      } else {
        if (x < (END - 1 - dX)) {
          // gradient -------
          //r = (shadow == gamma[shadow][29]) ? 200U : 0;
          r = (shadow == pgm_read_byte(&(gamma[shadow][29]))) ? 200U : 0;
          g = divider == 2U ? 0 : y * 2;
          b = divider == 2U ? 0 : 96U + y * 2;
        } else {
          // line gold -------
          r = 160U;
          g = 144U;
          b = 64U;
        }
      }
      uint8_t val = dX * 10;
      if (r > val) {
        r = r - val;
      } else {
        r = 0;
      }

      r = pgm_read_byte(&(gamma_color[r]));
      g = pgm_read_byte(&(gamma_color[g]));
      b = pgm_read_byte(&(gamma_color[b]));

      leds[XY(x, y)] = CRGB(r, g, b);
    }
    hue++;
  }

  float delta = 0.0;
  uint8_t posX = 0;

  // restore background --------
  for (uint8_t x = 0U; x < END - dX; x++) {
    if (x % 8 == 0) {
      // nature -----
      delta = 0.0;
      for (uint8_t y = 2U; y < tree_h; y++) {
        if (y % 3 == 0U) {
          uint8_t posX = delta;
          DrawLine(x - MAX + posX - deltaValue, y, x + MAX - posX - deltaValue, y, 0x007F00);
          delta = delta + STEP;
          if ( delta > MAX) delta = MAX;
        }
      }
      if ((x - deltaValue ) >= 0) {
        /* x | y | x1 y1 | start_color | end_color | start_br | end_br | saturate | angleDeg */
        gradientRect(x - deltaValue, deltaY, x - deltaValue, tree_h + deltaY, 90U, 90U, 190U, 96U, 255U, 90);
      }
    }
  }

  VirtualSnow(divider);

  if (dX == WIDTH - GRID * 2 ) {
    if (overprintText(gamma, shadow, 10, 240, 1)) {
      pcnt = 0;
    }
  }

  if (pcnt % 4U == 0U) {
    index++;
    if (pgm_read_byte(&(gamma[shadow][index])) > 0xC0) {
      index++;
    }
    if (index > 28) {
      index = 0;
    }
  }

  deltaValue++;
  if (deltaValue >= 8) {
    deltaValue = 0;
  }
  hue2 += 2;
}

// ========== Taste of Honey ============
//         SRS code by © Stepko
//        Adaptation © SlingMaster
//               Смак Меду
// --------------------------------------

void TasteHoney() {
  byte index;
  if (loadingFlag) {
    IsRandomMode();
    hue = modes[currentMode].Scale * 2.55;
    index = modes[currentMode].Scale / 10;
    clearNoiseArr();

    switch (index) {
      case 0:
        currentPalette = PartyColors_p;
        break;
      case 1:
        currentPalette = LavaColors_p;
        break;
      case 2:
      case 3:
        currentPalette = ForestColors_p;
        break;
      case 4:
        currentPalette = CloudColors_p;
        break;
      default :
        currentPalette = AlcoholFireColors_p;
        break;
    }
    FastLED.clear();
  }

  fillNoiseLED();
  memset8(&noise2[1][0][0], 255, (WIDTH + 1) * (HEIGHT + 1));
  for (byte x = 0; x < WIDTH; x++) {
    for (byte y = 0; y < HEIGHT; y++) {
      uint8_t n0 = noise2[0][x][y];
      uint8_t n1 = noise2[0][x + 1][y];
      uint8_t n2 = noise2[0][x][y + 1];
      int8_t xl = n0 - n1;
      int8_t yl = n0 - n2;
      int16_t xa = (x * 255) + ((xl * ((n0 + n1) << 1)) >> 3);
      int16_t ya = (y * 255) + ((yl * ((n0 + n2) << 1)) >> 3);
      CRGB col = CHSV(hue, 255U, 255U);
      wu_pixel(xa, ya, &col);
    }
  }
}

// ============= Genome UA ==============
//           base code © Stepko
//             © SlingMaster
//                Геном UA
// --------------------------------------
#define LIGHTERS_AM ((WIDTH+HEIGHT)/6)
static int lightersPosX[LIGHTERS_AM];
static int lightersPosY[LIGHTERS_AM];
static int PosRegX[LIGHTERS_AM];
static int PosRegY[LIGHTERS_AM];
static byte lightersSpeedX[LIGHTERS_AM];
static byte lightersSpeedY[LIGHTERS_AM];
static byte lightersSpeedZ[LIGHTERS_AM];
static byte lcolor[LIGHTERS_AM];
static byte mass[LIGHTERS_AM];
/* --------------------------------- */


void Spermatozoa() {
  const byte cenzor = 3;
  uint8_t speed = 127;
  uint8_t scale = 16;

  // msg -----
  static int16_t offset = 0;
  static bool print_msg;
  static const uint8_t msg[cenzor][52] PROGMEM = {
    { 0xd0, 0xa1, 0xd0, 0xbb, 0xd0, 0xb0, 0xd0, 0xb2, 0xd0, 0xb0, 0x20, 0xd0, 0xa3, 0xd0,
      0xba, 0xd1, 0x80, 0xd0, 0xb0, 0xd1, 0x97, 0xd0, 0xbd, 0xd1, 0x96, 0x20, 0xd0, 0x93,
      0xd0, 0xb5, 0xd1, 0x80, 0xd0, 0xbe, 0xd1, 0x8f, 0xd0, 0xbc, 0x20, 0xd0, 0xa1, 0xd0,
      0xbb, 0xd0, 0xb0, 0xd0, 0xb2, 0xd0, 0xb0, 0x21, 0x00, 0x60
    },
    {
      0xd0, 0x92, 0xd1, 0x96, 0xd0, 0xb4, 0xd1, 0x87, 0xd0, 0xb5, 0xd0, 0xbf, 0xd0, 0xb8,
      0xd1, 0x81, 0xd1, 0x8c, 0x2c, 0x20, 0xd0, 0xbd, 0xd0, 0xb0, 0xd0, 0xbc, 0x20, 0xd0,
      0xbd, 0xd0, 0xb5, 0x20, 0xd0, 0xb4, 0xd0, 0xbe, 0x20, 0xd1, 0x82, 0xd0, 0xb5, 0xd0,
      0xb1, 0xd0, 0xb5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0
    },
    {
      0xd0, 0x9d, 0xd0, 0xb5, 0x20, 0xd1, 0x87, 0xd1, 0x96, 0xd0, 0xbf, 0xd0, 0xb0, 0xd0,
      0xb9, 0x20, 0xd0, 0xb2, 0xd1, 0x96, 0xd0, 0xbb, 0xd1, 0x8c, 0xd0, 0xbd, 0xd1, 0x83,
      0x20, 0xd1, 0x81, 0xd0, 0xbf, 0xd1, 0x96, 0xd0, 0xbb, 0xd1, 0x8c, 0xd0, 0xbd, 0xd0,
      0xbe, 0xd1, 0x82, 0xd1, 0x83, 0x00, 0x00, 0x00, 0x00, 0x60
    }
  };


  if (loadingFlag) {
    IsRandomMode();
    pcnt++;
    if (pcnt > cenzor) {
      /* pcnt == 0 first loading ignore start print msg */
      pcnt = 0U;
    }

    hue = 0;
    offset = WIDTH;
    deltaValue = 255;                             // saturate
    FPSdelay = SOFT_DELAY;
    scale = 16; // custom_eff ? 8 : 16;

    randomSeed(millis());
    for (byte i = 0; i < LIGHTERS_AM; i++) {
      PosRegX[i] = random(0, WIDTH * 10);
      PosRegY[i] = random(0, HEIGHT * 10);
      lightersSpeedX[i] = random(25, 50);
      lightersSpeedY[i] = random(25, 50);
      mass[i] = random(30, 255);
      lcolor[i] = random(0, 9) * 28;

    }
    step = 0U;
    print_msg = true;
  }

  uint8_t color;
  fadeToBlackBy(leds, NUM_LEDS, 45);
  for (byte i = 0; i < map(scale, 1, 16, 2, LIGHTERS_AM); i++) {
    lcolor[i]++;
    if (print_msg & (pcnt > 0)) {
      deltaValue = 255;
      color =  (i % 2 == 0U) ? 48U : 160U;
      deltaValue = (pcnt == 2) ? 0U : 255U;
    } else {
      color = lcolor[i];
    }

    lightersPosX[i] = beatsin16(lightersSpeedX[i] * speed, PosRegX[i], PosRegX[i] + mass[i] * ((HEIGHT + WIDTH) / 16));
    lightersPosY[i] = beatsin16(lightersSpeedY[i] * speed, PosRegY[i], PosRegY[i] + mass[i] * ((HEIGHT + WIDTH) / 16));

    if (lightersPosX[i] < 0) lightersPosX[i] = (WIDTH - 1) * 10 - lightersPosX[i] - (WIDTH - 1) * 10;
    if (lightersPosX[i] > (WIDTH - 1) * 10) lightersPosX[i] = lightersPosX[i] - (WIDTH - 1) * 10;
    if (lightersPosY[i] < 0)lightersPosY[i] = (HEIGHT - 1) * 10 - lightersPosY[i] - (HEIGHT - 1) * 10;;
    if (lightersPosY[i] > (HEIGHT - 1) * 10) lightersPosY[i] = lightersPosY[i] - (HEIGHT - 1) * 10;

    drawPixelXYF((float)lightersPosX[i] / 10, (float)lightersPosY[i] / 10, CHSV(color, deltaValue, beatsin8(lightersSpeedZ[i] / map(speed, 1, 255, 10, 1), 128, 255)));
    drawPixelXYF((float)lightersPosX[i] / 10, (float)lightersPosY[i] / 10 - 1, CHSV(color, deltaValue, beatsin8(lightersSpeedZ[i] / map(speed, 1, 255, 10, 1), 128, 255)));
    // black shadow if white color spermatozoon
    drawPixelXYF((float)lightersPosX[i] / 10 - 1, (float)lightersPosY[i] / 10, CHSV(color, deltaValue, (pcnt == 2) ? 0U : beatsin8(lightersSpeedZ[i] / map(speed, 1, 255, 10, 1), 128, 255)));
  }

  // ptint random messages ---
  uint8_t i = 0, j = 0;

  if (print_msg & (pcnt > 0)) {
    if (overprintText(msg, pcnt - 1, 100, pgm_read_byte(&(msg[pcnt - 1][51])), 0)) {
      print_msg = false;
    }
  }
  hue++;

  EVERY_N_SECONDS(5) {
    for (byte i = 0; i < map(scale, 1, 16, 2, LIGHTERS_AM); i++) {
      lightersSpeedX[i] = random(25, 50);
      lightersSpeedY[i] = random(25, 50);
      mass[i] + random(-25, 25);
      PosRegX[i] - 20;
      if (PosRegX[i] < 0) PosRegX[i] = (WIDTH - 1) * 10;
      PosRegY[i] + 20;
      if (PosRegY[i] > (HEIGHT - 1) * 10) PosRegY[i] = 0;
    }
    step++;
    if (step > 12U) {
      /* timeout 60 sec for reset default msg view  */
      step = 0U;
      pcnt = 255U;
    }
  }
}

// =====================================
//           Rainbow Tornado
//  base code © Stepko and © Sutaburosu
//            © SlingMaster
//          Райдужний Торнадо
// =====================================
/* --------------------------------- */
void Tornado() {
  const uint8_t mapp = 255 / WIDTH;
  const byte OFFSET = 2U;
  const uint8_t H = HEIGHT - OFFSET;
  static uint32_t t;
  static byte scaleXY = 4;

  if (loadingFlag) {
    IsRandomMode();
    scaleXY = 2 + modes[currentMode].Scale / 10;

    FastLED.clear();
    for (int8_t x = -CENTER_X_MAJOR; x < CENTER_X_MAJOR; x++) {
      for (int8_t y = -OFFSET; y < H; y++) {
        noise3d[0][x + CENTER_X_MAJOR][y + OFFSET] = 128 * (atan2(y, x) / PI);
        noise3d[1][x + CENTER_X_MAJOR][y + OFFSET] = hypot(x, y);                    // thanks Sutaburosu
      }
    }
  }
  t += 8;
  for (uint8_t x = 0; x < WIDTH; x++) {
    for (uint8_t y = 0; y < HEIGHT; y++) {
      byte angle = noise3d[0][x][y];
      byte radius = noise3d[1][x][y];
      leds[XY(x, y)] = CHSV((angle * scaleXY) - t + (radius * scaleXY), constrain(y * 16, (y < 5 ? (y * 16) : 96), 255), (y < 5 ? 255 - ((5 - y) * 16) : 255));
    }
  }
}


// ========== Creative  Watch ===========
//             © SlingMaster
//          Креативний Годинник
// --------------------------------------
void drawDig(uint8_t x, uint8_t y, uint8_t num, CRGB color) {
  for (uint8_t i = 0U; i < 3U; i++) {
    uint8_t m = pgm_read_byte(&clockFont3x5[num][i]);
    for (uint8_t j = 0U; j < 5U; j++) {
      if ((m >> j) & 0x01) {
        drawPixelXY((x + i) % WIDTH, (y + j) % HEIGHT, color);
        drawPixelXY((x + i + 1) % WIDTH, (y + j) % HEIGHT, 0x000000);
      } else {
        drawPixelXY((x + i) % WIDTH, (y + j) % HEIGHT, 0x000000);
        drawPixelXY((x + i + 1) % WIDTH, (y + j) % HEIGHT, 0x000000);
      }
    }
  }
}

// ---------------------
void drawClockFace(uint8_t x, uint8_t y, uint8_t posR, uint8_t num,  CRGB color, bool hh) {
  uint8_t prev = num - 1;
  if (num == 0) {
    prev = hh ? 23 : 59;
  }
  drawDig(x - 4, y, prev / 10U % 10U, color);
  drawDig(x, y, prev % 10U, color);
  // next --------------
  drawDig(x - 4 + posR, y, num / 10U % 10U, color);
  drawDig(x + posR, y, num % 10U, color);
}

// ---------------------
void CreativeWatch() {
  // const uint8_t *exp_gamma  = gamma_color;
  const byte OFFSET_Y = (HEIGHT > 32) ? 2 : 0;
  const byte PADDING = (WIDTH > 10) ? (WIDTH) / 8 : 0;
  const uint8_t posR = 16; // WIDTH / 2 + 3;
  const uint8_t t_size = WIDTH * 0.25;
  const uint32_t dataColors[6] = {CRGB::OrangeRed, CRGB::Gold, CRGB::OliveDrab, CRGB::Goldenrod};
  static byte index;
  static uint8_t xx;
  static int offset = 0;
  static bool print_time;
  const uint8_t center = CENTER_Y_MAJOR + OFFSET_Y / 2;
  time_t currentLocalTime;
  uint8_t sec;
  // ---------------------

  if (loadingFlag) {
    IsRandomMode();
    if (WIDTH < 10) {
      FPSdelay = HIGH_DELAY;
    } else {
      FPSdelay = SOFT_DELAY;
    }

    deltaValue = 59;
    offset = 0;
    print_time = true;
    index = modes[currentMode].Scale / 26;
    FastLED.clear();
    for (uint8_t x = 0U; x < WIDTH; x++) {
      drawPixelXY(x, center, ((x % 2U == 0U) ? dataColors[index] : 0x000000));
    }
  }

  // change color ----
  static int64_t frameCount =  0;
  uint8_t t1 = cos8((42 * frameCount) / 30);
  uint8_t t2 = cos8((35 * frameCount) / 30);
  uint8_t t3 = cos8((38 * frameCount) / 30);
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;

  // ptint time ---
  currentLocalTime = getCurrentLocalTime();
  print_time = (deltaValue != minute(currentLocalTime));
  if (print_time) {
    if (deltaValue == 59) {
      drawClockFace(CENTER_X_MAJOR - offset, center + 2, posR, hour(currentLocalTime), CRGB::SlateGrey, true);
    }
    drawClockFace(CENTER_X_MAJOR - offset, center - 6, posR, minute(currentLocalTime), CRGB::SlateGrey, false);

    if (step % 2 == 0) {
      offset++;
    }
    if (offset > posR) {       // end print time
      print_time = false;
      deltaValue = minute(currentLocalTime);
      offset = 0;
      hue = 0;
    }
  }

  frameCount++;
  EVERY_N_SECONDS(1) {
    // EVERY_N_MILLISECONDS(500) {
    // seconds gear ---
    if (offset == 0) {
      print_time = true;
      offset = 0;
    }

    sec = second(currentLocalTime);
    drawPixelXY(WIDTH - PADDING, center, (sec % 2U == 0) ? ((sec == 50) ? CRGB::Red : dataColors[index]) : 0x000000 );
    for (uint8_t x = PADDING; x < WIDTH - PADDING; x++) {
      drawPixelXY(x, center, getPixColorXY(x + 1, center));
    }
  }

  // body if big height matrix ---------
  if (HEIGHT > 20U) {
    for (uint16_t y = OFFSET_Y; y < HEIGHT; y++) {
      for (uint16_t x = 0; x < WIDTH; x++) {
        r = sin8((x - 8) * cos8((y + 20) * 4) / 4);
        g = cos8((y << 3) + t1 + cos8((t3 >> 2) + (x << 3)));
        b = cos8((y << 3) + t2 + cos8(t1 + x + (g >> 2)));

        g = pgm_read_byte(&(gamma_color[g]));
        b = pgm_read_byte(&(gamma_color[b]));
        if (modes[currentMode].Scale < 50) {
          // green blue magenta --
          if (b < 20) b = pgm_read_byte(&(gamma_color[r]));
          r = (g < 128) ? pgm_read_byte(&(gamma_color[b])) / 3 : 0;
        } else {
          // green blue yellow ---
          if (g < 20) g = pgm_read_byte(&(gamma_color[r]));
          r = (b < 128) ? pgm_read_byte(&(gamma_color[g])) / 2 : 0;
        }
        // ---------------------
        if ((( (y < center - 11) || y > center + 10) || (x < PADDING) || (x > WIDTH - PADDING - 1) )) {
          leds[XY(x, y)] = CRGB(r, g, b);
        }
      }
    }
  }

  // pendulum --------
  for (uint8_t x = 0U; x < WIDTH; x++) {
    drawPixelXY( abs(WIDTH / 2 - xx) + x - t_size, center - 8, (((x > t_size) & (x < (WIDTH - t_size))) ? dataColors[index] : 0x000000));
    if (HEIGHT > 18U) {
      drawPixelXY( abs(WIDTH / 2 - xx) + x - t_size, center + 9, (((x > t_size) & (x < (WIDTH - t_size))) ? dataColors[index] : 0x000000));
      drawPixelXY( abs(WIDTH / 2 - xx) + x - t_size, center + 8, (((x > t_size) & (x < (WIDTH - t_size))) ? dataColors[index] : 0x000000));
      drawPixelXY( abs(WIDTH / 2 - xx) + x - t_size, center - 9, (((x > t_size) & (x < (WIDTH - t_size))) ? dataColors[index] : 0x000000));
      if (HEIGHT > 32U) {
        drawPixelXY( abs(WIDTH / 2 - xx) + x - t_size, 1, (((x > t_size) & (x < (WIDTH - t_size))) ? dataColors[index] : 0x000000));
        drawPixelXY( abs(WIDTH / 2 - xx) + x - t_size, 0, (((x > t_size) & (x < (WIDTH - t_size))) ? dataColors[index] : 0x000000));
      }
    }

  }
  xx++;
  if (xx > WIDTH) {
    xx = 0;
  }
  // -----------------
  step++;
}


// =========== Heat Networks ===========
//             © SlingMaster
//            Теплові Мережі
// =====================================
void getThermometry() {
  // https://arduinogetstarted.com/tutorials/arduino-http-request
  // http://api.thingspeak.com/channels/117345/feeds.json?results=1
  deltaHue = 255U;
  if (espMode == 1U && connect) {
    if (!HTTPclient.connect("api.thingspeak.com", 80)) {
      Serial.println(F("Connection failed"));
      deltaHue = 96; // return;
    }
    // Serial.println(" • Connected to server");
    if (deltaHue > 200U) {
      // Send HTTP request
      HTTPclient.println(F("GET /channels/117345/feeds.json?results=1 HTTP/1.0"));
      HTTPclient.println(F("Host: api.thingspeak.com"));
      HTTPclient.println(F("Connection: close"));
      if (HTTPclient.println() == 0) {
        Serial.println(F("Failed to send request"));
        HTTPclient.stop();
        hue = 160;
        deltaHue = 96; // return;
      }
    }
    // Check HTTP status
    if (deltaHue > 200U) {
      char status[32] = {0};
      HTTPclient.readBytesUntil('\r', status, sizeof(status));
      // It should be "HTTP/1.0 200 OK" or "HTTP/1.1 200 OK"
      if (strcmp(status + 9, "200 OK") != 0) {
        Serial.print(F("Unexpected response: "));
        Serial.println(status);
        HTTPclient.stop();
        hue = 170;
        deltaHue = 96; // return;
      }
    }

    // Skip HTTP headers ----
    if (deltaHue > 200U) {
      char endOfHeaders[] = "\r\n\r\n";
      if (!HTTPclient.find(endOfHeaders)) {
        Serial.println(F("Invalid response"));
        HTTPclient.stop();
        deltaHue = 96; // return;
      }
    }

    /* Allocate the JSON document
       Use https://arduinojson.org/v6/assistant to compute the capacity. */
    const size_t capacity = 1024;
    DynamicJsonDocument doc(capacity);

    if (deltaHue > 200U) {
      // Parse JSON object ----
      DeserializationError error = deserializeJson(doc, HTTPclient);
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        HTTPclient.stop();
        hue = 0;
        deltaHue = 96; // return;
      } else {
        // Extract values -----
        String tempStr = doc["feeds"][0]["field1"].as<const char*>();

        //        int dotIndex = tempStr.indexOf('.');
        //        if (dotIndex != -1) tempStr = tempStr.substring(0, dotIndex + 2);

        int temp = tempStr.toInt();
#ifdef USE_OLED
        showTemperature(String(temp));
#endif
        tempStr = doc["feeds"][0]["field4"].as<const char*>();
        hue2 = tempStr.toInt() > 0U; // fanState

        /* set lamp color */
        if ((temp > 36) & (temp < 37)) {
          hue = 72;
        } else {
          if (temp >= 37) {
            hue = 40 - (temp % 37) * 2.75;
          } else {
            hue = 176 - (temp % 37);
          }
        }

#ifdef GENERAL_DEBUG
        LOG.printf("Temperature %d°C | Color • %03d| Fan • %1d\n\r", temp, hue, hue2);
#endif
        doc.clear();
        // Disconnect -------
        HTTPclient.stop();
      }
    }
  }



}

// -------------------------------------
void HeatNetworks() {
  // ХТМ | KHN
  const byte PADDING = HEIGHT > 24U ? 4 : 3;
  const uint8_t BR = 200U;
  const uint16_t TIMEOUT = 500U;
  if (loadingFlag) {
    loadingFlag = false;
    ff_z = 128;
    ff_y = 0;
    if (modes[currentMode].Scale > 3) {
      // hue2 = (modes[currentMode].Scale % 2U) ? 0 : 1;
      /* fan demo --- */
      hue2 = (hue % 2U) ? 0 : 1;
    } else {
      getThermometry();
    }
    fillAll(CHSV(hue, 255, BR));
  }

  // fan on/off -------
  if (hue2) {
    ff_z = 12;
  } else {
    ff_z = 252;
  }

  // bubbles scroll up ----------
  for (uint8_t y = HEIGHT; y > 0U; y--) {
    for (uint8_t x = 0U; x < WIDTH; x++) {
      deltaHue++;
      uint32_t curColor = getPixColorXY(x, y - 1U);
      if (y < HEIGHT - PADDING) {
        if ((curColor == CHSV(hue, 255, BR)) || (curColor == CHSV(ff_x, 255, BR))) {
          /* bacground */
          drawPixelXY(x, y, CHSV(hue, 255, BR));
        } else {
          /* bubbles */
          drawPixelXY(x, y, CHSV(hue, y * 6, 255U));
        }
      }
    }
    deltaValue++;
  }

  /* added bubbles ----- */
  pcnt = random8(WIDTH - 1);
  // pcnt += random8(2);
  if (ff_y <= TIMEOUT) {
    // if (getPixColorXY(pcnt, 1U) != CHSV(hue, 255, BR)) {pcnt++;}

    for (uint8_t x = 0U; x < WIDTH; x++) {
      drawPixelXY(x, 1U, CHSV(hue, (x == pcnt) ? 0 : 255, (x == pcnt) ? 255 : BR));
    }
    DrawLine(0, 0,  WIDTH, 0, CHSV(hue, 255, BR));
  }
  /* fan -------------> */
  if (enlargedObjectNUM > ff_z) {
    enlargedObjectNUM -= 4;
  }
  if (enlargedObjectNUM < ff_z) {
    enlargedObjectNUM += 4;
  }

  drawPixelXY(WIDTH - 1, HEIGHT - 1, CHSV(hue, 255, BR));

  if (step % 4U == 0U) {
    drawPixelXY(WIDTH - 1, HEIGHT - 2, CHSV(hue, enlargedObjectNUM, 255));
    drawPixelXY(WIDTH - 2, HEIGHT - 3, CHSV(hue, enlargedObjectNUM, 255));
    if (PADDING == 4U) drawPixelXY(WIDTH - 1, HEIGHT - 4, CHSV(hue, enlargedObjectNUM, 255));
  } else {
    drawPixelXY(WIDTH - 1, HEIGHT - 2, CHSV(hue, 252, enlargedObjectNUM));
    drawPixelXY(WIDTH - 2, HEIGHT - 3, CHSV(hue, 252, enlargedObjectNUM));
    if (PADDING == 4U) drawPixelXY(WIDTH - 1, HEIGHT - 4, CHSV(hue, 252, enlargedObjectNUM));
  }
  /* --- | fan */

  /* <==== scroll ===== */

  for (uint8_t x = 0U ; x < WIDTH; x++) {
    drawPixelXY(x,  HEIGHT - 1, getPixColorXY(x + 1,  HEIGHT - 1));
    drawPixelXY(x,  HEIGHT - 2, getPixColorXY(x + 1,  HEIGHT - 2));
    drawPixelXY(x,  HEIGHT - 3, getPixColorXY(x + 1,  HEIGHT - 3));
    if (PADDING == 4U) drawPixelXY(x,  HEIGHT - 4, getPixColorXY(x + 1,  HEIGHT - 4));
  }

  if (modes[currentMode].Scale < 3) {
    // standard mode ----
    if (ff_y == TIMEOUT) {
      /* temporary stop of the fan */
      hue2 = 0;
    }
    if (ff_y > TIMEOUT + 65U) {
      getThermometry();
      DrawLine(0, 0,  WIDTH, 0, CHSV(hue, 255, BR));
      ff_y = 0U;
    }
  } else {
    // demo mode --------
    if (modes[currentMode].Scale > 95) {
      EVERY_N_SECONDS(10) {
        ff_y = 0;
        hue += 7;
      }
    } else {
      hue = modes[currentMode].Scale * 2.55;
    }
  }
  ff_y++;
  if (ff_y % HEIGHT == 0U) {
    ff_x = hue;
  }
  step++;
  // LOG.printf_P(PSTR("Step • %03d | Color • %03d | lastColor • %03d | Timer • %06d\n\r"), step, hue, ff_x, ff_y);
}


// ============== Spindle ==============
//             © SlingMaster
//                Веретено
// =====================================
void Spindle() {
  static bool dark;
  if (loadingFlag) {
    IsRandomMode();
    hue = random8(8) * 32; // modes[currentMode].Scale;
    hue2 = 255U;
    dark = modes[currentMode].Scale < 50U;
  }

  if  (modes[currentMode].Scale < 75) {
    blurScreen(32U);
  }

  /* <==== scroll ===== */
  for (uint8_t y = 0U ; y < HEIGHT; y++) {
    for (uint8_t x = 0U ; x < WIDTH - 1; x++) {
      hue2--;
      if (dark) {   // black delimiter -----
        drawPixelXY(WIDTH - 1, y, CHSV(hue, 255, hue2));
      } else {      // white delimiter -----
        drawPixelXY(WIDTH - 1, y, CHSV(hue, 64 + hue2 / 2, 255 - hue2 / 4));
      }
      drawPixelXY(x, y,  getPixColorXY(x + 1,  y));
    }
  }
  if (modes[currentMode].Scale < 5) {

    return;
  }
  if (modes[currentMode].Scale < 50) {
    hue += 4;
  } else {
    if (modes[currentMode].Scale < 25) {
      hue += 2;
    } else {
      hue += 3;
    }
  }
  // LOG.printf_P(PSTR("Step • %03d | Color • %03d | lastColor • %03d | Timer • %06d\n\r"), step, hue, ff_x, ff_y);
}

// ============ Lotus Flower ============
//             © SlingMaster
//             Квітка Лотоса
//---------------------------------------
void drawLotusFlowerFragment(uint8_t posX, byte line) {
  const uint8_t h = (HEIGHT > 24) ? HEIGHT * 0.9 : HEIGHT;
  uint8_t flover_color = 128 + abs(128 - hue);                        // 128 -- 255
  uint8_t gleam = 255 - abs(128 - hue2);                              // 255 -- 128
  float f_size = (128 - abs(128 - deltaValue)) / 150.0;               // 1.0 -- 0.0
  const byte lowBri = 96; // 112U;
  const byte posY = HEIGHT > 32 ? 2 : 0;

  // clear -----
  DrawLine(posX, posY, posX, h * 1.1, CRGB::Black);

  switch (line) {
    case 0:
      /* x | y | x1 y1 | start_color | end_color | start_br | end_br | saturate | angleDeg */
      gradientRect(posX, posY, posX + 1, posY + h * 0.25, 96, 96, 32, 255, 255U, 90);                         // green leaf c
      gradientRect(posX, h * 0.9, posX + 1, h * 1.1, 64, 48, 64, 205, gleam, 90);                             // pestle
      gradientRect(posX, posY + 8, posX + 1, h * 0.6, flover_color, flover_color, 128, lowBri, 255U, 90);     // ---
      break;
    case 2:
    case 6:
      gradientRect(posX, posY + h / 5, posX + 1, h - 4, flover_color, flover_color, lowBri, 255, gleam, 90);  //  -->
      gradientRect(posX, posY + h * 0.05, posX + 1, posY + h * 0.15, 96, 96, 32, 255, 255U, 90);              // green leaf
      break;
    case 3:
    case 5:
      gradientRect(posX, posY + h / 2, posX + 1, h - 2, flover_color, flover_color, lowBri, 255, 255U, 90);   // ---->
      break;
    case 4:
      gradientRect(posX, posY + 1 + h * f_size, posX + 1, h, flover_color, flover_color, lowBri, 255, gleam, 90); // ------>
      break;
    default:
      gradientRect(posX, posY + h * 0.05, posX + 1, posY + h / 5, 80, 96, 160, 64, 255U, 90);                 // green leaf m
      break;
  }
}

//---------------------------------------
void LotusFlower() {
  const byte STEP_OBJ = 8;
  static uint8_t deltaSpeed = 0;
  const byte posY = HEIGHT > 32 ? 2 : 0;
  if (loadingFlag) {
    IsRandomMode();
    step = 0U;
    hue2 = 128U;
    deltaValue = 0;
    hue = 224;
    FPSdelay = SpeedFactor(160);
    if ( getPixColorXY(0, 0) > 0) FastLED.clear();
    if (posY > 1) {
      drawPixelXY(0, 0,  CRGB::Black);
      DrawLine(0, 1, WIDTH - 1, 1, CHSV(96, 40, 56));
      DrawLine(1, 0, WIDTH - 1, 0, CHSV(160, 20, 64));
    }
  }

  if (modes[currentMode].Speed > 128U) {
    if (modes[currentMode].Scale > 50) {
      deltaSpeed = 80U + (128U - abs(128U - deltaValue)) / 1.25;
      FPSdelay = SpeedFactor(deltaSpeed);
      if (step % 256 == 0U ) hue += 32;           /* color morph */
    } else {
      FPSdelay = SpeedFactor(160);
      hue = 28U;
    }
    deltaValue++;     /* size morph  */
    /* <==== scroll ===== */
    drawLotusFlowerFragment(WIDTH - 1, (step % STEP_OBJ));
    for (uint8_t y = posY; y < HEIGHT; y++) {
      for (uint8_t x = 0U ; x < WIDTH; x++) {
        drawPixelXY(x - 1, y,  getPixColorXY(x,  y));
      }
    }
  } else {
    /* <==== morph ===== */
    for (uint8_t x = 0U ; x < WIDTH; x++) {
      drawLotusFlowerFragment(x, (x % STEP_OBJ));
      if (x % 2U) {
        hue2++;         /* gleam morph */
      }
    }
    deltaValue++;       /* size morph  */
    if (modes[currentMode].Scale > 50) {
      hue += 8; /* color morph */
    } else {
      hue = 28U;
    }
  }
  step++;
}

// ======== Digital Тurbulence =========
//             © SlingMaster
//        Цифрова Турбулентність
// =====================================
void drawRandomCol(uint8_t x, uint8_t y, uint8_t offset, uint32_t count) {
  const byte STEP = 32;
  const byte D = HEIGHT / 8;
  uint8_t color = floor(y / D) * STEP + offset;

  if (count == 0U) {
    drawPixelXY(x, y, CHSV(color, 255, random8(8U) == 0U ? (step % 2U ? 0 : 255) : 0));
  } else {
    drawPixelXY(x, y, CHSV(color, 255, (bitRead(count, y ) == 1U) ? (step % 5U ? 0 : 255) : 0));
  }
}

//---------------------------------------
void Turbulence() {
  const byte STEP_COLOR = 255 / HEIGHT;
  const byte STEP_OBJ = 8;
  const byte DEPTH = 2;
  static uint32_t count; // 16777216; = 65536
  uint32_t curColor;
  if (loadingFlag) {
    IsRandomMode();
    step = 0U;
    deltaValue = 0;
    hue = 0;
    if (modes[currentMode].Speed < 20U) {
      FPSdelay = SpeedFactor(30);
    }
    FastLED.clear();
  }

  deltaValue++;     /* size morph  */

  /* <==== scroll =====> */
  for (uint8_t y = HEIGHT; y > 0; y--) {
    drawRandomCol(0, y - 1, hue, count);
    drawRandomCol(WIDTH - 1, y - 1, hue + 128U, count);

    // left -----
    for (uint8_t x = CENTER_X_MAJOR - 1; x > 0; x--) {
      if (x > CENTER_X_MAJOR) {
        if (random8(2) == 0U) { /* scroll up */
          CRGB newColor = getPixColorXY(x, y - 1 );
        }
      }

      /* ---> */
      curColor = getPixColorXY(x - 1, y - 1);
      if (x < CENTER_X_MAJOR - DEPTH / 2) {
        drawPixelXY(x, y - 1, curColor);
      } else {
        if (curColor != 0U) drawPixelXY(x, y - 1, curColor);
      }
    }

    // right -----
    for (uint8_t x = CENTER_X_MAJOR + 1; x < WIDTH; x++) {
      if (x < CENTER_X_MAJOR + DEPTH ) {
        if (random8(2) == 0U)  {  /* scroll up */
          CRGB newColor = getPixColorXY(x, y - 1 );
        }
      }
      /* <---  */
      curColor = getPixColorXY(x, y - 1);
      if (x > CENTER_X_MAJOR + DEPTH / 2 ) {
        drawPixelXY(x - 1, y - 1, curColor);
      } else {
        if (curColor != 0U) drawPixelXY(x - 1, y - 1, curColor);
      }
    }

    /* scroll center up ---- */
    for (uint8_t x = CENTER_X_MAJOR - DEPTH; x < CENTER_X_MAJOR + DEPTH; x++) {
      drawPixelXY(x, y,  makeDarker(getPixColorXY(x, y - 1 ), 128 / y));
      if (y == 1) {
        drawPixelXY(x, 0, CRGB::Black);
      }
    }
    /* --------------------- */
  }

  if (modes[currentMode].Scale > 50) {
    count++;
    if (count % 256 == 0U) hue += 16U;
  } else {
    count = 0;
  }
  step++;
}

// ============== Python ===============
//          base code © Stepko
//             © SlingMaster
//                Пітон
// =====================================
void Python() {
  if (loadingFlag) {
    IsRandomMode();
    deltaValue = 0;

    if (modes[currentMode].Scale < 50U) {
      currentPalette = CopperFireColors_p;
    } else {
      currentPalette = HeatColors_p;
    }
  }

  float t = ( millis() - deltaValue) / 200.;
  for (int8_t y = 0; y < HEIGHT; y++) {
    for (int8_t x  = 0; x < WIDTH; x++) {
      leds[XY(x, y)] = ColorFromPalette(currentPalette, ((sin8((x * 16) + sin8(y * 5 - t * 5.)) + cos8(y / 2 * 10)) + 1) + t);
    }
  }

  if (deltaValue > HEIGHT * 2) {
    deltaValue = 0;
  }
  deltaValue++;
}



// ============== Popuri ===============
//             © SlingMaster
//                Попурі
// =====================================
void Popuri() {
  // const uint8_t *exp_gamma  = gamma_color;
  const byte PADDING = HEIGHT * 0.25;
  const byte step1 = WIDTH;
  const double freq = 3000;
  static int64_t frameCount;
  static byte index;
  // ---------------------

  if (loadingFlag) {
    IsRandomMode();
    hue = 0;
    frameCount = 0;
    currentPalette = LavaColors_p;
    index = modes[currentMode].Scale / 25;

    // ---------------------
    clearNoiseArr();
    if (index < 1) {
      currentPalette = LavaColors_p;
      currentPalette[8] = CRGB::DarkRed;
    } else {
      if (custom_eff) {
        currentPalette = PartyColors_p;
      } else {
        currentPalette = AlcoholFireColors_p;
      }
    }
    FastLED.clear();
  }

  // change color --------
  frameCount++;
  uint8_t t1 = cos8((42 * frameCount) / 30);
  uint8_t t2 = cos8((35 * frameCount) / 30);
  uint8_t t3 = cos8((38 * frameCount) / 30);
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
  // ---------------------

  uint16_t ms = millis();

  // float mn = 255.0 / 13.8;
  float mn = 255.0 / WIDTH; // 27.6;

  if (modes[currentMode].Scale < 50) {
    fillNoiseLED();
    memset8(&noise2[1][0][0], 255, (WIDTH + 1) * (HEIGHT + 1));
  } else {
    fadeToBlackBy(leds, NUM_LEDS, step1);
  }
  // body if big height matrix ---------
  for (uint16_t y = 0; y < HEIGHT; y++) {
    for (uint16_t x = 0; x < WIDTH; x++) {

      if ( (y <= PADDING - 1) || (y >=  HEIGHT - PADDING) ) {
        r = sin8((x - 8) * cos8((y + 20) * 4) / 4);
        g = cos8((y << 3) + t1 + cos8((t3 >> 2) + (x << 3)));
        b = cos8((y << 3) + t2 + cos8(t1 + x + (g >> 2)));

        g = pgm_read_byte(&(gamma_color[g]));
        b = pgm_read_byte(&(gamma_color[b]));

        // if (modes[currentMode].Scale < 50) {
        if (index % 2U == 0) {
          // green blue magenta --
          if (b < 20) b = pgm_read_byte(&(gamma_color[r]));
          r = (g < 128) ? pgm_read_byte(&(gamma_color[b])) / 3 : 0;
        } else {
          // green blue yellow ---
          if (g < 20) g = pgm_read_byte(&(gamma_color[r]));
          r = (b < 128) ? pgm_read_byte(&(gamma_color[g])) / 2 : 0;
        }
        if ( (y == PADDING - 1) || (y ==  HEIGHT - PADDING) ) {
          r = 0;
          g = 0;
          b = 0;
        }
        leds[XY(x, y)] = CRGB(r, g, b);
      } else {
        // ---------------------
        CRGB col;
        if (modes[currentMode].Scale < 50) {
          uint8_t n0 = noise2[0][x][y];
          uint8_t n1 = noise2[0][x + 1][y];
          uint8_t n2 = noise2[0][x][y + 1];
          int8_t xl = n0 - n1;
          int8_t yl = n0 - n2;
          int16_t xa = (x * 255) + ((xl * ((n0 + n1) << 1)) >> 3);
          int16_t ya = (y * 255) + ((yl * ((n0 + n2) << 1)) >> 3);

          col = CHSV(hue, 255U, 255U);
          wu_pixel(xa, ya, &col);
          // ---------------------
        } else {
          uint32_t xx = beatsin16(step1, 0, (HEIGHT - PADDING * 2 - 1) * 256, 0, x * freq);
          uint32_t yy = x * 256;

          if (hue < 80) {
            col = CHSV(0, 255U, 255U);
          } else {
            col = CHSV(hue, 255U, 255U);
          }
          wu_pixel (yy, xx + PADDING * 256, &col);
        }
      }
    }
    if (modes[currentMode].Scale > 50) {
      if (step % WIDTH == 0U) hue++;
    }
  }

  // -----------------
  step++;
}


// ============ Serpentine =============
//             © SlingMaster
//              Серпантин
// =====================================
void Serpentine() {
  const byte PADDING = HEIGHT * 0.25;
  const byte BR_INTERWAL = 64 / HEIGHT;
  const byte DELTA = WIDTH  * 0.25;
  // ---------------------

  if (loadingFlag) {
    IsRandomMode();
    deltaValue = 0;
    hue = 0;
    FastLED.clear();
  }
  // ---------------------

  byte step1 = map8(modes[currentMode].Speed, 10U, 60U);
  uint16_t ms = millis();
  double freq = 3000;
  float mn = 255.0 / 13.8;
  byte fade = 180 - abs(128 - step);
  fadeToBlackBy(leds, NUM_LEDS, fade);

  // -----------------
  for (uint16_t y = 0; y < HEIGHT; y++) {
    uint32_t yy = y * 256;
    uint32_t x1 = beatsin16(step1, WIDTH, (HEIGHT - 1) * 256, WIDTH, y * freq + 32768) / 2;

    // change color --------
    CRGB col1 = CHSV(ms / 29 + y * 256 / (HEIGHT - 1) + 128, 255, 255 - (HEIGHT - y) * BR_INTERWAL);
    CRGB col2 = CHSV(ms / 29 + y * 256 / (HEIGHT - 1), 255, 255 - (HEIGHT - y) * BR_INTERWAL);
    // CRGB col3 = CHSV(ms / 29 + y * 256 / (HEIGHT - 1) + step, 255, 255 - (HEIGHT - y) * BR_INTERWAL - fade);

    wu_pixel( x1 + hue * DELTA, yy - PADDING * (255 - hue), &col1);
    wu_pixel( abs((WIDTH - 1) * 256 - (x1 + hue * DELTA)), yy - PADDING * hue, &col2);
  }

  step++;
  if (step % 64) {
    if (deltaValue == 0) {
      hue++;
      if (hue >= 255) {
        deltaValue = 1;
      }
    } else {
      hue--;
      if (hue < 1) {
        deltaValue = 0;
      }
    }
  }
}

// ============== Scanner ==============
//             © SlingMaster
//                Сканер
// =====================================
void Scanner() {
  static byte i;
  static bool v_scanner = HEIGHT >= WIDTH;
  if (loadingFlag) {
    IsRandomMode();
    deltaValue = 0;
    hue = modes[currentMode].Scale * 2.55;
    deltaHue = modes[currentMode].Scale;
    i = 5;
    FastLED.clear();
  }

  if (step % 2U == 0U) {
    if (deltaValue == 0U) {
      i++;
    } else {
      i--;
    }
    if (deltaHue == 0U) {
      hue++;
    }
  }
  if (i > 250) {
    i = 0;
    deltaValue = 0;
  }
  fadeToBlackBy(leds, NUM_LEDS, v_scanner ? 50 : 30);

  if (v_scanner) {
    /* vertical scanner */
    if (i >= HEIGHT - 1) {
      deltaValue = 1;
    }

    for (uint16_t x = 0; x < WIDTH; x++) {
      leds[XY(x, i)] = CHSV(hue, 255U, 180U);
      if ((x == i / 2.0) & (i % 2U == 0U)) {
        if (deltaValue == 0U) {
          drawPixelXYF(random(WIDTH) - (random8(2U) ? 1.5 : 1), i * 0.9, CHSV(hue, 16U, 255U) );
        } else {
          drawPixelXYF(random(WIDTH) - 1.5, i * 1.1, CHSV(hue, 16U, 255U) );
        }
      }
    }
  } else {
    /* horizontal scanner */
    if (i >= WIDTH - 1) {
      deltaValue = 1;
    }

    for (uint16_t y = 0; y < HEIGHT; y++) {
      leds[XY(i, y)] = CHSV(hue, 255U, 180U);
      if ((y == i / 2.0) & (i % 2U == 0U)) {
        if (deltaValue == 0U) {
          drawPixelXYF(i * 0.9, random(HEIGHT) - (random8(2U) ? 1.5 : 1), CHSV(hue, 16U, 255U) );
        } else {
          drawPixelXYF( i * 1.1, random(HEIGHT) - 1.5, CHSV(hue, 16U, 255U) );
        }
      }
    }
  }
  step++;
}


// ============== Avrora ===============
//             © SlingMaster
//                Аврора
// =====================================
void Avrora() {
  const byte PADDING = HEIGHT * 0.25;
  const float BR_INTERWAL = WIDTH / HEIGHT;

  // ---------------------

  if (loadingFlag) {
    IsRandomMode();
    deltaValue = 0;
    hue = 0;
    FastLED.clear();
  }
  // ---------------------

  byte step1 = map8(modes[currentMode].Speed, 10U, 60U);
  uint16_t ms = millis();
  double freq = 3000;
  float mn = 255.0 / 13.8;
  const byte fade = 30; //60 - abs(128 - step) / 3;
  fadeToBlackBy(leds, NUM_LEDS, fade);

  // -----------------
  for (uint16_t y = 0; y < HEIGHT; y++) {
    uint32_t yy = y * 256;
    uint32_t x1 = beatsin16(step1, WIDTH, (HEIGHT - 1) * 256, WIDTH, y * freq + 32768) / 1.5;

    /* change color -------- */
    byte cur_color = ms / 29 + y * 256 / HEIGHT;
    CRGB color = CHSV(cur_color, 255, 255 - y * HEIGHT / 8);
    byte br = constrain(255 - y * HEIGHT / 5, 0, 200);
    CRGB color2 = CHSV(cur_color - 32, 255 - y * HEIGHT / 4, br);

    wu_pixel( x1 + hue + PADDING * hue / 2, yy, &color);
    wu_pixel( abs((WIDTH - 1) * 256 - (x1 + hue)), yy - PADDING * hue, &color2);
  }

  step++;
  if (step % 64) {
    if (deltaValue == 1) {
      hue++;
      if (hue >= 255) {
        deltaValue = 0;
      }
    } else {
      hue--;
      if (hue < 1) {
        deltaValue = 1;
      }
    }
  }
}

// =========== Rainbow Spot ============
//             © SlingMaster
//            Веселкова Пляма
// =====================================
void RainbowSpot() {
  const uint8_t STEP = 255 / CENTER_X_MINOR;
  float distance;

  if (loadingFlag) {
    IsRandomMode();
    deltaValue = modes[currentMode].Scale;
    hue = 96;
    emitterY = 0;
    FastLED.clear();
  }
  // Calculate the radius based on the sound value --
  float radius = abs(128 - step) / 127.0 * max(CENTER_X_MINOR, CENTER_Y_MINOR);

  // Loop through all matrix points -----------------
  for (uint8_t x = 0; x < WIDTH; x++) {
    for (uint8_t y = 0; y < HEIGHT; y++) {
      // Calculate the distance from the center to the current point
      distance = sqrt(pow(x - CENTER_X_MINOR - 1, 2) + pow(y - CENTER_Y_MINOR - emitterY, 2));
      hue = step + distance * radius;

      // Check if the point is inside the radius ----
      deltaHue = 200 - STEP * distance * 0.25;

      if (distance < radius) {
        if (modes[currentMode].Scale > 50) {
          if (x % 2 & y % 2) {
            drawPixelXYF(x, y - CENTER_Y_MINOR / 2 + emitterY, CHSV(hue, 255, 64));
          } else {
            leds[XY(x, y)] = CHSV(hue + 32, 255 - distance, deltaHue);
          }
        } else {
          leds[XY(x, y)] = CHSV(hue, 255 - distance, 255);
        }

      } else {
        if (modes[currentMode].Scale > 75) {
          leds[XY(x, y)] = CHSV(hue + 96, 255, deltaHue);
        } else {
          leds[XY(x, y)] = CHSV(hue, 255, deltaHue);
        }
      }
    }
  }
  blurScreen(48);
  if (modes[currentMode].Scale > 50) {
    if (emitterY > pcnt) {
      emitterY -= 0.25;
    } else {
      if (emitterY < pcnt) {
        emitterY += 0.25;
      } else {
        pcnt = random8(CENTER_Y_MINOR);
      }
    }
  } else {
    emitterY = 0;
  }
  step++;
}

// ============== Fountain =============
//             © SlingMaster
//                Водограй
// =====================================
void Fountain() {
  uint8_t const gamma[6] = {0, 96, 128, 160, 240, 112};
  const byte PADDING = round(HEIGHT / 8);
  byte br;

  if (loadingFlag) {
    IsRandomMode();
    deltaValue = modes[currentMode].Scale / 20;
    emitterY = 0;
    FastLED.clear();
  }
  DrawLine(0, 1, WIDTH - 1, 1, CHSV(gamma[deltaValue], 40, 64));
  DrawLine(0, 0, WIDTH - 1, 0, CHSV(160, 0, 64));

  float radius = abs(128 - step) / 127.0 * CENTER_Y_MINOR;
  for (uint8_t y = 2; y < HEIGHT; y++) {
    for (uint8_t x = 0; x < WIDTH; x++) {
      if (x % 2 == 0) {
        br = constrain(255 / (emitterY + 1) * y, 48, 255);

        if ((x % 4) == 0) {
          hue = gamma[deltaValue];
          if (y == byte(emitterY - radius) + random8(1, 4)) {
            if (step % 2 == 0) {
              drawPixelXYF(x, y + 0.5, CHSV(hue, 200, 255));
            } else {
              drawPixelXY(x, y, CHSV(hue, 200, 255));
            }
          } else {
            drawPixelXY(x, y, CHSV(hue, 255, (y > emitterY - radius / 2) ? 0 : br));
          }
        } else {
          hue = gamma[deltaValue + 1];
          if (y == byte(emitterY * 0.70 + radius + random8(3))) {
            drawPixelXYF(x, y - 0.5, CHSV(hue - radius, 160, 255));
          } else {
            byte delta = emitterY * 0.70 + radius;
            drawPixelXY(x, y, CHSV(hue - radius, 255,  ( y > delta) ? 0 : br));
          }
        }
      } else {
        // clear blur ----
        if (pcnt > PADDING + 2) drawPixelXY(x, y, CRGB::Black);
      }
    }
  }

  if ((emitterY <= PADDING * 2) || (emitterY > HEIGHT - PADDING - 1)) blurScreen(32);

  if (emitterY > pcnt) {
    emitterY -= 0.5;
    if (abs(pcnt - emitterY ) < PADDING) {
      if (emitterY > pcnt) emitterY -= 0.5;
    }
  } else {
    if (emitterY < pcnt) {
      emitterY += 3;
    } else {
      pcnt = random8(2, HEIGHT - PADDING - 1);
    }
  }
  step++;
}


// =============== Worms ===============
//             © SlingMaster
//                 Worms
// =====================================
void Worms() {
  const byte BR_STEP = 255 / (HEIGHT + 12);
  const byte STEP = (HEIGHT > 8) ? 3 : 1;
  const byte IDX = ((HEIGHT > WIDTH) ? CENTER_X_MINOR + STEP : HEIGHT);

  if (loadingFlag) {
    IsRandomMode();
    /* init worms */
    // noise3d[NUM_LAYERSMAX][WIDTH][HEIGHT];
    for (uint8_t i = 0; i < IDX; i++) {
      // set x position ----------
      noise3d[0][i][0] = random8(WIDTH + CENTER_X_MINOR);
      // set y position ----------
      noise3d[1][i][0] = i * STEP;
      // set color pixel ---------
      noise3d[0][IDX + i][0] = floor(random8(255) / 32) * 32;
    }

    // fade ----------------------
    pcnt = modes[currentMode].Scale + ((HEIGHT > 8) ? 20 : 0) ;

    FastLED.clear();
  }

  if (deltaValue & (HEIGHT > 8)) {
    blurScreen(64);
    //blur2d(leds, WIDTH, CENTER_Y_MINOR + random8(4), 48);
    deltaValue = 0;
  } else {
    fadeToBlackBy(leds, NUM_LEDS, pcnt );
  }

  for (uint8_t j = 0; j < IDX; j++) {
    hue =  noise3d[0][IDX + j][0];
    byte x = noise3d[0][j][0];
    byte y = noise3d[1][j][0]; // + (x % 2);
    noise3d[0][j][0] = x - 1;

    drawPixelXY(x, y, CHSV(hue, 255 - abs(128 - step) / 2, 255 - y * BR_STEP));

    if (x == 0) {
      noise3d[0][j][0] = WIDTH - 1;
      // noise3d[1][j][0] = (y > 1) ? y - 2 : y - 1;
      noise3d[1][j][0] = y - 1;
      if (y == 0) {
        noise3d[1][j][0] = HEIGHT - 1;
        noise3d[0][IDX + j][0] = hue + 32;
      }
      if (y % 2 == 0) deltaValue = 1;
    }
  }
  step++;
}

// =========== Rainbow Rings ===========
//    base code © Martin Kleppe @aemkei
//             © SlingMaster
//            Райдужні кільця
// =====================================
float codeEff(double t, double i, double x, double y) {
  hue = 255U; hue2 = 0U; // | CENTER_X_MAJOR
  return sin16((t - sqrt3((x - CENTER_X_MAJOR) * (x - CENTER_X_MAJOR) + (y - CENTER_Y_MAJOR) * (y - CENTER_Y_MAJOR))) * 8192.0) / 32767.0;
}

// --------------------------------------
void drawFrame(double t, double x, double y) {
  static uint32_t t_count;
  static byte scaleXY = 8;
  double i = (y * WIDTH) + x;
  double frame = constrain(codeEff(t, i, x, y), -1, 1) * 255;
  uint16_t tt = floor(i);
  byte xx;
  byte yy;
  byte angle;
  byte radius;

  if (frame > 0) {
    // white or black color
    if (modes[currentMode].Scale > 70) {
      if (modes[currentMode].Scale > 90) {
        drawPixelXY(x, y, CRGB(frame / 4, frame / 2, frame / 2));
      } else {
        drawPixelXY(x, y, CRGB(frame / 2, frame / 2, frame / 4));
      }

    } else {
      drawPixelXY(x, y, CRGB::Black);
    }
  } else {
    if (frame < 0) {
      switch (deltaHue2) {
        case 0:
          hue = step + y * x;
          break;
        case 1:
          hue = 64 + (y + x) * abs(128 - step) / CENTER_Y_MAJOR;
          break;
        case 2:
          hue = y * x + abs(y - CENTER_Y_MAJOR) * 4;
          break;
        case 3:
          xx = (byte)x;
          yy = (byte)y;
          angle = noise3d[0][xx][yy];
          radius = noise3d[1][xx][yy];
          if ((xx == 0) & (yy == 0))  t_count += 8;
          hue = (angle * scaleXY) + (radius * scaleXY) + t_count;
          break;
        default:
          hue = step + y * x;
          break;
      }
      drawPixelXY(x, y, CHSV( hue, frame * -1, frame * -1));
    } else {
      drawPixelXY(x, y, CRGB::Black);
    }
  }
}

// -------------------------------------
void RainbowRings() {
  if (loadingFlag) {
    IsRandomMode();
    deltaHue = 0;
    FPSdelay = 1;
    deltaHue2 = modes[currentMode].Scale / 22;
    hue = 255U; hue2 = 0U;

    for (int8_t x = -CENTER_X_MAJOR; x < CENTER_X_MAJOR; x++) {
      for (int8_t y = CENTER_X_MAJOR; y < HEIGHT; y++) {
        noise3d[0][x + CENTER_X_MAJOR][y] = 128 * (atan2(y, x) / PI);
        noise3d[1][x + CENTER_X_MAJOR][y] = hypot(x, y);                    // thanks Sutaburosu
      }
    }
  }

  // *****
  unsigned long milli = millis();
  double t = milli / 1000.0;

  for ( double x = 0; x < WIDTH; x++) {
    for (double y = 0; y < HEIGHT; y++) {
      drawFrame(t, x, y);
    }
  }
  step++;
}


// ========= Frost on window ===========
//             © SlingMaster
//             Іній на вікні
// =====================================
void drawSnow(uint8_t posX, uint8_t posY, uint8_t color, uint8_t transparency, uint8_t br) {
  if (HEIGHT > 32) posY += 2;
  static const uint8_t snow[9][9] PROGMEM = {
    {0, 0, 0, 0, 1, 0, 0, 0, 0 },
    {0, 1, 0, 1, 1, 1, 0, 1, 0 },
    {0, 0, 1, 0, 1, 0, 1, 0, 0 },
    {0, 1, 0, 1, 1, 1, 0, 1, 0 },
    {1, 1, 1, 1, 0, 1, 1, 1, 1 },
    {0, 1, 0, 1, 1, 1, 0, 1, 0 },
    {0, 0, 1, 0, 1, 0, 1, 0, 0 },
    {0, 1, 0, 1, 1, 1, 0, 1, 0 },
    {0, 0, 0, 0, 1, 0, 0, 0, 0 }
  };

  for (int y = 0U; y < 9; y++) {
    for (int x = 0U; x < 9; x++) {
      if (pgm_read_byte(&(snow[y][x])) == 1)
        drawPixelXY(posX + x, posY - y,  CHSV(color, transparency, br));
    }
  }
}

// -------------------------------------
void Frost() {
  const byte STEP = 8;
  const byte START_BR = 96;
  const byte MAX_BR = 190;
  const uint32_t max_val = pow(2, WIDTH);
  uint32_t value;
  CRGB curColor;
  // ---------------------

  if (loadingFlag) {
    IsRandomMode();
    pcnt = 0;
    FPSdelay = 200;
    hue = random8(2, 4);
    step = 0;


    for (uint8_t y = 0; y < HEIGHT; y++) {
      for (uint8_t x = 0; x < WIDTH; x++) {
        if (y == 0 || y == HEIGHT - 1) {
          noise3d[0][x][y] = random8(10, START_BR);
        } else {
          noise3d[0][x][y] = 0;
        }
      }
    }
    FastLED.clear();
  }

  if (pcnt == 0) {
    for (uint8_t y = 0; y < HEIGHT; y++) {
      for (uint8_t x = 0; x < WIDTH; x++) {
        // top -----
        if (y > CENTER_Y_MAJOR) {

          // if (random8(WIDTH) % 4 == x & (y % 4 == 0) & noise3d[0][x][y] > MAX_BR ) noise3d[0][x][y] = 255;

          drawPixelXY(x,  y, CHSV(145, 0, noise3d[0][x][y]));
          if (noise3d[0][x][y] < MAX_BR) {
            if (noise3d[0][x][y] > 0) noise3d[0][x][y] += STEP + x;
          } else {
            if (random8(WIDTH) % 8 == 0)  noise3d[0][x][y] -= 5;
            if (noise3d[0][x][y - 1] == 0) noise3d[0][x][y - 1] = random8(10, START_BR);
          }
        }
        // bottom --
        if (y < CENTER_Y_MAJOR) {
          int br = noise3d[0][x][y] - y * 6;
          if (br < 0) br = 5;
          drawPixelXY(x, y, CHSV(145, 0, br));
          if (noise3d[0][x][y] < MAX_BR) {
            if (noise3d[0][x][y] > 0) noise3d[0][x][y] += STEP + (WIDTH - x);
          } else {
            if (random8(WIDTH) % 4 == 0)  noise3d[0][x][y] -= 5;
            if (noise3d[0][x][y + 1] == 0) noise3d[0][x][y + 1] = random8(10, START_BR);
          }
        }
      }
    }
    if (noise3d[0][CENTER_X_MAJOR][CENTER_Y_MAJOR + hue] > 0) {
      pcnt = 1;
      FPSdelay = 50;
    }
  } else {
    if (step > 128)  FPSdelay = 205;
    if (step == 175) {
      for (uint8_t x = 0U; x < WIDTH; x++) {
        drawPixelXY(x, HEIGHT - 1, CHSV(160, 0, 0));
      }
    }
    if (step > 175) {
      /* scroll down */
      FPSdelay = 5;
      value = random(max_val);
      for (uint8_t x = 0U; x < WIDTH; x++) {
        if ( bitRead(value, x ) == 0) {
          for (uint8_t y = 0U; y < HEIGHT - 1; y++) {
            drawPixelXY(x, y, getPixColorXY(x, y + 1U));
          }
        }
      }
      pcnt++;
      if (pcnt > HEIGHT + 8) {
        loadingFlag = true;
      }
    } else {
      drawSnow(CENTER_X_MAJOR - 7, HEIGHT - 4, 160, step, MAX_BR + 16);
      drawSnow(CENTER_X_MAJOR - 2, 11, 145, step, MAX_BR + 16);
      step++;
    }
  }
}

// ============ Dew In Sun =============
//             © SlingMaster
//             Роса на сонці
// =====================================

void Grass(int8_t posX, uint8_t color) {
  uint8_t br = random8(6, 16) + (posX % 2) * 2;
  for (uint8_t y = 0U; y < CENTER_Y_MAJOR; y++) {
    drawPixelXY(posX, CENTER_Y_MAJOR - y - 1, CHSV(color, 255, y * br));
  }
}

// ---------------------
void Butterfly(uint8_t idx, uint8_t color) {
  if (step % 2) {
    uint8_t posX = noise3d[0][idx][HEIGHT - 1];
    uint8_t posY = noise3d[1][idx][HEIGHT - 1];
    for (uint8_t y = 0; y < 2; y++) {
      for (uint8_t x = 0; x < 2; x++) {
        drawPixelXY(posX + x, posY - y + 1, CHSV(color + 32 * idx, 255, 200 - y * 96));
      }
    }

    if (random8(5) > 2) {
      posX++;
    } else {
      posX--;
    }
    if (random8(5) > 2) {
      posY++;
    } else {
      posY--;
    }

    if (posX > WIDTH - 3) posX -= 2;
    if (posX < 1) posX = WIDTH - 2;
    if (posY > HEIGHT - 1) posY -= 2;
    if (posY < CENTER_Y_MINOR) posY += 2;
    noise3d[0][idx][HEIGHT - 1] = posX;
    noise3d[1][idx][HEIGHT - 1] = posY;
  }
}

// ---------------------
void DewInSun() {
  const byte PADDING = HEIGHT * 0.5;
  const byte START_BR = 210;
  static float posX;
  static float posY;
  const byte START_BYTE = PADDING + 1;
  // ---------------------

  if (loadingFlag) {
    IsRandomMode();
    /* MAX_BUTTERFLY */
    deltaValue = modes[currentMode].Scale / 10 + 1;
    /* reset array */
    for (uint8_t y = 0; y < PADDING; y ++) {
      for (uint8_t x = 0; x < WIDTH; x ++) {
        noise3d[0][x][y] = 0;
        noise3d[1][x][y] = 0;
      }
    }

    /* create dew */
    for (uint8_t y = 0; y < PADDING - 6; y ++) {
      for (uint8_t x = 0; x < WIDTH; x ++) {
        if (random8(128) > 100 & x % 2 == 0 & y % 2) {
          noise3d[0][x][y] = random8(11, START_BR);
        } else {
          noise3d[0][x][y] = 0;
        }
      }
    }

    /* create butterfly */
    for (uint8_t i = 0; i < deltaValue; i++) {
      noise3d[0][i][HEIGHT - 1] = random8(7, WIDTH) + i; /* x pos*/
      noise3d[1][i][HEIGHT - 1] = random8(CENTER_Y_MINOR, HEIGHT) + i; /* y pos*/
      Butterfly(i, 128);
    }

    posY = 0;
    FastLED.clear();
    /* draw Grass */
    for (uint8_t x = 0U; x < WIDTH; x++) {
      Grass(x, 96 - random8(16));
    }
  }

  dimmCustom(80, leds, CENTER_Y_MAJOR - 1, HEIGHT - 1);

  /* draw and update dew */
  for (uint8_t y = 0; y < PADDING; y++) {
    for (uint8_t x = 0; x < WIDTH; x++) {
      if (noise3d[0][x][y] > 10) {
        drawPixelXY(x, y, CHSV(30, (noise3d[0][x][y] == 255 ? 128 : 0), noise3d[0][x][y]));
        noise3d[0][x][y] -= 6;
        if (noise3d[0][x][y] < 48)  noise3d[0][x][y] = 255;
      }
    }
  }

  /* update butterfly */
  for (uint8_t i = 0; i < deltaValue; i++) {
    Butterfly(i, 128);
  }
  step++;
}


// =============== Atom ================
//             © SlingMaster
//                 Атом
// =====================================

void Atom() {
  static float angle;
  static float coreRadius;
  static float orbitRadius;
  static float coreRadiusSq;

  if (loadingFlag) {
    IsRandomMode();
    FastLED.clear();
    angle = 0.0;
    coreRadius = min(WIDTH, HEIGHT) * 0.35;
    orbitRadius = coreRadius + 7.5;
    hue2 = true;
    pcnt = 0;
  }

  fadeToBlackBy(leds, NUM_LEDS, 50);

  // --- dynamic color core ---
  deltaValue = abs(step - 127);
  hue = 210 + deltaValue / 3;
  coreRadiusSq = coreRadius * coreRadius;

  /* draw core */
  for (int y = -coreRadius - 2; y <= coreRadius + 2; y++) {
    for (int x = -coreRadius - 2; x <= coreRadius + 2; x++) {
      int px = CENTER_X_MINOR + x;
      int py = CENTER_Y_MINOR + y;

      int distSq = x * x + y * y;
      if (distSq <= coreRadiusSq) {
        uint8_t brightness = 255 - ((distSq * 215) / coreRadiusSq);
        drawPixelXY(px, py, CHSV(hue, 255, qsub8(brightness, deltaValue)));
      }
    }
  }

  // --- draw orbits ---
  const float angles3[3] = {0, 2 * PI / 3, 4 * PI / 3};
  const int trailLen = 15;

  float cosAngle = cos(angle);
  float sinAngle = sin(angle);
  float sinStep = sin(step * 0.05);

  for (int i = 0; i < 3; i++) {
    float planeAngle = angles3[i];
    float cosP = cos(planeAngle);
    float sinP = sin(planeAngle);

    // --- blue track ---
    for (int t = 0; t < trailLen; t++) {
      float a = angle - t * 0.05 + 0.01 * sinStep;
      float ox = orbitRadius * cos(a);
      float oy = orbitRadius * sin(a) * 0.51;
      float fx = CENTER_X_MINOR + ox * cosP - oy * sinP;
      float fy = CENTER_Y_MAJOR + ox * sinP + oy * cosP;
      uint8_t bright = 160 - (140 * t) / (trailLen - 1);
      drawPixelXYF(fx, fy, CHSV(160 - deltaValue / 4, 240, bright));
    }

    // --- white atom ---
    float ox = orbitRadius * cosAngle;
    float oy = orbitRadius * sinAngle * 0.51;
    float fx = CENTER_X_MINOR + ox * cosP - oy * sinP;
    float fy = CENTER_Y_MAJOR + ox * sinP + oy * cosP;

    float dx = fx - CENTER_X_MINOR;
    float dy = fy - CENTER_Y_MINOR;
    float d = sqrt(dx * dx + dy * dy);
    uint8_t v = 150 + (105.0 / (1 + d));
    drawPixelXYF(fx, fy, CHSV(160, 55, v));
    if (abs(fx - CENTER_X_MINOR) < 2)  drawPixelXYF(fx + 0.5, fy, CHSV(160, 10, 255));
  }

  angle += 0.05;
  if (angle > 2 * PI) angle -= 2 * PI;
  step++;
}


// =============== Lilac ===============
//             © SlingMaster
//     на базі колу від © SottNick
//                 Бузок
// =====================================

void Lilac() {
  const byte PADDING = HEIGHT * 0.2;
  const byte BR = 128; //45
  static const uint8_t colorLilac[4] PROGMEM = {0, 192, 216, 145 };
  if (loadingFlag) {
    IsRandomMode();
    hue = 0U;
    step = 0;

    deltaValue = (modes[currentMode].Speed / 23);              // вероятность пикселя загореться от 1/1 до 1/11
    pcnt = (modes[currentMode].Scale / 28);
    hue2 = pgm_read_byte(&(colorLilac[pcnt]));
    for (uint16_t idx = 0; idx < NUM_LEDS; idx++) {
      if (random8(deltaValue) == 0) {
        ledsbuff[idx].r = (random8(10) > 6) ? 96 : (hue2 + random8(8));                           // оттенок пикселя
        ledsbuff[idx].g = random8(1, TWINKLES_SPEEDS * 2 + 1); // скорость и направление (нарастает 1-4 или угасает 5-8)
        ledsbuff[idx].b = random8();                           // яркость
      }
      else
        ledsbuff[idx] = 0;                                     // всё выкл
    }
    gradientRect(0, PADDING, WIDTH, HEIGHT, hue2, hue2 + 16, ((pcnt == 0) ? 104 : 240), BR, (hue2 == 0) ? 0 : 255U, 45);
  }

  emitterY = step * 1.411764f;
  gradientRect(0, 0, WIDTH, PADDING, 96, 104, 160, 88, 255U, emitterY);
  for  (uint32_t x = 1; x < WIDTH - 1; x++) {
    if (x % 2 == 0) gradientRect(x, 1, x, PADDING, 96, 96, 144, 80, 255U, 90);
    for (uint32_t y = PADDING; y < HEIGHT; y++) {

      uint16_t idx = XY(x, y);
      if (ledsbuff[idx].b == 0) {
        if (random8(deltaValue) == 0 && hue > 0) {              // если пиксель ещё не горит, зажигаем каждый ХЗй
          if (ledsbuff[idx].r != 96) ledsbuff[idx].r = hue2;    // оттенок пикселя
          ledsbuff[idx].g = random8(1, TWINKLES_SPEEDS + 1);    // скорость и направление (нарастает 1-4, но не угасает 5-8)
          ledsbuff[idx].b = ledsbuff[idx].g;                    // яркость
          hue--;                                                // уменьшаем количество погасших пикселей
        }
      }
      else if (ledsbuff[idx].g <= TWINKLES_SPEEDS) {            // если нарастание яркости
        if (ledsbuff[idx].b > 255U - ledsbuff[idx].g - TWINKLES_MULTIPLIER) {           // если досигнут максимум
          ledsbuff[idx].b = 255U;
          ledsbuff[idx].g = ledsbuff[idx].g + TWINKLES_SPEEDS;
        }
        else
          ledsbuff[idx].b = ledsbuff[idx].b + ledsbuff[idx].g + TWINKLES_MULTIPLIER;
      }
      else {                                                    // если угасание яркости
        if (ledsbuff[idx].b <= ledsbuff[idx].g - TWINKLES_SPEEDS + TWINKLES_MULTIPLIER) { // если досигнут минимум
          ledsbuff[idx].b = 0;                                  // всё выкл
          hue++;                                                // считаем количество погасших пикселей
        }
        else
          ledsbuff[idx].b = ledsbuff[idx].b - ledsbuff[idx].g + TWINKLES_SPEEDS - TWINKLES_MULTIPLIER;
      }

      if (ledsbuff[idx].b == 0) {
        if (ledsbuff[idx].r == 96) {
          leds[idx] = CHSV(96, 255U, BR * 2);
        } else {
          leds[idx] = getPixColorXY(0, y);
        }
      } else {
        if (ledsbuff[idx].r != 96) leds[idx] = CHSV(hue2, (hue2 == 0) ? 0 : 255U, ledsbuff[idx].b);
      }
    }
  }
  step++;
}

// ============= Gradients =============
//             © SlingMaster
//               Градієнти
// =====================================
void Gradients() {
  const uint8_t BR = 104;
  const uint8_t STEP = 6;
  const byte PADDING = HEIGHT > 32 ? 2 : 0;

  if (loadingFlag) {
    IsRandomMode();
    step = 0;
    deltaValue = 1;
    emitterX = 45 * (modes[currentMode].Scale / 25 + 1);
  }

  if (modes[currentMode].Scale  > 75)  {
    emitterX++;
    if (emitterX > 359) emitterX = 0;
    gradientRect(0, PADDING, WIDTH, HEIGHT, 0, 255, 255, 255, 255U, emitterX);
  } else {
    if (step > 80) deltaValue = 0;
    if (step < 1) deltaValue = 1;
    if (deltaValue) step++; else step--;
    gradientRect(0, PADDING, WIDTH, HEIGHT, 80 + step, 160 + step, 255, 255, 255U, emitterX);
  }
  if (PADDING > 0) gradientRect(0, 0, WIDTH, 1, 48, 48, 255, 180, modes[currentMode].Scale / 25 * 40 , 90);
  for (int i = 0; i < 10; i++) {  /* black cross lines */
    DrawLine(-1, i * STEP, WIDTH - 1, i * STEP + WIDTH, CRGB::Black);
    DrawLine(-1, i * STEP + WIDTH, WIDTH - 1, i * STEP, CRGB::Black);
  }
}

// ======== Intertwined Colors =========
//             © SlingMaster
//         Переплетені Кольори
// =====================================
void IntertwinedColors() {
  const uint8_t BR = 96;
  const uint8_t BR_STEP = 255 / HEIGHT;
  const uint8_t OFFSET = 128;
  const byte PADDING = HEIGHT > 32 ? 2 : 0;

  if (loadingFlag) {
    IsRandomMode();
    hue2 = 0;
    FastLED.clear();
    hue = 160;
    step = 0;
    deltaValue = 1;
    pcnt = 6 + modes[currentMode].Scale / 25.0;
  }

  if (pcnt > 8) {
    fadeToBlackBy(leds, NUM_LEDS, 160);
    //    fillAll(CRGB::Silver);
  } else {
    FastLED.clear();
  }

  for (step = 0; step < WIDTH; step ++) {
    for (uint8_t y = 0; y < HEIGHT; y ++) {
      deltaValue = y;
      for (uint8_t x = 0; x < WIDTH + pcnt; x ++) {

        if ( ( (x + step) % pcnt == 0 ) & ( (y + step) % pcnt == 0)) {
          /* left > right */
          drawPixelXY(x + hue2 - pcnt, y + PADDING, CHSV(hue + deltaValue, 255, 255 - y * BR_STEP ));
          if (y < CENTER_X_MAJOR * 0.5) {
            drawPixelXY(x + hue2 - pcnt + 1, y + PADDING, CHSV(hue + deltaValue, 255, 128 - y * BR_STEP ));
          }

          /* left < right */
          drawPixelXY(WIDTH - x + hue2, y + PADDING, CHSV(hue + deltaValue + OFFSET, 255, 255 - y * BR_STEP));
          if (y < CENTER_Y_MAJOR) drawPixelXY(WIDTH - x - 1 + hue2, y + PADDING, CHSV(hue + deltaValue + OFFSET, 255, 128 - y * BR_STEP));
        }

#ifdef JAVELIN_VII
        if (x < WIDTH & y == 1) drawPixelXY(x, y, CHSV(hue + deltaValue, 255, 255));
        if (y == 0) {
          if ( (x + step) % pcnt == 0 & (y + step) % pcnt == 0) {
            drawPixelXY(hue2 + x, y, CHSV(hue + deltaValue + OFFSET, 255, BR));
            drawPixelXY(hue2 + x + 1, y, CHSV(hue + deltaValue + OFFSET,  255, BR));
          }
        }
#endif

      }
    }
  }

  hue2++;
  if (hue2 >= pcnt) {
    hue2 = 0;
  }
  hue++;
}

// ========= Fibonacci Spiral ==========
//        © SlingMaster & ChatGPT
//          Спіраль Фібоначчі
// =====================================
void FibonacciSpiral() {
  const float SC = min(WIDTH, HEIGHT ) * 0.30f;
  static int val;

  if (loadingFlag) {
    IsRandomMode();
    deltaValue = 1 + (modes[currentMode].Scale / 100.0f * 5);
    deltaHue2 = (128 - modes[currentMode].Speed) / 8;
    pcnt = deltaValue * 2.75;
    emitterX = 1.61803398875 / deltaValue;
    FastLED.clear();
  }

  float t = millis() * 0.0002f * speedfactor;   // animation
  for (uint8_t y = 0; y < HEIGHT; y++) {
    for (uint8_t x = 0; x < WIDTH; x++) {

      float dx = x - CENTER_X_MINOR;
      float dy = y - CENTER_Y_MAJOR + pcnt;
      float dist = sqrtf(dx * dx + dy * dy);

      float angle = atan2f(dy, dx);

      // phase Fibonacci
      float fibPhase = dist * emitterX + angle * 2.0f + t;
      hue2 = hue + (uint8_t)(fibPhase * 20.0f);
      drawPixelXY(x, y, CHSV(hue2, 255 - dist * SC, 255));
    }
  }
  hue += deltaHue2;
  step++;
}

// ============= Galaxies ==============
//        © SlingMaster & ChatGPT
//              Галактики
// =====================================
/* https://uk.wikipedia.org/wiki/%D0%9F%D0%B5%D1%80%D0%B5%D0%BB%D1%96%D0%BA_%D0%B3%D0%B0%D0%BB%D0%B0%D0%BA%D1%82%D0%B8%D0%BA */

/* -----------------------------------*/
void createStarfield(uint8_t maxStars, bool constellations) {
  // очистка
  const byte PADDING = HEIGHT > 32 ? 2 : 0;
  for (uint8_t y = 0; y < HEIGHT; y++) {
    for (uint8_t x = 0; x < WIDTH; x++) {
      noise3d[0][x][y] = 0;
    }
  }
  if (WIDTH < 18 || HEIGHT < 18) constellations = false;
  uint8_t stars = 0;
  uint16_t attempts = 0;
  const uint16_t maxAttempts = WIDTH * HEIGHT * 4;
  if (constellations) {
    stars = 15;
    /* Ursa Minor | Мала Ведмедиця */
    noise3d[0][CENTER_X_MINOR][CENTER_Y_MAJOR + 10] = 200;    /* α | Полярна зірка */
    noise3d[0][CENTER_X_MINOR - 2][CENTER_Y_MAJOR + 9] = 90;  /* δ */
    noise3d[0][CENTER_X_MINOR - 4][CENTER_Y_MAJOR + 8] = 100; /* ε */
    noise3d[0][CENTER_X_MINOR - 4][CENTER_Y_MAJOR + 6] = 90;  /* ζ */
    noise3d[0][CENTER_X_MINOR - 6][CENTER_Y_MAJOR + 5] = 90;  /* η */
    noise3d[0][CENTER_X_MINOR - 4][CENTER_Y_MAJOR + 3] = 180; /* β | Кохаб */
    noise3d[0][CENTER_X_MINOR - 6][CENTER_Y_MAJOR + 3] = 128; /* y | Феркад */

    /* The Big Dipper's Bucket | Ковш Великої Ведмедиці */
    noise3d[0][CENTER_X_MINOR + 5][CENTER_Y_MAJOR - 3] = 244; /* Дубхе*/
    noise3d[0][CENTER_X_MINOR + 6][CENTER_Y_MAJOR - 6] = 244; /* Мерак */
    noise3d[0][CENTER_X_MINOR + 1][CENTER_Y_MAJOR - 6] = 100; /* Мегрез */
    noise3d[0][CENTER_X_MINOR + 3][CENTER_Y_MAJOR - 8] = 200; /* Фекда */
    noise3d[0][CENTER_X_MINOR - 1][CENTER_Y_MAJOR - 6] = 160; /* Аліот */
    noise3d[0][CENTER_X_MINOR - 3][CENTER_Y_MAJOR - 6] = 90;  /* Міцар */
    noise3d[0][CENTER_X_MINOR - 5][CENTER_Y_MAJOR - 8] = 100; /* Алькаїд */

    noise3d[0][CENTER_X_MINOR - 4][CENTER_Y_MAJOR - 5] = 64;  /* Алькор */
  }

  while (stars < maxStars && attempts < maxAttempts) {
    attempts++;

    uint8_t x = random8(WIDTH);
    uint8_t y = random8(PADDING, HEIGHT);

    // не ставимо поруч із іншою зіркою (реалістичніше)
    if (noise3d[0][x][y] != 0) continue;

    bool nearStar = false;
    for (int8_t dx = -1; dx <= 1; dx++) {
      for (int8_t dy = -1; dy <= 1; dy++) {
        int8_t nx = x + dx;
        int8_t ny = y + dy;
        if (nx >= 0 && nx < WIDTH && ny >= 0 && ny < HEIGHT) {
          if (noise3d[0][nx][ny] > 0) {
            nearStar = true;
            break;
          }
        }
      }
    }
    if (nearStar) continue;
    uint8_t t = constellations ? 100 : random8();
    if (t < 200)      noise3d[0][x][y] = constellations ? random8(64, 80) : random8(80, 130);   // далекі
    else if (t < 245) noise3d[0][x][y] = random8(140, 200);  // середні
    else              noise3d[0][x][y] = random8(230, 244);  // яскраві
    stars++;
  }
}

/* -----------------------------------*/
void setStructureValue(float val, uint8_t spd) {
  /* set size spirales */
  if (val > 0.0f) {
    emitterX = 1.61803398875 / val;
  }
  /* speed rotate galaxy 1-100% */
  if (spd == 0) spd = 1;
  noise3d[1][1][1] = max<uint8_t>(1, 100 / spd);
}

/* -----------------------------------*/
void Galaxies() {
  const uint8_t JAMP_DELAY = 10;
  const uint16_t SWAP_DELAY = 512;
  static int direct;
  static int offset;
  static bool swapGalaxy;
  static bool hyperJump;
  const char* eff_name;

  if (loadingFlag) {
    IsRandomMode();
    noise3d[1][0][0] = 6;   /* id галактики 1-6 | при старті панорама з «Землі»*/
    noise3d[1][1][0] = 0;   /* активація обертання небосхила | bool */
    noise3d[1][1][1] = 1;   /* напрям обертання | bool */
    noise3d[1][1][2] = 255; /* яскравість небосхила / плавне затухання 1-255 */
    ff_z = 0;               /* timeline events effect*/
    swapGalaxy = true;
    hyperJump = false;
    offset = 0;
    step = 0;
    setStructureValue(11.5f, 10);
    if (modes[currentMode].Speed < 128) noise3d[1][0][0] = modes[currentMode].Scale / 15;
    createStarfield(random8(WIDTH + 5, HEIGHT + 10), noise3d[1][0][0] > 5);
    speedfactor = 0.001f;
  }

  float t = millis() * 0.0002f * speedfactor;   // animation

  /* prepare to hyper jump */
  if (swapGalaxy) {
    if (ff_z > JAMP_DELAY) {
#ifdef USE_OLED
      if (!develop) showMsg(icon_rocket24, 0, 24, 24, "h y p e r", "J U M P" , 1, 2);
#endif
      hyperJump = true;
    } else {
      fadeToBlackBy(leds, NUM_LEDS, 70);
    }
  }

  if (hyperJump) {  /* hyper jump */
    for (uint8_t i = 0; i < CENTER_Y_MAJOR; i++) {
      drawPixelXY(0, i, CRGB::Black);
      drawPixelXY(0, HEIGHT - i, CRGB::Black);
      drawPixelXY(WIDTH - 1, i, CRGB::Black);
      drawPixelXY(WIDTH - 1, HEIGHT - i, CRGB::Black);

      for (uint8_t x = 1; x < WIDTH - 1; x++) {
        int deltaX = (x > CENTER_X_MINOR) ? 1 : -1;
        if (i > (CENTER_Y_MAJOR - CENTER_X_MINOR)) deltaX = 0;
        drawPixelXY(x, CENTER_Y_MAJOR + i, getPixColorXY(x + deltaX, CENTER_Y_MAJOR + i + 1U));
        drawPixelXY(x, CENTER_Y_MAJOR - i - 1U, getPixColorXY(x + deltaX, CENTER_Y_MAJOR - i - 2U));
      }
    }

    /* set galaxy state */
    if (ff_z > JAMP_DELAY + CENTER_Y_MAJOR) {
      hyperJump = false;
      switch (noise3d[1][0][0]) { /* id галактики */
        case 0: {
            eff_name = "Fibonacci Galaxy";
            //CRGBPalette16 NASA_Galaxy = NASA_Galaxy_gp;
            currentPalette = NASA_Galaxy_gp;
            setStructureValue(2, 20);
            offset = CENTER_Y_MINOR * 0.25f;
            direct = 1;
          }
          break;
        case 1: {
            eff_name = "Andromeda Galaxy";
            //CRGBPalette16 NASA_Andromeda = NASA_Andromeda_gp;
            currentPalette = NASA_Andromeda_gp;
            setStructureValue(3.5f, 50);
            offset = CENTER_Y_MINOR * 0.75f;
            direct = 1;
          }
          break;
        case 2: {
            eff_name = "Milky Way Galaxy";
            //CRGBPalette16 NASA_MilkyWay = NASA_MilkyWay_gp;
            currentPalette = NASA_MilkyWay_gp;
            setStructureValue(3, 25);
            offset = 0;
            direct = 1;
          }
          break;
        case 3: {
            eff_name = "CR7 Galaxy";
            currentPalette = NASA_CR7_gp;

            setStructureValue(3.5f, 10);
            offset = CENTER_Y_MINOR * -0.5f;
            direct = 1;
          }
          break;
        case 4: {
            eff_name = "Black Hole";
            currentPalette = NASA_BlackHole_gp;
            setStructureValue(2.7f, 100);
            offset = CENTER_Y_MINOR * -0.2f;
            /* пришвидшене обертання для чорної діри */
            direct = -4;
          }
          break;
        case 5: {
            eff_name = "Veil Nebula";
            currentPalette = NASA_VeilNebula_gp;
            setStructureValue(2.7f, 30);
            offset = CENTER_Y_MINOR * -0.25f;
            direct = 1;
          }
          break;
        default: {
            eff_name = "Stars Map";
            currentPalette = NASA_DeepSpace_gp;
            setStructureValue(11.5f, 10);
            offset = - (CENTER_Y_MINOR + 2);
            direct = 0;
            /* визначений кут та оптимальне коливання */
            hue = 235;
            noise3d[1][1][2] = 255;
          }
          break;
      }

      /* set structure galaxy */
      swapGalaxy = false;
      hyperJump = false;

      createStarfield(random8(WIDTH + 5, HEIGHT + 10), noise3d[1][0][0] > 5);
#ifdef USE_OLED
      if (!develop) {
        String txt = "Effect " + String(currentMode);
        // eff_name.toUpperCase();
        showMsg(ico_eff32, 0, 32, 32, txt, eff_name, 1, 1);
        oledON(SHOW_DEF);
      }
#endif
    }
  } else {          /* draw galaxy */

    for (uint8_t y = 0; y < HEIGHT; y++) {
      if (y % 2) {
        /* fade out | неба */
        if (ff_z > 255 && noise3d[1][0][0] > 5) noise3d[1][1][2] = max<uint8_t>(8, noise3d[1][1][2] - 1);
      }
      /* fade in is start | неба & galaxy */
      if (step < 240 && ff_z > JAMP_DELAY) step++;

      for (uint8_t x = 0; x < WIDTH; x++) {

        float fibPhase;
        float dx = x - CENTER_X_MINOR;
        float dy = y - CENTER_Y_MAJOR + offset;

        float dist  = sqrtf(dx * dx + dy * dy);
        float angle = atan2f(dy, dx);

        if (noise3d[1][0][0] == 3)  {
          /* CR7 Galaxy & Veil Nebula*/
          fibPhase = angle * 2.0f + t + (dx + dy) * emitterX;
        } else {
          /* базова спіраль (Фібоначчі / класична) */
          fibPhase = angle * 2.0f + t + dist * emitterX;
        }
        /* гібридний хаос на віддалених зірках */
        if (noise3d[1][0][0] == 3 || noise3d[1][0][0] == 5) {
          float noiseFactor = 0.5f; // регулює інтенсивність хаосу
          uint8_t noiseVal = inoise8(x * 17, y * 23, millis() >> 3);
          // fibPhase += noiseFactor * ((noiseVal / 255.0f) - 0.5f) * dist;
          fibPhase += noiseFactor * ((noiseVal / 255.0f) - 0.5f) * sqrtf(dist);

        }

        hue2 = hue + (uint8_t)(fibPhase * 20.0f);
        uint8_t fade = noise3d[0][x][y];
        CRGB col = ColorFromPalette(currentPalette, hue2);
        /* fade in */
        if (step < 240) {
          col.nscale8(step);
        } else {
          if ((ff_z > 255) && (noise3d[1][0][0] > 5)) col.nscale8(noise3d[1][1][2]);
        }

        if (fade > 0) {
          /* draw stars | трохи теплий відтінок, як у реальних зірок */
          /* додаємо мерехтіння зікам */
          uint8_t tw = inoise8(x * 17, y * 23, millis() >> 3);
          uint8_t v = (tw < 128) ? fade + 10 : fade;
          drawPixelXY(x, y, CHSV( 58, (fade > 230) ? 6 : 12, v ));
        } else {
          /* draw galaxy */
          if (!swapGalaxy && !hyperJump) drawPixelXY(x, y, col);
        }
      }
    }
  }

  if (direct == 0) {
    /* swap direction for firmament */
    if (ff_z % 32 == 0) noise3d[1][1][0] = !noise3d[1][1][0];
    if (ff_z % noise3d[1][1][1] == 0) hue += (noise3d[1][1][0] ? 1 : -1);
  } else {
    if (ff_z % noise3d[1][1][1] == 0) hue -= direct;
  }

  if (ff_z > SWAP_DELAY) {         /* гіпер перехід в слідуючу галактику */
    ff_z = 0;
    noise3d[1][1][2] = 255;
    step = 0;
    if (modes[currentMode].Speed > 128) {
      noise3d[1][0][0]++;   /* next id galaxy */
      if (noise3d[1][0][0] > 6) noise3d[1][0][0] = 0;
      swapGalaxy = true;
    }
  }
  ff_z++;
}

// ========== Grandma's Apron ==========
//             © SlingMaster
//            Бабусин Фартушок
// =====================================
void Apron() {
  const byte PADDING = HEIGHT > 32 ? 2 : 0;
  if (loadingFlag) {
    IsRandomMode();
    deltaHue = (modes[currentMode].Scale / 100.0f * 6.4f);
    // FastLED.clear();
  }
  uint8_t x1 = random8(CENTER_X_MINOR);
  uint8_t y1 = random8(PADDING, CENTER_Y_MINOR);
  uint8_t x2 = random8(CENTER_X_MAJOR, WIDTH);
  uint8_t y2 = random8(CENTER_Y_MAJOR, HEIGHT);

  if (step % 4 == 0) {
    if ((x2 - x1) < 6 && (y2 - y1) > CENTER_Y_MAJOR) {
      emitterX = random(70, 110);
      if (step < 64) {
        hue = step;
        hue2 = 160 + step;
      }
      else if (step > 200) {
        hue = step - 200;
        hue2 = step;
      }
    } else {
      emitterX = random(360);
      if (step < 128) {
        hue = step;
        hue2 = 64 + deltaHue + step;
      } else {
        hue = step;
        hue2 = step - (64 + deltaHue);
      }
    }
    gradientRect(x1, y1, x2, y2, hue, hue2, 255U, 255U, 255U, emitterX);
  } else {
    if (deltaHue > 5) blurScreen(beatsin8(5U, 40U, 5U));
    // fadeToBlackBy(leds, NUM_LEDS, 5);
  }
  if (PADDING > 0) {
    DrawLine(0, 1, WIDTH - 1, 1, CHSV(hue2, 255, 255));
    DrawLine(0, 0, WIDTH - 1, 0, CHSV(hue, 255, 255));
  }
  step++;
}

// =====================================
//               ColorDrops
//             © SlingMaster
// =====================================
void ColorDrops() {
#define Sat (255)
#define MaxRad (WIDTH + HEIGHT)
  const byte SRS_SIZE = 4;
  static int rad[(HEIGHT + WIDTH) / 8];
  static byte posx[(HEIGHT + WIDTH) / 8], posy[(HEIGHT + WIDTH) / 8];

  if (loadingFlag) {
    IsRandomMode();
    currentPalette = PartyColors_p;
    FastLED.clear();
    hue = modes[currentMode].Scale * 2.55;
    for (int i = 0; i < ((HEIGHT + WIDTH) / 8) - 1; i++)  {
      posx[i] = random(WIDTH - 1);
      posy[i] = random(HEIGHT - 1);
      rad[i] = random(-1, MaxRad);
    }
  }

  fill_solid( currentPalette, 16, CHSV(hue, Sat, 230));
  currentPalette[10] = CHSV(hue, Sat - 60, 255);
  currentPalette[9] = CHSV(hue, 255 - Sat, 210);
  currentPalette[8] = CHSV(hue, 255 - Sat, 210);
  currentPalette[7] = CHSV(hue, Sat - 60, 255);
  // fillAll(ColorFromPalette(currentPalette, 1));
  //if (step % 2) blurScreen(beatsin8(5U, 10U, 5U)); else fadeToBlackBy(leds, NUM_LEDS, 128);

  fadeToBlackBy(leds, NUM_LEDS,  64);
  for (uint8_t i = ((HEIGHT + WIDTH) / 8 - 1); i > 0 ; i--) {
    if (rad[i] > SRS_SIZE + 3) {
      drawCircle(posx[i], posy[i], rad[i], ColorFromPalette(currentPalette, (256 / 16) * 8.5 - rad[i]));
      drawCircle(posx[i], posy[i], rad[i] - 1, ColorFromPalette(currentPalette, (256 / 16) * 6.5 - rad[i]));
      //      drawCircleF(posx[i], posy[i], rad[i] - 2, CRGB::Black);
      //      drawCircle(posx[i], posy[i], rad[i] - 3, CRGB::Black);
    } else {
      deltaValue = (rad[i] == 1) ? 0 : abs(128 - step);
      circleFill(posx[i], posy[i], SRS_SIZE, hue2, 255 - deltaValue, deltaValue);
    }
    if (rad[i] >= MaxRad) {
      rad[i] = 0; // random(-1, MaxRad);
      posx[i] = random(WIDTH);
      posy[i] = random(HEIGHT);
      hue2 = random8(2, 5) * 32 + hue;
    } else {
      rad[i]++;
    }
  }
  if (modes[currentMode].Scale == 100) hue++;

  step++;
  blur2d(leds, WIDTH, HEIGHT, 32);
  FPSdelay = abs(128 - step) + 16;
}

// ==============
// END ==============
// ==============
