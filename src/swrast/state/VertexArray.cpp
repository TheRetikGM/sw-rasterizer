/**
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @brief Implementation of VertexArray.h
 * @file VertexArray.cpp
 *
 */
#include "state/VertexArray.h"
#include "error.hpp"

using namespace swrast;

VertexArray::VertexArray(const std::vector<VertexAttribute>&& attributes,
                         std::optional<ObjectHandle<IndexBuffer>> index_buffer)
    : m_indexBuffer(index_buffer), m_attribs(attributes) {}

void VertexArray::AddAttribute(const VertexAttribute &attr) {
  m_attribs.push_back(attr);
}

void VertexArray::Use() { RAISEn(NotImplementedException); }

template<>
OptRef<VertexArray> State::GetObject(ObjectId id) {
  if (State::m_vaos.count(id) == 0)
    return {};
  return State::m_vaos[id];
}

template<>
ObjectHandle<VertexArray> State::CreateObject(VertexArray&& vao) {
  return {
    .obj_ptr = &(State::m_vaos.emplace(vao.Id, std::move(vao)).first->second),
    .obj_id = vao.Id,
  };
}
