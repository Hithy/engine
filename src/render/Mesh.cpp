#include <string>
#include <unordered_map>

#include "Mesh.h"
#include "Shader.h"
#include "glad/glad.h"

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<Texture>& textures)
{
  this->_vertices = vertices;
  this->_indices = indices;
  this->_textures = textures;

  SetupMesh();
}

void Mesh::Draw(Shader* shader) const
{
  static std::unordered_map<TextureType, std::string> texture_name_map = {
    {TextureType_Diffuse, "texture_diffuse"},
    {TextureType_Specular, "texture_specular"},
    {TextureType_Normal, "texture_normal"},
    // {TextureType_Height, "texture_height"},
  };
  int texture_count_map[TextureType_MAX] = { 0 };

  for (auto const& item : texture_name_map) {
    shader->SetInt((item.second + "_enable").c_str(), 0);
  }

  for (int i = 0; i < _textures.size(); i++) {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, _textures[i].id);
    auto texture_type = _textures[i].type;
    auto texture_count = ++texture_count_map[texture_type];

    std::string texture_param = texture_name_map[texture_type] + std::to_string(texture_count);
    shader->SetInt(texture_param.c_str(), i);
    shader->SetInt((texture_name_map[texture_type] + "_enable").c_str(), 1);
  }

  glBindVertexArray(_vao);
  glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  for (auto const& item : texture_name_map) {
    shader->SetInt((item.second + "_enable").c_str(), 0);
  }
}

void Mesh::SetupMesh()
{
  glGenVertexArrays(1, &_vao);
  glBindVertexArray(_vao);

  glGenBuffers(1, &_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(Vertex), &_vertices[0], GL_STATIC_DRAW);

  glGenBuffers(1, &_ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(unsigned int), &_indices[0], GL_STATIC_DRAW);

  // pos
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
  // nor
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));
  // uv
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, TexCoords));
  // tangent
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
  // bitangent
  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
