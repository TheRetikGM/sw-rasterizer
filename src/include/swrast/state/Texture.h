/**
 * @brief This file contains declaration of texture object.
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @file state/TextureObject.h
 */
#pragma once
#include "state/State.h"
#include "swrast_private.h"
#include <cstdint>
#include <glm/glm.hpp>
#include <vector>


namespace swrast {
  /// Internal texture types.
  enum class TexFormat : uint8_t {
    undefined,  ///< Unspecified, this means that it will be deduced or error.
    r = 1,    ///< Single channel
    rgb = 3,  ///< 3 color channels
    rgba = 4, ///< 4 color channels
  };

  /// Method to use when scaling.
  enum class ScaleMethod : uint8_t {
    Linear,   ///< Scale using linear interpolation.
    Nearest,  ///< Scale using nearest neighbour.
  };

  /// How to wrap the texures when indexed out-of-bounds.
  enum class WrapMethod : uint8_t {
    Repeat,        ///< Repeat the texure
    RepeatMirror,  ///< Repeat the texture and mirror it each time.
    ClampToEdge,   ///< Extend the texture edges.
  };

  /// Represents how the texture data are stored in TextureObject
  using TextureData = std::vector<uint8_t>;   // For now, we assume that the data is always an array of unsigned bytes.

  /// Specification of TextureObject used in constructor.
  struct TextureSpec {
    TexFormat int_format = TexFormat::undefined;    // Use the same format as the `data_format` argument by default.
    ScaleMethod mag_filter = ScaleMethod::Linear;
    ScaleMethod min_filter = ScaleMethod::Nearest;
    WrapMethod wrap_s = WrapMethod::Repeat;
    WrapMethod wrap_t = WrapMethod::Repeat;
  };

  /// Represents a single texture object.
  class Texture : public UniqueId<Texture> {
  public:
    Texture(
        std::optional<TextureData> tex_data,
        glm::uvec2 tex_size,
        TexFormat data_format,
        const TextureSpec& spec = {}
    );
    Texture() = default;

    inline void SetMagFilter(ScaleMethod m) { m_magFilter = m; }
    inline void SetMinFilter(ScaleMethod m) { m_minFilter = m; }
    inline void SetWrapS(WrapMethod w) { m_wrapS = w; }
    inline void SetWrapT(WrapMethod w) { m_wrapT = w; }

    inline const glm::uvec2& GetSize() const noexcept { return m_size; }

    // TODO: Add some kind of sample method?

  private:
    TextureData m_tex;
    glm::uvec2 m_size;
    TexFormat m_intFormat;
    ScaleMethod m_magFilter;
    ScaleMethod m_minFilter;
    WrapMethod m_wrapS;
    WrapMethod m_wrapT;
  };

  template<>
  OptRef<Texture> State::GetObject(ObjectId id);
  template<>
  ObjectHandle<Texture> State::CreateObject(Texture&& tex);
} // namespace swrast
