#ifndef SHADER_H
#define SHADER_H

#include "glew/glew.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/mat4x4.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

class Shader {

public:
  // program ID
  unsigned int ID;

  Shader(const char *vertexShaderPath, const char *fragmentShaderPath);
  Shader(const char *shaderName);
  // use activate the program
  void use() const;

  void setBool(const std::string &name, bool value) const;
  void setFloat(const std::string &name, float value) const;
  void setInt(const std::string &name, int value) const;
  void setMat4(const std::string &name, glm::mat4 value) const;
  void setMat3(const std::string &name, glm::mat3 value) const;
  void setVec3(const std::string &name, glm::vec3 value) const;
};

#endif // SHADER_H
