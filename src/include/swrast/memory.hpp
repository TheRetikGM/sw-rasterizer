/**
 * @brief This file contains utilities for memory management.
 * @author Jakub Kloub, xklobu03, VUT FIT
 * @file memory.hpp
 */
#pragma once
#include <memory>

namespace swrast {
  template<class T>
  using Ref = std::shared_ptr<T>;
} // namespace swrast
