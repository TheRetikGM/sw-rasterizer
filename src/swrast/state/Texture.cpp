/**
 * @brief Implementation of TextureObject
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @file TextureObject.cpp
 */
#include "state/Texture.h"
#include "error.hpp"
#include <cassert>
#include <cstring>
#include <stdexcept>

using namespace swrast;

int swrast::channel_count(TexFormat f) {
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
  : m_IntFormat(spec.int_format)
  , m_MagFilter(spec.mag_filter)
  , m_MinFilter(spec.min_filter)
  , m_WrapS(spec.wrap_s)
  , m_WrapT(spec.wrap_t)
  , m_size(tex_size)
{
  assert(!tex_data.has_value() || tex_data.value().size() == (tex_size.x * tex_size.y) * size_t(data_format));
  assert(data_format != TexFormat::undefined);
  assert(tex_size.x != 0 && tex_size.y != 0);

  if (m_IntFormat == TexFormat::undefined)
    m_IntFormat = data_format;

  // TODO: Convert to internal format.
  if (data_format != m_IntFormat)
    RAISEn(NotImplementedException);

  if (!tex_data.has_value()) {
    // Create a new blank texture.
    m_tex = TextureData(tex_size.x * tex_size.y * channel_count(m_IntFormat));
    std::fill(m_tex.begin(), m_tex.end(), 0);
  }
}

void Texture::Fill(glm::vec4 c) {
  auto pixel_count = m_size.x * m_size.y;
  auto channels = channel_count(m_IntFormat);
  assert(channels == 1 || channels == 3 || channels == 4);
  auto color = glm::vec<4, uint8_t>(c.r * 255, c.g * 255, c.b * 255, c.a * 255);

  if (channels == 1) {
    std::fill(m_tex.begin(), m_tex.end(), color.r);
    return;
  }

  for (uint32_t i = 0; i < pixel_count * channels; i += channels) {
    m_tex[i] = color.r;
    m_tex[i + 1] = color.g;
    m_tex[i + 2] = color.b;
    if (channels == 4)
      m_tex[i + 3] = color.a;
  }
}

uint8_t* Texture::GetPixel(glm::uvec2 pos) {
  if (pos.x >= m_size.x || pos.y >= m_size.y)
    return nullptr;
  size_t index = (pos.y * m_size.x + pos.x) * channel_count(m_IntFormat);
  return &m_tex[index];
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
