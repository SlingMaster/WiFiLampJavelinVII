/* eff_func.h */
// ======================================
// Global Function for Effects
// ======================================
#pragma once
#include <FastLED.h>
#include <math.h>

// ==============================
inline void keepColor(CRGB &c, uint8_t minv = 3) {
  if (c.r < minv) c.r = 0;
  if (c.g < minv) c.g = 0;
  if (c.b < minv) c.b = 0;
}

// ==============================
inline uint8_t validMinMax(float val, uint8_t minV, uint8_t maxV) {
  if (val <= minV) return minV;
  if (val >= maxV) return maxV;
  return (uint8_t)ceilf(val);
}

// ==============================
inline CRGB makeDarker(const CRGB &c, uint8_t amount) {
  CRGB r = c;
  r.nscale8(255 - amount);
  return r;
}

//// ==============================
//inline CRGB makeBrighter(const CRGB &c, uint8_t amount) {
//  CRGB r = c;
//  r.nscale8_video(255 - amount);
//  return r;
//}
//
//// ==============================
//inline float clampf(float v, float minv, float maxv) {
//  if (v < minv) return minv;
//  if (v > maxv) return maxv;
//  return v;
//}
//
//// ==============================
//template <typename T>
//inline void swap_val(T &a, T &b) {
//  T t = a;
//  a = b;
//  b = t;
//}


// ==============
// END ==============
// ==============
