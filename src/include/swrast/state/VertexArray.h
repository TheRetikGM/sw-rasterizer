/**
 * @file state/VertexBuffer.h
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @brief This file contains a vertex buffer declarations.
 */
#pragma once
#include "state/State.h"
#include "state/VertexBuffer.h"
#include "swrast_private.h"
#include <glm/glm.hpp>
#include <optional>
#include <vector>

namespace swrast {
  enum class AttributeType : uint8_t {
    Float32, Int32,
  };

  /// Represents a single vertex array attribute.
  struct VertexAttribute {
    ObjectHandle<VertexBuffer> vbo;   ///< Buffer to sample from
    uint32_t count;                   ///< Number of elements to sample
    AttributeType type;               ///< Type of single element component.
    size_t stride;                    ///< Number of bytes to skip to get new sample.
    size_t offset;                    ///< Offset to first sample
  };

  /**
   * @brief Represents a vertex array ObjectHandle
   *
   * Vertex array object is group of vertex attributes that say what buffers should be
   */
  class VertexArray : public UniqueId<VertexArray> {
  public:
    using Data = std::vector<VertexAttribute>;
    VertexArray(const Data&& attributes,
                std::optional<ObjectHandle<IndexBuffer>> index_buffer = {});
    VertexArray() : m_indexBuffer({}), m_attribs() {}

    void AddAttribute(const VertexAttribute& attr);
    void Use();

  private:
    std::optional<ObjectHandle<IndexBuffer>> m_indexBuffer;
    std::vector<VertexAttribute> m_attribs;
  };

  template<>
  OptRef<VertexArray> State::GetObject(ObjectId id);

  template<>
  ObjectHandle<VertexArray> State::CreateObject(VertexArray&& vao);
} // namespace swrast
