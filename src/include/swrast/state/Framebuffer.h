/**
 * @brief Framebuffer declarations
 * @file state/Framebuffer.h
 * @author Jakub Kloub, xkloub03, VUT FIT
 */
#pragma once
#include "state/State.h"
#include "state/Texture.h"
#include <atomic>
#include <glm/glm.hpp>


namespace swrast {
  enum class FramebufferState : uint8_t {
    Complete = 0,
    MissingColor,
    SizeMismatch,
  };
  const char* to_string(FramebufferState state);

  struct FramebufferSpec {
    Opt<ObjectHandle<Texture>> depth_buffer;
    std::vector<ObjectHandle<Texture>> color_atts;
  };

  class Framebuffer : public UniqueId<Framebuffer> {
  public:
    Framebuffer(glm::uvec2 size, FramebufferSpec spec);
    Framebuffer() = default;

    /// Check for framebuffer state. Use this to verify its state before use.
    inline FramebufferState CheckState() const noexcept { return m_state; }

    /**
     * @brief Use this framebuffer for rendering.
     * @note Calling this is equivalent to calling State::SetActiveFramebuffer()
     */
    Framebuffer& Use();

    /// Refer to Framebuffer::Use() method for more details.
    const Framebuffer& Use() const;

    /**
     * @param Clear the framebuffer contents
     * @param color Color to clear all the color attachments with.
     * @param depth Flag signiffying if the depth should be clear too.
     * @note If you don't want to clear color buffer, then set the `color` argument to {}
     */
    Framebuffer& Clear(Opt<Color> color, bool depth = true);

    /// Create a basic framebuffer with 1 depth buffer and 1 color attachment.
    static Framebuffer CreateBasic(glm::uvec2 size);

    /// Get the dimensions of this framebuffer.
    inline const glm::uvec2& GetSize() const { return m_size; }

    inline Opt<ObjectHandle<Texture>> GetDepthBuffer() const { return m_depthBuffer; }
    Opt<ObjectHandle<Texture>> GetColorAttach(uint32_t index) const;
  private:
    FramebufferState m_state;
    glm::uvec2 m_size;
    Opt<ObjectHandle<Texture>> m_depthBuffer;
    std::vector<ObjectHandle<Texture>> m_colorAtts;
  };

  template<>
  OptRef<Framebuffer> State::GetObject(ObjectId id);
  template<>
  ObjectHandle<Framebuffer> State::CreateObject(Framebuffer&& fb);
} // namespace swrast
