
#ifndef UTILITIES_H
#define UTILITIES_H

#include "glew/glew.h"
#include "glm/gtc/type_ptr.hpp"
#include "shader.h"
#include <string>

GLuint loadCubemap(const std::string &cubmapName, bool flip = false);
GLuint createCubMapVAO();

void ShaderBlockBinding(GLuint UBO, const Shader &shader,
                        const std::string &blockName, GLuint bindingPoint);
GLuint genUbo(GLuint dataSizeBytes);
void UboBlocBinding(GLuint UBO, GLuint dataSizeBytes, GLuint bindingPoint);

template <typename T>
void updateUbo(GLuint UBO, GLuint offsetBytes, const T &data) {
  glBindBuffer(GL_UNIFORM_BUFFER, UBO);
  glBufferSubData(GL_UNIFORM_BUFFER, offsetBytes, sizeof(T),
                  glm::value_ptr(data));
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
#endif // !UTILITIES_H
