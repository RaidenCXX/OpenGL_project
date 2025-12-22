#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>

#include "enums.h"
#include "shader.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "glew/glew.h"
#include "glm/ext/vector_float3.hpp"

struct Vertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
};

struct Texture {
  GLuint id;
  TextureType type;
  std::string path;
};

class Mesh {
public:
  // mesh data
  std::vector<Vertex> m_vertices;
  std::vector<GLuint> m_indices;
  std::vector<Texture> m_textures;

  Mesh(std::vector<Vertex> &vertices, std::vector<GLuint> &indices,
       std::vector<Texture> &texture);
  void Draw(Shader &shader);

private:
  GLuint m_VAO, m_VBO, m_EBO;
  void setupMesh();
};

class Model {
public:
  Model(char *path);
  void Draw(Shader &shader);

private:
  // model data
  std::vector<Texture> m_textures_loaded;
  std::vector<Mesh> m_meshes;
  std::string m_directory;

  void loadModel(const std::string &path);
  void processNode(aiNode *node, const aiScene *scene);
  Mesh processMesh(aiMesh *mesh, const aiScene *scene);
  std::vector<Texture> loadMaterialTexture(aiMaterial *mat, aiTextureType type,
                                           const std::string &typeName);
  unsigned int TextureFromFile(const char *path, const std::string &directory,
                               bool gamma = true);
};
#endif // MODEL_H
