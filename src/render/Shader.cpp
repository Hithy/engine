#include "Shader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <fstream>
#include <string>
#include <cstring>
#include <iostream>

namespace render {

  static std::string get_file_content(const char* file_path) {
    std::string res;

    const int BUFF_SIZE = 4096;

    auto file = std::fstream(file_path);
    if (file.is_open()) {
      char buffer[BUFF_SIZE + 1] = { 0 };

      while (!file.eof()) {
        ::memset(buffer, 0, BUFF_SIZE);
        file.read(buffer, BUFF_SIZE);

        res += buffer;
      }
    }

    return res;
  }

  Shader::Shader(const char* compute_path)
  {
    char infoLog[512];
    int success;

    auto src_str = get_file_content(compute_path);

    const auto* src = src_str.c_str();

    unsigned int compute_shader;

    compute_shader = glCreateShader(GL_COMPUTE_SHADER);

    glShaderSource(compute_shader, 1, &src, NULL);

    glCompileShader(compute_shader);
    glGetShaderiv(compute_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
      glGetShaderInfoLog(compute_shader, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::COMPUTE::COMPILATION_FAILED\n" << infoLog << std::endl;
    };

    id = glCreateProgram();
    glAttachShader(id, compute_shader);
    glLinkProgram(id);

    glDeleteShader(compute_shader);
  }

  Shader::Shader(const char* vert_path, const char* frag_path)
  {
    char infoLog[512];
    int success;

    auto vert_src_str = get_file_content(vert_path);
    auto frag_src_str = get_file_content(frag_path);

    const auto* vert_src = vert_src_str.c_str();
    const auto* frag_src = frag_src_str.c_str();

    unsigned int vert_shader, frag_shader;

    vert_shader = glCreateShader(GL_VERTEX_SHADER);
    frag_shader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vert_shader, 1, &vert_src, NULL);
    glShaderSource(frag_shader, 1, &frag_src, NULL);

    glCompileShader(vert_shader);
    glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
      glGetShaderInfoLog(vert_shader, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::VS::COMPILATION_FAILED\n" << infoLog << std::endl;
    };

    glCompileShader(frag_shader);
    glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
      glGetShaderInfoLog(frag_shader, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::FG::COMPILATION_FAILED\n" << infoLog << std::endl;
    };

    id = glCreateProgram();
    glAttachShader(id, vert_shader);
    glAttachShader(id, frag_shader);
    glLinkProgram(id);
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(id, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);
  }

  Shader::Shader(const char* vert_path, const char* gs_path, const char* frag_path)
  {
    auto vert_src_str = get_file_content(vert_path);
    auto gs_src_str = get_file_content(gs_path);
    auto frag_src_str = get_file_content(frag_path);

    const auto* vert_src = vert_src_str.c_str();
    const auto* gs_src = gs_src_str.c_str();
    const auto* frag_src = frag_src_str.c_str();

    unsigned int vert_shader, gs_shader, frag_shader;

    vert_shader = glCreateShader(GL_VERTEX_SHADER);
    gs_shader = glCreateShader(GL_GEOMETRY_SHADER);
    frag_shader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vert_shader, 1, &vert_src, NULL);
    glShaderSource(gs_shader, 1, &gs_src, NULL);
    glShaderSource(frag_shader, 1, &frag_src, NULL);

    glCompileShader(vert_shader);
    glCompileShader(gs_shader);
    glCompileShader(frag_shader);

    id = glCreateProgram();
    glAttachShader(id, vert_shader);
    glAttachShader(id, gs_shader);
    glAttachShader(id, frag_shader);
    glLinkProgram(id);

    glDeleteShader(vert_shader);
    glDeleteShader(gs_shader);
    glDeleteShader(frag_shader);
  }

  void Shader::Use()
  {
    glUseProgram(id);
  }

  void Shader::SetFloat(const char* name, float value)
  {
    int param_pos = glGetUniformLocation(id, name);
    glUniform1f(param_pos, value);
  }

  void Shader::SetInt(const char* name, int value)
  {
    int param_pos = glGetUniformLocation(id, name);
    glUniform1i(param_pos, value);
  }

  void Shader::SetUInt(const char* name, unsigned int value)
  {
    int param_pos = glGetUniformLocation(id, name);
    glUniform1ui(param_pos, value);
  }

  void Shader::SetFM4(const char* name, const float* ptr)
  {
    int param_pos = glGetUniformLocation(id, name);
    glUniformMatrix4fv(param_pos, 1, GL_FALSE, ptr);
  }

  void Shader::SetFV3(const char* name, const float* ptr)
  {
    int param_pos = glGetUniformLocation(id, name);
    glUniform3f(param_pos, ptr[0], ptr[1], ptr[2]);
  }

  void Shader::SetFV2(const char* name, const float* ptr)
  {
    int param_pos = glGetUniformLocation(id, name);
    glUniform2f(param_pos, ptr[0], ptr[1]);
  }

  void Shader::SetFV3(const char* name, float x, float y, float z)
  {
    int param_pos = glGetUniformLocation(id, name);
    glUniform3f(param_pos, x, y, z);
  }

  void Shader::SetFV4(const char* name, const float* ptr)
  {
    int param_pos = glGetUniformLocation(id, name);
    glUniform4f(param_pos, ptr[0], ptr[1], ptr[2], ptr[3]);
  }

  void Shader::SetFV4(const char* name, float x, float y, float z, float w)
  {
    int param_pos = glGetUniformLocation(id, name);
    glUniform4f(param_pos, x, y, z, w);
  }

  void Shader::Compute(unsigned int x, unsigned int y, unsigned int z)
  {
    glDispatchCompute(x, y, z);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  }

  void Shader::Validate()
  {
    char infoLog[512] = { 0 };
    int success;
    glValidateProgram(id);
    glGetProgramiv(id, GL_VALIDATE_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(id, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::VS::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
  }

}