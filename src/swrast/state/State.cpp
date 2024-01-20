/**
 * @brief State implementation
 * @file state/State.cpp
 * @author Jakub Kloub, xkloub03, VUT FITj
 */
#include "state/State.h"
#include "render/render.h"
#include "state/VertexArray.h"
#include "state/VertexBuffer.h"
#include "state/IndexBuffer.h"
#include "state/Texture.h"
#include "state/Framebuffer.h"
#include "state/Program.h"
#include "error.hpp"

using namespace swrast;

// Initialization of static member variables.
std::unordered_map<ObjectId, VertexBuffer> swrast::State::m_vbos = {};
std::unordered_map<ObjectId, IndexBuffer> swrast::State::m_ibos = {};
std::unordered_map<ObjectId, VertexArray> swrast::State::m_vaos;
std::unordered_map<ObjectId, Texture> swrast::State::m_textures = {};
std::unordered_map<ObjectId, Framebuffer> swrast::State::m_fbos = {};
std::unordered_map<ObjectId, Ref<Shader>> swrast::State::m_shaders = {};
std::unordered_map<ObjectId, Program> swrast::State::m_programs = {};
ObjectId swrast::State::m_activeFb = 0;
ObjectId swrast::State::m_defaultFb = 0;
Opt<ObjectId> swrast::State::m_activeProgram = {};
Opt<ObjectId> swrast::State::m_activeVao = {};

void State::Init(glm::uvec2 fb_size) {
  // Create default framebuffer.
  auto default_fb = CreateObject<Framebuffer>(Framebuffer(fb_size, {
    .depth_buffer = CreateObject<Texture>(Texture({}, fb_size, TexFormat::rgba)),
    .color_atts = { CreateObject<Texture>(Texture({}, fb_size, TexFormat::rgba)) },
  }));
  m_defaultFb = default_fb->Id;
  m_activeFb = m_defaultFb;
}
void State::Destroy() {
  m_fbos.clear();
  m_vaos.clear();
  m_programs.clear();
  m_shaders.clear();
  m_vbos.clear();
  m_ibos.clear();
  m_textures.clear();
  m_activeFb = 0;
  m_defaultFb = 0;
}

void State::DrawArrays(Primitive primitive, size_t offset, size_t count) {
  RenderCommand cmd = {
    .draw_primitive = primitive,
    .is_indexed = false,
    .count = count,
    .offset = offset,
  };
  RenderState::Draw(cmd);
}
void State::DrawIndexed(Primitive primitive, size_t count) {
  RenderCommand cmd = {
    .draw_primitive = primitive,
    .is_indexed = true,
    .count = count,
    .offset = 0,
  };
  RenderState::Draw(cmd);
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

void State::SetActiveProgram(ObjectId prg_id) {
  if (m_programs.count(prg_id) == 0)
      RAISE(ObjectNotFoundException, prg_id);
  m_activeProgram = prg_id;
}

void State::SetActiveVertexArray(Opt<ObjectId> vao_id) {
  if (!vao_id.has_value()) {
    m_activeVao = {};
    return;
  }
  if (m_vaos.count(vao_id.value()) == 0)
      RAISE(ObjectNotFoundException, vao_id.value());
  m_activeVao = vao_id.value();
}

void State::Clear(Opt<Color> color, bool depth) {
  m_fbos[m_activeFb].Clear(color, depth);
}

ObjectHandle<Framebuffer> State::GetActiveFramebuffer() {
  return {
    .obj_ptr = &m_fbos[m_activeFb],
    .obj_id = m_activeFb,
  };
}

