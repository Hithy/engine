#include "utils.h"

#include <vector>
#include <iostream>
#include <unordered_map>

#include "glad/glad.h"
#include "glm/glm.hpp"

#include "stb_image.h"

namespace render {

  static unsigned int quadVAO = 0;
  static unsigned int quadVBO = 0;

  void renderQuad()
  {
    if (quadVAO == 0)
    {
      float quadVertices[] = {
        // positions        // texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
      };
      // setup plane VAO
      glGenVertexArrays(1, &quadVAO);
      glGenBuffers(1, &quadVBO);
      glBindVertexArray(quadVAO);
      glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
      glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
  }

  static unsigned int boxVAO = 0;
  static unsigned int boxVBO = 0;

  // pos, normal, tex
  void renderBox() {
    if (!boxVAO) {
      float cube_vertices[] = {
        // back face
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
        1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
        1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
        1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
        -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
        // front face
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
        1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
        1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
        1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
        -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
        // left face
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
        -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
        // right face
        1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
        1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
        1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
        1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
        1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
        1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
        // bottom face
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
        1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
        1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
        1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
        -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
        // top face
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
        1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
        1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
        1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
        -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
      };

      glGenBuffers(1, &boxVBO);
      glGenVertexArrays(1, &boxVAO);

      glBindBuffer(GL_ARRAY_BUFFER, boxVBO);
      glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

      glBindVertexArray(boxVAO);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
      glEnableVertexAttribArray(0);
      glEnableVertexAttribArray(1);
      glEnableVertexAttribArray(2);
      glBindVertexArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    glBindVertexArray(boxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
  }

  static unsigned int sphereVAO = 0;
  static unsigned int indexCount;
  void renderSphere()
  {
    if (sphereVAO == 0)
    {
      glGenVertexArrays(1, &sphereVAO);

      unsigned int vbo, ebo;
      glGenBuffers(1, &vbo);
      glGenBuffers(1, &ebo);

      std::vector<glm::vec3> positions;
      std::vector<glm::vec2> uv;
      std::vector<glm::vec3> normals;
      std::vector<unsigned int> indices;

      const unsigned int X_SEGMENTS = 64;
      const unsigned int Y_SEGMENTS = 64;
      const float PI = 3.14159265359;
      for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
      {
        for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
        {
          float xSegment = (float)x / (float)X_SEGMENTS;
          float ySegment = (float)y / (float)Y_SEGMENTS;
          float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
          float yPos = std::cos(ySegment * PI);
          float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

          positions.push_back(glm::vec3(xPos, yPos, zPos));
          uv.push_back(glm::vec2(xSegment, ySegment));
          normals.push_back(glm::vec3(xPos, yPos, zPos));
        }
      }

      bool oddRow = false;
      for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
      {
        if (!oddRow) // even rows: y == 0, y == 2; and so on
        {
          for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
          {
            indices.push_back(y * (X_SEGMENTS + 1) + x);
            indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
          }
        }
        else
        {
          for (int x = X_SEGMENTS; x >= 0; --x)
          {
            indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
            indices.push_back(y * (X_SEGMENTS + 1) + x);
          }
        }
        oddRow = !oddRow;
      }
      indexCount = indices.size();

      std::vector<float> data;
      for (unsigned int i = 0; i < positions.size(); ++i)
      {
        data.push_back(positions[i].x);
        data.push_back(positions[i].y);
        data.push_back(positions[i].z);
        if (normals.size() > 0)
        {
          data.push_back(normals[i].x);
          data.push_back(normals[i].y);
          data.push_back(normals[i].z);
        }
        if (uv.size() > 0)
        {
          data.push_back(uv[i].x);
          data.push_back(uv[i].y);
        }
      }
      glBindVertexArray(sphereVAO);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
      unsigned int stride = (3 + 2 + 3) * sizeof(float);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
      glEnableVertexAttribArray(2);
      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    }

    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
  }

  unsigned int loadHDRFile(const char* path)
  {
    unsigned int hdrTexture = 0;

    stbi_set_flip_vertically_on_load(true);
    int width, height, nrComponents;
    float* data = stbi_loadf(path, &width, &height, &nrComponents, 0);
    if (data)
    {
      glGenTextures(1, &hdrTexture);
      glBindTexture(GL_TEXTURE_2D, hdrTexture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      stbi_image_free(data);
    }
    else {
      std::cout << "Failed to load HDR image: " << path << std::endl;
    }

    return hdrTexture;
  }

  static std::unordered_map<int, std::pair<int, int>> channel_map = {
      {1, {GL_R16F, GL_RED}},
      {2, {GL_RG16F, GL_RG}},
      {3, {GL_RGB16F, GL_RGB}},
      {4, {GL_RGBA16F, GL_RGBA}},
  };

  unsigned int genTextureCube(unsigned int width, unsigned int height, bool with_mipmap, int NR)
  {
    unsigned int res;
    glGenTextures(1, &res);
    glBindTexture(GL_TEXTURE_CUBE_MAP, res);
    for (int i = 0; i < 6; i++) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, channel_map[NR].first,
        width, height, 0, channel_map[NR].second, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, with_mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (with_mipmap) {
      glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return res;
  }

  unsigned int genTexture2D(unsigned int width, unsigned int height, bool with_mipmap, int NR, const void* data, bool is_hdr)
  {
    unsigned int res;
    glGenTextures(1, &res);
    glBindTexture(GL_TEXTURE_2D, res);
    glTexImage2D(GL_TEXTURE_2D, 0, channel_map[NR].first, width, height, 0,
      channel_map[NR].second, is_hdr ? GL_FLOAT : GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, with_mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (with_mipmap) {
      glGenerateMipmap(GL_TEXTURE_2D);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    return res;
  }

}