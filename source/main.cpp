#include <charconv>
#include <cmath>
#include <cstdlib>
#include <iostream>

#include "glew/glew.h"
#include "glfw/glfw3.h"
#include "glm/common.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/quaternion_geometric.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "glm/geometric.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/mat4x4.hpp"
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

//clang-format off
float verticesBox[] = {
    // Position       Normals           UV
    // FRONT
    -0.5, -0.5, -0.5, 0.0f, 0.0f, -1.0f, 0.f, 0.f, //
    0.5, -0.5, -0.5, 0.0f, 0.0f, -1.0f, 1.f, 0.f,  //
    0.5, 0.5, -0.5, 0.0f, 0.0f, -1.0f, 1.f, 1.f,   //
    -0.5, 0.5, -0.5, 0.0f, 0.0f, -1.0f, 0.f, 1.f,  //

    // BACK
    -0.5, -0.5, 0.5, 0.0f, 0.0f, 1.0f, 0.f, 0.f, //
    0.5, -0.5, 0.5, 0.0f, 0.0f, 1.0f, 1.f, 0.f,  //
    0.5, 0.5, 0.5, 0.0f, 0.0f, 1.0f, 1.f, 1.f,   //
    -0.5, 0.5, 0.5, 0.0f, 0.0f, 1.0f, 0.f, 1.f,  //

    // LEFT
    -0.5, -0.5, -0.5, -1.0f, 0.0f, 0.0f, 0.f, 0.f, //
    -0.5, -0.5, 0.5, -1.0f, 0.0f, 0.0f, 1.f, 0.f,  //
    -0.5, 0.5, 0.5, -1.0f, 0.0f, 0.0f, 1.f, 1.f,   //
    -0.5, 0.5, -0.5, -1.0f, 0.0f, 0.0f, 0.f, 1.f,  //

    // RIGHT
    0.5, -0.5, -0.5, 1.0f, 0.0f, 0.0f, 0.f, 0.f, //
    0.5, -0.5, 0.5, 1.0f, 0.0f, 0.0f, 1.f, 0.f,  //
    0.5, 0.5, 0.5, 1.0f, 0.0f, 0.0f, 1.f, 1.f,   //
    0.5, 0.5, -0.5, 1.0f, 0.0f, 0.0f, 0.f, 1.f,  //

    // TOP
    -0.5, 0.5, 0.5, 0.0f, 1.0f, 0.0f, 0.f, 0.f,  //
    0.5, 0.5, 0.5, 0.0f, 1.0f, 0.0f, 1.f, 0.f,   //
    0.5, 0.5, -0.5, 0.0f, 1.0f, 0.0f, 1.f, 1.f,  //
    -0.5, 0.5, -0.5, 0.0f, 1.0f, 0.0f, 0.f, 1.f, //

    // BOTTOM
    -0.5, -0.5, 0.5, 0.0f, -1.0f, 0.0f, 0.f, 0.f, //
    0.5, -0.5, 0.5, 0.0f, -1.0f, 0.0f, 1.f, 0.f,  //
    0.5, -0.5, -0.5, 0.0f, -1.0f, 0.0f, 1.f, 1.f, //
    -0.5, -0.5, -0.5, 0.0f, -1.0f, 0.0f, 0.f, 1.f //
};

GLuint elementBufferBox[] = {
    // FRONT FACE
    0, 1, 2, 2, 3, 0,

    // BACK FACE
    4, 5, 6, 6, 7, 4,

    // LEFT FACE
    8, 9, 10, 10, 11, 8,

    // RIGHT FACE
    12, 13, 14, 14, 15, 12,

    // TOP FACE
    16, 17, 18, 18, 19, 16,

    // BOTTOM FACE
    20, 21, 22, 22, 23, 20

};

// BoxPosition
glm::vec3 boxPosition[6] = {glm::vec3{0.0f, 0.0f, -1.0f},      //
                            glm::vec3{0.0f, 0.0f, -3.0f},      //
                            glm::vec3{0.0f, 0.0f, -6.0f},      //
                            glm::vec3{3.0f, 0.0f, -1.0f},      //
                            glm::vec3{3.0f, 0.0f, -3.0f},      //
                            glm::vec3{3.0f, 0.0f, -6.0f}};     //
glm::vec3 PointlightPosition[6]{glm::vec3{0.5f, 2.0f, -1.0f},  //
                                glm::vec3{-0.5f, 2.0f, -3.0f}, //
                                glm::vec3{0.2f, 2.0f, -6.0f},  //
                                glm::vec3{-1.0f, 2.0f, -1.0f}, //
                                glm::vec3{2.0f, 2.0f, -3.0f},  //
                                glm::vec3{4.0f, 2.0f, -6.0f}}; //
//clang-format on

int main() {

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  System App{"LearnOpenGL", 800, 800};
  App.setCamera(Camera(glm::vec3(0.0f, 2.0f, -11.0f),
                       glm::vec3(0.0f, 0.0f, 0.0f),
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

  // Load textures
  {
    stbi_set_flip_vertically_on_load(true);

    int width, height, nrChannels;
    width = 500;
    height = 500;
    nrChannels = 4;

    unsigned char *dataContainerDiff = stbi_load(
        "assets/container_diffuse.png", &width, &height, &nrChannels, 0);

    GLuint textureContainerDiff;
    glGenTextures(1, &textureContainerDiff);

    if (dataContainerDiff) {
      glBindTexture(GL_TEXTURE_2D, textureContainerDiff);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                   GL_UNSIGNED_BYTE, dataContainerDiff);

      setTexFilteringWrapping();
      glGenerateMipmap(GL_TEXTURE_2D);
      stbi_image_free(dataContainerDiff);
    } else {
      std::cout << "Failed to load texture" << " brick" << std::endl;
    }

    width = 500;
    height = 500;
    nrChannels = 4;

    unsigned char *dataContainerSpec = stbi_load(
        "assets/container_specular.png", &width, &height, &nrChannels, 0);

    GLuint textureContainerSpec;
    glGenTextures(1, &textureContainerSpec);

    if (dataContainerSpec) {
      glBindTexture(GL_TEXTURE_2D, textureContainerSpec);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                   GL_UNSIGNED_BYTE, dataContainerSpec);

      setTexFilteringWrapping();
      glGenerateMipmap(GL_TEXTURE_2D);
      stbi_image_free(dataContainerSpec);
    } else {
      std::cout << "Failed to load texture" << " smile" << std::endl;
    }

    // Bind texture to texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureContainerDiff);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureContainerSpec);
  }

  Shader ObjectShader("object");
  Shader LightShader{"light"};

  Vertex vertex;
  vertex.Position = glm::vec3(0.2f, 0.4f, 0.6f);
  vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
  vertex.TexCoords = glm::vec2(1.0f, 0.0f);

  // Box*********************************************************
  GLuint VAObox;
  glGenVertexArrays(1, &VAObox);

  GLuint VBObox;
  glGenBuffers(1, &VBObox);

  GLuint EBObox;
  glGenBuffers(1, &EBObox);

  //  Bind vao as VAO quad
  glBindVertexArray(VAObox);
  // Bind VBO quad
  glBindBuffer(GL_ARRAY_BUFFER, VBObox);
  // Bind element buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBObox);

  // Push element info to gpu
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elementBufferBox),
               elementBufferBox, GL_STATIC_DRAW);

  // Push quad vertex data to gpu memory
  glBufferData(GL_ARRAY_BUFFER, sizeof(verticesBox), verticesBox,
               GL_STATIC_DRAW);
  // Seting spacification for 0 atribut
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  // Box*********************************************************

  // Light*******************************************************
  GLuint VAOlight;
  glGenVertexArrays(1, &VAOlight);

  GLuint VBOlight;
  glGenBuffers(1, &VBOlight);

  GLuint EBOlight;
  glGenBuffers(1, &EBOlight);

  glBindVertexArray(VAOlight);
  glBindBuffer(GL_ARRAY_BUFFER, VBOlight);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOlight);

  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elementBufferBox),
               elementBufferBox, GL_STATIC_DRAW);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verticesBox), verticesBox,
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  // Light*******************************************************

  glBindVertexArray(0);

  // Projection matrix
  glm::mat4 projection;
  projection = glm::perspective(glm::radians(45.f), 1.0f, 0.1f, 100.f);

  glEnable(GL_DEPTH_TEST);
  while (!glfwWindowShouldClose(App.m_Window)) {
    App.update();

    // uniformvvvvvvvvvvvvvvvvv
    float time = (sin(glfwGetTime() * 0.1f) + 1.0f) / 2.0f;
    // uniform^^^^^^^^^^^^^^^^^

    // Clear
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // input
    processInput(App);

    ObjectShader.use();
    glBindVertexArray(VAObox);
    glm::mat4 model;
    glm::vec3 offset(cos(glfwGetTime() * 1.0f), sin(glfwGetTime() * 0.5f),
                     sin(glfwGetTime() * 0.5f));
    for (int i = 0; i < 6; ++i) {
      model = glm::mat4{1.0f};
      model = glm::translate(model, boxPosition[i]);
      model = glm::rotate(model,
                          glm::radians(static_cast<float>(glfwGetTime()) * 6),
                          glm::vec3(0.f, 1.0f, 0.0f));
      // material properties
      ObjectShader.setInt("material.diffuse", 0);
      ObjectShader.setInt("material.specular", 1);
      ObjectShader.setFloat("material.shininess", 64.f);
      // Direction light properties
      ObjectShader.setVec3("dirLight.direction",
                           glm::vec3(App.m_Camera.getView() *
                                     glm::vec4(0.0f, -1.0f, 0.0f, 0.0f)));
      ObjectShader.setVec3("dirLight.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
      ObjectShader.setVec3("dirLight.diffuse", glm::vec3(0.3f, 0.3f, 0.3f));
      ObjectShader.setVec3("dirLight.specular", glm::vec3(0.3f, 0.3f, 0.3f));

      // Pointlight properties

      std::string pointLightName{"pointLight[0]."};
      for (int i = 0; i < 6; ++i) {
        std::string position, constant, linear, quadratic, ambient, diffuse,
            specular;
        pointLightName[11] = '0' + i;
        position = pointLightName + "position";
        ObjectShader.setVec3(
            position.c_str(),
            glm::vec3(App.m_Camera.getView() *
                      glm::vec4((PointlightPosition[i] + offset), 1.0f)));
        constant = pointLightName + "constant";
        ObjectShader.setFloat(constant.c_str(), 1.0f);
        linear = pointLightName + "linear";
        ObjectShader.setFloat(linear.c_str(), 0.14f);
        quadratic = pointLightName + "quadratic";
        ObjectShader.setFloat(quadratic.c_str(), 0.07f);

        ambient = pointLightName + "ambient";
        ObjectShader.setVec3(ambient.c_str(), glm::vec3(0.0f, 0.0f, 0.0f));
        diffuse = pointLightName + "diffuse";
        ObjectShader.setVec3(diffuse.c_str(), glm::vec3(1.f, 1.0f, 1.0f));
        specular = pointLightName + "specular";
        ObjectShader.setVec3(specular.c_str(), glm::vec3(1.0f, 1.0f, 1.0f));
      }

      // Flash light properties
      ObjectShader.setVec3("flashLight.position",
                           glm::vec3(App.m_Camera.getView() *
                                     glm::vec4(1.5f, 3.0f, -2.5f, 1.0f)));
      glm::vec3 rot =
          glm::vec3(cos(glfwGetTime() * 1.f), -1.0f, sin(glfwGetTime() * 1.f));

      rot *= glm::vec3(0.5f);
      rot.y = -1.0f;

      ObjectShader.setVec3(
          "flashLight.direction",
          glm::vec3(App.m_Camera.getView() * glm::vec4(rot, 0.0f)));
      ObjectShader.setVec3("flashLight.diffuse", glm::vec3(0.8f, 0.8f, 0.0f));
      ObjectShader.setVec3("flashLight.specular", glm::vec3(0.8f, 0.8f, 0.0f));
      ObjectShader.setFloat("flashLight.cutOff", cos(glm::radians(20.f)));
      ObjectShader.setFloat("flashLight.outerCutOff", cos(glm::radians(17.f)));

      // Matrix
      ObjectShader.setMat4("model", model);
      ObjectShader.setMat4("projection", projection);
      ObjectShader.setMat4("view", App.m_Camera.getView());
      ObjectShader.setMat3("inverse", glm::mat3(glm::transpose(glm::inverse(
                                          App.m_Camera.getView() * model))));

      glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(VAOlight);
    LightShader.use();
    for (int i = 0; i < 6; ++i) {
      model = glm::mat4{1.0f};
      model = glm::translate(model, (PointlightPosition[i] + offset));
      model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
      LightShader.setMat4("model", model);
      LightShader.setMat4("projection", projection);
      LightShader.setMat4("view", App.m_Camera.getView());
      glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }

    // check and call events and swap the buffers
    glfwSwapBuffers(App.m_Window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
