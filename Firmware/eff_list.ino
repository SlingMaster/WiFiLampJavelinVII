/* eff_list.ino */

// ========================================
String getEffectName(int idx) {
  String res;
  EffData effectsSRAM;
  memcpy_P( &effectsSRAM, &effectsPROGMEM[idx], sizeof(EffData));
  res = effectsSRAM.name_en;
  // res += " | ";
  // res = effectsSRAM.name_ua;
  return res;
}

// ========================================
void setDefState(bool all, uint8_t idx) {
  EffData effectsSRAM;
  memcpy_P( &effectsSRAM, &effectsPROGMEM[idx], sizeof(EffData));
  if (all) modes[idx].Brightness = effectsSRAM.br;
  modes[idx].Speed  = effectsSRAM.speed;
  modes[idx].Scale  = effectsSRAM.scale;
}

// ========================================
void setFPS() {
  EffData effectsSRAM;
  memcpy_P( &effectsSRAM, &effectsPROGMEM[currentMode], sizeof(EffData));
  /* set effects frame delay ----- */
  if (effectsSRAM.fps_delay == DYNAMIC) {
    FPSdelay = 256U - modes[currentMode].Speed;
  } else {
    if (effectsSRAM.fps_delay != SOFT_DELAY) {
      FPSdelay = effectsSRAM.fps_delay;
    }
  }
#ifdef GENERAL_DEBUG
  // LOG.printf_P(PSTR(" Effect • %03d | FPSdelay • %d | Eff Auto • %d |\n\r"), currentMode, FPSdelay, eff_auto);
#endif
}

// ========================================
String createListPackage(int start_idx) {
  EffData effectsSRAM;

  /*
    br | speed | scale | fps_delay | min_spd | max_spd | min_scl | max_scl | type | name_ua | name_en |
  */

  StaticJsonDocument<2048> jsonBuffer;
  JsonArray resultArray = jsonBuffer.to<JsonArray>();
  int end_idx = start_idx + 10;
  if (end_idx > MODE_AMOUNT) end_idx = MODE_AMOUNT;

  for (int i = start_idx; i < end_idx; i++) {
    memcpy_P( &effectsSRAM, &effectsPROGMEM[i], sizeof(EffData));

#ifdef GENERAL_DEBUG
    //    if (i % 10U == 0) LOG.printf("\n\r| BRI | SPD | SCL | FPS | * |   SPEED   |   SCALE   | T | EFFECT  NAME   | Heap %d bytes\n\r", system_get_free_heap_size());
    //    LOG.printf("| %3d ", effectsSRAM.br);
    //    LOG.printf("| %3d ", effectsSRAM.speed);
    //    LOG.printf("| %3d ", effectsSRAM.scale);
    //    LOG.printf("| %3d ", effectsSRAM.fps_delay);
    //    LOG.print("| * ");
    //    LOG.printf("| %3d ", effectsSRAM.min_spd);
    //    LOG.printf(". %3d ", effectsSRAM.max_spd);
    //    LOG.printf("| %3d ", effectsSRAM.min_scl);
    //    LOG.printf(". %3d ", effectsSRAM.max_scl);
    //    LOG.printf("| %1d | ", effectsSRAM.type);
    //    LOG.print(effectsSRAM.name_ua);
    //    LOG.print(" | ");
    //    LOG.println(effectsSRAM.name_en);
#endif

    JsonObject newObject = resultArray.createNestedObject();
    JsonArray vArray = newObject.createNestedArray("v");
    /* Copy data to arr "v" */
    vArray.add(effectsSRAM.min_spd);
    vArray.add(effectsSRAM.max_spd);
    vArray.add(effectsSRAM.min_scl);
    vArray.add(effectsSRAM.max_scl);
    vArray.add(effectsSRAM.type);
    // Копіювання рядкових полів "name"
    newObject["n"] = effectsSRAM.name_ua;
    newObject["e"] = effectsSRAM.name_en;
  }

  String jsonStr;
  serializeJson(jsonBuffer, jsonStr);
  return jsonStr;
}

// ========================================
void  restoreDefaultSettings() {
  EffData effectsSRAM;

  for (int i = 0; i < MODE_AMOUNT; i++) {
    memcpy_P( &effectsSRAM, &effectsPROGMEM[i], sizeof(EffData));
#ifdef GENERAL_DEBUG
    LOG.printf("%3d. | br • %3d | FPS • %2d\n\r", i, effectsSRAM.br, effectsSRAM.fps_delay);
#endif
    setDefState(true, i);
  }
}

// ========================================
// END
// ========================================
