/**
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @file swrast_private.h
 */
#pragma once

#include <optional>
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
  public:
    inline static ObjectId Id = 0;
    UniqueId() { Id++; }

    // Operator overloads used for STL containers.
    bool operator==(const UniqueId<T>& rhs) const { return Id == rhs.Id; }
    bool operator<(const UniqueId<T>& rhs) const { return Id < rhs.Id; }
    bool operator>(const UniqueId<T>& rhs) const { return Id > rhs.Id; }
  };
};
