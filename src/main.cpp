#include <iostream>
#include <unordered_map>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"
#include "Shader.h"
#include "Model.h"

const int width = 1920;
const int height = 1080;
const char title[] = "engine";
Camera camera;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
  static float lastX = xpos;
  static float lastY = ypos;

  float xoffset = xpos - lastX;
  float yoffset = ypos - lastY;

  lastX = xpos;
  lastY = ypos;

  const float sensitivity = 0.1f;
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  camera.yaw += xoffset;
  camera.pitch -= yoffset;

  if (camera.pitch > 89.0f) {
    camera.pitch = 89.0f;
  }

  if (camera.pitch < -89.0f) {
    camera.pitch = -89.0f;
  }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
  camera.fov -= (float)yoffset;
  if (camera.fov < 1.0f) {
    camera.fov = 1.0f;
  }

  if (camera.fov > 60.0f) {
    camera.fov = 60.0f;
  }
}

static void framebuffer_size_callback(GLFWwindow* window, int width,
  int height) {
  glViewport(0, 0, width, height);
}

void process_input(GLFWwindow* window) {
  const float camera_speed = 0.05f;

  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }

  auto view = camera.GetView();
  view = glm::inverse(view);

  auto forward = view * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
  auto right = view * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    camera.pos += glm::vec3(forward * camera_speed);
  }
  else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    camera.pos -= glm::vec3(forward * camera_speed);
  }
  else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    camera.pos -= glm::vec3(right * camera_speed);
  }
  else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    camera.pos += glm::vec3(right * camera_speed);
  }
}

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* _window = glfwCreateWindow(width, height, title, NULL, NULL);
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

  auto test_shader = new Shader("shader/common.vert", "shader/common.frag");
  auto test_model = new Model("/home/yao/Documents/sgb/untitled.obj");

  glEnable(GL_DEPTH_TEST);

  while (!glfwWindowShouldClose(_window)) {
    process_input(_window);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    test_shader->Use();
    auto view = camera.GetView();
    auto projection = glm::perspective(glm::radians(camera.fov), 1920.0f / 1080.0f,
      0.1f, 100.0f);
    auto model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
    auto scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f, 0.01f, 0.01f));
    model = model * scale;

    test_shader->SetFM4("view", glm::value_ptr(view));
    test_shader->SetFM4("projection", glm::value_ptr(projection));
    test_shader->SetFM4("model", glm::value_ptr(model));

    test_model->Draw(test_shader);

    glfwSwapBuffers(_window);
    glfwPollEvents();
  }

  return 0;
}
