#pragma once
#include <string>
#include <vector>

#include "scene.h"

struct GLFWwindow;

namespace ECS {

struct InputContext {
  float sensitivity = 0.1f;

  double move_delta_x = 0.0;
  double move_delta_y = 0.0;

  double scroll_delta_x = 0.0;
  double scroll_delta_y = 0.0;

  int W_Status = -1;
  int S_Status = -1;
  int A_Status = -1;
  int D_Status = -1;

  void Clear() {
    move_delta_x = 0.0;
    move_delta_y = 0.0;
    scroll_delta_x = 0.0;
    scroll_delta_y = 0.0;
    W_Status = -1;
    S_Status = -1;
    A_Status = -1;
    D_Status = -1;
  }
};

struct WorldContext {
  std::string title;

  int window_width;
  int window_height;

  InputContext input;
};

class World {
public:
  static World &GetInstance() {
    static World inst;
    return inst;
  }

  void AddScene(Scene *scn) { _scenes.push_back(scn); }

  void Init();
  void Run();

  WorldContext ctx;

private:
  World();
  void initGL();
  void initPhysx();

  void updateInput();
  void logic();
  void render();

private:
  std::vector<Scene *> _scenes;

  GLFWwindow *_window;
};
} // namespace ECS
