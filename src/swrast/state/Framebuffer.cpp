/**
 * @brief This file contains implementation of state/Framebuffer.h
 * @file state/Framebuffer.cpp
 * @author Jakub Kloub, xkloub03, VUT FIT
 */
#include "state/Framebuffer.h"
#include <algorithm>

using namespace swrast;

const char* to_string(FramebufferState state) {
  static const char* S[] = {
    "Complete",
    "Missing color attachment",
    "Some attachments doesn't have the same size as framebuffer",
  };
  return S[uint8_t(state)];
}

Framebuffer::Framebuffer(glm::uvec2 size, FramebufferSpec spec)
  : m_size(size)
  , m_depthBuffer(spec.depth_buffer)
  , m_colorAtts(spec.color_atts)
{
  if (!std::all_of(
        spec.color_atts.begin(),
        spec.color_atts.end(),
        [&size](ObjectHandle<Texture>& att) { return att->GetSize() == size; }
      )) {
    m_state = FramebufferState::SizeMismatch;
    return;
  }
  if (spec.depth_buffer.has_value() && spec.depth_buffer.value()->GetSize() != size) {
    m_state = FramebufferState::SizeMismatch;
    return;
  }
  if (spec.color_atts.size() == 0) {
    m_state = FramebufferState::MissingColor;
  }
}

Framebuffer Framebuffer::CreateBasic(glm::uvec2 size) {
  return Framebuffer(size, {
    .depth_buffer = State::CreateObject(Texture({}, size, TexFormat::rgba)),  // 32 bits precision of depth.
    .color_atts = { State::CreateObject(Texture({}, size, TexFormat::rgba)) },
  });
}

template<>
OptRef<Framebuffer> State::GetObject(ObjectId id) {
  if (m_fbos.count(id) == 0)
    return {};
  return m_fbos[id];
}

template<>
ObjectHandle<Framebuffer> State::CreateObject(Framebuffer&& fb) {
  return {
    .obj_ptr = &(State::m_fbos.emplace(fb.Id, std::move(fb)).first->second),
    .obj_id = fb.Id,
  };
}

Framebuffer& Framebuffer::Use() {
  State::SetActiveFramebuffer(this->Id);
  return *this;
}

const Framebuffer& Framebuffer::Use() const {
  State::SetActiveFramebuffer(this->Id);
  return *this;
}

Framebuffer& Framebuffer::Clear(Opt<Color> color, bool depth) {
  if (color.has_value()) {
    for (auto& ca : m_colorAtts)
      ca->Fill(color.value());
  }
  if (depth && m_depthBuffer.has_value()) {
    m_depthBuffer.value()->Fill(glm::vec4(1.0f));
  }
  return *this;
}

Opt<ObjectHandle<Texture>> Framebuffer::GetColorAttach(uint32_t index) const {
  if (index >= m_colorAtts.size())
    return {};
  return m_colorAtts[index];
}

