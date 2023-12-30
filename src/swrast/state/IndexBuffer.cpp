/**
 * @brief Implementation of state/IndexBuffer.h
 * @file state/IndexBuffer.cpp
 * @author Jakub Kloub, xkloub03, VUT FIT
 */
#include "state/IndexBuffer.h"
#include "state/State.h"

using namespace swrast;

template<>
OptRef<IndexBuffer> State::GetObject(ObjectId id) {
  if (m_ibos.count(id) == 0)
    return {};
  return m_ibos[id];
}

template<>
ObjectHandle<IndexBuffer> State::CreateObject(IndexBuffer&& ibo) {
  return {
    .obj_ptr = &(State::m_ibos.emplace(ibo.Id, std::move(ibo)).first->second),
    .obj_id = ibo.Id,
  };
}
