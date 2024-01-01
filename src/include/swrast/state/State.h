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
#include <any>
#include "swrast_private.h"
#include "memory.hpp"


namespace swrast {
  template<class T> struct ObjectHandle;
  class VertexBuffer;
  class IndexBuffer;
  class VertexArray;
  class Texture;
  class Framebuffer;
  class Shader;
  class Program;
  enum class Primitive : uint8_t;

  enum class CullFace {
    None, CW, CCW
  };

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
    static std::unordered_map<ObjectId, Ref<Shader>> m_shaders;
    static std::unordered_map<ObjectId, Program> m_programs;
    static ObjectId m_activeFb;
    static ObjectId m_defaultFb;
    static Opt<ObjectId> m_activeProgram;
    static Opt<ObjectId> m_activeVao;
    static CullFace m_cullFace;
    inline static bool m_depthTest = false;

    friend class RenderState;

  public:

    /**
     * @brief Initialize the state.
     * @param fb_size Dimensions of the default framebuffer.
     */
    static void Init(glm::uvec2 fb_size);

    /// Destroy the state and all its allocated resources.
    static void Destroy();

    /// Enable/disable depth testing.
    static void SetDepthTest(bool b) { m_depthTest = b; }

    static ObjectHandle<Framebuffer> GetActiveFramebuffer();

    /**
     * @brief Clear the active framebuffer with given color.
     * @param color Color to clear the fb with.
     * @param depth If the depth buffer should be cleared too.
     */
    static void Clear(Opt<Color> color, bool depth = true);
    /**
     * @brief Draw unindexed
     * @param primitive Primitives to draw
     * @param offset Offset into the VBO's
     * @param count Number of vertices to be rendered.
     */
    static void DrawArrays(Primitive primitive, size_t offset, size_t count);
    /**
     * @brief Draw indexed
     * @param primitive PRimitives to draw
     * @param count Number of indices to use.
     */
    static void DrawIndexed(Primitive primitive, size_t count);

    /**
     * @brief Set how the faces should be culled.
     * @param cull Culling method to use.
     */
    inline static void SetCullFace(CullFace cull) { m_cullFace = cull; }

    /**
     * @brief Set the cufrent active program.
     * @param prg_id Id of the program to set.
     */
    static void SetActiveProgram(ObjectId prg_id);

    /**
     * @brief Use given framebuffer for rendering.
     * @param fb_id Id of the framebuffer to use. If not defined then the default framebuffer is used.
     * @except ObjectNotFoundException on invalid `id`.
     */
    static void SetActiveFramebuffer(Opt<ObjectId> fb_id = {});

    /**
     * @brief Use given vertex array.
     * @param vao_id Id of the vao to use. If none, then current VAO is unbound.
     */
    static void SetActiveVertexArray(Opt<ObjectId> vao_id);

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
    static ObjectHandle<T> FromId(ObjectId obj_id);
  };
} // namespace swrast
