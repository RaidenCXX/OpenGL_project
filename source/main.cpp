#include <cmath>
#include <cstdlib>
#include <iostream>

#include "enums.h"
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
#include "utilities.h"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include "stb/stb_image.h"

struct OffscreenFBO {
  GLuint fbo = 0;
  GLuint colorTex = 0;
  GLuint rbo = 0;
  int w = 0;
  int h = 0;
};

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  System *app = static_cast<System *>(glfwGetWindowUserPointer(window));

  if (!app)
    return;

  std::cout << "Framebufer callback: Old size = " << app->m_FbWidth << ", "
            << app->m_FbHight << ". New size = " << width << ", " << height
            << "." << std::endl;

  app->m_FbWidth = width;
  app->m_FbHight = height;
}

void window_size_callback(GLFWwindow *window, int width, int height) {
  System *app = static_cast<System *>(glfwGetWindowUserPointer(window));
  if (!app)
    return;
  app->m_Width = width;
  app->m_Height = height;
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
  System *App = static_cast<System *>(glfwGetWindowUserPointer(window));
  if (!App)
    return;
  App->m_Camera.process_mouse(xpos, ypos);
}

static void destroyFBO(OffscreenFBO &framebufer) {
  if (framebufer.rbo)
    glDeleteRenderbuffers(1, &framebufer.rbo);
  if (framebufer.colorTex)
    glDeleteTextures(1, &framebufer.colorTex);
  if (framebufer.fbo)
    glDeleteFramebuffers(1, &framebufer.fbo);
  framebufer = OffscreenFBO{};
}

static void createFramebuffer(System &App, OffscreenFBO &framebufer) {
  if (App.m_FbWidth <= 0 || App.m_FbHight <= 0)
    return;

  if (framebufer.fbo && framebufer.w == App.m_FbWidth &&
      framebufer.h == App.m_FbHight)
    return;

  destroyFBO(framebufer);

  framebufer.w = App.m_FbWidth;
  framebufer.h = App.m_FbHight;

  glGenFramebuffers(1, &framebufer.fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, framebufer.fbo);

  glGenTextures(1, &framebufer.colorTex);
  glBindTexture(GL_TEXTURE_2D, framebufer.colorTex);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, App.m_FbWidth, App.m_FbHight, 0,
               GL_RGB, GL_UNSIGNED_BYTE, NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         framebufer.colorTex, 0);

  glGenRenderbuffers(1, &framebufer.rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, framebufer.rbo);

  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, App.m_FbWidth,
                        App.m_FbHight);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, framebufer.rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "ERROR::FRAMEBUFFER:: framebuffer is not complete!"
              << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void drawQuad(Shader &shader, OffscreenFBO framebufer) {

  static GLuint VAO = 0, VBO = 0;
  if (VAO == 0) {

    float quadVertices[] = {
        -1.0f, 1.0f,  0.0f, 1.0f, //
        -1.0f, -1.0f, 0.0f, 0.0f, //
        1.0f,  -1.0f, 1.0f, 0.0f, //

        -1.0f, 1.0f,  0.0f, 1.0f, //
        1.0f,  -1.0f, 1.0f, 0.0f, //
        1.0f,  1.0f,  1.0f, 1.0f  //
    };

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, framebufer.w, framebufer.h);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_STENCIL_TEST);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, framebufer.colorTex);
  shader.use();
  shader.setInt("screenTexture", 0);

  glBindVertexArray(VAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
  glEnable(GL_DEPTH_TEST);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_STENCIL_TEST);
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

  if (glfwGetKey(system.m_Window, GLFW_KEY_0) == GLFW_PRESS)
    system.m_effectType = EffectType::NoEffect;
  else if (glfwGetKey(system.m_Window, GLFW_KEY_1) == GLFW_PRESS)
    system.m_effectType = EffectType::Inversion;
  else if (glfwGetKey(system.m_Window, GLFW_KEY_2) == GLFW_PRESS)
    system.m_effectType = EffectType::Grayscale;
  else if (glfwGetKey(system.m_Window, GLFW_KEY_3) == GLFW_PRESS)
    system.m_effectType = EffectType::Sharpen;
  else if (glfwGetKey(system.m_Window, GLFW_KEY_4) == GLFW_PRESS)
    system.m_effectType = EffectType::Blur;
  else if (glfwGetKey(system.m_Window, GLFW_KEY_5) == GLFW_PRESS)
    system.m_effectType = EffectType::Edge;

  glfwSetCursorPosCallback(system.m_Window, mouse_callback);

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

int main() {

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

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

  glfwSetWindowUserPointer(App.m_Window, &App);
  glfwSetFramebufferSizeCallback(App.m_Window, framebuffer_size_callback);
  glfwSetWindowSizeCallback(App.m_Window, window_size_callback);

  Shader VizNormalShader("normal");
  Shader RefractionShader("refraction");
  Shader MirrorShader("mirror");
  Shader CubeMapShader("cubemap");
  Shader EdgeShader("edge");
  Shader BlurShader("blur");
  Shader SharpenShader("sharpen");
  Shader GrayscaleShader("grayscale");
  Shader InversShader("invers");
  Shader ScreenShader("screen");
  Shader ObjectShader("object");
  Shader TestShader("test");
  Shader LightShader{"light"};
  Shader DepthShader{"depth"};
  Shader OutLineShader("outline");
  Shader TranspShader("transparent");
  Shader GlassShader("glass");
  std::vector<glm::vec3> windowPos = {
      glm::vec3(0.1f, 1.0f, 3.0f),  //
      glm::vec3(-0.2f, 1.0f, 5.0f), //
      glm::vec3(0.3f, 1.0f, 7.0f),  //
      glm::vec3(-0.4f, 1.0f, 8.0f), //
      glm::vec3(0.5f, 1.0f, 10.0f)  //
  };

  std::cout << "Ball: start loadet" << std::endl;
  Model modelBall{"assets/ball/ball.obj"};
  std::cout << "Ball: end loadet" << std::endl;
  std::cout << "Stand: start loadet" << std::endl;
  Model modelStand{"assets/stand/stand.obj"};
  std::cout << "Stand: end loadet" << std::endl;
  std::cout << "Leaf: start loadet" << std::endl;
  Model modelLeaf{"assets/leaf/leaf.obj"};
  std::vector<glm::vec3> vegetationPos = {glm::vec3(-3.5f, 0.0f, -1.2f), //
                                          glm::vec3(0.2f, 0.0f, 4.0f),   //
                                          glm::vec3(0.0f, 0.0f, 6.1f),   //
                                          glm::vec3(-0.9f, 0.0f, -6.9)}; //

  Model modelWindow{"assets/window/window.obj"};

  glBindVertexArray(0);

  // Projection matrix
  glm::mat4 projection;

  // Sort distance window position

  // Depth properties
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_STENCIL_TEST);
  glDepthFunc(GL_LESS);

  // Offscreen framebuffer
  OffscreenFBO framebufer;

  // Load cubmap
  GLuint CubemapTex = loadCubemap("LancellottiChapel");
  GLuint CubemapVAO = createCubMapVAO();

  GLuint matrixUbo = genUbo(sizeof(glm::mat4) * 2);
  UboBlocBinding(matrixUbo, sizeof(glm::mat4) * 2, 0);

  ShaderBlockBinding(matrixUbo, ObjectShader, "Matrices", 0);
  ShaderBlockBinding(matrixUbo, OutLineShader, "Matrices", 0);
  ShaderBlockBinding(matrixUbo, TranspShader, "Matrices", 0);
  ShaderBlockBinding(matrixUbo, GlassShader, "Matrices", 0);
  ShaderBlockBinding(matrixUbo, RefractionShader, "Matrices", 0);
  ShaderBlockBinding(matrixUbo, MirrorShader, "Matrices", 0);

  glm::mat4 model;
  while (!glfwWindowShouldClose(App.m_Window)) {

    updateUbo(matrixUbo, 0, projection);
    updateUbo(matrixUbo, sizeof(glm::mat4), App.m_Camera.getView());

    float time = glfwGetTime();

    App.update();
    float aspect = (float)App.m_FbWidth / (float)App.m_FbHight;
    projection = glm::perspective(glm::radians(45.f), aspect, 0.1f, 40.f);

    // input
    processInput(App);

    // Creating a custom framebufer //////////////////////
    createFramebuffer(App, framebufer);
    // Creating a custom framebufer \\\\\\\\\\\\\\\\\\\\\\

    glBindFramebuffer(GL_FRAMEBUFFER, framebufer.fbo);
    glViewport(0, 0, App.m_FbWidth, App.m_FbHight);

    // Clear
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    model = glm::mat4{1.0f};

    if (!App.m_Camera.getDepthModeStatus()) {

      // Ball model {
      glEnable(GL_DEPTH_TEST);
      glDepthMask(GL_TRUE);
      glDepthFunc(GL_LESS);
      glEnable(GL_STENCIL_TEST);
      glStencilMask(0xFF);
      glStencilFunc(GL_ALWAYS, 1, 0xFF);

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
      ObjectShader.setVec3("flashLight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
      ObjectShader.setVec3("flashLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
      ObjectShader.setFloat("flashLight.cutOff", cos(glm::radians(20.f)));
      ObjectShader.setFloat("flashLight.outerCutOff", cos(glm::radians(13.f)));

      // Matrix
      model = glm::mat4(1.0f);
      {
        float angle = time * 7;
        model = glm::rotate(model, glm::radians(angle),
                            glm::vec3{0.0f, 1.0f, 0.0f});
      }
      ObjectShader.setMat4("model", model);
      ObjectShader.setMat3("inverse", glm::mat3(glm::transpose(glm::inverse(
                                          App.m_Camera.getView() * model))));
      modelBall.Draw(ObjectShader);
      if (true) {
        // Normals visualization {
        // Matrix
        VizNormalShader.use();
        model = glm::mat4(1.0f);
        {
          float angle = time * 7;
          model = glm::rotate(model, glm::radians(angle),
                              glm::vec3{0.0f, 1.0f, 0.0f});
        }
        VizNormalShader.setMat4("model", model);
        VizNormalShader.setMat3(
            "inverse", glm::mat3(glm::transpose(
                           glm::inverse(App.m_Camera.getView() * model))));
        modelBall.Draw(VizNormalShader);
        // Normals visualization }
      }
      // Ball model }

      // Ball outLine model {
      glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
      glStencilMask(0x00);

      OutLineShader.use();
      OutLineShader.setMat4("model", model);
      OutLineShader.setMat3("inverse", glm::mat3(glm::transpose(glm::inverse(
                                           App.m_Camera.getView() * model))));
      modelBall.Draw(OutLineShader);

      glEnable(GL_DEPTH_TEST);
      glDepthMask(0xFF);
      glDisable(GL_STENCIL_TEST);
      // Ball outLine model }

      // Stand model {
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
      ObjectShader.setVec3("flashLight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
      ObjectShader.setVec3("flashLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
      ObjectShader.setFloat("flashLight.cutOff", cos(glm::radians(20.f)));
      ObjectShader.setFloat("flashLight.outerCutOff", cos(glm::radians(13.f)));

      // Matrix
      model = glm::mat4(1.0f);
      ObjectShader.setMat4("model", model);
      ObjectShader.setMat3("inverse", glm::mat3(glm::transpose(glm::inverse(
                                          App.m_Camera.getView() * model))));
      modelStand.Draw(ObjectShader);

      // Stand model }

      // Leaf model {
      glEnable(GL_DEPTH_TEST);
      glStencilFunc(GL_ALWAYS, 1, 0x00);

      TranspShader.use();
      for (int i = 0; i < vegetationPos.size(); ++i) {
        // Matrix
        model = glm::mat4{1.0f};
        model = glm::translate(model, vegetationPos[i]);
        {
          float angle = time * 10;
          model = glm::rotate(model, glm::radians(angle),
                              glm::vec3{0.0f, 1.0f, 0.0f});
          model = glm::rotate(model, glm::radians(90.f),
                              glm::vec3{1.0f, 0.0f, 0.0f});
        }
        TranspShader.setMat4("model", model);

        modelLeaf.Draw(TranspShader);
      }
      model = glm::mat4{1.0f};
      // Leaf model }

      // Ball mirror model {
      MirrorShader.use();

      glBindTexture(GL_TEXTURE_CUBE_MAP, CubemapTex);

      // Matrix
      model = glm::mat4(1.0f);
      {
        model = glm::translate(model, glm::vec3{2.0f, 0.0f, 0.0f});
        float angle = time * 7;
        model = glm::rotate(model, glm::radians(angle),
                            glm::vec3{0.0f, 1.0f, 0.0f});
      }
      MirrorShader.setMat4("model", model);
      MirrorShader.setMat3("inverse", glm::mat3(glm::transpose(glm::inverse(
                                          App.m_Camera.getView() * model))));
      modelBall.Draw(MirrorShader, false);

      // Ball mirror model }

      // Ball diamond model {
      RefractionShader.use();

      glBindTexture(GL_TEXTURE_CUBE_MAP, CubemapTex);

      // Matrix
      model = glm::mat4(1.0f);
      {
        model = glm::translate(model, glm::vec3{-2.0f, 0.0f, 0.0f});
        float angle = time * 7;
        model = glm::rotate(model, glm::radians(angle),
                            glm::vec3{0.0f, 1.0f, 0.0f});
      }
      RefractionShader.setFloat("ROI", 1.309f);
      RefractionShader.setMat4("model", model);
      RefractionShader.setMat3(
          "inverse", glm::mat3(glm::transpose(
                         glm::inverse(App.m_Camera.getView() * model))));
      modelBall.Draw(RefractionShader, false);

      // Ball diamond model }

      // Cubemap {
      glDepthFunc(GL_LEQUAL);
      CubeMapShader.use();
      CubeMapShader.setMat4("view",
                            glm::mat4(glm::mat3(App.m_Camera.getView())));
      CubeMapShader.setMat4("projection", projection);
      glBindVertexArray(CubemapVAO);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_CUBE_MAP, CubemapTex);
      glDrawArrays(GL_TRIANGLES, 0, 36);
      // Cubemap }

      // Window model {
      // Sort transparent window
      std::map<float, glm::vec3> sorted;
      for (unsigned int i = 0; i < windowPos.size(); ++i) {
        float distance = glm::length(App.m_Camera.getPosition() - windowPos[i]);
        sorted[distance] = windowPos[i];
      }

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glDepthMask(GL_FALSE);

      GlassShader.use();
      for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin();
           it != sorted.rend(); ++it) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, it->second);
        model = glm::rotate(model, glm::radians(90.f), glm::vec3(0, 1, 0));

        GlassShader.setMat4("model", model);

        modelWindow.Draw(GlassShader);
      }
      glDepthMask(GL_TRUE);
      glDisable(GL_BLEND);
      // Window model }

      glDepthMask(GL_TRUE);
      glStencilMask(0xFF);
      glStencilFunc(GL_ALWAYS, 1, 0xFF);
      glDepthFunc(GL_LESS);
    } else {
      DepthShader.use();
      DepthShader.setFloat("near", 0.1f);
      DepthShader.setFloat("far", 10.f);

      // Matrix
      DepthShader.setMat4("model", model);
      DepthShader.setMat4("projection", projection);
      DepthShader.setMat4("view", App.m_Camera.getView());
      modelBall.Draw(DepthShader, false);
    }

    if (!App.m_Camera.getDepthModeStatus()) {
      switch (App.m_effectType) {
      case EffectType::NoEffect:
        drawQuad(ScreenShader, framebufer);
        break;
      case EffectType::Inversion:
        drawQuad(InversShader, framebufer);
        break;
      case EffectType::Grayscale:
        drawQuad(GrayscaleShader, framebufer);
        break;
      case EffectType::Sharpen:
        drawQuad(SharpenShader, framebufer);
        break;
      case EffectType::Blur:
        drawQuad(BlurShader, framebufer);
        break;
      case EffectType::Edge:
        drawQuad(EdgeShader, framebufer);
        break;
      }
    } else {
      drawQuad(ScreenShader, framebufer);
    }

    // check and call events and swap the buffers
    glfwSwapBuffers(App.m_Window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
