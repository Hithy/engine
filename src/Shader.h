#pragma once
class Shader
{
public:
  Shader(const char* vert_path, const char* frag_path);

  void Use();
  void SetFloat(const char* name, float value);
  void SetInt(const char* name, int value);
  void SetFM4(const char* name, float* ptr);
  void SetFV3(const char* name, float* ptr);
  void SetFV3(const char* name, float x, float y, float z);
  void SetFV4(const char* name, float* ptr);
  void SetFV4(const char* name, float x, float y, float z, float w);

private:
  unsigned int id;
};

