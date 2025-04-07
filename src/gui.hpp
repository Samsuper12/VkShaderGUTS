#pragma once
#include "layer.hpp"
#include "util.hpp"
#include <imgui.h>
#include <thread>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glbinding/getProcAddress.h>
#include <glbinding/gl/gl.h>

#include <glbinding-aux/debug.h>
#include <glbinding/glbinding.h>

#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

namespace impl {
using namespace gl;

class Gui {
public:
  Gui() : enable(true), context(nullptr), window(nullptr) {

    util::envContainsTrue("VK_SHADER_GUTS_GUI", enable);

    if (!enable)
      return;

    InitWindow();
    InitImgui();
    glfwMakeContextCurrent(nullptr);
  }

  auto Draw() -> void {
    if (!enable)
      return;

    glfwMakeContextCurrent(window);

    while (enable && !glfwWindowShouldClose(window)) {
      glClear(gl::GL_COLOR_BUFFER_BIT);
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      // content here
      ImGui::Begin("Win1");

      ImGui::Text("Delta: %.5f", 0.33325235);
      ImGui::End();

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
      glfwSwapBuffers(window);
      glfwPollEvents();
    }

    glfwMakeContextCurrent(nullptr);
    glfwTerminate();
  }

private:
  auto InitWindow() -> void {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    window = glfwCreateWindow(800, 600, "VkShaderGUTS", nullptr, nullptr);

    if (!window) {
      std::clog << "[VK_SHADER_GUTS][GUI][err]: Failed to create GLFW window\n";
      enable = false;
    }

    glfwMakeContextCurrent(window);

    glbinding::initialize(glfwGetProcAddress, false);
    glbinding::aux::enableGetErrorCallback();

    glViewport(0, 0, 800, 600);
  }

  auto InitImgui() -> void {
    const char *glsl_version = "#version 330";

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;

    ImGui::StyleColorsDark();

    auto res1 = ImGui_ImplGlfw_InitForOpenGL(window, true);
    auto res2 = ImGui_ImplOpenGL3_Init(glsl_version);

    if (!(res1 || res2))
      enable = false;
  }

private:
  bool enable;

  ImGuiContext *context;
  ImVec2 winSize;

  GLFWwindow *window;
};
}; // namespace impl
