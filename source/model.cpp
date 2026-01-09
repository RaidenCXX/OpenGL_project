#include "model.h"
#include "assimp/Importer.hpp"
#include "assimp/cexport.h"
#include "assimp/material.h"
#include "assimp/mesh.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/types.h"
#include "enums.h"
#include "glm/ext/vector_float3.hpp"
#include "shader.h"
#include "stb/stb_image.h"
#include <cstddef>
#include <string>
#include <vector>

Mesh::Mesh(std::vector<Vertex> &vertices, std::vector<GLuint> &indices,
           std::vector<Texture> &texture)
    : m_vertices(vertices), m_indices(indices), m_textures(texture) {
  setupMesh();
}

void Mesh::setupMesh() {

  glGenVertexArrays(1, &m_VAO);
  glGenBuffers(1, &m_VBO);
  glGenBuffers(1, &m_EBO);

  glBindVertexArray(m_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

  glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex),
               m_vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(GLuint),
               m_indices.data(), GL_STATIC_DRAW);

  // vertex position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);

  // vertex normals
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, Normal));

  // vertex tex TexCoords
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, TexCoords));

  glBindVertexArray(0);
}

void Mesh::Draw(Shader &shader, bool drawTexture) {
  if (drawTexture) {
    GLuint diffuseNr = 1;
    GLuint specularNr = 1;
    for (GLuint i = 0; i < m_textures.size(); ++i) {
      glActiveTexture(GL_TEXTURE0 + i);

      TextureType type = m_textures[i].type;
      std::string number;
      std::string name;

      if (type == TextureType::Diffuse) {
        number = std::to_string(diffuseNr++);
        name = "material.texture_diffuse" + number;
      } else if (type == TextureType::Specular) {
        number = std::to_string(specularNr++);
        name = "material.texture_specular" + number;
      }

      shader.setInt(name.c_str(), i);
      glBindTexture(GL_TEXTURE_2D, m_textures[i].id);
    }
    shader.setFloat("material.shininess", 64.f);
    shader.setFloat("pointLight.constant", 1.0f);
    shader.setFloat("pointLight.linear", 0.14f);
    shader.setFloat("pointLight.quadratic", 0.07f);
  }
  // draw mesh
  glBindVertexArray(m_VAO);
  glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  // glActiveTexture(GL_TEXTURE0);
}

Model::Model(const char *path, bool flipTexture, bool gamma)
    : m_flipTexture(flipTexture) {
  loadModel(path);
}

void Model::Draw(Shader &shader, bool drawTexture) {
  for (GLuint i = 0; i < m_meshes.size(); i++)
    m_meshes[i].Draw(shader, drawTexture);
}

void Model::loadModel(const std::string &path) {
  Assimp::Importer import;

  const aiScene *scene =
      import.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
    return;
  } else
    std::cout << "ASSIMP READ FILE: " << path << std::endl;

  m_directory = path.substr(0, path.find_last_of('/'));
  processNode(scene->mRootNode, scene);

  std::cout << "Meshes count: " << m_meshes.size() << std::endl;
}

void Model::processNode(aiNode *node, const aiScene *scene) {
  // process all the nodes meshes
  for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    m_meshes.push_back(processMesh(mesh, scene));
  }

  // the same for each of its children
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    processNode(node->mChildren[i], scene);
  }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;

  for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
    Vertex vertex;

    vertex.Position.x = mesh->mVertices[i].x;
    vertex.Position.y = mesh->mVertices[i].y;
    vertex.Position.z = mesh->mVertices[i].z;

    if (mesh->HasNormals()) {
      vertex.Normal.x = mesh->mNormals[i].x;
      vertex.Normal.y = mesh->mNormals[i].y;
      vertex.Normal.z = mesh->mNormals[i].z;
    }

    if (mesh->HasTextureCoords(0)) {
      vertex.TexCoords.x = mesh->mTextureCoords[0][i].x;
      vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;
    } else
      vertex.TexCoords = glm::vec2(0.0f, 0.0f);

    vertices.push_back(vertex);
  }

  for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; ++j)
      indices.push_back(face.mIndices[j]);
  }

  if (mesh->mMaterialIndex >= 0) {
    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

    std::vector<Texture> diffuseMaps =
        loadMaterialTexture(material, aiTextureType_DIFFUSE);
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    std::vector<Texture> specularMaps =
        loadMaterialTexture(material, aiTextureType_SPECULAR);
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
  }

  return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTexture(aiMaterial *mat,
                                                aiTextureType type) {
  std::vector<Texture> textures;
  for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
    aiString str;
    mat->GetTexture(type, i, &str);

    bool skip = false;
    for (unsigned int j = 0; j < m_textures_loaded.size(); j++) {
      if (std::strcmp(m_textures_loaded[j].path.data(), str.C_Str()) == 0) {
        textures.push_back(m_textures_loaded[j]);
        skip = true; // a texture with the same filepath has already been
        break;
      }
    }
    if (!skip) { // if texture hasn't been loaded already, load it
      Texture texture;
      texture.id = TextureFromFile(str.C_Str(), this->m_directory);

      switch (type) {
      case aiTextureType_DIFFUSE:
        texture.type = TextureType::Diffuse;
        break;
      case aiTextureType_SPECULAR:
        texture.type = TextureType::Specular;
        break;
      default:
        break;
      }

      texture.path = str.C_Str();
      textures.push_back(texture);
      m_textures_loaded.push_back(
          texture); // store it as texture loaded for entire model, to ensure we
    }
  }
  return textures;
};

unsigned int Model::TextureFromFile(const char *path,
                                    const std::string &directory, bool gamma) {
  std::string filename = path;
  filename = directory + '/' + filename;

  std::cout << "Texture path: " << filename << std::endl;

  unsigned int textureID;
  glGenTextures(1, &textureID);

  stbi_set_flip_vertically_on_load(m_flipTexture);
  int width, height, nrComponents;
  unsigned char *data =
      stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
  if (data) {
    GLenum format;
    if (nrComponents == 1) {
      format = GL_RED;
      std::cout << "Format: GL_RED" << std::endl;
    } else if (nrComponents == 3) {
      format = GL_RGB;
      std::cout << "Format: GL_RGB" << std::endl;
    } else if (nrComponents == 4) {
      format = GL_RGBA;
      std::cout << "Format: GL_RGBA" << std::endl;
    }

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  } else {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    stbi_image_free(data);
  }

  return textureID;
}
