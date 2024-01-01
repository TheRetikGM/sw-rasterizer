/**
 * @brief This file contains the implementation of utils
 * @file utils.cpp
 * @author Jakub Kloub, xkloub03, VUT FIT
 */
#include "utils.h"

using namespace swrast;

uint32_t swrast::to_rgba(const Color& color) {
  return to_rgba(uint8_t(color.r * 255), uint8_t(color.g * 255), uint8_t(color.b * 255), uint8_t(color.a * 255));
}

uint32_t swrast::to_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  uint32_t c = 0;
  c |= uint32_t(r) << (3 * 8);
  c |= uint32_t(g) << (2 * 8);
  c |= uint32_t(b) << (1 * 8);
  c |= uint32_t(a) << (0 * 8);
  return c;
}
