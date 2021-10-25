#include "world.h"
#include <iostream>

#include <PxPhysicsAPI.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "python_ecs.h"
#include "pybind/pybind.h"

#include "render/render.h"
#include "render/Model.h"
#include "render/resource_mgr.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

namespace ECS {

static physx::PxDefaultAllocator gAllocator;
static physx::PxDefaultErrorCallback gErrorCallback;

static void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
  static double lastX = xpos;
  static double lastY = ypos;

  auto &world = World::GetInstance();

  if (world.IsMouseCaptured()) {
    world.ctx.input.move_delta_x += xpos - lastX;
    world.ctx.input.move_delta_y += ypos - lastY;
  }

  lastX = xpos;
  lastY = ypos;
}

static void scroll_callback(GLFWwindow *window, double xoffset,
                            double yoffset) {
  auto &world = World::GetInstance();
  if (world.IsMouseCaptured()) {
    world.ctx.input.scroll_delta_x = xoffset;
    world.ctx.input.scroll_delta_y = yoffset;
  }
}

static void framebuffer_size_callback(GLFWwindow *window, int width,
                                      int height) {
  glViewport(0, 0, width, height);
}

World::World() {
  ctx.title = "hello_engine";
  ctx.window_width = 1920;
  ctx.window_height = 1080;

  _capture_mouse = false;
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
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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

  glfwSetCursorPosCallback(_window, mouse_callback);
  glfwSetScrollCallback(_window, scroll_callback);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  // glEnable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
  // glEnable(GL_DEBUG_OUTPUT);
  // glDebugMessageCallback(MessageCallback, 0);

  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForOpenGL(_window, true);
  ImGui_ImplOpenGL3_Init("#version 330");
}

void World::initPhysx() {
  _foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
  _pvd = physx::PxCreatePvd(*_foundation);
  _physics = PxCreatePhysics(PX_PHYSICS_VERSION, *_foundation, physx::PxTolerancesScale(), true, _pvd);
  PxInitExtensions(*_physics, _pvd);
}

void World::initRender()
{
  render::Render::GetInstance().Init();
}

void World::startScript()
{
  InitPythonPost();
}

void World::updateInput() {
  if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(_window, true);
  }

  if (!IsMouseCaptured()) {
    return;
  }

  if (glfwGetKey(_window, GLFW_KEY_Q) == GLFW_PRESS) {
    ToggleMouse();
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

  TickPython();

  ctx.input.ClearEachFrame();
}

void World::render() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGui::Begin("Contrl Panel");
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
  if (ImGui::Button("Control Camera")) {
    ToggleMouse();
  }
  if (ImGui::Button("Connet PVD")) {
    ConnectPVD();
  }

  render::Render::GetInstance().DoRender();

  ImGui::End();
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  glfwSwapBuffers(_window);
}

void World::ConnectPVD()
{
  physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
  _pvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);
}

void World::ToggleMouse()
{
  _capture_mouse = !_capture_mouse;
  glfwSetInputMode(_window, GLFW_CURSOR, _capture_mouse ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

bool World::IsMouseCaptured()
{
  return _capture_mouse;
}

std::vector<double> World::GetMoveDelta()
{
  std::vector<double> res(2);
  res[0] = ctx.input.move_delta_x;
  res[1] = ctx.input.move_delta_y;
  return res;
}

bool World::IsMousePressed(int key)
{
  return glfwGetMouseButton(_window, key) == GLFW_PRESS;
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
  initRender();

  startScript();
}

void World::Run() {
  uint64_t curr_frame = 0;
  while (!glfwWindowShouldClose(_window)) {
    logic();

    render();

    glfwPollEvents();
    curr_frame++;
  }
}

BIND_CLS_FUNC_DEFINE(World, GetActiveScene)
BIND_CLS_FUNC_DEFINE(World, AddScene)
BIND_CLS_FUNC_DEFINE(World, GetMoveDelta)
BIND_CLS_FUNC_DEFINE(World, IsMousePressed)

static PyMethodDef type_methods[] = {
  {"get_active_scene", BIND_CLS_FUNC_NAME(World, GetActiveScene), METH_NOARGS, 0},
  {"AddScene", BIND_CLS_FUNC_NAME(World, AddScene), METH_VARARGS, 0},
  {"GetMoveDelta", BIND_CLS_FUNC_NAME(World, GetMoveDelta), METH_NOARGS, 0},
  {"IsMousePressed", BIND_CLS_FUNC_NAME(World, IsMousePressed), METH_VARARGS, 0},
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
