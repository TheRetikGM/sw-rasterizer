/**
 * @brief This file implements the render pipeline.
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @file render.cpp
 */
#include "render/render.h"
#include "error.hpp"
#include "render/RenderPrimitive.h"
#include "state/State.h"
#include "state/VertexArray.h"
#include "state/VertexBuffer.h"
#include "state/Program.h"
#include "state/ObjectHandleFromId.hpp"
#include <cstring>
#include <stdexcept>

using namespace swrast;

/// Call func for each vertex with given vertexID
template<class Func>
void for_each_vertex_id(const RenderContext& ctx, const Func& func) {
  if (ctx.vao->HasIndexBuffer()) {
    auto ibo = ctx.vao->GetIndexBuffer();
    for (auto& index : ibo->data)
      func(index);
  } else {
    for (uint32_t i = 0; i < ctx.cmd.count; i++)
      func(i);
  }
}

template<class T> T get(float* data) {
    T d;
    d = *reinterpret_cast<T*>(data);
    return d;
}
std::any any_from_attributetype(AttributeType type, float* data) {
  switch(type) {
  case AttributeType::Int32: return int(data[0]);
  case AttributeType::Float32: return data[0];
  case AttributeType::Vec2: return get<glm::vec2>(data);
  case AttributeType::IVec2: return get<glm::ivec2>(data);
  case AttributeType::Vec3: return get<glm::vec3>(data);
  case AttributeType::IVec3: return get<glm::ivec3>(data);
  case AttributeType::Vec4: return get<glm::vec4>(data);
  case AttributeType::IVec4: return get<glm::ivec4>(data);
  case AttributeType::Mat3: return get<glm::mat3>(data);
  case AttributeType::Mat4: return get<glm::mat4>(data);
  }
  throw std::invalid_argument("any_from_attributetype: Invalid AttributeType");
}

void assemble_vertex_attributes(const RenderContext& ctx, ObjectHandle<VertexShader>& vs, uint32_t vertex_id) {
  static float attr_data[16] = { 0.0f };  // 4x4 matrix has 16 floats
  vs->m_VertexId = vertex_id;
  int location = 0;
  for (const auto& attr : ctx.vao->GetAttributes()) {
    uint8_t* data = (uint8_t*)&attr.vbo->data[0];
    std::memcpy(attr_data, data + attr.offset + attr.stride * vertex_id, get_byte_size(attr.type));
    vs->m_Attributes[location] = any_from_attributetype(attr.type, attr_data);
    location++;
  }
}

void pfo(FragmentShader* fs) {
  auto fb = RenderState::ctx.fb;
  auto tex_idx = glm::uvec2(fs->m_FragCoord);

  // Depth test
  auto depth_buffer = fb->GetDepthBuffer();
  if (RenderState::ctx.depth && depth_buffer.has_value()) {
    float* depth = (float*)(depth_buffer->Get().GetPixel(tex_idx));
    if (fs->m_FragCoord.z >= *depth)
      return;

    // Depth write
    *depth = fs->m_FragCoord.z;
  }

  // Write into color buffer
  auto col_buf = fb->GetColorAttach(0);
  if (col_buf.has_value()) {
    uint8_t* pixel = col_buf->Get().GetPixel(tex_idx);
    glm::vec<4, uint8_t> col = fs->m_FragColor * glm::vec4(255);
    std::memcpy(pixel, &col, channel_count(col_buf->Get().m_IntFormat));
  }

  // TODO: Write blended pixel into framebuffer.
}

void process_pixel(RenderPrimitive* prim, glm::vec4& pix_pos) {
  auto& fs = RenderState::ctx.prg->GetFragmentShader();

  // Interpolate VS output variables and pixel's depth.
  prim->Interpolate(pix_pos, fs->InVars());
  fs->m_FragCoord = pix_pos;
  fs->Execute();

  pfo(fs.obj_ptr);
}

void process_primitive(RenderPrimitive* prim) {
  TrianglePrimitive* p = dynamic_cast<TrianglePrimitive*>(prim);
  prim->Clip([](RenderPrimitive* prim) {
    prim->PerpDiv();
    prim->NdcTransform();
    if (prim->Cull())
      return;
    prim->Rasterize([prim](glm::vec4 pix_pos){ process_pixel(prim, pix_pos); });
  });
}


RenderPrimitive* new_primitive(const RenderContext& ctx) {
  static auto tprim = TrianglePrimitive();

  switch (ctx.cmd.draw_primitive) {
  case Primitive::Points:
  case Primitive::Lines:
  case Primitive::LineStrip:
  case Primitive::LineLoop:
  case Primitive::Polygon:
  case Primitive::TriangleStrip:
  case Primitive::TriangleFan:
    RAISEn(NotImplementedException);
  case Primitive::Triangles:
    return &tprim;
  }
  throw std::invalid_argument("new_primitive: Invalid draw primitive");
}

void RenderState::Draw(const RenderCommand& render_command) {
  ctx = {
    .cmd = render_command,
    .prg = ObjectHandle<Program>::FromId(State::m_activeProgram.value()),
    .vao = ObjectHandle<VertexArray>::FromId(State::m_activeVao.value()),
    .fb = ObjectHandle<Framebuffer>::FromId(State::m_activeFb),
    .cull = State::m_cullFace,
    .depth = State::m_depthTest,
  };
  if (ctx.prg->GetVertexShader()->m_Attributes.size() < ctx.vao->GetAttributes().size())
    ctx.prg->GetVertexShader()->m_Attributes.resize(ctx.vao->GetAttributes().size());
  ctx.prg->GetFragmentShader()->InVars().clear();

  RenderPrimitive* p = new_primitive(ctx);
  p->m_OnEmit = process_primitive;

  for_each_vertex_id(ctx, [&](uint32_t vertex_id){
    auto& vs = ctx.prg->GetVertexShader();
    assemble_vertex_attributes(ctx, vs, vertex_id);
    vs->Execute();
    p->ProcessVertex(vs->m_Position);
  });
}
