/**
 * @brief Implementation of pipeline functions for render primitives.
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @file RenderPrimitive.cpp
 */
#include "render/render.h"
#include "render/RenderPrimitive.h"
#include "state/Framebuffer.h"
#include "state/Program.h"
#include <immintrin.h> // SIMD instructions
#include <glm/gtc/type_ptr.hpp>

using namespace swrast;

void bresenham_line(glm::ivec2 a, glm::ivec2 b, const RenderPrimitive::FragFunc& func) {
  glm::ivec2 u = b - a;
  bool flip_x = false;
  bool flip_y = false;

  if (u.x < 0) {
    u.x = -u.x;
    a.x = -a.x;
    b.x = -b.x;
    flip_x = true;
  }
  if (u.y < 0) {
    u.y = -u.y;
    a.y = -a.y;
    b.y = -b.y;
    flip_y = true;
  }

  int x = a.x, y = a.y;
  int e = 0.5f * (u.x - u.y);

  while (x <= b.x && y <= b.y) {
    func(glm::vec4((flip_x ? -x : x) + 0.5f, (flip_y ? -y : y) + 0.5f, 0.0f, 1.0f));
    if (e < 0) {
      y++;
      e += u.x;
    } else {
      x++;
      e -= u.y;
    }
  }
}

inline glm::vec4 simd_interp(const glm::vec4& a, const glm::vec4& b, const glm::vec4& c, const glm::vec3& p) {
  __m128 va = _mm_loadu_ps(glm::value_ptr(a));
  __m128 vb = _mm_loadu_ps(glm::value_ptr(b));
  __m128 vc = _mm_loadu_ps(glm::value_ptr(c));

  __m128 vpx = _mm_set_ps(p.x, p.x, p.x, p.x);
  __m128 vpy = _mm_set_ps(p.y, p.y, p.y, p.y);
  __m128 vpz = _mm_set_ps(p.z, p.z, p.z, p.z);

  __m128 result = _mm_add_ps(_mm_add_ps(_mm_mul_ps(vpx, va), _mm_mul_ps(vpy, vb)), _mm_mul_ps(vpz, vc));

  glm::vec4 v_result;
  _mm_storeu_ps(glm::value_ptr(v_result), result);

  return v_result;
}


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
  a.x /= a.w;
  a.y /= a.w;
  a.z /= a.w;

  b.x /= b.w;
  b.y /= b.w;
  b.z /= b.w;

  c.x /= c.w;
  c.y /= c.w;
  c.z /= c.w;
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
void TrianglePrimitive::rasterize(const FragFunc& func) {
  glm::vec2 v[] = { glm::vec2(a), glm::vec2(b), glm::vec2(c) };

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
      if (t1 >= 0 && t2 >= 0 && t3 >= 0) {
        func(glm::vec4((float)x + 0.5f, (float)y + 0.5f, 0.0f, 1.0f));   // NOTE: Fragment depth is added in fragment_interpolate().
      }
      t1 -= d1.y; t2 -= d2.y; t3 -= d3.y;
    }
    e1 += d1.x; e2 += d2.x; e3 += d3.x;
  }
}

// Liang-Barsky line clipping algorithm.
bool line_clip(glm::vec2& a, glm::vec2& b, glm::vec2 min, glm::vec2 max) {
  const auto maxi = [](float arr[],int n) -> float {
    float m = 0;
    for (int i = 0; i < n; ++i)
      if (m < arr[i])
        m = arr[i];
    return m;
  };
  const auto mini = [](float arr[], int n) -> float {
    float m = 1;
    for (int i = 0; i < n; ++i)
      if (m > arr[i])
        m = arr[i];
    return m;
  };

  float p1 = -(b.x - a.x);
  float p2 = -p1;
  float p3 = -(b.y - a.y);
  float p4 = -p3;

  float q1 = a.x - min.x;
  float q2 = max.x - a.x;
  float q3 = a.y - min.y;
  float q4 = max.y - a.y;

  float posarr[5], negarr[5];
  int posind = 1, negind = 1;
  posarr[0] = 1;
  negarr[0] = 0;

  if ((p1 == 0 && q1 < 0) || (p2 == 0 && q2 < 0) || (p3 == 0 && q3 < 0) || (p4 == 0 && q4 < 0))
      return false;
  if (p1 != 0) {
    float r1 = q1 / p1;
    float r2 = q2 / p2;
    if (p1 < 0) {
      negarr[negind++] = r1; // for negative p1, add it to negative array
      posarr[posind++] = r2; // and add p2 to positive array
    } else {
      negarr[negind++] = r2;
      posarr[posind++] = r1;
    }
  }
  if (p3 != 0) {
    float r3 = q3 / p3;
    float r4 = q4 / p4;
    if (p3 < 0) {
      negarr[negind++] = r3;
      posarr[posind++] = r4;
    } else {
      negarr[negind++] = r4;
      posarr[posind++] = r3;
    }
  }

  float rn1, rn2;
  rn1 = maxi(negarr, negind); // maximum of negative array
  rn2 = mini(posarr, posind); // minimum of positive array

  if (rn1 > rn2)
    return false;

  b.x = a.x + p2 * rn2;
  b.y = a.y + p4 * rn2;
  a.x = a.x + p2 * rn1;
  a.y = a.y + p4 * rn1;

  return true;
}

void TrianglePrimitive::wireframe(const FragFunc& func) {
  glm::vec2 a1 = glm::vec2(a), a2 = glm::vec2(b);
  glm::vec2 b1 = glm::vec2(b), b2 = glm::vec2(c);
  glm::vec2 c1 = glm::vec2(c), c2 = glm::vec2(a);
  glm::vec2 min{ 0, 0 };
  glm::vec2 max = State::GetActiveFramebuffer()->GetSize() - glm::uvec2(1);

  if (line_clip(a1, a2, min, max))
    bresenham_line(a1, a2, func);
  if (line_clip(b1, b2, min, max))
    bresenham_line(b1, b2, func);
  if (line_clip(c1, c2, min, max))
    bresenham_line(c1, c2, func);
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

  /// Perspectively Correct Lambdas
  float s =  la / a.w + lb / b.w + lc / c.w;
  glm::vec3 pcl = { la / (a.w * s), lb / (b.w * s), lc / (c.w * s) };

  // Interpolate attributes
  auto ait = a_attr.begin();
  auto bit = b_attr.begin();
  auto cit = c_attr.begin();
  for (; ait != a_attr.end(); ait++, bit++, cit++) {
    if (ait->second.integer)
      vars[ait->first].i4 = ait->second.i4;
    else
      vars[ait->first].f4 = simd_interp(ait->second.f4, bit->second.f4, cit->second.f4, pcl);
  }

  // Interpolate depth.
  pos.z = pcl.x * a.z + pcl.y * b.z + pcl.z * c.z;
}

void TrianglePrimitive::SetPrimitive(Primitive prim) {
  assert((uint8_t)prim >= 0x40 && (uint8_t)prim <= 0x42);
  m_prim = prim;
}


LinePrimitive::LinePrimitive(const std::array<Vertex, 2>& vertices)
  : m_prim(Primitive::Lines)
  , m_Vertices(vertices)
{
  m_ab = (glm::vec2)b - (glm::vec2)a;
}
void LinePrimitive::SetPrimitive(Primitive prim) {
  assert((uint8_t)prim >= 0x20 && (uint8_t)prim <= 0x22);
  m_prim = prim;
}
void LinePrimitive::ProcessVertex(glm::vec4& position) {
  m_Vertices[m_currentVertex].pos = position;
  m_Vertices[m_currentVertex].vars = RenderState::ctx.prg->GetVertexShader()->OutVars();
  m_currentVertex++;
  if (m_currentVertex == 2) {
    m_currentVertex = 0;
    m_ab = (glm::vec2)b - (glm::vec2)a;
    m_OnEmit(this);
  }
}
void LinePrimitive::Clip(const PrimFunc& func) {
  unsigned sit = 0;
  sit |= uint8_t(a.z < -a.w) << 1;
  sit |= uint8_t(b.z < -b.w) << 0;

  auto pa = 0;
  auto pb = 1;

  if (sit == 0b11)  // All vertices in front of near plane.
    return;
  if (sit == 0b00) {   // All vertices beind the near plane.
    func(this);
    return;
  }
  if (sit == 0b01)
    std::swap(pa, pb);

  auto& a = m_Vertices[pa];
  auto& b = m_Vertices[pb];

  // Cut the line with near plane.
  glm::vec4 u = (b.pos / b.pos.w) - (a.pos / a.pos.w);
  float t = (-a.pos.w - a.pos.z) / u.z;
  a.pos.x += u.x * t;
  a.pos.y += u.y * t;
  a.pos.z = -a.pos.w;

  // Interpolate attributes
  for (auto& [name, var] : a.vars) {
    if (!var.integer)
      var.f4 = (1 - t) * var.f4 + t * b.vars[name].f4;
  }

  func(this);
}
void LinePrimitive::PerpDiv() {
  a.x /= a.w;
  a.y /= a.w;
  a.z /= a.w;

  b.x /= b.w;
  b.y /= b.w;
  b.z /= b.w;
}

void LinePrimitive::NdcTransform() {
  a.x = (a.x + 1) * RenderState::ctx.fb->GetSize().x * 0.5f;
  a.y = (a.y + 1) * RenderState::ctx.fb->GetSize().y * 0.5f;

  b.x = (b.x + 1) * RenderState::ctx.fb->GetSize().x * 0.5f;
  b.y = (b.y + 1) * RenderState::ctx.fb->GetSize().y * 0.5f;
}
bool LinePrimitive::Cull() { return false; }

void LinePrimitive::rasterize(const FragFunc& func) {
  if (line_clip((glm::vec2&)a, (glm::vec2&)b, glm::vec2(0), glm::vec2(State::GetActiveFramebuffer()->GetSize() - glm::uvec2(1))))
    bresenham_line(glm::ivec2(glm::round(a)), glm::ivec2(glm::round(b)), func);
}

void LinePrimitive::wireframe(const FragFunc& func) { rasterize(func); }

void LinePrimitive::Interpolate(glm::vec4& pos, Shader::InOutVars& vars) {
  auto ab = b - a;
  float lb = (-ab.y * (pos.y - a.y) - ab.x * (pos.x - a.x))
           / (-(ab.x * ab.x + ab.y * ab.y));
  float la = 1.0f - lb;

  // Perspectively correct lambdas.
  float k = (la / a.w + lb / b.w);
  glm::vec2 pcl = { la / (a.w * k), lb / (b.w * k) };

  // Interpolate attributes.
  auto ait = a_attr.begin();
  auto bit = b_attr.begin();
  for (; ait != a_attr.end(); ait++, bit++) {
    if(ait->second.integer)
      vars[ait->first].i4 = ait->second.i4;
    else
      vars[ait->first].f4 = ait->second.f4 * pcl.x + bit->second.f4 * pcl.y;
  }

  // Interpolate depth
  pos.z = pcl.x * a.z + pcl.y * b.z;
}

