#pragma once
#include "guts.hpp"
#include "layer.hpp"
#include "util.hpp"
#include <cstdlib>
#include <filesystem>
#include <imgui.h>
#include <string_view>

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
namespace fs = std::filesystem;

class Gui {
  static constexpr std::string_view gutsFolder = "VkShaderGUTS";
  static constexpr std::string_view saveFile = "imgui.ini";
  static constexpr const char *glsl_version = "#version 330";

public:
  struct GuiState {
    bool play = true;
  };

  Gui(ShaderGuts &guts)
      : enable(true), context(nullptr), window(nullptr), guts(guts) {

    util::envContainsTrue("VK_SHADER_GUTS_GUI", enable);

    if (!enable)
      return;

    InitWindow();
    PrepareSaveFile();
    InitImgui();
    glfwMakeContextCurrent(nullptr);
  }

  auto DrawPlaybackMenu() -> void {
    ImGui::SetNextWindowSize(ImVec2(180, 75), ImGuiCond_FirstUseEver);
    ImGui::Begin("Playback");
    ImGui::Text("Current frame: %ld", guts.GetFrameCount());

    if (ImGui::Button(state.play ? "Pause" : "Play")) {
      state.play = !state.play;
      guts.SetPlayback(state.play);
    }
    ImGui::SameLine();

    if (ImGui::ArrowButton("button_frameStep", ImGuiDir::ImGuiDir_Right))
      guts.SetPlayStep();

    ImGui::End();
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

      DrawPlaybackMenu();

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
      glfwSwapBuffers(window);
      glfwPollEvents();
    }

    if (!saveConfigPath.empty())
      ImGui::SaveIniSettingsToDisk(saveConfigPath.c_str());

    glfwMakeContextCurrent(nullptr);
    glfwTerminate();
  }

private:
  auto PrepareSaveFile() -> void {
    const fs::path homeFolder = std::getenv("HOME");
    const fs::path configFolder = homeFolder / ".config" / gutsFolder;

    if (!fs::exists(configFolder))
      if (!fs::create_directory(configFolder)) {
        std::clog
            << "[VK_SHADER_GUTS][GUI][err]: Can't create save folder by path: "
            << configFolder << ". Default ImGui folder is enabled.\n";
      }

    saveConfigPath = configFolder / saveFile;
  }

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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;

    if (!saveConfigPath.empty()) {
      io.IniFilename = nullptr;
      ImGui::LoadIniSettingsFromDisk(saveConfigPath.c_str());
    }

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
  GuiState state;

  fs::path saveConfigPath;
  ShaderGuts &guts;
};
}; // namespace impl
