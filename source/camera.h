#ifndef CAMERA_H
#define CAMERA_H

#include "glew/glew.h"
#include "glfw/glfw3.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/geometric.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/trigonometric.hpp"
#include <ios>

static const float YAW = -90.f;
static const float PITCH = 0.0f;
static const float SPEED = 2.0f;
static const float SENSITIVITY = 0.1f;
static const float ZOOM = 45.0f;

class Camera {

private:
  // camera Attributes
  glm::vec3 m_Position;
  glm::vec3 m_Front;
  glm::vec3 m_Up;
  glm::mat4 m_View;

  float m_CameraSpeed = SPEED;
  bool m_validViewMat = false;

  // Mouse pos
  float m_LastX;
  float m_LastY;
  bool m_FirstMouse = true;

  float m_Yaw = -90.f;
  float m_Pitch = 0.f;

  void updateView();

public:
  Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
         glm::vec3 target = glm::vec3(0.0f, 0.0f, 1.0f),
         glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f));

  void setPosition(const glm::vec3 &position);
  void setFront(const glm::vec3 &target);
  void setUp(const glm::vec3 &up);
  void setCameraSpeed(const float speed) { m_CameraSpeed = speed; }

  const glm::vec3 &getPosition() const { return m_Position; }
  const glm::vec3 &getFront() const { return m_Front; }
  const glm::vec3 &getUp() const { return m_Up; }
  const glm::mat4 getView();
  const float getCameraSpeed() const { return m_CameraSpeed; }

  static void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    Camera *cam = static_cast<Camera *>(glfwGetWindowUserPointer(window));
    if (cam)
      cam->process_mouse(xpos, ypos);
  }

  void process_mouse(double xpos, double ypos);
};

#endif // !CAMERA_H
