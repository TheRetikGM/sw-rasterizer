/**
 * #brief Camera implementation
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @file camera.cpp
 */
#include "camera.h"
#include <glm/ext/matrix_transform.hpp>
#include <imgui.h>

Camera::Camera() {
  m_ViewMatrix = glm::lookAt(m_pos, m_pos + m_front, UP);
}

void Camera::handleInput(float dt) {
  auto& io = ImGui::GetIO();
  glm::vec2 mouse_delta(io.MouseDelta.x, io.MouseDelta.y);
  if (mouse_delta != glm::vec2(0.0f)) {
    m_hRot += mouse_delta.x * m_rotSpeed;
    m_vRot += mouse_delta.y * m_rotSpeed;
    m_vRot = glm::clamp(m_vRot, PI / 16.0f, PI - PI / 16.0f);
    m_hRotSin = glm::sin(m_hRot);
    m_hRotCos = glm::cos(m_hRot);
    m_vRotSin = glm::sin(m_vRot);
    m_vRotCos = glm::cos(m_vRot);
    m_front.x = m_hRotCos * m_vRotSin;
    m_front.z = m_hRotSin * m_vRotSin;
    m_front.y = m_vRotCos;
  }

  // Compute right vector
  glm::vec3 right = glm::cross(m_front, UP);
  glm::vec3 move_dir(0.0f);

  if (ImGui::IsKeyDown(ImGuiKey_W))
    move_dir += glm::vec3(m_front.x, 0.0f, m_front.z);
  if (ImGui::IsKeyDown(ImGuiKey_A))
    move_dir += -right;
  if (ImGui::IsKeyDown(ImGuiKey_S))
    move_dir += -glm::vec3(m_front.x, 0.0f, m_front.z);
  if (ImGui::IsKeyDown(ImGuiKey_D))
    move_dir += right;
  if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
    move_dir += -UP;
  if (ImGui::IsKeyDown(ImGuiKey_Space))
    move_dir += UP;

  if (move_dir != glm::vec3(0.0f)) {
    move_dir = glm::normalize(move_dir);
    m_pos += move_dir * m_moveSpeed * dt;
  }
}

void Camera::Update(float dt) {
  handleInput(dt);
  m_ViewMatrix = glm::lookAt(m_pos, m_pos + m_front, UP);
}
