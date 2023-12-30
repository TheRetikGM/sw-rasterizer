/**
 * @brief This file contains utilities for handling errors.
 * @file error.hpp
 * @author Jakub Kloub, xkloub03, VUT FIT
 */
#pragma once
#include "ren_utils/basic.h"
#include "swrast_private.h"
#include <atomic>
#include <exception>
#include <string>

namespace swrast {
  class Exception {
  protected:
    const char* m_file;
    int m_line;
    std::string m_msg;
  public:
    Exception(const char* file, int line)
      : m_file(file)
      , m_line(line)
      , m_msg(format("%s:%i: ", file, line)) {}

    virtual const char* what() const noexcept { return m_msg.c_str(); }
  };

  /// Raise exception with variable arguments.
  #define RAISE(e, ...) throw e(__FILE__, __LINE__, __VA_ARGS__)
  /// Raise exception without arguments.
  #define RAISEn(e) throw e(__FILE__, __LINE__)

  struct NotImplementedException : public Exception {
    NotImplementedException(const char* file, int line) : Exception(file, line) {
      m_msg += "Not yet implemented";
    }
  };

  struct ObjectNotFoundException : public Exception {
    ObjectNotFoundException(const char* file, int line, ObjectId id) : Exception(file, line) {
      m_msg += format("Object with ID = %i could not be found.", id);
    }
  };
} // namespace swrast
