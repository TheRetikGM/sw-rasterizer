/**
 * #brief Camera declaration
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @file camera.hpp
 */
#pragma once
#include <glm/ext/scalar_constants.hpp>
#include <glm/glm.hpp>

class Camera {
  const float PI = glm::pi<float>();
  const glm::vec3 UP = { 0, 1, 0 };
  glm::vec3 m_front = { 0, 0, -1 };
  glm::vec3 m_pos = { 0, 0, 5 };
  float m_moveSpeed = 2.0f;
  float m_rotSpeed = 0.005f;
  float m_hRot = 1.5f * PI;
  float m_vRot = 0.5f * PI;

  float m_hRotSin = 0.0f;
  float m_hRotCos = 0.0f;
  float m_vRotSin = 0.0f;
  float m_vRotCos = 0.0f;
public:
  glm::mat4 m_ViewMatrix{ 0.0f };

  Camera();

  void Update(float dt);
private:
  void handleInput(float dt);
};

