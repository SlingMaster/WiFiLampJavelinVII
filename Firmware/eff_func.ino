/* eff_func.ino */
// ======================================
// Global Function for Effects
// ======================================
#include <vector>
#include "eff_utils.h"
// ======================================


// ======================================
//аналог ардуино функции map(), но только для float
float fmap(const float x, const float in_min, const float in_max, const float out_min, const float out_max) {
  return (out_max - out_min) * (x - in_min) / (in_max - in_min) + out_min;
}

// ======================================
void setModeSettings(uint8_t Scale = 0U, uint8_t Speed = 0U) {
  EffData effectsSRAM;
  memcpy_P( &effectsSRAM, &effectsPROGMEM[currentMode], sizeof(EffData));
  modes[currentMode].Scale = Scale ? Scale : effectsSRAM.scale;
  modes[currentMode].Speed = Speed ? Speed : effectsSRAM.speed;
  selectedSettings = false;
}

// ======================================
void IsRandomMode() {
  if (selectedSettings) {
    EffData effectsSRAM;
    memcpy_P( &effectsSRAM, &effectsPROGMEM[currentMode], sizeof(EffData));
    //              scale                                           | speed
    setModeSettings(random(effectsSRAM.min_scl, effectsSRAM.max_scl), random(effectsSRAM.min_spd, effectsSRAM.max_spd));
  }
  loadingFlag = false;
}

// ======================================
void clearNoiseArr() {
  for (uint8_t x = 0U; x < WIDTH; x++) {
    for (uint8_t y = 0U; y < HEIGHT; y++) {
      noise3d[0][x][y] = 0;
      noise3d[1][x][y] = 0;
    }
  }
}

// ======================================
void fillAll(CRGB color) {
  for (int16_t i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
  }
}

// =====================================
/* функция отрисовки точки по координатам X Y */
inline void drawPixelXY(int8_t x, int8_t y, CRGB color) {
  if (x < 0 || x > (WIDTH - 1) || y < 0 || y > (HEIGHT - 1)) return;
  uint32_t thisPixel = XY((uint8_t)x, (uint8_t)y) * SEGMENTS;
  for (uint8_t i = 0; i < SEGMENTS; i++) {
    leds[thisPixel + i] = color;
  }
}

// =====================================
/* функция получения цвета пикселя по его номеру */
uint32_t getPixColor(uint32_t thisSegm) {
  uint32_t thisPixel = thisSegm * SEGMENTS;
  if (thisPixel > NUM_LEDS - 1) return 0;
  return (((uint32_t)leds[thisPixel].r << 16) | ((uint32_t)leds[thisPixel].g << 8 ) | (uint32_t)leds[thisPixel].b); // а почему не просто return (leds[thisPixel])?
}

// =====================================
/* функция получения цвета пикселя в матрице по его координатам */
uint32_t getPixColorXY(uint8_t x, uint8_t y) {
  return getPixColor(XY(x, y));
}

// =====================================
uint8_t SpeedFactor(uint8_t spd) {
  return (uint16_t)spd * NUM_LEDS / 1024U;
}

// ======================================
void setCurrentPalette() {
  if (modes[currentMode].Scale > 100U) modes[currentMode].Scale = 100U; // чтобы не было проблем при прошивке без очистки памяти
  curPalette = palette_arr[(uint8_t)(modes[currentMode].Scale / 100.0F * ((sizeof(palette_arr) / sizeof(TProgmemRGBPalette16 *)) - 0.01F))];
}

// ======================================Fast
void blurScreen(fract8 blur_amount, CRGB *LEDarray = leds) {
  // blur_amount 0-255, 0 = без розмиття, 255 = максимальне
  for (uint8_t y = 0; y < HEIGHT; y++) {
    for (uint8_t x = 0; x < WIDTH; x++) {
      uint32_t idx = XY(x, y);   // ваш XY-індекс
      CRGB cur = LEDarray[idx];

      // отримуємо сусідів (з врахуванням меж)
      CRGB left  = (x > 0) ? LEDarray[XY(x - 1, y)] : cur;
      CRGB right = (x < WIDTH - 1) ? LEDarray[XY(x + 1, y)] : cur;
      CRGB up    = (y > 0) ? LEDarray[XY(x, y - 1)] : cur;
      CRGB down  = (y < HEIGHT - 1) ? LEDarray[XY(x, y + 1)] : cur;

      // просте усереднення з застосуванням blur_amount
      cur.r = ((cur.r * (255 - blur_amount)) + ((left.r + right.r + up.r + down.r) / 4) * blur_amount) >> 8;
      cur.g = ((cur.g * (255 - blur_amount)) + ((left.g + right.g + up.g + down.g) / 4) * blur_amount) >> 8;
      cur.b = ((cur.b * (255 - blur_amount)) + ((left.b + right.b + up.b + down.b) / 4) * blur_amount) >> 8;

      LEDarray[idx] = cur;
    }
  }
}

// =====================================
/* приблизний, швидкий квадратний корінь з одним кроком Ньютон-Рафсона */
float sqrt3(const float x) {
  if (x <= 0.0f) return 0.0f;

  union {
    int i;
    float x;
  } u;

  u.x = x;
  u.i = (1 << 29) + (u.i >> 1) - (1 << 22);  // базове наближення

  // один крок Ньютон-Рафсона для покращення точності
  u.x = 0.5f * (u.x + x / u.x);

  return u.x;
}

// ======================================
template <size_t Rows, size_t Cols>
bool overprintText(const uint8_t (&msg)[Rows][Cols], uint8_t idx, uint8_t delay_scroll, uint8_t color, uint32_t shadow) {
  if (idx >= Rows) {
    return true;
  }
  uint8_t i = 0, j = 0;
  static int16_t offset = WIDTH - 5;

  // Перебір символів рядка msg[idx]
  while (pgm_read_byte(&(msg[idx][i])) != 0) {
    uint8_t currentChar = pgm_read_byte(&(msg[idx][i]));
    if (currentChar > 191) {
      i++;
    } else {
      uint16_t posX = offset + j * (LET_WIDTH + SPACE);
      /* if shadow == 1 transparent overprint */
      if ( shadow != 1) drawRec(posX, TEXT_HEIGHT, posX + LET_WIDTH + 1, TEXT_HEIGHT + LET_HEIGHT + 1, shadow);
      drawLetter(pgm_read_byte(&(msg[idx][i - 1])), currentChar, posX, CHSV(color, (color == 255) ? 0 : 255, 255), shadow);
      i++;
      j++;
    }
  }

  if (offset < (int16_t)(-j * (LET_WIDTH + SPACE))) {   /* Перевірка завершення скролу */
    offset = WIDTH + 5;                                 /* в початкову позицію */
    return true;
  } else {
    EVERY_N_MILLISECONDS(delay_scroll) {
      // LOG.printf("scroll | offset %3d\n\r", offset);
      offset--;
    }
    return false;
  }
}

// ======================================
void dimAll(uint8_t value, CRGB *LEDarray = leds) {
  nscale8(LEDarray, NUM_LEDS, value);
}

// ======================================
void dimmCustom(int8_t value, CRGB *LEDarray, int8_t startRow, int8_t endRow) {
  for (int row = startRow; row <= endRow; ++row) {
    for (int x = 0; x < WIDTH; x++) {
      LEDarray[XY(x, row)].nscale8(value);
    }
  }
}
// ======================================
void brightenAll(CRGB* leds, uint16_t numLeds, uint8_t amount) {
  for (uint16_t i = 0; i < numLeds; i++) {
    leds[i].r = qadd8(leds[i].r, amount);
    leds[i].g = qadd8(leds[i].g, amount);
    leds[i].b = qadd8(leds[i].b, amount);
  }
}
// ======================================
void brightenLedsBy(CRGB* leds, uint8_t x, uint8_t y, uint8_t amount, bool toBlack) {
  if (x < 0 || x > (WIDTH - 1) || y < 0 || y > (HEIGHT - 1)) return;
  uint16_t i = XY(x, y);
  if (toBlack) {
    leds[i].nscale8(amount);
  } else {
    leds[i].r = qadd8(leds[i].r, amount);
    leds[i].g = qadd8(leds[i].g, amount);
    leds[i].b = qadd8(leds[i].b, amount);
  }
}

// ======================================
// Підсвітка прямокутної області
// LEDarray – масив світлодіодів
// x0, y0 – верхній лівий кут
// x1, y1 – нижній правий кут (включно)
// amount – величина підсвічення (0–255)
void brightenRect(CRGB* LEDarray = leds, int x0 = 0, int y0 = 0, int x1 = WIDTH - 1, int y1 = HEIGHT - 1, uint8_t amount = 10) {
  // обмежуємо координати допустимими значеннями
  if (x0 < 0) x0 = 0;
  if (y0 < 0) y0 = 0;
  if (x1 >= WIDTH) x1 = WIDTH - 1;
  if (y1 >= HEIGHT) y1 = HEIGHT - 1;

  for (int y = y0; y <= y1; ++y) {
    for (int x = x0; x <= x1; ++x) {
      uint16_t idx = XY(x, y);
      LEDarray[idx] += CRGB(amount, amount, amount); // безпечне підсвічення
    }
  }
}

// ======================================
//   Additional drawing functions
// ======================================
void DrawLine(int x1, int y1, int x2, int y2, CRGB color) {
  int deltaX = abs(x2 - x1);
  int deltaY = abs(y2 - y1);
  int signX = x1 < x2 ? 1 : -1;
  int signY = y1 < y2 ? 1 : -1;
  int error = deltaX - deltaY;

  drawPixelXY(x2, y2, color);
  while (x1 != x2 || y1 != y2) {
    drawPixelXY(x1, y1, color);
    int error2 = error * 2;
    if (error2 > -deltaY) {
      error -= deltaY;
      x1 += signX;
    }
    if (error2 < deltaX) {
      error += deltaX;
      y1 += signY;
    }
  }
}

// ======================================
/* по мотивам /https://gist.github.com/sutaburosu/32a203c2efa2bb584f4b846a91066583 */
void drawPixelXYF(float x, float y, CRGB color)  {
  //  if (x<0 || y<0) return; //не похоже, чтобы отрицательные значения хоть как-нибудь учитывались тут // зато с этой строчкой пропадает нижний ряд
  // extract the fractional parts and derive their inverses
  uint8_t xx = (x - (int)x) * 255, yy = (y - (int)y) * 255, ix = 255 - xx, iy = 255 - yy;
  // calculate the intensities for each affected pixel
#define WU_WEIGHT(a,b) ((uint8_t) (((a)*(b)+(a)+(b))>>8))
  uint8_t wu[4] = {WU_WEIGHT(ix, iy), WU_WEIGHT(xx, iy),
                   WU_WEIGHT(ix, yy), WU_WEIGHT(xx, yy)
                  };
  /* multiply the intensities by the colour, and saturating-add them to the pixels */
  for (uint8_t i = 0; i < 4; i++) {
    int16_t xn = x + (i & 1), yn = y + ((i >> 1) & 1);
    CRGB clr = getPixColorXY(xn, yn);
    clr.r = qadd8(clr.r, (color.r * wu[i]) >> 8);
    clr.g = qadd8(clr.g, (color.g * wu[i]) >> 8);
    clr.b = qadd8(clr.b, (color.b * wu[i]) >> 8);
    drawPixelXY(xn, yn, clr);
  }
}

// ======================================
void DrawLineF(float x1, float y1, float x2, float y2, CRGB color) {
  float deltaX = std::fabs(x2 - x1);
  float deltaY = std::fabs(y2 - y1);
  float error = deltaX - deltaY;

  float signX = x1 < x2 ? 0.5 : -0.5;
  float signY = y1 < y2 ? 0.5 : -0.5;

  while (x1 != x2 || y1 != y2) { // (true) - а я то думаю - "почему функция часто вызывает вылет по вачдогу?" А оно вон оно чё, Михалычь!
    if ((signX > 0 && x1 > x2 + signX) || (signX < 0 && x1 < x2 + signX)) break;
    if ((signY > 0 && y1 > y2 + signY) || (signY < 0 && y1 < y2 + signY)) break;
    drawPixelXYF(x1, y1, color); // интересно, почему тут было обычное drawPixelXY() ???
    float error2 = error;
    if (error2 > -deltaY) {
      error -= deltaY;
      x1 += signX;
    }
    if (error2 < deltaX) {
      error += deltaX;
      y1 += signY;
    }
  }
}

// ======================================
void drawCircle(int x0, int y0, int radius, const CRGB & color) {
  int a = radius, b = 0;
  int radiusError = 1 - a;

  if (radius == 0) {
    drawPixelXY(x0, y0, color);
    return;
  }

  while (a >= b)  {
    drawPixelXY(a + x0, b + y0, color);
    drawPixelXY(b + x0, a + y0, color);
    drawPixelXY(-a + x0, b + y0, color);
    drawPixelXY(-b + x0, a + y0, color);
    drawPixelXY(-a + x0, -b + y0, color);
    drawPixelXY(-b + x0, -a + y0, color);
    drawPixelXY(a + x0, -b + y0, color);
    drawPixelXY(b + x0, -a + y0, color);
    b++;
    if (radiusError < 0)
      radiusError += 2 * b + 1;
    else
    {
      a--;
      radiusError += 2 * (b - a + 1);
    }
  }
}

// ======================================
void drawCircleF(float x0, float y0, float radius, CRGB color) {
  float x = 0, y = radius, error = 0;
  float delta = 1. - 2. * radius;
  while (y >= 0) {
    drawPixelXYF(fmod(x0 + x + WIDTH, WIDTH), y0 + y, color); // сделал, чтобы круги были бесшовными по оси х
    drawPixelXYF(fmod(x0 + x + WIDTH, WIDTH), y0 - y, color);
    drawPixelXYF(fmod(x0 - x + WIDTH, WIDTH), y0 + y, color);
    drawPixelXYF(fmod(x0 - x + WIDTH, WIDTH), y0 - y, color);
    error = 2. * (delta + y) - 1.;
    if (delta < 0 && error <= 0) {
      ++x;
      delta += 2. * x + 1.;
      continue;
    }
    error = 2. * (delta - x) - 1.;
    if (delta > 0 && error > 0) {
      --y;
      delta += 1. - 2. * y;
      continue;
    }
    ++x;
    delta += 2. * (x - y);
    --y;
  }
}
// ======================================
void circleFill(int16_t centerX, int16_t centerY, float radius, uint8_t color, uint8_t sat, uint8_t brightness) {
  //  int16_t centerX = circle.centerX;
  //  int16_t centerY = circle.centerY;
  //  int hue = circle.hue;
  //  float radius = circle.radius();

  int16_t startX = centerX - ceil(radius);
  int16_t endX = centerX + ceil(radius);
  int16_t startY = centerY - ceil(radius);
  int16_t endY = centerY + ceil(radius);

  for (int16_t x = startX; x < endX; x++) {
    for (int16_t y = startY; y < endY; y++) {
      int16_t index = XY(x, y);
      if (index < 0 || index > NUM_LEDS)
        continue;
      double distance = sqrt(sq(x - centerX) + sq(y - centerY));
      if (distance > radius)
        continue;
      //      uint8_t deltaValue;
      //      uint8_t brightness;
      if (radius < 1) { // last pixel
        // brightness = 0; //255.0 * radius;
        sat = 20;
        brightness = 180;
        // brightness = 0;
      } else {
        // sat = 200;
        double percentage = distance / radius;
        double fraction = 1.0 - percentage;
        brightness = 255.0 * fraction;
      }
      leds[index] += CHSV(color, sat, brightness);
    }
  }
}

//---------------------------------------
// Global Function
//---------------------------------------
// ======================================
void drawRec(uint8_t startX, uint8_t startY, uint8_t endX, uint8_t endY, CRGB color) {
  for (uint8_t y = startY; y < endY; y++) {
    for (uint8_t x = startX; x < endX; x++) {
      drawPixelXY(x, y, color);
    }
  }
}

// ======================================
// Плавний градієнт прямокутника під будь-яким кутом 0–360°
// з коректною нормалізацією та anti-banding
// ======================================
void gradientRect( int x0, int y0, int x1, int y1, uint8_t start_color, uint8_t end_color,
                   uint8_t start_br, uint8_t end_br, uint8_t saturate, float angleDeg) {
  // формула для плавної зміни кута emitterY = step * 1.411764f;
  // ---- обмеження ----
  x0 = constrain(x0, 0, WIDTH - 1);
  x1 = constrain(x1, 0, WIDTH - 1);
  y0 = constrain(y0, 0, HEIGHT - 1);
  y1 = constrain(y1, 0, HEIGHT - 1);

  if (x0 > x1) {
    int t = x0;
    x0 = x1;
    x1 = t;
  }
  if (y0 > y1) {
    int t = y0;
    y0 = y1;
    y1 = t;
  }

  // ---- напрям ----
  float rad = angleDeg * DEG_TO_RAD;
  float dx = cosf(rad);
  float dy = sinf(rad);

  // ---- шукаємо min/max проєкцію ----
  float minP =  1e9;
  float maxP = -1e9;

  for (int y = y0; y <= y1; y++) {
    for (int x = x0; x <= x1; x++) {
      float p = x * dx + y * dy;
      if (p < minP) minP = p;
      if (p > maxP) maxP = p;
    }
  }

  float range = maxP - minP;
  if (range < 0.0001f) range = 1.0f;

  // ---- малюємо ----
  for (int y = y0; y <= y1; y++) {
    for (int x = x0; x <= x1; x++) {

      float p = x * dx + y * dy;
      float t = (p - minP) / range;

      // плавність (anti-banding)
      t = t * t * (3.0f - 2.0f * t);   // smoothstep

      uint8_t hue = start_color + (end_color - start_color) * t;
      uint8_t bri = start_br    + (end_br    - start_br)    * t;

      leds[XY(x, y)] = CHSV(hue, saturate, bri);
    }
  }
}

// ======================================
// функції для роботи з бінарними файлами
// ======================================
std::vector<byte> binImage;
// Використовуємо вектор замість вказівника для використання динамічної пам'яті

void readBinFile(const char *fileName, size_t maxLen) {
  if (binImage.capacity() < maxLen) {
    binImage.reserve(maxLen);
  }
  binImage.clear();

  File binFile = SPIFFS.open(fileName, "r");
  if (!binFile) {
#ifdef GENERAL_DEBUG
    LOG.println("Bin file not found");
#endif
    return;
  }

  size_t size = binFile.size();
  if (size == 0 || size > maxLen) {
#ifdef GENERAL_DEBUG
    LOG.println("Invalid bin file size");
#endif
    binFile.close();
    return;
  }

  binImage.resize(size);
  binFile.read(binImage.data(), size);
  binFile.close();
}

// --------------------------------------
/* функція отримання розміру зображення із заголовка файлу */
uint16_t getSizeValue(byte* buffer, byte b ) {
  return  (buffer[b + 1] << 8) + buffer[b];
}

// --------------------------------------
/* функция скрола зображення по осі X */
void scrollImage(uint16_t imgW, uint16_t imgH, uint16_t start_row) {
  const uint16_t HEADER = 16;
  const uint16_t BYTES_PER_PIXEL = 2;

  if (binImage.empty()) return;

  uint8_t padding = (HEIGHT > imgH) ? (HEIGHT - imgH) / 2 : 0;
  int16_t topPos = HEIGHT - padding - 1;

  for (uint16_t x = 0; x < WIDTH; x++) {

    uint16_t imgX = start_row + x;
    if (imgX >= imgW) imgX -= imgW;   // ✅ wrap по картинці

    for (uint16_t y = 0; y < imgH; y++) {
      uint32_t pixIndex = HEADER + (imgX + y * imgW) * BYTES_PER_PIXEL;
      // захист від OOB
      if (pixIndex + 1 >= binImage.size()) continue;
      uint8_t r =  binImage[pixIndex + 1] & 0xF8;
      uint8_t g = ((binImage[pixIndex + 1] & 0x07) << 5) | ((binImage[pixIndex] & 0xE0) >> 3);
      uint8_t b = (binImage[pixIndex] & 0x1F) << 3;

      int16_t drawY = topPos - y;
      if (drawY < 0 || drawY >= HEIGHT) continue;
      leds[XY(x, drawY)] = CRGB(r, g, b);
    }
  }

  // create background top and bottom
  if (start_row == 0 && padding > 0) {
    drawRec(0, 0, WIDTH, padding, getPixColorXY(0, topPos));
    drawRec(0, HEIGHT - padding, WIDTH, HEIGHT, getPixColorXY(0, topPos));
  }
}

// ==============
// END ==============
// ==============
