/**
 * @brief This file contiains the implementation of state/Program.h
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @file Program.cpp
 */
#include "state/Program.h"
#include <cstring>

using namespace swrast;

Program& Program::Use() {
  State::SetActiveProgram(Id);
  return *this;
}

template<> OptRef<VertexShader> State::GetObject(ObjectId id) {
  if (m_shaders.count(id) == 0)
    return {};
  Shader* shader = m_shaders[id].get();
  if (shader->GetType() != ShaderType::Vertex)
    return {};
  return *dynamic_cast<VertexShader*>(shader);
}
template<> OptRef<FragmentShader> State::GetObject(ObjectId id) {
  if (m_shaders.count(id) == 0)
    return {};
  Shader* shader = m_shaders[id].get();
  if (shader->GetType() != ShaderType::Fragment)
    return {};
  return *dynamic_cast<FragmentShader*>(shader);
}
template<> OptRef<Program> State::GetObject(ObjectId id) {
  if (m_programs.count(id) == 0)
    return {};
  return m_programs[id];
}
template<> ObjectHandle<VertexShader> State::CreateObject(VertexShader&& obj) {
  auto ref = Ref<Shader>(new VertexShader(obj));
  m_shaders.emplace(ref->Id, ref);
  return {
    .obj_ptr = dynamic_cast<VertexShader*>(ref.get()),
    .obj_id = obj.Id,
  };
}
template<> ObjectHandle<FragmentShader> State::CreateObject(FragmentShader&& obj) {
  auto ref = Ref<Shader>(new FragmentShader(obj));
  m_shaders.emplace(ref->Id, ref);
  return {
    .obj_ptr = dynamic_cast<FragmentShader*>(ref.get()),
    .obj_id = obj.Id,
  };
}
template<> ObjectHandle<Program> State::CreateObject(Program&& obj) {
  Program* ptr = &(State::m_programs.emplace(obj.Id, std::move(obj)).first->second);
  ptr->m_vertexShader->uniforms = &ptr->m_uniforms;
  ptr->m_fragmentShader->uniforms = &ptr->m_uniforms;
  return {
    .obj_ptr = ptr,
    .obj_id = obj.Id,
  };
}

template<> int32_t& Shader::getInOut(Shader::InOutVars* vars, StrId name) {
  auto& v = (*vars)[name];
  v.integer = true;
  return v.i1;
}
template<> glm::ivec2& Shader::getInOut(Shader::InOutVars* vars, StrId name) {
  auto& v = (*vars)[name];
  v.integer = true;
  return v.i2;
}
template<> glm::ivec3& Shader::getInOut(Shader::InOutVars* vars, StrId name) {
  auto& v = (*vars)[name];
  v.integer = true;
  return v.i3;
}
template<> glm::ivec4& Shader::getInOut(Shader::InOutVars* vars, StrId name) {
  auto& v = (*vars)[name];
  v.integer = true;
  return v.i4;
}
template<> float& Shader::getInOut(Shader::InOutVars* vars, StrId name) {
  auto& v = (*vars)[name];
  v.integer = false;
  return v.f1;
}
template<> glm::vec2& Shader::getInOut(Shader::InOutVars* vars, StrId name) {
  auto& v = (*vars)[name];
  v.integer = false;
  return v.f2;
}
template<> glm::vec3& Shader::getInOut(Shader::InOutVars* vars, StrId name) {
  auto& v = (*vars)[name];
  v.integer = false;
  return v.f3;
}
template<> glm::vec4& Shader::getInOut(Shader::InOutVars* vars, StrId name) {
  auto& v = (*vars)[name];
  v.integer = false;
  return v.f4;
}

