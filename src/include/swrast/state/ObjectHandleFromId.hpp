/**
 * @file state/ObjectHandleFromId.hpp
 * @author Jakub Kloub, xklobu03, VUT FIT
 * @brief This file contains ObjectHandle implementation.
 */
#pragma once
#include "Framebuffer.h"
#include "IndexBuffer.h"
#include "Program.h"
#include "Texture.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "error.hpp"

namespace swrast {
  template<class T>
  ObjectHandle<T> ObjectHandle<T>::FromId(ObjectId obj_id) {
    auto obj = State::GetObject<T>(obj_id);
    if (!obj.has_value())
      RAISE(ObjectNotFoundException, obj_id);
    return ObjectHandle<T>{ &obj.value().get(), obj_id };
  }
} // namespace swrast
