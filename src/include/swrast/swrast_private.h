/**
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @file swrast_private.h
 */
#pragma once

#include <optional>
#include <glm/glm.hpp>

namespace swrast {
  /// Optional reference.
  template<class T>
  using OptRef = std::optional<std::reference_wrapper<T>>;

  template<class T>
  using Opt = std::optional<T>;

  using ObjectId = unsigned int;

  /**
   * @brief Class used for giving unique ids to objects of given type
   *
   * This class helps uniquelly identify childs from some type.
   * @tparam T Type to identify
   */
  template<class T>
  class UniqueId {
    inline static ObjectId id = 0;
  public:
    ObjectId Id;
    UniqueId() { Id = id++; }

    // Operator overloads used for STL containers.
    bool operator==(const UniqueId<T>& rhs) const { return Id == rhs.Id; }
    bool operator<(const UniqueId<T>& rhs) const { return Id < rhs.Id; }
    bool operator>(const UniqueId<T>& rhs) const { return Id > rhs.Id; }
  };

  using Color = glm::vec4;
  struct Colors {
    inline static Color Red = { 1.0f, 0.0f, 0.0f, 1.0f };
    inline static Color Green = { 0.0f, 1.0f, 0.0f, 1.0f };
    inline static Color Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
    inline static Color Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
    inline static Color Magenta = { 1.0f, 0.0f, 1.0f, 1.0f };
    inline static Color Cyan = { 0.0f, 1.0f, 1.0f, 1.0f };
    inline static Color White = { 1.0f, 1.0f, 1.0f, 1.0f };
    inline static Color Gray = { 0.1f, 0.1f, 0.1f, 1.0f };
  };
};
