#include "world.h"
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "python_ecs.h"
#include "pybind/pybind.h"

namespace ECS {

static void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
  static float lastX = xpos;
  static float lastY = ypos;

  auto &world = World::GetInstance();

  world.ctx.input.move_delta_x = xpos - lastX;
  world.ctx.input.move_delta_y = ypos - lastY;

  lastX = xpos;
  lastY = ypos;
}

static void scroll_callback(GLFWwindow *window, double xoffset,
                            double yoffset) {
  auto &world = World::GetInstance();

  world.ctx.input.scroll_delta_x = xoffset;
  world.ctx.input.scroll_delta_y = yoffset;
}

static void framebuffer_size_callback(GLFWwindow *window, int width,
                                      int height) {
  glViewport(0, 0, width, height);
}

World::World() {
  ctx.title = "hello_engine";
  ctx.window_width = 1920;
  ctx.window_height = 1080;
}

void World::initPython()
{
  InitPython();
}

void GLAPIENTRY
MessageCallback(GLenum source,
  GLenum type,
  GLuint id,
  GLenum severity,
  GLsizei length,
  const GLchar* message,
  const void* userParam)
{
  fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
    (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
    type, severity, message);
}

void World::initGL() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

  _window = glfwCreateWindow(ctx.window_width, ctx.window_height,
                             ctx.title.c_str(), NULL, NULL);
  if (!_window) {
    throw "fail to create window";
  }

  glfwMakeContextCurrent(_window);
  glfwSetFramebufferSizeCallback(_window, framebuffer_size_callback);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    throw "fail to glad load gl loader\n";
  }

  glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(_window, mouse_callback);
  glfwSetScrollCallback(_window, scroll_callback);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(MessageCallback, 0);
}

void World::initPhysx() {}

void World::updateInput() {
  if (glfwGetKey(_window, GLFW_KEY_Q) == GLFW_PRESS) {
    glfwSetWindowShouldClose(_window, true);
  }

  ctx.input.W_Status = glfwGetKey(_window, GLFW_KEY_W);
  ctx.input.S_Status = glfwGetKey(_window, GLFW_KEY_S);
  ctx.input.A_Status = glfwGetKey(_window, GLFW_KEY_A);
  ctx.input.D_Status = glfwGetKey(_window, GLFW_KEY_D);
  ctx.input.H_Status = glfwGetKey(_window, GLFW_KEY_H);

  for (int i = GLFW_KEY_0; i <= GLFW_KEY_GRAVE_ACCENT; i++) {
    auto new_status = glfwGetKey(_window, i);
    ctx.input.KeyToggled[i] = bool(ctx.input.KeyStatus[i] != new_status);
    ctx.input.KeyStatus[i] = new_status;
  }
}

void World::logic() {
  updateInput();

  for (auto const &scn : _scenes) {
    scn->Logic();
  }

  ctx.input.ClearEachFrame();
}

void World::render() {
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  for (auto const &scn : _scenes) {
    scn->Render();
  }

  glfwSwapBuffers(_window);
  glfwPollEvents();
}

void World::AddScene(Scene* scn)
{
  _scenes.push_back(scn);
  scn->OnAddedToWorld();
}

void World::Init() {
  initPython();
  initGL();
  initPhysx();
}

void World::Run() {
  uint64_t curr_frame = 0;
  while (!glfwWindowShouldClose(_window)) {

    logic();

    TickPython();

    render();

    curr_frame++;
  }
}

BIND_CLS_FUNC_DEFINE(World, GetActiveScene)

static PyMethodDef type_methods[] = {
  {"get_active_scene", BIND_CLS_FUNC_NAME(World, GetActiveScene), METH_NOARGS, 0},
  {0, nullptr, 0, 0},
};

PyTypeObject* World::GetPyType() {
  static PyTypeObject* new_type = nullptr;
  if (new_type) {
    return new_type;
  }
  new_type = new PyTypeObject
  {
    PyVarObject_HEAD_INIT(NULL, 0)
    "World",
    sizeof(PyBindObject)
  };
  new_type->tp_dealloc = pybind__dealloc__;
  new_type->tp_methods = type_methods;
  return new_type;
}

} // namespace ECS
