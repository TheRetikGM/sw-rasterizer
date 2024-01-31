/**
 * @brief This file continas primitive-specific functionality for rendering.
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @file RenderPrimitive.h
 */
#pragma once
#include <array>
#include <functional>
#include <glm/glm.hpp>
#include "state/Program.h"

namespace swrast {
  struct InOutVar;

  class RenderPrimitive {
  public:
    using PrimFunc = std::function<void(RenderPrimitive*)>;
    using FragFunc = std::function<void(glm::vec4)>;
    /// Function called each time a new primitive is constructed.
    PrimFunc m_OnEmit = [](auto){};

    virtual ~RenderPrimitive() {}

    virtual void ProcessVertex(glm::vec4& position) = 0;
    virtual void Clip(const PrimFunc& func) = 0;
    virtual void PerpDiv() = 0;
    virtual void NdcTransform() = 0;
    virtual bool Cull() = 0;
    void Rasterize(const FragFunc& func) {
      if (State::m_WriteFrame)
        wireframe(func);
      else
        rasterize(func);
    }
    virtual void Interpolate(glm::vec4& pos, Shader::InOutVars& vars) = 0;

    /**
     * @brief Set specific primitive.
     *
     * This function is used to further specify what kind of primitive is this and how should the
     * primitives be emitted based on that. For example for TrianglePrimitive there are primitives
     * such as TriangleStrip and TriangleFan.
     * @param prim Primitive to set.
     */
    virtual void SetPrimitive(Primitive prim) = 0;
    /// Reset the vertex emitting state.
    virtual void Reset() {};
  protected:

    virtual void rasterize(const FragFunc& func) = 0;
    virtual void wireframe(const FragFunc& func) = 0;
  };

  /// Struct represeting a single vertex in a primitie.
  struct Vertex {
    /// Position of the vertex.
    glm::vec4 pos;
    /// Variables to be interpolated.
    Shader::InOutVars vars;
  };

  class TrianglePrimitive : public RenderPrimitive {
    float m_invArea;
    unsigned m_currentVertex = 0;
    Primitive m_prim;
    std::array<glm::vec4, 2> m_posBackup;
    bool m_even = true;

  public:
    std::array<Vertex, 3> m_Vertices;
    glm::vec4& a = m_Vertices[0].pos;
    glm::vec4& b = m_Vertices[1].pos;
    glm::vec4& c = m_Vertices[2].pos;
    Shader::InOutVars& a_attr = m_Vertices[0].vars;
    Shader::InOutVars& b_attr = m_Vertices[1].vars;
    Shader::InOutVars& c_attr = m_Vertices[2].vars;

    TrianglePrimitive(const std::array<Vertex, 3>& vertices);
    TrianglePrimitive() = default;

    void ProcessVertex(glm::vec4& position) override;
    void Clip(const PrimFunc& fun) override;
    void PerpDiv() override;
    void NdcTransform() override;
    bool Cull() override;
    void Interpolate(glm::vec4& pos, Shader::InOutVars& vars) override;
    void SetPrimitive(Primitive prim) override;
    void Reset() override;

  protected:
    void rasterize(const FragFunc& func) override;
    void wireframe(const FragFunc& func) override;
  };

  class LinePrimitive : public RenderPrimitive {
    /// Current processed vertex. This is used during ProcessVertex.
    unsigned m_currentVertex = 0;
    /// Vector from a to b used during interpolation.
    glm::vec2 m_ab;
    Primitive m_prim;
  public:
    std::array<Vertex, 2> m_Vertices;
    glm::vec4& a = m_Vertices[0].pos;
    glm::vec4& b = m_Vertices[1].pos;
    Shader::InOutVars& a_attr = m_Vertices[0].vars;
    Shader::InOutVars& b_attr = m_Vertices[1].vars;

    LinePrimitive(const std::array<Vertex, 2>& vertices);
    LinePrimitive() = default;

    void ProcessVertex(glm::vec4& position) override;
    void Clip(const PrimFunc& fun) override;
    void PerpDiv() override;
    void NdcTransform() override;
    bool Cull() override;
    void Interpolate(glm::vec4& pos, Shader::InOutVars& vars) override;
    void SetPrimitive(Primitive prim) override;
    void Reset() override;

  protected:
    void rasterize(const FragFunc& func) override;
    void wireframe(const FragFunc& func) override;
  };
} // namespace swrast

