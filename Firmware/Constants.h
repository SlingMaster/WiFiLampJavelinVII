/*
  Constants.h
  ===============================================================================================================
  УВАГА!!! Частина налаштувань у data/config і ConstantsUser (користувацькі для різних схем/матриць).
  Розділи: | МАТРИЦЯ | СХЕМОТЕХНІКА | ЕФЕКТИ | та розділ для розробників.
  ===============================================================================================================
*/
#pragma once
String configSetup = "{}";

struct EffData {
  uint8_t br;
  uint8_t speed;
  uint8_t scale;
  uint8_t fps_delay;
  uint8_t min_spd;
  uint8_t max_spd;
  uint8_t min_scl;
  uint8_t max_scl;
  uint8_t type;
  char* name_ua;
  char* name_en;
};

// ============= НАЛАШТУВАННЯ =============
// --- ESP -----------------------------
#define ESP_CONF_TIMEOUT      (60U)                         // час у секундах, який лампа чекатиме на введення пароля для OTA оновлення (пароль співпадає з паролем точки доступу)

// --- ESP (WiFi клієнт) ---------------
// SSID та пароль вашої WiFi-мережі задаються на веб-сторінці лампи в режимі WiFi точки доступу за IP 192.168.4.1
// Там же задається час у секундах (таймаут), протягом якого ESP буде намагатися підключитися до WiFi мережі; після його завершення автоматично розгорне WiFi точку доступу
#define  INTERNET_CHECK_PERIOD (45U)                         // період повторної перевірки наявності інтернету у секундах

// --- AP (WiFi точка доступу) ---------
String AP_NAME = "";                                        // змінна для імені точки доступу. Задається на веб-сторінці
String AP_PASS = "";                                        // змінна для пароля точки доступу. Задається на веб-сторінці
String LAMP_NAME = "";                                      // змінна для імені лампи. Задається на веб-сторінці
const uint8_t AP_STATIC_IP[] = {192, 168, 4, 1};            // статична IP-адреса точки доступу (краще не змінювати)
uint8_t DONT_TURN_ON_AFTER_SHUTDOWN;                        // Не видаляйте і не коментуйте цей рядок

// --- ЧАС ---------------------------
uint32_t AUTOMATIC_OFF_TIME (SLEEP_TIMER * 60UL * 60UL * 1000UL);  // Не видаляйте і не коментуйте цей рядок
// #define WARNING_IF_NO_TIME      (7U)                        // якщо розкоментувати цей рядок, лампа буде підмигувати в нижньому ряді світлодіодів, коли вона не знає, котра зараз година
//                                                           7 - це яскравість миготливих точок (максимум - 255U), коли лампа вимкнена
//#define WARNING_IF_NO_TIME_ON_EFFECTS_TOO                 // якщо розкоментувати цей рядок, підмигування буде навіть під час роботи ефектів; яскравість точок буде така ж, як у працюючого ефекту
char NTP_ADDRESS [32];                                      // адреса NTP | автоматичний вибір часової зони
#define NTP_INTERVAL          (59 * 60UL * 1000UL)          // інтервал синхронізації часу (59 хвилин)




// =====================================
// JSON Effects List default state -----
// =====================================
/* constants field 3 for jsonListEff
   константи затримок зміни кадрів -  */
#define DYNAMIC                (0U)   // динамічна затримка для кадрів ( використовуватиметься бігунок Швидкість )
#define SOFT_DELAY             (1U)   // затримка для зміни кадрів FPS delay задається програмно прямо в тілі ефекту
#define LOW_DELAY             (15U)   // низька фіксована затримка для зміни кадрів
#define HIGH_DELAY            (50U)   // висока фіксована затримка для зміни кадрів

// ================ РЕЕСТР ДОСТУПНЫХ ЭФФЕКТОВ ===================
const EffData effectsPROGMEM[] PROGMEM = {
  /*
    BRI| SPD| SCL|     FPS   |  SPEED  |  SCALE  | T | EFFECT  NAME   | */
  {10, 252,  32, HIGH_DELAY,   1, 255,   1, 255, 0, "Зміна кольорів", "Change colors"},
  {11,  33,  58, HIGH_DELAY,   1, 150,   1, 100, 0, "Божевілля", "Madness"},
  { 8,   4,  34, HIGH_DELAY,   1,  15,   1,  50, 0, "Хмари", "Clouds"},
  { 8,   9,  24, HIGH_DELAY,   5,  60,   1, 100, 0, "Лaвa", "Lava"},
  {11,  19,  59, HIGH_DELAY,   1,  30,   1, 100, 0, "Плазма", "Plasma"},
  {11,  13,  60, HIGH_DELAY,   1,  70,   1, 100, 0, "Веселка 3D", "Rainbow 3D"},
  {11,   5,  12, HIGH_DELAY,   1,  15,   1,  30, 0, "Павич", "Peacock"},
  { 7,   8,  21, HIGH_DELAY,   1,  30,   7,  40, 0, "Зебра", "Zebra"},
  { 7,   8,  95, HIGH_DELAY,   2,  30,  70, 100, 0, "Гай", "Grove"},
  { 7,   6,  12, HIGH_DELAY,   2,  15,   4,  30, 0, "Океан", "Ocean"},
  {24, 255,  26,  LOW_DELAY,   1, 255,   1, 100, 0, "М'ячі", "Balls"},
  {18,  11,  70,  LOW_DELAY,   1, 255,   1, 100, 0, "М'ячі без кордонів", "Balls no borders"},
  {19,  32,  16,  LOW_DELAY,   1, 255,   1, 100, 0, "Попкорн", "Popcorn"},
  { 9,  46,   3,  LOW_DELAY,   1, 255,   1, 100, 0, "Спіралі", "Spirals"},
  {17, 100,   2,  LOW_DELAY,   1, 255,   1, 100, 0, "Призмата", "The prism"},
  {12,  44,  17,  LOW_DELAY,   1, 255,   1, 100, 0, "Димові шашки", "Smoke bombs"},
  { 9,  51,  11,  LOW_DELAY,   1, 255,   1, 100, 0, "Вогонь 2021", "Fire 2021"},
  {55, 127, 100,  LOW_DELAY,   1, 255, 100, 100, 0, "Тихий океан", "Pacific Ocean"},
  {39,  77,   1,  LOW_DELAY,   1, 255,   1, 100, 0, "Тіні", "Shadows"},
  {15,  77,  95,  LOW_DELAY,   1, 255,   1, 100, 0, "ДНК", "DNA"},
  {15, 136,   4,  LOW_DELAY,   1, 255,   1, 100, 0, "Зграя", "Flock"},
  {15, 122,  65,  LOW_DELAY,   1, 255,   1, 100, 0, "Зграя та хижак", "Flock and predator"},
  {11,  53,  87,  LOW_DELAY,   1, 255,   1, 100, 0, "Метелики", "Butterflies"},
  { 7,  61, 100,  LOW_DELAY,   1, 255,   1, 100, 1, "Лампа з метеликами", "Lamp & butterflies"},
  { 9,  96,  31,  LOW_DELAY,   1, 255,   1, 100, 0, "Змійки", "Snakes"},
  {19,  60,  20,  LOW_DELAY,   1, 255,   1, 100, 0, "Nexus", "Nexus"},
  { 9,  85,  85,  LOW_DELAY,   1, 255,   1, 100, 0, "Кулі", "Spheres"},
  { 7, 122,  83,  LOW_DELAY,   1, 255,   1, 100, 0, "Синусоїд", "Sinusoid"},
  { 7,  85,  83,  LOW_DELAY,   1, 255,   1, 100, 0, "Метаболізм", "Metabolism"},
  {12,  73,  38,  LOW_DELAY,   1, 255,   1, 100, 1, "Північне сяйво", "Northern Lights"},

  { 8,  59,  18,  LOW_DELAY,   1, 255,   1, 100, 0, "Плазмова лампа", "Plasma lamp"},
  {23, 203,   1,  LOW_DELAY,   1, 255,   1, 100, 1, "Лавова лампа", "Lava lamp"},
  {11,  63,   1,  LOW_DELAY,   1, 255,   1, 100, 1, "Рідка лампа", "Liquid lamp"},
  {11, 124,  39,  LOW_DELAY,   1, 255,   1, 100, 0, "Рідка лампа авто", "Liquid lamp auto"},
  {23,  71,  59,  LOW_DELAY,   1, 255,   1, 100, 1, "Краплі на склі", "Drops on glass"},
  {40,  70,  35, SOFT_DELAY,   1, 150,   1, 100, 0, "Строб | Хаос | Дифузія", "Strobe | Chaos"},

  //  { 9, 225,  59,    DYNAMIC,  99, 252,   1, 100, 0, "Вогонь 2012", "Fire 2012"},
  //  {57, 225,  15,    DYNAMIC,  99, 252,   1, 100, 0, "Вогонь 2018", "Fire 2018"},

  { 9, 210,  20,    DYNAMIC, 120, 252,   1, 100, 0, "Oгoнь 2020", "Fire 2020"},
  { 9, 210,   1,    DYNAMIC,  99, 252,   1, 100, 1, "Вихори полум'я", "Vortices of flame"},
  { 9, 210,  86,    DYNAMIC,  99, 252,   1, 100, 0, "Kольорові вихори", "Colored vortices"},
  { 9, 198,  20,    DYNAMIC, 150, 252,   1, 100, 0, "Магма", "Magma"},
  { 7, 210,  18,    DYNAMIC, 170, 252,   1, 100, 0, "Кипіння", "Boiling"},
  { 5, 212,  54,    DYNAMIC,  99, 252,   1, 100, 1, "Водоспад", "Waterfall"},
  { 7, 197,  22,    DYNAMIC,  99, 252,   1, 100, 0, "Водоспад 4 в 1", "Waterfall 4 in 1"},
  { 8, 222,  63,    DYNAMIC,  99, 252,   1, 100, 1, "Басейн", "Basin"},
  {12, 185,   6,    DYNAMIC,  99, 252,   1, 100, 0, "Пульс", "Pulse"},
  {11, 185,  31,    DYNAMIC,  99, 252,   1, 100, 0, "Райдужний пульс", "Rainbow pulse"},
  { 9, 179,  11,    DYNAMIC,  99, 252,   1, 100, 0, "Білий пульс", "White pulse"},
  { 8, 208, 100,    DYNAMIC,  99, 252,   1, 100, 0, "Осцилятор", "Oscillator"},
  {15, 233,  77,    DYNAMIC,  99, 252,   1, 100, 0, "Джерело", "Wellspring"},
  {19, 212,  44,    DYNAMIC,  99, 252,   1, 100, 0, "Фея", "Fairy"},
  {16, 220,  28,    DYNAMIC,  99, 252,   1, 100, 0, "Комета", "Comet"},
  {14, 212,  69,    DYNAMIC,  99, 252,   1, 100, 1, "Однобарвна комета", "Monochrome comet"},
  //  {27, 186,  19,    DYNAMIC,  99, 252,   1, 100, 0, "Дві комети", "Two comets"},
  {24, 186,   9,    DYNAMIC,  99, 252,   1, 100, 0, "Три комети", "Three comets"},
  {21, 203,  65,    DYNAMIC, 160, 252,   1, 100, 0, "Тяжіння", "Gravity"},
  {26, 206,  15,    DYNAMIC,  99, 252,   1, 100, 0, "Парячий вогонь", "Soaring fire"},
  {26, 190,  15,    DYNAMIC,  99, 252,   1, 100, 0, "Верховий вогонь", "Mounted fire"},
  {12, 178,   1,    DYNAMIC,  99, 252,   1, 100, 0, "Райдужний змій", "Rainbow snake"},
  {16, 142,  63,    DYNAMIC,  99, 252,   1, 100, 0, "Конфетті", "Confetti"},
  {25, 236,   4,    DYNAMIC,  60, 252,   1, 100, 0, "Мерехтіння", "Flicker"},
  { 9, 157, 100,    DYNAMIC,  99, 252,   1, 100, 1, "Дим", "Smoke"},

  { 9, 157,  30,    DYNAMIC,  99, 252,   1, 100, 0, "Різнокольоровий дим", "Multicolored smoke"},
  { 9, 189,  43,    DYNAMIC,  99, 252,   1, 100, 0, "Пікассо", "Picasso"},
  { 9, 236,  80,    DYNAMIC, 220, 252,   1, 100, 0, "Хвилі", "Waves"},
  { 9, 195,  80,    DYNAMIC,  99, 252,   1, 100, 0, "Кольорові драже", "Colored pills"},
  {10, 255,  30,    DYNAMIC,   1, 254,   1, 100, 1, "Міраж", "Mirage"},
  {30, 233,   2,    DYNAMIC,  99, 252,   1, 100, 0, "Хмарка в банці", "Cloud in jar"},
  {20, 236,  10,    DYNAMIC,  99, 252,   1, 100, 0, "Гроза в банці", "Thunderstorm"},
  {15, 198,  99,    DYNAMIC,  99, 252,   0, 255, 1, "Опади", "Precipitation"},
  {15, 225,   1,    DYNAMIC,  99, 252,   1, 100, 1, "Різнобарвний дощ", "Colorful rain"},
  {20, 199,  54,    DYNAMIC,  99, 252,   1, 100, 0, "Зіркопад | Завірюха", "Starfall"},
  {24, 203,   5,    DYNAMIC, 150, 252,   1, 100, 0, "Стрибуни", "Jumpers"},
  {15, 157,  23,    DYNAMIC,  50, 252,   1, 100, 0, "Світлячки", "Fireflies"},
  {21, 198,  93,    DYNAMIC,  99, 252,   1, 100, 0, "Світлячки зі шлейфом", "Fireflies & trail"},
  {14, 223,  40,    DYNAMIC,   1, 252,   1, 100, 0, "Люмен'єр", "Lumenier"},
  //  {21, 236,   7,    DYNAMIC, 215, 252,   1, 100, 0, "Пейнтбол", "Paintball"},
  { 8, 196,  50,    DYNAMIC,  50, 252,   1, 100, 0, "Веселка", "Rainbow"},
  {12, 170,  95,    DYNAMIC,  1, 255,    1, 100, 0, "Вино", "Wine"},
  {80, 210,  50,    DYNAMIC,  50, 252,   1, 100, 1, "Завиток", "Curl"},
  {50, 231,   1,    DYNAMIC, 200, 250,   1, 100, 0, "Моя країна - Україна", "Ukraine"},
  {12,  50,  50, SOFT_DELAY,   1, 255,   1, 100, 1, "Геном UA", "Genome UA"},
  //  {55, 135,  50,    DYNAMIC,   1, 210,   1, 100, 0, "Масляні фарби", "Oil paints"},
  {40, 149,  75,    DYNAMIC,   1, 255,   1, 100, 0, "Масляні фарби", "Oil paints"},
  //  {16, 254,  99,    DYNAMIC,  20, 252,   1, 100, 1, "Акварель", "Watercolor"},
  {25, 150,  50,    DYNAMIC,   1, 200,   1, 100, 0, "Річки Ботсвани", "Botswana rivers"},
  {55, 220,  18,    DYNAMIC, 170, 255,   1,  99, 0, "Свічка", "Candle"},
  {30, 215, 100,    DYNAMIC,  15, 240,   1, 100, 1, "Пісочний годинник", "Sand clock"},
  {10, 175,  60,    DYNAMIC,  25, 240,   1, 100, 1, "Kонтакти", "Contacts"},
  {10, 215,  50,    DYNAMIC,  50, 255,  49,  50, 0, "Радіальна хвиля", "Radial Wave"},
  {22,  35,  50,    DYNAMIC,  20, 100,   1, 100, 0, "Вогонь з іскрами", "Fire with Sparks"},
  {10, 210,  70,    DYNAMIC, 150, 215,   1, 100, 0, "Spectrum", "Spectrum"},

  {50, 200,  10, SOFT_DELAY,   1, 254,   1,  99, 0, "Квітка Лотоса", "Lotus Flower"},
  {24,  90,  50,    DYNAMIC,   1, 160,   1, 100, 0, "Новорічна ялинка", "Christmas tree"},
  {45, 150,  30,    DYNAMIC,   1, 200,   1, 100, 0, "Побічний ефект", "A side effect"},
  {90,  50,   0, SOFT_DELAY,  10, 245,   1,  95, 1, "Феєрверк", "Firework"},
  {22, 128,  50, SOFT_DELAY,  10, 245,   5,  95, 0, "Мрія Дизайнера", "Designer's dream"},
  //  {55, 128,  25, SOFT_DELAY, 128, 128,  10,  90, 0, "Кольорові кучері", "Colored curls"},
  { 8, 160,  65,    DYNAMIC, 120, 215,   1, 100, 1, "Краплі на воді", "Droplets on water"},
  {45,  30,  60,    DYNAMIC,   1, 245,   1, 100, 1, "Чарівний Ліхтар", "Magic Lantern"},
  {10, 215,  60,    DYNAMIC, 150, 254,   1, 100, 0, "Червона Рута", "Red Ruth"},
  {11, 250,  65,    DYNAMIC, 200, 255,   1, 100, 1, "Опахало", "Hand Fan"},
  {10, 128,  50,    DYNAMIC, 127, 128,   1, 100, 0, "Плазмові Хвилі", "Plasma Waves"},
  { 8, 165,   0, SOFT_DELAY,   1, 254,   1, 100, 1, "Tixi Land", "Tixy Land"},
  { 8, 215,  99,    DYNAMIC,  80, 254,   1, 100, 0, "Зірки", "Stars"},
  {12, 160,  95,    DYNAMIC,  80, 255,   1, 100, 0, "Cвітлофільтр", "Light Filter"},
  {12, 215,  90,    DYNAMIC, 100, 255,   1, 100, 0, "Бамбук", "Bamboo"},
  {12, 128,  75,    DYNAMIC,  50, 160,   1, 100, 0, "Новорічна листівка", "New Year's Card"},
  {12, 215,  15,    DYNAMIC, 128, 255,   1, 100, 1, "Смак Меду", "A Taste of Honey"},
  {10, 160,  50,    DYNAMIC,   1, 254,   1, 100, 0, "Райдужний Торнадо", "Rainbow Tornado"},
  {10, 128,  30, SOFT_DELAY, 128, 128,   1, 100, 1, "Креативний Годинник", "Creative Watch"},
  {12, 200,   0,    DYNAMIC, 128, 255,   1, 100, 1, "Теплові Мережі", "Heat Networks"},
  {16, 215,  35,    DYNAMIC, 128, 255,   1, 100, 0, "Веретено", "Spindle"},
  {18,  50,  90, SOFT_DELAY,  10, 255,   1, 100, 0, "Різнобарвні Kульбаби", "Colored Dandelions"},
  {16, 215,  35,    DYNAMIC,   1, 255,   1, 100, 0, "Цифрова Турбулентність", "Digital Turbulence"},
  { 9, 128,  25,  LOW_DELAY, 127, 128,   1, 100, 0, "Пітон", "Python"},
  { 8, 128,  20, HIGH_DELAY, 128, 254,   1, 100, 1, "Попурі", "Popuri"},
  {12,  75,  50, HIGH_DELAY,   1, 255,   1, 100, 0, "Серпантин", "Serpentine"},
  {50, 255,  40,    DYNAMIC,   1, 255,   1, 100, 1, "Сканер", "Scanner"},
  {35,  90,  50, HIGH_DELAY,   1, 255,  50,  50, 0, "Аврора", "Avrora"},
  {22, 200,  40,    DYNAMIC,   1, 255,   1, 100, 0, "Веселкова Пляма", "Rainbow Spot"},
  {20, 250,  75,    DYNAMIC,   1, 255,   1, 100, 1, "Водограй", "Fountain"}, // 40
  {30, 255,  50,    DYNAMIC,  50, 255,  20, 100, 0, "Worms", "Worms"},       // 40
  {12, 255,  25,    DYNAMIC, 127, 128,   1, 100, 0, "Райдужні кільця", "Rainbow Rings"},
  //  { 7, 255,  50,    DYNAMIC, 128, 250,  49,  50, 0, "Плазмова куля", "Plasma Ball"},
  { 8, 150,  50,    DYNAMIC,   1, 255,  1,  100, 0, "Градієнти", "Gradients"},
  { 7,  50,  50, SOFT_DELAY, 127, 128,  49,  50, 0, "Іній на віконці", "Frost on window"},
  {15, 160,  60,    DYNAMIC,   1, 255,   1, 100, 0, "Роса на сонці", "Dew in sun"},
  {15, 250,  25,    DYNAMIC,  50, 255,   1, 100, 0, "Атом", "Atom"},
  {40,  75,  50, HIGH_DELAY,  60, 255,   1, 100, 1, "Бузок", "Lilac"},
  {40, 230,  80,    DYNAMIC,   1, 255,   1, 100, 0, "Переплетені Кольори", "Intertwined Colors"},
  {10, 200,  30, HIGH_DELAY,   1, 255,   1, 100, 0, "Спіраль Фібоначчі", "Fibonacci Spiral"},
  {25, 200, 100, SOFT_DELAY,   1, 255,   1, 100, 0, "Галактики", "Galaxies"},
  {10, 160, 100,    DYNAMIC,   1, 255,   1, 100, 0, "Бабусин Фартушок", "Grandma's Apron"},
  {15, 160, 100, SOFT_DELAY,  20, 255,   1, 100, 1, "Кольорові Краплі", "Color Drops"},
  //  {60, 200,   0,    DYNAMIC,   1, 255,   1, 100, 0, "Світанок", "Sunrise"},

  /* • нижче фіксоване положення ефектів в списку які не можна змінювати • */
  {10, 150,  75, SOFT_DELAY,   1, 254,   1, 100, 0, "Планета Земля", "Planet Earth"},
  {22, 225,   1,    DYNAMIC, 128, 254,   1, 100, 1, "Oгoнь Intim", "Fire Intim"},
  {27, 186,  23,    DYNAMIC, 128, 254,   1, 100, 0, "Matrix", "Matrix"},
  { 9, 180,  99, HIGH_DELAY, 128, 254,   1, 100, 1, "Колір", "Color"},
  {40, 128,  50, HIGH_DELAY, 128, 254,   1, 100, 1, "Біле cвітло", "White light"},
  {10, 128,  64,    DYNAMIC,   1, 252,   1, 100, 1, "Текст із прокруткою", "Scroll text"}
};

#define MODE_AMOUNT (sizeof(effectsPROGMEM) / sizeof(EffData)) // the number of effects is calculated automatically

/* фіксовані номери ефектів  --------------------------------*/
#define EFF_MATRIX             (MODE_AMOUNT - 4)            // Matrix
#define EFF_WHITE_COLOR        (MODE_AMOUNT - 2)            // White light

// =====================================
// === ІНШЕ ДЛЯ РОЗРОБНИКІВ =====
// =====================================
#define BRIGHTNESS            (5U)                         // стандартна максимальна яскравість (0-255). Використовується тільки під час ввімкнення живлення лампи

#if defined (ESP_USE_BUTTON)
#define BUTTON_STEP_TIMEOUT   (100U)                        // кожні BUTTON_STEP_TIMEOUT мс буде генеруватися подія утримання кнопки (для регулювання яскравості)
#define BUTTON_CLICK_TIMEOUT  (500U)                        // максимальний час між натисканнями кнопки в мс, після якого серія послідовних натискань вважається завершеною
#if (BUTTON_IS_SENSORY == 1U)
#define BUTTON_SET_DEBOUNCE   (20U)                         // час антидребезгу мс для сенсорної кнопки
#else
#define BUTTON_SET_DEBOUNCE   (55U)                         // час антидребезгу мс для механічної кнопки
#endif
#endif
#define ESP_RESET_ON_START    (false)                       // true – якщо при старті натиснута кнопка (або кнопки немає!), збережені налаштування будуть скинуті; false – не будуть
#define ESP_HTTP_PORT         (80U)                         // номер порту, який буде використовуватися під час першого встановлення імені WiFi мережі (і пароля), до якої потім лампа підключатиметься в режимі WiFi клієнта (краще не змінювати)
#define ESP_UDP_PORT          (8888U)                       // номер порту, який буде "слухати" UDP сервер під час роботи лампи як у режимі WiFi точки доступу, так і в режимі WiFi клієнта (краще не змінювати)
#define WIFIMAN_DEBUG         (false)                       // вивід відлагоджувальних повідомлень при підключенні до WiFi мережі: true – виводяться, false – не виводяться; налаштування не залежить від GENERAL_DEBUG

// --- СХІД СОНЦЯ -------------------------
#define DAWN_BRIGHT           (70U)                         // максимальная яркость рассвета (0-255)
uint8_t DAWN_TIMEOUT;                                       // сколько рассвет светит после времени будильника, минут. Может быть изменено в установках будильника

#define MAX_UDP_BUFFER_SIZE   (255U)                        // максимальный размер буффера UDP сервера
#define MAX_FRAME_BUFER (NUM_LEDS + 7U)

// Remote control ------- | не лізь бо вб'є |
// команди дистанційного керування через HTTP
#define CMD_STATE             (0U)
#define CMD_POWER             (1U)
#define CMD_NEXT_EFF          (2U)
#define CMD_PREV_EFF          (3U)
#define CMD_BRIGHT_UP         (4U)
#define CMD_BRIGHT_DW         (5U)
#define CMD_SPEED             (6U)
#define CMD_SCALE             (7U)
#define CMD_AUTO              (8U)
#define CMD_TEXT              (9U)
#define CMD_INFO              (10U)
#define CMD_WHITE             (11U)
#define CMD_FAV               (12U)
#define CMD_RESET             (13U)
#define CMD_RESET_EFF         (14U)
#define CMD_SHOW_EFF          (15U)
#define CMD_DEFAULT           (16U)
#define CMD_RANDOM            (17U)
#define CMD_GLOBAL_BRI        (19U)
#define CMD_INTIM             (20U)
#define CMD_OTA               (21U)
//#define CMD_IP                (22U)
#define CMD_TEST_MATRIX       (23U)
#define CMD_SCAN              (32U)
#define CMD_ECHO              (33U)
#define CMD_GROUP_INIT        (35U)
#define CMD_GROUP_DESTROY     (36U)
#define CMD_FS_DIR            (40U)
#define CMD_DEL_FILE          (41U)
#define CMD_LIST              (55U)
#define CMD_CONFIG            (60U)
#define CMD_SAVE_CFG          (61U)
#define CMD_SAVE_ALARMS       (62U)
#define CMD_CUSTOM_EFF        (66U)
#define CMD_FW_INFO           (69U)
#define CMD_CONNECT           (77U)
#define CMD_DIAGNOSTIC        (90U)
#define CMD_ACTIVATE          (95U)
#define CMD_EFF_JAVELIN       (99U)
#define CMD_DISCOVER          (50U)
// -------------------------------------
String VERSION = "7.1 " + String(MODE_AMOUNT);
