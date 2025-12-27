#include "system.h"
#include "glfw/glfw3.h"

System::System(const std::string &name, GLuint width, GLuint height)
    : m_Time(glfwGetTime()), m_DeltaTime(0.0f), m_LastFrame(0.0f),
      m_Width(width), m_Height(height) {
  m_Window = glfwCreateWindow(width, height, name.c_str(), NULL, NULL);
}

void System::update() {
  m_Time = glfwGetTime();
  m_DeltaTime = m_Time - m_LastFrame;
  m_LastFrame = m_Time;
}

void System::setCamera(const Camera &camera) { m_Camera = camera; }
