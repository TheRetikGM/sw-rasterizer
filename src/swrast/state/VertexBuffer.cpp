/**
 * @brief Implementation of state/VertexBuffer.h
 * @file state/VertexBuffer.cpp
 * @author Jakub Kloub, xkloub03, VUT FIT
 */
#include "state/VertexBuffer.h"
#include "state/State.h"

using namespace swrast;

template<>
OptRef<VertexBuffer> State::GetObject(ObjectId id) {
  if (State::m_vbos.count(id) == 0)
    return {};
  return OptRef<VertexBuffer>(m_vbos[id]);
}

template<>
ObjectHandle<VertexBuffer> State::CreateObject(VertexBuffer&& vbo) {
  return {
    .obj_ptr = &(State::m_vbos.emplace(vbo.Id, std::move(vbo)).first->second),
    .obj_id = vbo.Id,
  };
}
