#pragma once
#include <vector>
#include <string>
#include "Mesh.h"

struct aiNode;
struct aiMesh;
struct aiScene;


namespace render {
  class Shader;

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
    std::string _directory;
  };

}