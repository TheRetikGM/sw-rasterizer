/**
 * @brief This file contains the main rendering interface.
 * @author Jakub Kloub, xkluob03, vUT FIT
 * @file render/render.h
 */
#pragma once
#include "state/State.h"
#include <cstddef>
#include <cstdint>

namespace swrast {
  /// Represents the basic primitives used during rendering.
  enum class BasicPrimitive : uint8_t {
    Triangle, Line, Point
  };

  /// Represents the different rendering modes.
  enum class Primitive : uint8_t {
    Points, Lines, LineStrip, LineLoop,
    Polygon, Triangles, TriangleStrip,
    TriangleFan,
  };

  struct RenderCommand {
    Primitive draw_primitive;
    bool is_indexed;
    size_t count;
    size_t offset;
  };

  struct RenderContext {
    RenderCommand cmd;
    ObjectHandle<Program> prg;
    ObjectHandle<VertexArray> vao;
    ObjectHandle<Framebuffer> fb;
    CullFace cull;
    bool depth;
  };

  class RenderState {
  public:
    inline static RenderContext ctx{};
    static void Draw(const RenderCommand& render_command);
  };
} // namespace swrast
