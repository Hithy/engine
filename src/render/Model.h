#pragma once
#include <vector>
#include "Mesh.h"

class Shader;
struct aiNode;
struct aiMesh;
struct aiScene;

class Model
{
public:
  Model(const char* path);

  void Draw(Shader* shader);

private:
  void LoadModel(const char* path);
  void ProcessNode(aiNode* node, const aiScene* scene);
  Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

private:
  std::vector<Mesh> _meshes;


};

