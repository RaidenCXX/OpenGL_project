#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>

#include "camera.h"
#include "glfw/glfw3.h"

class System {

public:
  GLFWwindow *m_Window;
  GLuint m_Width, m_Height;
  std::string m_Name;

  Camera m_Camera;

  float m_Time;
  float m_DeltaTime;
  float m_LastFrame;

  void update();

  System(const std::string &name, GLuint width, GLuint height);

  void setCamera(const Camera &camera);
};

#endif // !SYSTEM_H
