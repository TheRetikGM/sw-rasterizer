/**
 * @brief This file contains useful utilities
 * @file utils.h
 * @author Jakub Kloub, xkloub03, VUT FIT
 */
#pragma once
#include "swrast_private.h"
#include <cstdint>
#include <glm/glm.hpp>

namespace swrast {
  uint32_t to_rgba(const Color& color);
  uint32_t to_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

} // namespace swrast
