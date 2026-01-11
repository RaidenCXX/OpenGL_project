#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "shader.h"
#include "stb/stb_image.h"
#include "utilities.h"

GLuint loadCubemap(const std::string &cubmapName) {

  std::vector<std::string> faces = {
      std::string{"/posx.jpg"}, //
      std::string{"/negx.jpg"}, //
      std::string{"/posy.jpg"}, //
      std::string{"/negy.jpg"}, //
      std::string{"/posz.jpg"}, //
      std::string{"/negz.jpg"}  //
  };

  GLuint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  GLint width, height, nrChannels;
  for (unsigned int i = 0; i < faces.size(); ++i) {
    unsigned char *data =
        stbi_load(("assets/cubemaps/" + cubmapName + faces[i]).c_str(), &width,
                  &height, &nrChannels, 0);
    if (data) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height,
                   0, GL_RGB, GL_UNSIGNED_BYTE, data);
      stbi_image_free(data);
    } else {
      std::cout << "Cubemap failed to load at path: " << faces[i] << std::endl;
      stbi_image_free(data);
    }
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  return textureID;
}

GLuint createCubMapVAO() {

  float boxVertices[] = {
      // positions
      -1.0f, 1.0f,  -1.0f, //
      -1.0f, -1.0f, -1.0f, //
      1.0f,  -1.0f, -1.0f, //
      1.0f,  -1.0f, -1.0f, //
      1.0f,  1.0f,  -1.0f, //
      -1.0f, 1.0f,  -1.0f, //

      -1.0f, -1.0f, 1.0f,  //
      -1.0f, -1.0f, -1.0f, //
      -1.0f, 1.0f,  -1.0f, //
      -1.0f, 1.0f,  -1.0f, //
      -1.0f, 1.0f,  1.0f,  //
      -1.0f, -1.0f, 1.0f,  //

      1.0f,  -1.0f, -1.0f, //
      1.0f,  -1.0f, 1.0f,  //
      1.0f,  1.0f,  1.0f,  //
      1.0f,  1.0f,  1.0f,  //
      1.0f,  1.0f,  -1.0f, //
      1.0f,  -1.0f, -1.0f, //

      -1.0f, -1.0f, 1.0f, //
      -1.0f, 1.0f,  1.0f, //
      1.0f,  1.0f,  1.0f, //
      1.0f,  1.0f,  1.0f, //
      1.0f,  -1.0f, 1.0f, //
      -1.0f, -1.0f, 1.0f, //

      -1.0f, 1.0f,  -1.0f, //
      1.0f,  1.0f,  -1.0f, //
      1.0f,  1.0f,  1.0f,  //
      1.0f,  1.0f,  1.0f,  //
      -1.0f, 1.0f,  1.0f,  //
      -1.0f, 1.0f,  -1.0f, //

      -1.0f, -1.0f, -1.0f, //
      -1.0f, -1.0f, 1.0f,  //
      1.0f,  -1.0f, -1.0f, //
      1.0f,  -1.0f, -1.0f, //
      -1.0f, -1.0f, 1.0f,  //
      1.0f,  -1.0f, 1.0f   //
  };

  GLuint VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  GLuint VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glBufferData(GL_ARRAY_BUFFER, sizeof(boxVertices), boxVertices,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
  return VAO;
}

void ShaderBlockBinding(GLuint UBO, const Shader &shader,
                        const std::string &blockName, GLuint bindingPoint) {
  GLuint blocIndex = glGetUniformBlockIndex(shader.ID, blockName.c_str());
  if (blocIndex == GL_INVALID_INDEX) {
    std::cout << "SHADER BLOCK BINDING: INVALID INDEX: " << blockName
              << std::endl;
    return;
  }
  glUniformBlockBinding(shader.ID, blocIndex, bindingPoint);
};

GLuint genUbo(GLuint dataSizeBytes) {
  GLuint UBO;
  glGenBuffers(1, &UBO);
  glBindBuffer(GL_UNIFORM_BUFFER, UBO);
  glBufferData(GL_UNIFORM_BUFFER, dataSizeBytes, NULL, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  return UBO;
}

void UboBlocBinding(GLuint UBO, GLuint dataSizeBytes, GLuint bindingPoint) {

  glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, UBO, 0, dataSizeBytes);
}
