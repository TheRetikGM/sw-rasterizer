/**
 * @brief Implementation of TextureObject
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @file TextureObject.cpp
 */
#include "state/Texture.h"
#include "error.hpp"
#include <cassert>
#include <stdexcept>

using namespace swrast;

int channel_count(TexFormat f) {
  switch (f) {
  case TexFormat::undefined: return 0;
  case TexFormat::r: return 1;
  case TexFormat::rgb: return 3;
  case TexFormat::rgba: return 4;
  }
  throw std::invalid_argument(format("channel_count: Invalid TexFormat (with value '%i').", int(f)));
}

Texture::Texture(
    std::optional<TextureData> tex_data,
    glm::uvec2 tex_size,
    TexFormat data_format,
    const TextureSpec& spec
)
  : m_size(tex_size)
  , m_intFormat(spec.int_format)
  , m_magFilter(spec.mag_filter)
  , m_minFilter(spec.min_filter)
  , m_wrapS(spec.wrap_s)
  , m_wrapT(spec.wrap_t)
{
  assert(!tex_data.has_value() || tex_data.value().size() == (tex_size.x * tex_size.y) * size_t(data_format));
  assert(data_format != TexFormat::undefined);
  assert(tex_size.x != 0 && tex_size.y != 0);

  if (m_intFormat == TexFormat::undefined)
    m_intFormat = data_format;

  // TODO: Convert to internal format.
  if (data_format != m_intFormat)
    RAISEn(NotImplementedException);

  if (!tex_data.has_value()) {
    // Create a new blank texture.
    m_tex = TextureData(tex_size.x * tex_size.y * channel_count(m_intFormat));
    std::fill(m_tex.begin(), m_tex.end(), 0);
  }

}

template<>
OptRef<Texture> State::GetObject(ObjectId id) {
  if (m_textures.count(id) == 0)
    return {};
  return m_textures[id];
}

template<>
ObjectHandle<Texture> State::CreateObject(Texture&& tex) {
  return {
    .obj_ptr = &(State::m_textures.emplace(tex.Id, std::move(tex)).first->second),
    .obj_id = tex.Id,
  };
}
