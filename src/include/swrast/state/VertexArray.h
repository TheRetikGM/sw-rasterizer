/**
 * @file state/VertexBuffer.h
 * @brief This file contains a vertex buffer declarations.
 * @author Jakub Kloub, xkloub03, VUT FIT
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
    Int32 = 0, Float32, Vec2, IVec2, Vec3, IVec3, Vec4, IVec4, Mat3, Mat4,
  };

  uint32_t get_byte_size(AttributeType type);

  /// Represents a single vertex array attribute.
  struct VertexAttribute {
    ObjectHandle<VertexBuffer> vbo;   ///< Buffer to sample from
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
    inline void SetIndexBuffer(const ObjectHandle<IndexBuffer>& i) { m_indexBuffer = i; }
    inline bool HasIndexBuffer() const { return m_indexBuffer.has_value(); }
    void Use();

    inline const std::vector<VertexAttribute>& GetAttributes() const  { return m_attribs; }
    inline const ObjectHandle<IndexBuffer>& GetIndexBuffer() const { return m_indexBuffer.value(); }

  private:
    std::optional<ObjectHandle<IndexBuffer>> m_indexBuffer;
    std::vector<VertexAttribute> m_attribs;
  };

  template<>
  OptRef<VertexArray> State::GetObject(ObjectId id);
  template<>
  ObjectHandle<VertexArray> State::CreateObject(VertexArray&& vao);
} // namespace swrast
