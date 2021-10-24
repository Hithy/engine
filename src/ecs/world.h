#pragma once
#include <string>
#include <vector>

#include "scene.h"
#include "pybind/pyobject.h"

struct GLFWwindow;

namespace ECS {

struct InputContext {
  float sensitivity = 10.0f;

  double move_delta_x = 0.0;
  double move_delta_y = 0.0;

  double scroll_delta_x = 0.0;
  double scroll_delta_y = 0.0;

  int W_Status = -1;
  int S_Status = -1;
  int A_Status = -1;
  int D_Status = -1;
  int H_Status = -1;

  int KeyStatus[128] = { 0 };
  int KeyToggled[128] = { 0 };

  void ClearEachFrame() {
    move_delta_x = 0.0;
    move_delta_y = 0.0;
    scroll_delta_x = 0.0;
    scroll_delta_y = 0.0;
    W_Status = -1;
    S_Status = -1;
    A_Status = -1;
    D_Status = -1;
    H_Status = -1;
  }
};

struct WorldContext {
  std::string title;

  int window_width;
  int window_height;

  InputContext input;
};

class World : public BindObject {
public:
  DECLEAR_PYCXX_OBJECT_TYPE(World);

  static World &GetInstance() {
    static World inst;
    return inst;
  }

  void AddScene(Scene* scn);

  Scene* GetActiveScene() {
    return _scenes[0];
  }

  void Init();
  void Run();

  void ToggleMouse();
  bool IsMouseCaptured();

  std::vector<double> GetMoveDelta();
  bool IsMousePressed(int key);

  WorldContext ctx;

private:
  World();
  void initPython();
  void initGL();
  void initPhysx();
  void initRender();

  void startScript();

  void updateInput();
  void logic();
  void render();

private:
  std::vector<Scene *> _scenes;

  GLFWwindow *_window;

  bool _capture_mouse;
};
} // namespace ECS
