/**
 * @file state/Program.h
 * @brief This file contains the declaration of programs
 * @author Jakub Kloub, xklobu03, VUT FIT
 */
#pragma once
#include "error.hpp"
#include "state/State.h"
#include "swrast_private.h"
#include "utils.h"
#include <functional>
#include <any>

namespace swrast {
  using Uniform = std::any;
  using UniformGroup = std::unordered_map<StrId, Uniform>;

  enum class ShaderType { Vertex, Fragment };

  struct InOutVar {
    union {
      int32_t i1;
      glm::ivec2 i2;
      glm::ivec3 i3;
      glm::ivec4 i4;
      float f1;
      glm::vec2 f2;
      glm::vec3 f3;
      glm::vec4 f4;
    };
    /// Flag that tells is if this is a integer type. We use this during interpolation (integer types aren't interpolated).
    bool integer;
  };

  /**
   * @brief Represents any shader and their shared properties.
   */
  class Shader : public UniqueId<Shader> {
  public:
    using InOutVars = std::unordered_map<StrId, InOutVar>;

    /// Pointer to all program uniforms
    /// @note This property is assigned to when the swrast::Program is constructed with this shader.
    UniformGroup* uniforms = nullptr;

    /// Get the type of shader (vertex/fragment/...)
    inline ShaderType GetType() const noexcept { return m_type; }
    /// Execute this shader.
    virtual void Execute() = 0;
    /**
     * @brief Get the uniform value
     * @param name Name of the uniform.
     * @tparam T Type of the uniform
     * @return Optinal reference to the uniform variable.
     */
    template<class T>
    OptRef<T> Uniform(StrId name) {
      if (uniforms->count(name) == 0)
        return {};
      return std::any_cast<T&>(uniforms->operator[](name));
    }

    template<class T> inline T& In(StrId name) { return getInOut<T>(&m_in, name); }
    template<class T> inline T& Out(StrId name) { return getInOut<T>(&m_out, name); }

    inline InOutVars& InVars() { return m_in; }
    inline InOutVars& OutVars() { return m_out; }

  private:
    ShaderType m_type;
    /// Shader input variables
    InOutVars m_in;
    /// Shader output variables
    InOutVars m_out;

    template<class T> T& getInOut(InOutVars* vars, StrId name);
  protected:
    /**
     * @brief Construct the shader
     * @param func Function to execute for this shader.
     * @param type Type of shader
     */
    Shader(ShaderType type) : m_type(type), m_in(), m_out() {}
    virtual ~Shader() = default;
  };

  /// Represents the vertex shader
  class VertexShader : public Shader {
  public:
    /// Output position of the vertex.
    glm::vec4 m_Position;
    /// Input vertex index.
    uint32_t m_VertexId;
    /// Input vertex attributes as defined in vertex array.
    std::vector<std::any> m_Attributes;

    VertexShader(std::function<void(VertexShader*)> func)
      : Shader(ShaderType::Vertex)
      , m_Position(), m_VertexId(0), m_Attributes(), m_func(func) {};

    /**
     * @brief Get the attribute value
     * @param location Index of the vertex attribute. This is the same index as when vertex array was constructed.
     * @tparam Type of the vertex attribute.
     * @return Optional reference to the vertex attribute.
     */
    template<class T>
    OptRef<T> Attribute(uint8_t location) {
      if (m_Attributes.size() <= location)
        return {};
      return std::any_cast<T&>(m_Attributes[location]);
    }

    void Execute() override {
      m_func(this);
    }
  private:
    std::function<void(VertexShader*)> m_func;
  };

  /// This class represents the fragment shader.
  class FragmentShader : public Shader {
  public:
    /// Input fragment coordinates.
    glm::vec4 m_FragCoord;
    /// Input front facing flag
    bool m_FrontFacing;
    /// Input point coordinates
    glm::vec2 m_PointCoord;
    /// Output color.
    glm::vec4 m_FragColor;

    FragmentShader(std::function<void(FragmentShader*)> func)
      : Shader(ShaderType::Fragment)
      , m_FragCoord(), m_FrontFacing(false), m_PointCoord(), m_func(func) {}

    /// Discards the current fragment.
    void Discard() { RAISEn(NotImplementedException); }

    void Execute() override {
      m_func(this);
    }
  protected:
    std::function<void(FragmentShader*)> m_func;
  };

  /// This struct represents parameters passed to Program.
  /// @note We use this, because there could be new optionall shadere entries in the future.
  struct ProgramSpec {
    ObjectHandle<VertexShader> vertex_shader;
    ObjectHandle<FragmentShader> fragment_shader;
  };

  class Program : public UniqueId<Program> {
    UniformGroup m_uniforms;
    ObjectHandle<VertexShader> m_vertexShader;
    ObjectHandle<FragmentShader> m_fragmentShader;
    template<typename T>
    friend ObjectHandle<T> State::CreateObject(T&& obj);
  public:
    Program(const ProgramSpec&& spec) : m_uniforms(), m_vertexShader(spec.vertex_shader) , m_fragmentShader(spec.fragment_shader) {
    }
    Program() = default;

    Program& Use();

    inline void SetUniform(StrId name, const std::any& value) {
      m_uniforms[name] = value;
    }

    auto& GetVertexShader() { return m_vertexShader; }
    auto& GetFragmentShader() { return m_fragmentShader; }
  };

  template<> OptRef<VertexShader> State::GetObject(ObjectId id);
  template<> OptRef<FragmentShader> State::GetObject(ObjectId id);
  template<> OptRef<Program> State::GetObject(ObjectId id);
  template<> ObjectHandle<VertexShader> State::CreateObject(VertexShader&& obj);
  template<> ObjectHandle<FragmentShader> State::CreateObject(FragmentShader&& obj);
  template<> ObjectHandle<Program> State::CreateObject(Program&& obj);

  template<> int32_t& Shader::getInOut(Shader::InOutVars* vars, StrId name);
  template<> glm::ivec2& Shader::getInOut(Shader::InOutVars* vars, StrId name);
  template<> glm::ivec3& Shader::getInOut(Shader::InOutVars* vars, StrId name);
  template<> glm::ivec4& Shader::getInOut(Shader::InOutVars* vars, StrId name);
  template<> float& Shader::getInOut(Shader::InOutVars* vars, StrId name);
  template<> glm::vec2& Shader::getInOut(Shader::InOutVars* vars, StrId name);
  template<> glm::vec3& Shader::getInOut(Shader::InOutVars* vars, StrId name);
  template<> glm::vec4& Shader::getInOut(Shader::InOutVars* vars, StrId name);
} // namespace swrast
