#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Vertex
{
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
  glm::vec3 Tangent;
  glm::vec3 Bitangent;
};

enum TextureType {
  TextureType_Unknown = 0,
  TextureType_Diffuse,
  TextureType_Specular,
  TextureType_Normal,
  TextureType_Height,

  TextureType_MAX,
};

struct Texture {
  unsigned int id;
  TextureType type;
};

class Shader;

class Mesh
{
public:
  Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<Texture>& textures);
  void Draw(Shader* shader) const;

private:
  void SetupMesh();

private:
  std::vector<Vertex> _vertices;
  std::vector<unsigned int> _indices;
  std::vector<Texture> _textures;

  unsigned int _vao;
  unsigned int _vbo;
  unsigned int _ebo;
};

