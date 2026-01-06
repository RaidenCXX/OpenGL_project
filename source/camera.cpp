#include "camera.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/geometric.hpp"

Camera::Camera(glm::vec3 position, glm::vec3 target, glm::vec3 up)
    : m_Position(position), m_Front(target - position), m_Up(up), m_Yaw(-90.f),
      m_Pitch(0.f) {
  updateView();
}

void Camera::updateView() {
  m_View = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
  m_validViewMat = true;
}

// Set vvvvv
void Camera::setPosition(const glm::vec3 &position) {
  m_Position = position;
  m_validViewMat = false;
}

void Camera::setFront(const glm::vec3 &frontDir) {
  m_Front = glm::normalize(frontDir);
  m_validViewMat = false;
}

void Camera::setUp(const glm::vec3 &up) {
  m_Up = up;
  m_validViewMat = false;
}
// Set *****
// Get vvvvv
const glm::mat4 Camera::getView() {

  if (!m_validViewMat)
    updateView();

  return m_View;
}
// Get *****

void Camera::process_mouse(double xpos, double ypos) {

  if (m_FirstMouse) {
    m_LastX = xpos;
    m_LastY = ypos;
    m_FirstMouse = false;
  }

  float deltaX = xpos - m_LastX;
  float deltaY = m_LastY - ypos;
  m_LastX = xpos;
  m_LastY = ypos;

  deltaX *= SENSITIVITY;
  deltaY *= SENSITIVITY;

  m_Yaw += deltaX;
  m_Pitch += deltaY;

  if (m_Pitch > 89.f)
    m_Pitch = 89.f;
  if (m_Pitch < -89.f)
    m_Pitch = -89.f;

  glm::vec3 direction;
  direction.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
  direction.y = sin(glm::radians(m_Pitch));
  direction.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
  m_Front = glm::normalize(direction);
  m_validViewMat = false;
}
