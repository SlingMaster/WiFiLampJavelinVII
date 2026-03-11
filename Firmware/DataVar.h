/* DataVar.h
  • FastLED Colors
  http://fastled.io/docs/3.1/struct_c_r_g_b.html
  https://github.com/FastLED/FastLED/wiki/Pixel-reference
*/

#pragma once
/* Загальні змінні та буфери для ефектів і функцій */

#define SQRT_VARIANT sqrt3                 // Функція обчислення квадратного кореня (швидша альтернатива)

uint8_t hue, hue2;                         // Циклічні лічильники відтінку
uint8_t deltaHue, deltaHue2;               // Додаткові лічильники відтінку
uint8_t step;                              // Лічильник кадрів / кроків
uint8_t pcnt;                              // Лічильник прогресу
uint8_t deltaValue;                        // Універсальна змінна
float speedfactor;                         // Коефіцієнт швидкості ефектів
float emitterX, emitterY;                  // Динамічні координати

CRGB ledsbuff[NUM_LEDS];                   // Буфер-копія масиву leds[]

#define NUM_LAYERSMAX 2
uint8_t noise3d[NUM_LAYERSMAX][WIDTH][HEIGHT]; // 2-шарова маска / дані для матриці

uint8_t line[WIDTH];                       // Дані рядка матриці
uint8_t shiftHue[HEIGHT];                  // Зсув відтінку по стовпцях
uint8_t shiftValue[HEIGHT];                // Зсув яскравості по стовпцях

uint16_t ff_x, ff_y, ff_z;                 // Допоміжні великі лічильники


uint8_t noise2[2][WIDTH + 1][HEIGHT + 1];

/* Масиви стану об’єктів для використання в ефектах */

#define trackingOBJECT_MAX_COUNT   (100U)           // Максимальна кількість відстежуваних об’єктів
float   trackingObjectPosX[trackingOBJECT_MAX_COUNT];
float   trackingObjectPosY[trackingOBJECT_MAX_COUNT];
float   trackingObjectSpeedX[trackingOBJECT_MAX_COUNT];
float   trackingObjectSpeedY[trackingOBJECT_MAX_COUNT];
float   trackingObjectShift[trackingOBJECT_MAX_COUNT];
uint8_t trackingObjectHue[trackingOBJECT_MAX_COUNT];
uint8_t trackingObjectState[trackingOBJECT_MAX_COUNT];
bool    trackingObjectIsShift[trackingOBJECT_MAX_COUNT];
#define enlargedOBJECT_MAX_COUNT   (WIDTH * 2)    // Максимальна кількість складних відстежуваних об’єктів (менше за trackingOBJECT_MAX_COUNT)
uint8_t enlargedObjectNUM;                        // Кількість об’єктів, що використовуються в ефекті
long    enlargedObjectTime[enlargedOBJECT_MAX_COUNT];
float    liquidLampHot[enlargedOBJECT_MAX_COUNT];
float    liquidLampSpf[enlargedOBJECT_MAX_COUNT];
unsigned liquidLampMX[enlargedOBJECT_MAX_COUNT];
unsigned liquidLampSC[enlargedOBJECT_MAX_COUNT];
unsigned liquidLampTR[enlargedOBJECT_MAX_COUNT];


//константы размера матрицы вычисляется только здесь и не меняется в эффектах
const uint8_t CENTER_X_MINOR =  (WIDTH / 2) -  ((WIDTH - 1) & 0x01); // центр матрицы по ИКСУ, сдвинутый в меньшую сторону, если ширина чётная
const uint8_t CENTER_Y_MINOR = (HEIGHT / 2) - ((HEIGHT - 1) & 0x01); // центр матрицы по ИГРЕКУ, сдвинутый в меньшую сторону, если высота чётная
const uint8_t CENTER_X_MAJOR =   WIDTH / 2  + (WIDTH % 2);           // центр матрицы по ИКСУ, сдвинутый в большую сторону, если ширина чётная
const uint8_t CENTER_Y_MAJOR =  HEIGHT / 2  + (HEIGHT % 2);          // центр матрицы по ИГРЕКУ, сдвинутый в большую сторону, если высота чётная


// палітра для типу "реалістичний водоспад" (якщо повзунок "Масштаб" встановлено на 100)
extern const TProgmemRGBPalette16 WaterfallColors_p FL_PROGMEM = {0x000000, 0x060707, 0x101110, 0x151717, 0x1C1D22, 0x242A28, 0x363B3A, 0x313634, 0x505552, 0x6B6C70, 0x98A4A1, 0xC1C2C1, 0xCACECF, 0xCDDEDD, 0xDEDFE0, 0xB2BAB9};

// добавлено изменение текущей палитры (используется во многих эффектах ниже для бегунка Масштаб)
const TProgmemRGBPalette16 *palette_arr[] = {
  &PartyColors_p,
  &OceanColors_p,
  &LavaColors_p,
  &HeatColors_p,
  &WaterfallColors_p,
  &CloudColors_p,
  &ForestColors_p,
  &RainbowColors_p,
  &RainbowStripeColors_p
};
const TProgmemRGBPalette16 *curPalette = palette_arr[0];

// при такій кількості палітр (9 шт.) кожен діапазон "Масштаб" (від 1 до 100) можна розбити на ділянки по 11 значень
// значення від 0 до 10 = ((modes[currentMode].Scale - 1U) % 11U)
// значення від 1 до 11 = ((modes[currentMode].Scale - 1U) % 11U + 1U)
// а 100-е значення Масштабу можна використовувати для білого кольору

// додаткові палітри для вогню
// для запису в PROGMEM перетворював з 4 кольорів у 16 на сайті https://colordesigner.io/gradient-generator,
// але не впевнений, що це еквівалент CRGBPalette16()
// значення кольорових констант тут: https://github.com/FastLED/FastLED/wiki/Pixel-reference

extern const TProgmemRGBPalette16 WoodFireColors_p FL_PROGMEM = {CRGB::Black, 0x330e00, 0x661c00, 0x992900, 0xcc3700, CRGB::OrangeRed, 0xff5800, 0xff6b00, 0xff7f00, 0xff9200, CRGB::Orange, 0xffaf00, 0xffb900, 0xffc300, 0xffcd00, CRGB::Gold};             //* Orange
extern const TProgmemRGBPalette16 NormalFire_p FL_PROGMEM = {CRGB::Black, 0x330000, 0x660000, 0x990000, 0xcc0000, CRGB::Red, 0xff0c00, 0xff1800, 0xff2400, 0xff3000, 0xff3c00, 0xff4800, 0xff5400, 0xff6000, 0xff6c00, 0xff7800};                             // пытаюсь сделать что-то более приличное
extern const TProgmemRGBPalette16 NormalFire2_p FL_PROGMEM = {CRGB::Black, 0x560000, 0x6b0000, 0x820000, 0x9a0011, CRGB::FireBrick, 0xc22520, 0xd12a1c, 0xe12f17, 0xf0350f, 0xff3c00, 0xff6400, 0xff8300, 0xffa000, 0xffba00, 0xffd400};                      // пытаюсь сделать что-то более приличное
extern const TProgmemRGBPalette16 LithiumFireColors_p FL_PROGMEM = {CRGB::Black, 0x240707, 0x470e0e, 0x6b1414, 0x8e1b1b, CRGB::FireBrick, 0xc14244, 0xd16166, 0xe08187, 0xf0a0a9, CRGB::Pink, 0xff9ec0, 0xff7bb5, 0xff59a9, 0xff369e, CRGB::DeepPink};        //* Red
extern const TProgmemRGBPalette16 SodiumFireColors_p FL_PROGMEM = {CRGB::Black, 0x332100, 0x664200, 0x996300, 0xcc8400, CRGB::Orange, 0xffaf00, 0xffb900, 0xffc300, 0xffcd00, CRGB::Gold, 0xf8cd06, 0xf0c30d, 0xe9b913, 0xe1af1a, CRGB::Goldenrod};           //* Yellow
extern const TProgmemRGBPalette16 CopperFireColors_p FL_PROGMEM = {CRGB::Black, 0x001a00, 0x003300, 0x004d00, 0x006600, CRGB::Green, 0x239909, 0x45b313, 0x68cc1c, 0x8ae626, CRGB::GreenYellow, 0x94f530, 0x7ceb30, 0x63e131, 0x4bd731, CRGB::LimeGreen};     //* Green
extern const TProgmemRGBPalette16 AlcoholFireColors_p FL_PROGMEM = {CRGB::Black, 0x000033, 0x000066, 0x000099, 0x0000cc, CRGB::Blue, 0x0026ff, 0x004cff, 0x0073ff, 0x0099ff, CRGB::DeepSkyBlue, 0x1bc2fe, 0x36c5fd, 0x51c8fc, 0x6ccbfb, CRGB::LightSkyBlue};  //* Blue
extern const TProgmemRGBPalette16 RubidiumFireColors_p FL_PROGMEM = {CRGB::Black, 0x0f001a, 0x1e0034, 0x2d004e, 0x3c0068, CRGB::Indigo, CRGB::Indigo, CRGB::Indigo, CRGB::Indigo, CRGB::Indigo, CRGB::Indigo, 0x3c0084, 0x2d0086, 0x1e0087, 0x0f0089, CRGB::DarkBlue};        //* Indigo
extern const TProgmemRGBPalette16 PotassiumFireColors_p FL_PROGMEM = {CRGB::Black, 0x0f001a, 0x1e0034, 0x2d004e, 0x3c0068, CRGB::Indigo, 0x591694, 0x682da6, 0x7643b7, 0x855ac9, CRGB::MediumPurple, 0xa95ecd, 0xbe4bbe, 0xd439b0, 0xe926a1, CRGB::DeepPink}; //* Violet
const TProgmemRGBPalette16 *firePalettes[] = {
  &WoodFireColors_p,
  &NormalFire_p,
  &NormalFire2_p,
  &LithiumFireColors_p,
  &SodiumFireColors_p,
  &CopperFireColors_p,
  &AlcoholFireColors_p,
  &RubidiumFireColors_p,
  &PotassiumFireColors_p
};


// ======================================
//  Galaxy NASA space palette
// ======================================
DEFINE_GRADIENT_PALETTE(NASA_Galaxy_gp) {
  0,   0,   0,   0,        // чорний космос
  128, 0,  40, 160,       // глибокий фіолетово-синій
  200, 30, 20, 90,        // темний пурпур
  235, 90, 30, 90,        // магента
  245, 0,   0,   0,       // чорна порожнеча
  255, 10,  0,  10        // ледь помітний темно-фіолетовий
};

DEFINE_GRADIENT_PALETTE(OLDNASA_Andromeda_gp) {
  0,    0,   0,   0,      // чорний
  50,   80, 100, 50,      // оливково-зелений
  70,   25,  25,  25,     // темно-сірий
  128,  0,   25,  60,     // темно-синій
  195,  0,  140, 120,     // бірюзово-зелений
  210,  20, 170, 140,     // яскравий бірюзовий
  255,  0,   0,   0       // чорний
};

DEFINE_GRADIENT_PALETTE(NASA_Andromeda_gp) {
  0,   0,   0,   0,       // чорний
  50,  80,  0,  30,       // темно-червоний
  128, 92, 42, 20,        // коричнево-бурштиновий
  195, 0,   0,   0,       // чорний
  210, 128, 0,   0,       // темно-червоний
  255, 0,   0,   0        // чорний
};

DEFINE_GRADIENT_PALETTE(NASA_MilkyWay_gp) {
  0,   0,   0,   0,       // чорний
  30,  0,   0,   0,       // чорний
  42,  160, 140, 90,      // теплий жовто-білий (зорі)
  50,  0,   0,   0,       // чорний
  60,  20,  60,  100,     // холодний синьо-фіолетовий
  140, 0,   0,   0,       // чорний
  155, 160, 140, 90,      // зоряний пил
  162, 0,   0,   0,       // чорний
  180, 20,  60,  100,     // синьо-фіолетовий
  255, 0,   0,   0        // чорний
};

DEFINE_GRADIENT_PALETTE(NASA_CR7_gp) {
  0,   0,   0,   0,       // чорний
  40,  0,  20,  40,       // темно-синій
  80,  0,  40,  60,       // синьо-фіолетовий
  120, 0,  85, 110,       // бірюзовий
  160, 40, 140, 170,      // світло-блакитний
  170, 50, 70, 255,       // яскраво-синій
  190, 40, 120, 140,      // холодний блакитний
  255, 0,   0,   0        // чорний
};

DEFINE_GRADIENT_PALETTE(NASA_BlackHole_gp) { // red
  0,   0,   0,   0,       // чорний
  40,  20,  0,   0,       // темно-червоний
  80,  60,  5,   5,       // темно-багряний
  110, 120, 20, 10,       // червоно-оранжевий
  130, 180, 40, 20,       // яскравий помаранчевий
  138, 255, 80, 40,       // вогняний
  150, 120, 20, 10,       // приглушений червоний
  180, 40,  5,   5,       // темний багряний
  220, 5,   0,   0,       // майже чорний
  255, 0,   0,   0        // чорний
};

DEFINE_GRADIENT_PALETTE(NASA_VeilNebula_gp) { // blue
  0,    0,   0,   0,      // чорний
  40,   0,   0,  20,      // темно-синій
  80,   5,   5,  60,      // синьо-фіолетовий
  110, 20,  10, 120,      // насичений синій
  130, 20,  40, 180,      // блакитний
  138, 40,  80, 255,      // яскраво-блакитний
  150, 20,  10, 120,      // темно-синій
  180, 5,   5,  40,       // приглушений синій
  182, 0,   40, 42,       // зеленкувато-синій
  184, 0,   0,   5,       // майже чорний
  255, 0,   0,  0         // чорний
};

DEFINE_GRADIENT_PALETTE(NASA_DeepSpace_gp) {
  0,   0,   0,   40,      // темно-синій
  30,  80,  40,  60,      // фіолетово-бордовий
  80,  0,   0,   8,       // майже чорний
  205, 0,   0,  32,       // глибокий синій
  255, 0,   20, 100       // насичений синьо-фіолетовий
};


/* Wine palette */
DEFINE_GRADIENT_PALETTE(WINE_Rose_gp) {
  0,   80,   20,  30,    // темно-рожевий, малиновий відтінок
  20,  180, 100, 130,   // блідо-рожевий, пелюстка троянди
  255, 80,   20,  30    // темно-рожевий, ягідний
};
DEFINE_GRADIENT_PALETTE(WINE_WhiteS_gp) {
  0,    58,  64,  32,  // темно-оливковий, тінь фужера
  50,  106, 112,  46,  // соломʼяно-жовтий з зеленуватим тоном
  150, 154, 160,  86,  // світло-соломʼяний, мʼяке світло
  255,  90,  96,  38   // приглушений жовто-оливковий
};
DEFINE_GRADIENT_PALETTE(WINE_White_gp) {
  0,    48,  56,  22,  // темний соломʼяний
  50,   96, 112,  46,  // світло-жовтий з зеленуватим відтінком
  150, 144, 160,  86,  // блідо-соломʼяний
  255,  80,  96,  38   // жовто-оливковий, край келиха
};
DEFINE_GRADIENT_PALETTE(WINE_Orange_gp) {
  0,    50,  30,  0,   // темно-бурштиновий
  60,   90,  60,  10,  // мідно-помаранчевий
  150, 120, 90,  20,   // золотисто-бурштиновий
  220,  70,  50,  10,  // блідо-помаранчевий
  255,  50,  40,  0    // темний бурштин, край
};
DEFINE_GRADIENT_PALETTE(WINE_Red_gp) {
  0,   255, 40, 80,   // темний бордо
  80,  60,  5,  5,    // темно-багряний
  110, 120, 10, 20,   // червоно-цегляний
  130, 180, 20, 40,   // яскравий рубіновий
  138, 255, 40, 80,   // вогняно-червоний
  150, 120, 10, 20,   // приглушений червоний
  180, 40,  5,  5,    // темний бордовий
  220, 128, 0,  20,   // майже чорний червоний
  255, 160, 0,  20    // чорний з червоним відтінком
};
DEFINE_GRADIENT_PALETTE(WINE_RedOld_gp) {
  0,   245, 40, 30,  // темний гранатовий
  80,  60,  5,  5,   // темно-багряний
  110, 110, 20, 10,  // цегляно-червоний
  130, 160, 40, 20,  // коричнево-червоний
  138, 235, 80, 40,  // теплий бурштиново-червоний
  150, 110, 20, 10,  // приглушений старий червоний
  180, 40,  5,  5,   // темний винний
  220, 118, 0,  0,   // майже чорний бордо
  255, 150, 0,  0    // чорний з винним підтоном
};
DEFINE_GRADIENT_PALETTE(Sunrise_gp) {
  0,   240, 230, 180, // сонце зійшло 0 центр
  200, 125, 52,  0,
  255, 0,   0,   20
};
