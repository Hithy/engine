#pragma once

namespace render {

  class Shader
  {
  public:
    Shader(const char* compute_path);
    Shader(const char* vert_path, const char* frag_path);
    Shader(const char* vert_path, const char* gs_path, const char* frag_path);

    void Use();
    void SetFloat(const char* name, float value);
    void SetInt(const char* name, int value);
    void SetUInt(const char* name, unsigned int value);
    void SetFM4(const char* name, const float* ptr);
    void SetFV3(const char* name, const float* ptr);
    void SetFV2(const char* name, const float* ptr);
    void SetFV3(const char* name, float x, float y, float z);
    void SetFV4(const char* name, const float* ptr);
    void SetFV4(const char* name, float x, float y, float z, float w);

    void Compute(unsigned int x, unsigned int y, unsigned int z);

    void Validate();

  private:
    unsigned int id;
  };

}