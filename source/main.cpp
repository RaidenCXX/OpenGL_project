#include <cmath>
#include <cstdlib>
#include <iostream>

#include "glew/glew.h"
#include "glfw/glfw3.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/geometric.hpp"
#include "glm/matrix.hpp"
#include "glm/trigonometric.hpp"

#include "camera.h"
#include "model.h"
#include "shader.h"
#include "system.h"

#include <cstddef>
#include <string>

#include "stb/stb_image.h"

void framebuffer_size_callback(GLFWwindow *window, GLint width, GLint height) {
  glViewport(0, 0, width, height);
}

void processInput(System &system) {

  if (glfwGetKey(system.m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(system.m_Window, true);
  else if (glfwGetKey(system.m_Window, GLFW_KEY_C) == GLFW_PRESS)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else if (glfwGetKey(system.m_Window, GLFW_KEY_F) == GLFW_PRESS)
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // Camera move ********************
  Camera &camera = system.m_Camera;
  if (glfwGetKey(system.m_Window, GLFW_KEY_W) == GLFW_PRESS)
    camera.setPosition(camera.getPosition() +
                       camera.getFront() *
                           (camera.getCameraSpeed() * system.m_DeltaTime));
  if (glfwGetKey(system.m_Window, GLFW_KEY_S) == GLFW_PRESS)
    camera.setPosition(camera.getPosition() -
                       camera.getFront() *
                           (camera.getCameraSpeed() * system.m_DeltaTime));
  if (glfwGetKey(system.m_Window, GLFW_KEY_A) == GLFW_PRESS)
    camera.setPosition(
        camera.getPosition() -
        glm::normalize(glm::cross(camera.getFront(), camera.getUp())) *
            (camera.getCameraSpeed() * system.m_DeltaTime));
  if (glfwGetKey(system.m_Window, GLFW_KEY_D) == GLFW_PRESS)
    camera.setPosition(
        camera.getPosition() +
        glm::normalize(glm::cross(camera.getFront(), camera.getUp())) *
            (camera.getCameraSpeed() * system.m_DeltaTime));

  if (glfwGetKey(system.m_Window, GLFW_KEY_Z) == GLFW_PRESS)
    camera.enableDepthMode();
  if (glfwGetKey(system.m_Window, GLFW_KEY_F) == GLFW_PRESS)
    camera.disableDepthMode();

  glfwSetWindowUserPointer(system.m_Window, &camera);
  glfwSetCursorPosCallback(system.m_Window, Camera::mouse_callback);

  // Camera move *******************
}

void setTexFilteringWrapping() {
  // Texture wraping
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

  // Texture filtering
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Mip map filtering
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

glm::vec3 PointlightPosition{glm::vec3{0.5f, 2.0f, -1.0f}}; //
//clang-format on

int main() {

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  System App{"LearnOpenGL", 800, 800};
  App.setCamera(Camera(glm::vec3(0.0f, 0.0f, -4.0f),
                       glm::vec3(0.0f, 0.0f, 1.0f),
                       glm::vec3(0.0f, 1.0f, 0.0f)));

  glfwSetInputMode(App.m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  if (App.m_Window == NULL) {
    std::cout << "Failed tp create GLFW window" << std::endl;
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(App.m_Window);

  GLenum err = glewInit();
  if (err != GLEW_OK) {
    std::cerr << "Glew init fails" << glewGetErrorString(err) << std::endl;
    return 1;
  }

  glfwSetFramebufferSizeCallback(App.m_Window, framebuffer_size_callback);

  Shader ObjectShader("object");
  Shader TestShader("test");
  Shader LightShader{"light"};
  Shader DepthShader{"depth"};
  Shader OutLine("outline");
  Model modelBackpack{"assets/backpack/backpack.obj"};
  std::cout << "Texture count loaded: " << modelBackpack.getTextures().size()
            << std::endl;

  glBindVertexArray(0);

  // Projection matrix
  glm::mat4 projection;
  float aspect = (float)App.m_Width / (float)App.m_Height;
  projection = glm::perspective(glm::radians(45.f), aspect, 0.1f, 40.f);

  // Depth properties
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_STENCIL_TEST);
  glDepthFunc(GL_LESS);

  while (!glfwWindowShouldClose(App.m_Window)) {
    App.update();

    // input
    processInput(App);

    // Clear
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glm::mat4 model;

    model = glm::mat4{1.0f};
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));

    // Matrix
    DepthShader.setMat4("model", model);
    DepthShader.setMat4("projection", projection);
    DepthShader.setMat4("view", App.m_Camera.getView());

    if (!App.m_Camera.getDepthModeStatus()) {

      glStencilFunc(GL_ALWAYS, 1, 0xFF);
      glStencilMask(0xFF);

      ObjectShader.use();
      // Direction light properties
      ObjectShader.setVec3("dirLight.direction",
                           glm::vec3(App.m_Camera.getView() *
                                     glm::vec4(0.0f, -1.0f, 0.0f, 0.0f)));
      ObjectShader.setVec3("dirLight.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
      ObjectShader.setVec3("dirLight.diffuse", glm::vec3(0.3f, 0.3f, 0.3f));
      ObjectShader.setVec3("dirLight.specular", glm::vec3(0.3f, 0.3f, 0.3f));

      // Pointlight properties
      ObjectShader.setVec3(
          "pointLight.position",
          glm::vec3(App.m_Camera.getView() *
                    glm::vec4(glm::vec3(0.0f, 2.0f, 0.0f), 1.0f)));

      ObjectShader.setVec3("pointLight.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
      ObjectShader.setVec3("pointLight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
      ObjectShader.setVec3("pointLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

      // Flash light properties
      ObjectShader.setVec3(
          "flashLight.position",
          glm::vec3(App.m_Camera.getView() *
                    glm::vec4(App.m_Camera.getPosition(), 1.0f)));
      ObjectShader.setVec3("flashLight.direction",
                           glm::vec3(App.m_Camera.getView() *
                                     glm::vec4(App.m_Camera.getFront(), 0.0f)));
      ObjectShader.setVec3("flashLight.diffuse", glm::vec3(0.3f, 0.7f, 0.3f));
      ObjectShader.setVec3("flashLight.specular", glm::vec3(0.3f, 0.7f, 0.3f));
      ObjectShader.setFloat("flashLight.cutOff", cos(glm::radians(20.f)));
      ObjectShader.setFloat("flashLight.outerCutOff", cos(glm::radians(13.f)));

      // Matrix
      ObjectShader.setMat4("model", model);
      ObjectShader.setMat4("projection", projection);
      ObjectShader.setMat4("view", App.m_Camera.getView());
      ObjectShader.setMat3("inverse", glm::mat3(glm::transpose(glm::inverse(
                                          App.m_Camera.getView() * model))));
      modelBackpack.Draw(ObjectShader);

      glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
      glStencilMask(0x00);
      glDisable(GL_DEPTH_TEST);
      OutLine.use();

      OutLine.setMat4("model", model);
      OutLine.setMat4("projection", projection);
      OutLine.setMat4("view", App.m_Camera.getView());
      OutLine.setMat3("inverse", glm::mat3(glm::transpose(glm::inverse(
                                     App.m_Camera.getView() * model))));
      modelBackpack.Draw(OutLine);
      glStencilMask(0xFF);
      glStencilFunc(GL_ALWAYS, 1, 0xFF);
      glEnable(GL_DEPTH_TEST);
    } else {
      DepthShader.use();
      DepthShader.setFloat("near", 0.1f);
      DepthShader.setFloat("far", 10.f);

      // Matrix
      DepthShader.setMat4("model", model);
      DepthShader.setMat4("projection", projection);
      DepthShader.setMat4("view", App.m_Camera.getView());
      modelBackpack.Draw(DepthShader, false);
    }

    // check and call events and swap the buffers
    glfwSwapBuffers(App.m_Window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
