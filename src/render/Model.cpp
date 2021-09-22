#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>

#include "Model.h"
#include "Shader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Model::Model(const char* path)
{
  LoadModel(path);
}

void Model::Draw(Shader* shader)
{
  for (const auto& mesh : _meshes) {
    mesh.Draw(shader);
  }
}

void Model::LoadModel(const char* path)
{
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
  {
    std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
    return;
  }

  ProcessNode(scene->mRootNode, scene);
}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    auto ai_mesh = scene->mMeshes[node->mMeshes[i]];
    _meshes.push_back(ProcessMesh(ai_mesh, scene));
  }

  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    ProcessNode(node->mChildren[i], scene);
  }
}

static unsigned int GenTexture(const char* path) {
  unsigned int texture = 0;
  int width, height, nrChannels;

  unsigned char* data =
    stbi_load(path, &width, &height, &nrChannels, 0);
  if (data) {
    GLenum format;
    if (nrChannels == 1)
      format = GL_RED;
    else if (nrChannels == 3)
      format = GL_RGB;
    else if (nrChannels == 4)
      format = GL_RGBA;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  }

  return texture;
}

static std::vector<Texture> 
loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureType texture_type)
{
  static std::unordered_map<std::string, Texture> texture_cache;
  std::vector<Texture> textures;

  for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
    aiString path;
    mat->GetTexture(type, i, &path);

    std::string path_str = path.C_Str();
    auto cache_itr = texture_cache.find(path_str);
    if (cache_itr != texture_cache.end()) {
      textures.push_back(cache_itr->second);
      continue;
    }

    Texture texture;
    texture.id = GenTexture(path.C_Str());
    texture.type = texture_type;
    textures.push_back(texture);

    texture_cache[path_str] = texture;
  }

  return textures;
}

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;

  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    Vertex new_vec;
    new_vec.Position.x = mesh->mVertices[i].x;
    new_vec.Position.y = mesh->mVertices[i].y;
    new_vec.Position.z = mesh->mVertices[i].z;

    if (mesh->HasNormals()) {
      new_vec.Normal.x = mesh->mNormals[i].x;
      new_vec.Normal.y = mesh->mNormals[i].y;
      new_vec.Normal.z = mesh->mNormals[i].z;
    }

    if (mesh->mTextureCoords[0]) {
      new_vec.TexCoords.x = mesh->mTextureCoords[0][i].x;
      new_vec.TexCoords.y = mesh->mTextureCoords[0][i].y;

      new_vec.Tangent.x = mesh->mTangents[i].x;
      new_vec.Tangent.y = mesh->mTangents[i].y;
      new_vec.Tangent.z = mesh->mTangents[i].z;

      new_vec.Bitangent.x = mesh->mBitangents[i].x;
      new_vec.Bitangent.y = mesh->mBitangents[i].y;
      new_vec.Bitangent.z = mesh->mBitangents[i].z;
    }
    else {
      new_vec.TexCoords = glm::vec2(0.0f, 0.0f);
    }

    vertices.push_back(new_vec);
  }

  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++) {
      indices.push_back(mesh->mFaces[i].mIndices[j]);
    }
  }

  auto material = scene->mMaterials[mesh->mMaterialIndex];

  // 1. diffuse maps
  std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, TextureType_Diffuse);
  textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
  // 2. specular maps
  std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, TextureType_Specular);
  textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
  // 3. normal maps
  std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, TextureType_Normal);
  textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

  return Mesh(vertices, indices, textures);
}
