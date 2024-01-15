/**
 * @file state/VertexBuffer.h
 * @brief This file contains a vertex buffer declarations.
 * @author Jakub Kloub, xkloub03, VUT FIT
 */
#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "swrast_private.h"
#include "state/State.h"

namespace swrast {
  class VertexBuffer : public UniqueId<VertexBuffer> {
  public:
    using Data = std::vector<float>;
    Data data;

    VertexBuffer(const Data&& data) : data(data) {}
    VertexBuffer() : data() {}
  };

  template<>
  OptRef<VertexBuffer> State::GetObject(ObjectId id);
  template<>
  ObjectHandle<VertexBuffer> State::CreateObject(VertexBuffer&& vbo);
} // namespace swrast

