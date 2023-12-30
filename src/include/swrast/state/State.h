/**
 * @file State.h
 * @brief Main rasterizer state.
 * @author Jakub Kloub, xkloub03, VUT FIT
 */
#pragma once

#include <functional>
#include <glm/glm.hpp>
#include <optional>
#include <unordered_map>
#include "error.hpp"
#include "swrast_private.h"


namespace swrast {
  template<class T> struct ObjectHandle;
  class VertexBuffer;
  class IndexBuffer;
  class VertexArray;
  class Texture;
  class Framebuffer;

  /**
   * @brief Main rasterizer state.
   *
   * This class contains everything that represents rasterizer state.
   */
  struct State {
    static std::unordered_map<ObjectId, VertexBuffer> m_vbos;
    static std::unordered_map<ObjectId, IndexBuffer> m_ibos;
    static std::unordered_map<ObjectId, VertexArray> m_vaos;
    static std::unordered_map<ObjectId, Texture> m_textures;
    static std::unordered_map<ObjectId, Framebuffer> m_fbos;
    static ObjectId m_activeFb;
    static ObjectId m_defaultFb;

  public:

    static void Init(glm::uvec2 fb_size);
    static void Destroy();

    /**
     * @brief Use given framebuffer for rendering.
     * @param fb_id Id of the framebuffer to use. If not defined then the default framebuffer is used.
     * @except ObjectNotFoundException on invalid `id`.
     */
    static void SetActiveFramebuffer(Opt<ObjectId> fb_id = {});

    /**
     * @brief Get reference to given object.
     * @param id ObjectId of the given type.
     * @tparam T Object type to get the instance of. The ObjectId needs to belong to the object of this type.
     * @return Optional reference to the object's instance.
     */
    template<class T>
    static OptRef<T> GetObject(ObjectId id) = delete;

    /**
     * @brief Let state manage this object.
     * @param obj Object to take ownership of.
     * @tparam T Type of the object to register.
     * @return New handle to the stored object.
     * @fixme (?) Maybe this should have name RegisterObject instead for clarity.
     */
    template<class T>
    static ObjectHandle<T> CreateObject(T&& obj) = delete;
  };

  template<class T>
  struct ObjectHandle {
    T* obj_ptr = NULL;
    ObjectId obj_id = 0;

    inline T& operator*() { return *obj_ptr; }
    inline T* operator->() { return obj_ptr; }
    inline const T& operator*() const { return *obj_ptr; }
    inline const T* operator->() const { return obj_ptr; }
    inline T& Get() { return *obj_ptr; }
    inline const T& Get() const { return *obj_ptr; }

    /**
     * @brief Create a new handle from object Id
     * @param obj_id Id of the object of type T. This object needs to exist.
     * @except ObjectNotFoundException on invalid object id.
     * @return Handle of referencing given object.
     */
    static ObjectHandle<T> FromId(ObjectId obj_id) {
      auto obj = State::GetObject<T>(obj_id);
      if (!obj.has_value())
        RAISE(ObjectNotFoundException, obj_id);
      return { obj.value(), obj_id };
    }
  };
} // namespace swrast
