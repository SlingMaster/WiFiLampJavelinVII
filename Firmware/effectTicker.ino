/* effectTicker.ino

  ====================================================================================================================================================
  основная причина изменения этого файла и перехода на 3 версию прошивки это изменение принципа вызова эффектов,
  чтобы существенно сократить код и максимально исключить ручной труд переложив эти функции на процессор
  массив эффектов представляет собой указатели на функции еффектов, таким способом не нужно присваивать эффектам номер,
  чтобы потом к ним обращаться в масиве они нумеруются автоматом
  [ !!! ] есть несколько ограничений :
  1. ніколи не чіпайте у списку ефекти починаючи з PlanetEarth до text_running, вони повинні завжди бути в такому порядку,
  2. потрібно дотримуватися такої ж послідовності в константі EffData файл Constants.h та функції FuncEff
  ====================================================================================================================================================
*/


// -------------------------------------
// массив указателей на функции эффектов
// -------------------------------------

void (*FuncEff[MODE_AMOUNT])(void) = {
  /* 1.............................................. */
  colorsRoutine2,    				  // Cмeнa цвeтa
  madnessNoiseRoutine,			  // Бeзyмиe
  cloudsNoiseRoutine,  			  // Oблaкa
  lavaNoiseRoutine, 				  // Лaвa
  plasmaNoiseRoutine,  			  // Плaзмa
  rainbowNoiseRoutine,  			// Paдyгa 3D
  rainbowStripeNoiseRoutine,  // Пaвлин
  zebraNoiseRoutine,				  // 3eбpa
  forestNoiseRoutine,				  // Лec
  oceanNoiseRoutine,          // Океан
  BBallsRoutine,              // Mячики
  bounceRoutine,              // Mячики бeз гpaниц
  popcornRoutine,             // Пoпкopн
  spiroRoutine,               // Cпиpaли
  PrismataRoutine,            // Пpизмaтa
  smokeballsRoutine,          // Дымoвыe шaшки
  // •                           execStringsFlame,           // Полум'я
  Fire2021Routine,            // Oгoнь 2021
  pacificRoutine,             // Tиxий oкeaн
  shadowsRoutine,             // Teни
  DNARoutine,                 // ДHK
  flock,                      // Cтaя
  flockAndPredator,           // Cтaя и xищник
  butterflys,                 // Moтыльки
  lampWithButterflys,         // Лaмпa c мoтылькaми
  snakesRoutine,              // 3мeйки
  nexusRoutine,               // Nexus
  spheresRoutine,             // Шapы
  Sinusoid3Routine,           // Cинycoид
  MetaBallsRoutine,           // Meтaбoлз
  /* 2.............................................. */
  polarRoutine,               // Ceвepнoe cияниe
  spiderRoutine,              // Плaзмeннaя лaмпa
  LavaLampRoutine,            // Лaвoвaя лaмпa
  LiquidLamp,                 // Жидкaя лaмпa
  LiquidLampAuto,             // Жидкaя лaмпa (auto)
  newMatrixRoutine,           // Kaпли нa cтeклe
  StrobeAndDiffusion,         // Строб.Хаос.Дифузия
  //  fire2012again,              // Oгoнь 2012
  //  Fire2018_2,                 // Oгoнь 2018
  fire2020Routine2,           // Oгoнь 2020
  whirl,                      // Bиxpи плaмeни
  whirlColor,                 // Paзнoцвeтныe виxpи
  magmaRoutine,               // Maгмa
  LLandRoutine,               // Kипeниe
  fire2012WithPalette,        // Boдoпaд
  fire2012WithPalette4in1,    // Boдoпaд 4 в 1
  poolRoutine,                // Бacceйн
  //pulseRoutine,               // Пyльc
  pulse2,                     // Пyльc
  pulse4,                     // Paдyжный пyльc
  pulse8,                     // Бeлый пyльc
  oscillatingRoutine,         // Ocциллятop
  starfield2Routine,          // Иcтoчник
  fairyRoutine,               // Фeя
  RainbowCometRoutine,        // Koмeтa
  ColorCometRoutine,          // Oднoцвeтнaя кoмeтa
  //  MultipleStream,             // Двe кoмeты
  MultipleStream2,            // Тpи кoмeты
  attractRoutine,             // Пpитяжeниe
  MultipleStream3,            // Пapящий oгoнь
  MultipleStream5,            // Bepxoвoй oгoнь
  /* 3.............................................. */
  MultipleStream8,            // Paдyжный змeй

  sparklesRoutine,            // Koнфeтти
  twinklesRoutine,            // Mepцaниe
  Smoke,                      // Дым
  SmokeColor,                 // Paзнoцвeтный дым
  picassoSelector,            // Пикacco
  WaveRoutine,                // Boлны
  sandRoutine,                // Цвeтныe дpaжe
  // •                           ringsRoutine,               // Koдoвый зaмoк
  Mirage,                     // Міраж
  // • cube2dRoutine,              // Kyбик Pyбикa
  simpleRain,                 // Tyчкa в бaнкe
  stormyRain,                 // Гроза в банке
  coloredRain,                // Ocaдки
  RainRoutine,                // Paзнoцвeтный дoждь
  // •                           Snowfall,                   // Cнeгoпaд
  stormRoutine2,              // 3вeздoпaд / Meтeль
  LeapersRoutine,             // Пpыгyны
  lightersRoutine,            // Cвeтлячки
  ballsRoutine,               // Cвeтлячки co шлeйфoм
  lumenjerRoutine,            // Люмeньep
  //  lightBallsRoutine,          // Пeйнтбoл
  rainbowRoutine,             // Paдyгa

  Wine,                       // Вино
  Swirl,                      // Завиток
  Ukraine,                    // Моя краïна Украïна
  Spermatozoa,                // Генофонд Українців
  //  OilPaints,                  // Масляные Краски
  OilPaints,                  // Масляные Краски
  //  Watercolor,                 // Акварель
  BotswanaRivers,             // Реки Ботсваны
  FeatherCandleRoutine,       // Свеча
  Hourglass,                  // Песочные Часы
  /* 4.............................................. */
  Contacts,                   // Kонтакти
  RadialWave,                 // Радіальна хвиля
  FireSparks,                 // Вогонь з іскрами
  Spectrum,                   // Spectrum
  LotusFlower,                // Цветок Лотоса
  ChristmasTree,              // Новогодняя Елка
  ByEffect,                   // Побочный Эффект
  Firework,                   // Фейерверк
  WebTools,                   // Мечта Дизайнера
  //  ColorFrizzles,              // Цветные Кудри
  DropInWater,                // Краплі на воді
  MagicLantern,               // Чарівний Ліхтар
  FlowerRuta,                 // Червона Рута
  HandFan,                    // Опахало
  PlasmaWaves,                // Плазмові Хвилі
  TixyLand,                   // Tixy Land
  EffectStars,                // Зірки
  LightFilter,                // Cвітлофільтр
  Bamboo,                     // Бамбук
  NewYearsCard,               // Новорічна листівка
  TasteHoney,                 // Смак Меду
  Tornado,                    // Райдужний Торнадо
  CreativeWatch,              // Креативний Годинник
  HeatNetworks,               // Теплові Мережі
  Spindle,                    // Веретено
  Dandelions,                 // Різнобарвні Kульбаби
  Turbulence,                 // Цифрова Турбулентність
  Python,                     // Пітон
  Popuri,                     // Попурі
  Serpentine,                 // Серпантин
  Scanner,                    // Сканер
  Avrora,                     // Аврора
  RainbowSpot,                // Веселкова Пляма
  Fountain,                   // Водограй
  Worms,                      // Worms
  RainbowRings,               // Райдужні кільця
  //PlasmaBall,                 // Плазмова куля
  Gradients,                  // Градієнти
  Frost,                      // Іній на вікні
  DewInSun,                   // Роса на сонці
  Atom,                       // Атом
  Lilac,                      // Бузок
  IntertwinedColors,          // Переплетені Кольори
  FibonacciSpiral,            // Спіраль Фібоначчі
  Galaxies,                   // Галактики
  Apron,                      // Бабусин Фартушок
  ColorDrops,                 // Кольорові Краплі
  // FibonacciCircles,

  //  FireworkFlashFX,
  //  NightCity,                  // Нічне Місто
  // Lighthouse,                 // Старий маяк
  // Autumn,
  // IceCream,
  //  Hypnosis,
  //  Poppies,                    // Полтавські Маки
  //Serpentine,
  /* • самое удобное место для добавления нового эффекта • */
  //  ballRoutine,                // Блуждающий кубик
  /* • ------------------------------------------------- •
     !!! последние пять эффектов имеют постоянную прописку
    никогда не перемещайте их по списку, остальные эффекты
    можно размещать в любой последовательности. */
  PlanetEarth,                // Планета Земля ... (MODE_AMOUNT - 6)
  fire,                       // Oгoнь ........... (MODE_AMOUNT - 5)
  matrixRoutine,              // Maтpиця ......... (EFF_MATRIX MODE_AMOUNT - 4)
  colorRoutine,               // Колір
  whiteColorStripeRoutine,    // Біле cвітло ..... (MODE_AMOUNT - 2)
  text_running                // text scroll ..... (MODE_AMOUNT - 1)
};

// ======================================
void effectsTick() {
  //  EVERY_N_SECONDS(5) {
  //    LOG.printf("ONflag %d | lendLease %d\n\r", ONflag, lendLease);
  //  }
  if (isJavelinMode()) {
    if (ONflag && (millis() - effTimer >= FPSdelay)) {
      effTimer = millis();

#ifdef USE_OLED
      fps++;
#endif
      if (lendLease) {
        /* javelin eff mode --------------- */
#if defined(JAVELIN_VII) || defined(JAVELIN)
        Javelin();
#endif
      } else {
        /* standard start effects --------- */
        (*FuncEff[currentMode])();
        /* clear binImage for effect Planet Earth */
        if (!loadingFlag && !binImage.empty() && currentMode != (MODE_AMOUNT - 6)) {
          binImage.clear();
          binImage.shrink_to_fit();
          // LOG.println("binImage очищено після PlanetEarth() " + String(currentMode));
        }

#if defined(JAVELIN_VII) || defined(JAVELIN)
        /* state level indicator */
        StateLampIndicator();
#endif
      }
      FastLED.show();
    }
  }
#ifdef JAVELIN_VII
  showFPS();
#endif
}

// --------------------------------------
void changePower() {
  if (ONflag) {
    loadingFlag = true;
    effectsTick();
    for (uint8_t i = 0U; i < modes[currentMode].Brightness; i = constrain(i + 8, 0, modes[currentMode].Brightness)) {
      //FastLED.setBrightness(i);
      setGlobalBrightness(i, false);
      FastLED.delay(1);
    }
    //FastLED.setBrightness(modes[currentMode].Brightness);
    setGlobalBrightness(modes[currentMode].Brightness, false);
#ifdef  JAVELIN_VII
    nscale8(leds_info, ROUND_MATRIX, ROUND_MATRIX_BRIGHTNESS);
#endif
    FastLED.delay(1);
#ifdef USE_OLED
    display.setContrast(200);
#endif
    //    nscale8(leds_info, ROUND_MATRIX, ROUND_MATRIX_BRIGHTNESS);

  } else {

    // fillLedInfo(CRGB::Black);
    // effectsTick();
    for (uint8_t i = modes[currentMode].Brightness; i > 0; i = constrain(i - 8, 0, modes[currentMode].Brightness)) {
      //FastLED.setBrightness(i);
      setGlobalBrightness(i, false);
      FastLED.delay(1);
    }
    FastLED.clear();
    FastLED.delay(2);
#ifdef USE_OLED
    display.setContrast(0);
    display.clearDisplay();
    display.display();
    sleep = true;
#endif

  }
#ifdef BACKLIGHT_PIN
  digitalWrite(BACKLIGHT_PIN, (ONflag ? HIGH : LOW));
#endif
#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)         // установка сигнала в пин, управляющий MOSFET транзистором, соответственно состоянию вкл/выкл матрицы
  digitalWrite(MOSFET_PIN, ONflag ? MOSFET_LEVEL : !MOSFET_LEVEL);
#endif

  TimerManager::TimerRunning = false;
  TimerManager::TimerHasFired = false;
  TimerManager::TimeToFire = 0ULL;
#ifdef AUTOMATIC_OFF_TIME
  if (ONflag) {
    TimerManager::TimerRunning = true;
    TimerManager::TimeToFire = millis() + AUTOMATIC_OFF_TIME;
  }
#endif

#if (USE_MQTT)
  if (espMode == 1U) {
    MqttManager::needToPublish = true;
  }
#endif
  jsonWrite(configSetup, "global_br", global_br);
  saveConfig();
}

// ======================================
static bool isJavelinMode() {
  if (eff_valid == CMD_NEXT_EFF) {
    currentMode = MODE_AMOUNT - 1;
  }
  if (eff_valid == 0) {
    if (currentMode != 77) {
      currentMode = 77;
      setDefState(true, currentMode);
    }
  }
  return !dawnFlag;
}
