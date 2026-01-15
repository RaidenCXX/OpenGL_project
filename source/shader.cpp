#include "shader.h"
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

// #define RELEASE

Shader::Shader(const char *vertexShaderPath, const char *fragmentShaderPath) {
  std::string vertexCode;
  std::string fragmentCode;
  std::ifstream vShaderFile;
  std::ifstream fShaderFile;

  vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  try {

    // openFiles
    vShaderFile.open(vertexShaderPath);
    fShaderFile.open(fragmentShaderPath);
    std::stringstream vShaderStream, fShaderStream;

    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();

    vShaderFile.close();
    fShaderFile.close();

    vertexCode = vShaderStream.str();
    fragmentCode = fShaderStream.str();

  } catch (std::ifstream::failure e) {
    std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
  }
  const char *vShaderCode = vertexCode.c_str();
  const char *fShaderCode = fragmentCode.c_str();

  unsigned int vertex, fragment;
  int success;
  char infoLog[512];

  // vertex Shader
  vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex, 1, &vShaderCode, NULL);
  glCompileShader(vertex);

  glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog
              << std::endl;
  }

  fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment, 1, &fShaderCode, NULL);
  glCompileShader(fragment);

  glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: " << infoLog
              << std::endl;
  }

  ID = glCreateProgram();
  glAttachShader(ID, vertex);
  glAttachShader(ID, fragment);

  glLinkProgram(ID);

  glGetProgramiv(ID, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(ID, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER_PROGRAM::LINK_FAILED: " << infoLog << std::endl;
  }

  glDeleteShader(vertex);
  glDeleteShader(fragment);
}

Shader::Shader(const char *shaderName) {
  std::string vertexShaderPath{"shader/"};
  std::string fragmentShaderPath{"shader/"};
  std::string geometryShaderPath{"shader/"};

  vertexShaderPath += shaderName;
  fragmentShaderPath += shaderName;
  geometryShaderPath += shaderName;

  vertexShaderPath += "/vertex.vs";
  fragmentShaderPath += "/fragment.fs";
  geometryShaderPath += "/geometry.gs";

  std::string vertexCode;
  std::string fragmentCode;
  std::string geometryCode;
  std::ifstream vShaderFile;
  std::ifstream fShaderFile;
  std::ifstream gShaderFile;

  bool enableGeo = std::filesystem::exists(geometryShaderPath.c_str());

  vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  if (enableGeo)
    gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  try {
    // openFiles
    vShaderFile.open(vertexShaderPath);
    fShaderFile.open(fragmentShaderPath);
    if (enableGeo)
      gShaderFile.open(geometryShaderPath);
    std::stringstream vShaderStream, fShaderStream, gShaderStream;

    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();
    if (enableGeo)
      gShaderStream << gShaderFile.rdbuf();

    vShaderFile.close();
    fShaderFile.close();
    if (enableGeo)
      gShaderFile.close();

    vertexCode = vShaderStream.str();
    fragmentCode = fShaderStream.str();
    if (enableGeo)
      geometryCode = gShaderStream.str();

  } catch (std::ifstream::failure e) {
    std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    enableGeo = false;
  }
  const char *vShaderCode = vertexCode.c_str();
  const char *fShaderCode = fragmentCode.c_str();
  const char *gShaderCode = nullptr;
  if (enableGeo)
    gShaderCode = geometryCode.c_str();

  unsigned int vertex, fragment, geometry;
  int success;
  char infoLog[512];

  // vertex shader
  vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex, 1, &vShaderCode, NULL);
  glCompileShader(vertex);

  glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog
              << std::endl;
  }
  // fragment shader
  fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment, 1, &fShaderCode, NULL);
  glCompileShader(fragment);

  glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: " << infoLog
              << std::endl;
  }

  // geomety shader
  if (enableGeo) {
    geometry = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometry, 1, &gShaderCode, NULL);
    glCompileShader(geometry);

    glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(geometry, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED: " << infoLog
                << std::endl;
    }
  }

  ID = glCreateProgram();
  glAttachShader(ID, vertex);
  glAttachShader(ID, fragment);
  if (enableGeo)
    glAttachShader(ID, geometry);
  glLinkProgram(ID);

  glGetProgramiv(ID, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(ID, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER_PROGRAM::LINK_FAILED: " << infoLog << std::endl;
  }

  glDeleteShader(vertex);
  glDeleteShader(fragment);
  if (enableGeo)
    glDeleteShader(geometry);
}

void Shader::use() const { glUseProgram(ID); }

void Shader::setFloat(const std::string &name, float value) const {
  GLint location = glGetUniformLocation(ID, name.c_str());
  if (location == -1) {
#ifdef RELEASE
    std::cout << "Uniform float: " << name << " cant find" << std::endl;
#endif // RELISE
    return;
  }
  glUniform1f(location, value);
}

void Shader::setInt(const std::string &name, int value) const {
  GLint location = glGetUniformLocation(ID, name.c_str());
  if (location == -1) {
#ifdef RELEASE
    std::cout << "Uniform int: " << name << " cant find" << std::endl;
#endif // RELISE
    return;
  }
  glUniform1i(location, value);
}

void Shader::setMat4(const std::string &name, glm::mat4 value) const {
  GLint location = glGetUniformLocation(ID, name.c_str());
  if (location == -1) {
#ifdef RELEASE
    std::cout << "Uniform mat4: " << name << " cant find." << std::endl;
#endif // RELISE
    return;
  }
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setMat3(const std::string &name, glm::mat3 value) const {
  GLint location = glGetUniformLocation(ID, name.c_str());
  if (location == -1) {
#ifdef RELEASE
    std::cout << "Uniform mat3: " << name << " cant find." << std::endl;
#endif // RELISE
    return;
  }
  glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setVec3(const std::string &name, glm::vec3 value) const {
  GLint location = glGetUniformLocation(ID, name.c_str());
  if (location == -1) {
#ifdef RELEASE
    std::cout << "Uniform vec3: " << name << " cant find." << std::endl;
#endif // RELISE
    return;
  }
  glUniform3f(location, value.x, value.y, value.z);
}
