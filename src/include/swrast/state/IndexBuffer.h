/**
 * @brief This file contains declaration of index buffer.
 * @author Jakub Kloub, xkloub03, VUT  FIT
 * @file IndexBuffer.h
 */
#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "state/State.h"
#include "swrast_private.h"

namespace swrast {
  class IndexBuffer : public UniqueId<IndexBuffer> {
  public:
    using Data = std::vector<uint32_t>;
    Data data;

    IndexBuffer(const Data&& data) : data(data) {}
    IndexBuffer() : data() {}
  };

  template<>
  OptRef<IndexBuffer> State::GetObject(ObjectId id);

  template<>
  ObjectHandle<IndexBuffer> State::CreateObject(IndexBuffer&& ibo);
} // namespace swrast

