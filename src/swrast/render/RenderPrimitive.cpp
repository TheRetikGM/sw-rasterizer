/**
 * @brief Implementation of primitive-specific pipeline functions.
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @file RenderPrimitive.cpp
 */
#include "render/RenderPrimitive.h"
#include "state/Framebuffer.h"
#include "state/Program.h"

using namespace swrast;


TrianglePrimitive::TrianglePrimitive(const std::array<Vertex, 3>& vertices)
  : m_Vertices(vertices) {
  auto ac = c - a;
  auto ab = b - a;
  m_invArea = 1.0f / glm::abs(ac.x * ab.y - ac.y * ab.x);
}

void TrianglePrimitive::ProcessVertex(glm::vec4& position) {
  m_Vertices[m_currentVertex].pos = position;
  m_Vertices[m_currentVertex].vars = RenderState::ctx.prg->GetVertexShader()->OutVars();
  if (++m_currentVertex == 3) {
    m_currentVertex = 0;
    auto ac = c - a;
    auto ab = b - a;
    m_invArea = 1.0f / glm::abs(ac.x * ab.y - ac.y * ab.x);
    m_OnEmit(this);
  }
}

inline bool is_ccw(const TrianglePrimitive& prim) {
  const glm::vec2 ab = glm::vec2(prim.m_Vertices[1].pos) - glm::vec2(prim.m_Vertices[0].pos);
  const glm::vec2 ac = glm::vec2(prim.m_Vertices[2].pos) - glm::vec2(prim.m_Vertices[0].pos);
  return ac.x * ab.y - ac.y * ab.x <= 0.0f;
}


void TrianglePrimitive::Clip(const PrimFunc& func)  {
  std::array<uint8_t, 3> pi = { 0, 1, 2 };

  // Sort the vertices by z, so we can easily decide situation.
  std::sort(pi.begin(), pi.end(), [this](uint8_t a, uint8_t b) {
      return m_Vertices[a].pos.z < m_Vertices[b].pos.z;
  });

  // Decide the position situation between primitive and near plane.
  uint8_t situation = 0;
  situation |= uint8_t(m_Vertices[0].pos.z >= -m_Vertices[0].pos.w) << 2;
  situation |= uint8_t(m_Vertices[1].pos.z >= -m_Vertices[1].pos.w) << 1;
  situation |= uint8_t(m_Vertices[2].pos.z >= -m_Vertices[2].pos.w) << 0;

  // Early exit for primitives entirely before/behind near plane.
  if (situation == 7) {   // All vertices behind near plane.
    func(this);
    return;
  } else if (situation == 0)   // All vertices before near plane.
    return;

  // Split edge with near plane and return the interpolated intersection OutVertex.
  const auto& cut_edge = [](Vertex& a, Vertex& b) -> Vertex {
    float t = (a.pos.z + a.pos.w) / (a.pos.z - b.pos.z + a.pos.w - b.pos.w);
    Vertex x;
    x.pos = (1.0f - t) * a.pos + t * b.pos;
    // Interpolate the cutted edge points.
    for (auto& [name, var] : a.vars) {
      if (var.integer)
        x.vars[name] = var;
      else
        x.vars[name].f4 = (1 - t) * a.vars[name].f4 + t * b.vars[name].f4;
    }
    return x;
  };

  if (situation == 3) {   // First vertex before near plane.
    Shader::InOutVars i1_attrs, i2_attrs;
    Vertex i1 = cut_edge(m_Vertices[1], m_Vertices[0]);
    Vertex i2 = cut_edge(m_Vertices[2], m_Vertices[0]);

    auto p1 = TrianglePrimitive({ m_Vertices[1], i1, i2 });
    auto p2 = TrianglePrimitive({ m_Vertices[1], i2, m_Vertices[2] });
    if (is_ccw(*this) ^ is_ccw(p1)) {
      std::swap(p1.m_Vertices[1], p1.m_Vertices[2]);
      std::swap(p2.m_Vertices[1], p2.m_Vertices[2]);
    }
    func(&p1);
    func(&p2);
  } else if (situation == 1) {   // First two vertices before near plane.
    auto p1 = TrianglePrimitive({
        cut_edge(m_Vertices[2], m_Vertices[1]),
        cut_edge(m_Vertices[2], m_Vertices[0]),
        m_Vertices[2]
    });
    if (is_ccw(*this) ^ is_ccw(p1))
      std::swap(p1.m_Vertices[1], p1.m_Vertices[2]);
    func(&p1);
  }
}

void TrianglePrimitive::PerpDiv() {
  a /= a.w;
  b /= b.w;
  c /= c.w;
}

void TrianglePrimitive::NdcTransform() {
  a.x = (a.x + 1) * RenderState::ctx.fb->GetSize().x * 0.5f;
  a.y = (a.y + 1) * RenderState::ctx.fb->GetSize().y * 0.5f;

  b.x = (b.x + 1) * RenderState::ctx.fb->GetSize().x * 0.5f;
  b.y = (b.y + 1) * RenderState::ctx.fb->GetSize().y * 0.5f;

  c.x = (c.x + 1) * RenderState::ctx.fb->GetSize().x * 0.5f;
  c.y = (c.y + 1) * RenderState::ctx.fb->GetSize().y * 0.5f;
}

bool TrianglePrimitive::Cull() {
  auto cull = RenderState::ctx.cull;
  if (cull == CullFace::None)
    return false;
  if (cull == CullFace::CCW)
    return is_ccw(*this);
  return !is_ccw(*this);
}

// Implementation of Pineda's rasterization algorithm.
void TrianglePrimitive::Rasterize(const FragFunc& func) {
    /* Rasterize primitive using Pineda's rasterization algorithm. */
  glm::vec2 v[] = {
    glm::vec2(a),
    glm::vec2(b),
    glm::vec2(c),
  };

  // If the vertices aren't in CCW order, then convert them to CCW.
  glm::vec2 ab = v[1] - v[0];
  glm::vec2 ac = v[2] - v[0];
  if (ac.x * ab.y - ac.y * ab.x >= 0.0f)
    std::swap(v[1], v[2]);

  // Compute bounding box for this primitive
  glm::vec2 bmin = glm::floor(glm::min(glm::min(v[0], v[1]), v[2]));
  glm::vec2 bmax = glm::ceil(glm::max(glm::max(v[0], v[1]), v[2]));

  // Clip the bounding box to the size of the framebuffer.
  bmin = glm::max({ 0.0f, 0.0f }, bmin);
  bmax = glm::min(glm::vec2(RenderState::ctx.fb->GetSize()), bmax);

  // Compute the edge vectors and functions.
  glm::vec2 d1 = v[1] - v[0];
  glm::vec2 d2 = v[2] - v[1];
  glm::vec2 d3 = v[0] - v[2];
  float e1 = (bmin.y - v[0].y + 0.5f) * d1.x - (bmin.x - v[0].x + 0.5f) * d1.y;
  float e2 = (bmin.y - v[1].y + 0.5f) * d2.x - (bmin.x - v[1].x + 0.5f) * d2.y;
  float e3 = (bmin.y - v[2].y + 0.5f) * d3.x - (bmin.x - v[2].x + 0.5f) * d3.y;

  // Go over every pixel in bounding box and call `func` if inside triangle.
  // TODO: Optimization as mentioned in presentation.
  for (int y = bmin.y; y < (int)bmax.y; y++) {
    float t1 = e1, t2 = e2, t3 = e3;
    for (int x = bmin.x; x < (int)bmax.x; x++) {
      if (t1 >= 0 && t2 >= 0 && t3 >= 0)
        func(glm::vec4((float)x + 0.5f, (float)y + 0.5f, 0.0f, 1.0f));   // NOTE: Fragment depth is added in fragment_interpolate().
      t1 -= d1.y; t2 -= d2.y; t3 -= d3.y;
    }
    e1 += d1.x; e2 += d2.x; e3 += d3.x;
  }
}

void TrianglePrimitive::Interpolate(glm::vec4& pos, Shader::InOutVars& vars) {
  // Vectors from the fragment to all vertices of primitive.
  glm::vec2 fa = glm::vec2(a) - glm::vec2(pos);
  glm::vec2 fb = glm::vec2(b) - glm::vec2(pos);
  glm::vec2 fc = glm::vec2(c) - glm::vec2(pos);

  // Compute volume of triangle between two vectors.
  const auto& get_volume = [](const glm::vec2& u, const glm::vec2& v) -> float { return std::abs((u.x * v.y - u.y * v.x)); };

  // Compute the interpolation lambdas.
  float la = get_volume(fb, fc) * m_invArea;
  float lb = get_volume(fa, fc) * m_invArea;
  float lc = get_volume(fb, fa) * m_invArea;

  // Interpolate attributes
  float s = la / a.w + lb / a.w + lc / a.w;
  /// Perspectively Correct Lambdas
  glm::vec3 pcl = { la / (a.w * s), lb / (b.w * s), lc / (c.w * s) };
  for (auto& [name, val] : a_attr) {
    if (val.integer)
      vars[name].i4 = a_attr[name].i4;
    else
      vars[name].f4 = pcl.x * a_attr[name].f4 + pcl.y * b_attr[name].f4 + pcl.z * c_attr[name].f4;
  }

  // Interpolate depth.
  pos.z = pcl.x * a.z + pcl.y * b.z + pcl.z * c.z;
}

