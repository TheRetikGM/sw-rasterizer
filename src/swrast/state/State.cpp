/**
 * @brief State implementation
 * @file state/State.cpp
 * @author Jakub Kloub, xkloub03, VUT FITj
 */
#include "state/State.h"
#include "state/VertexArray.h"
#include "state/VertexBuffer.h"
#include "state/IndexBuffer.h"
#include "state/Texture.h"
#include "state/Framebuffer.h"
#include "error.hpp"

using namespace swrast;

// Initialization of static member variables.
std::unordered_map<ObjectId, VertexBuffer> swrast::State::m_vbos = {};
std::unordered_map<ObjectId, IndexBuffer> swrast::State::m_ibos = {};
std::unordered_map<ObjectId, VertexArray> swrast::State::m_vaos;
std::unordered_map<ObjectId, Texture> swrast::State::m_textures = {};
std::unordered_map<ObjectId, Framebuffer> swrast::State::m_fbos = {};
ObjectId swrast::State::m_activeFb = 0;
ObjectId swrast::State::m_defaultFb = 0;

void State::Init(glm::uvec2 fb_size) {
  // Create default framebuffer.
  auto default_fb = CreateObject<Framebuffer>(Framebuffer(fb_size, {
    .depth_buffer = CreateObject<Texture>(Texture({}, fb_size, TexFormat::r)),
    .color_atts = { CreateObject<Texture>(Texture({}, fb_size, TexFormat::rgba)) },
  }));
  m_defaultFb = default_fb->Id;
  m_activeFb = m_defaultFb;
}
void State::Destroy() {
  // TODO: Destroy default framebuffer.
}

void State::SetActiveFramebuffer(Opt<ObjectId> fb_id) {
  if (fb_id.has_value()) {
    if (m_fbos.count(fb_id.value()) == 0)
      RAISE(ObjectNotFoundException, fb_id.value());
    m_activeFb = fb_id.value();
  } else {
    m_activeFb = m_defaultFb;
  }
}
