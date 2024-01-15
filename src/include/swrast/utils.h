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

  using StrId = unsigned int;

  /// Convert string to ID used for indexing uniforms and in/out variables.
  constexpr StrId str_to_id(const char* str) noexcept {
    size_t hash = 0;
    while (*str) {
        hash = (hash * 31) + *str;
        ++str;
    }
    return hash;
  }

  constexpr StrId operator"" _sid(const char* str, size_t) noexcept {
    return str_to_id(str);
  }

} // namespace swrast
