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

namespace render {

  Model::Model(const char* path) : _directory(path)
  {
    int idx = _directory.size() - 1;
    while (idx > 0 && path[idx] != '/' && path[idx] != '\\') {
      idx--;
    }
    _directory = _directory.substr(0, idx + 1);
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

    return Mesh(vertices, indices, textures);
  }
}