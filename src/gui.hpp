#pragma once
#include "PipelineLibrary.hpp"
#include "guts.hpp"
#include "layer.hpp"
#include "util.hpp"
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <imgui.h>
#include <ranges>
#include <string_view>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glbinding/getProcAddress.h>
#include <glbinding/gl/gl.h>

#include <glbinding-aux/debug.h>
#include <glbinding/glbinding.h>

#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "imgui/imgui_util.hpp"

namespace impl {
using namespace gl;
namespace fs = std::filesystem;

class Gui {
  static constexpr std::string_view gutsFolder = "VkShaderGUTS";
  static constexpr std::string_view saveFile = "imgui.ini";
  static constexpr const char *glsl_version = "#version 330";

public:
  struct GuiState {
    enum class PipelineTabs { all, lastFrame, edited };

    PipelineTabs currentPipelineTab;
    bool play;
    uint32_t selectedRow;
  };

  Gui(ShaderGuts &guts)
      : enable(true), context(nullptr), window(nullptr), guts(guts),
        pipeLibrary(guts.GetPipeLineLibrary()) {

    bool pauseOnStart = false;
    util::envContainsTrue("VK_SHADER_GUTS_GUI", enable);
    util::envContainsTrue("VK_SHADER_GUTS_GUI_PAUSE_ON_START", pauseOnStart);

    if (!enable)
      return;

    InitWindow();
    PrepareSaveFile();
    InitImgui();
    glfwMakeContextCurrent(nullptr);

    state.play = !pauseOnStart;
    guts.SetPlayback(state.play);
  }

  auto DrawPlaybackMenu() -> void;
  auto DrawPipilinesMenu() -> void;
  auto DrawPipelines(const std::ranges::input_range auto &pipelines) -> void;

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
      DrawPipilinesMenu();

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
        return;
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
  GuiState state{};

  fs::path saveConfigPath;
  ShaderGuts &guts;
  PipelineLibrary &pipeLibrary;

  std::vector<PipelineLibrary::Pipeline> allPipelines;
  std::vector<PipelineLibrary::Pipeline> lastFramePipelines;
  std::vector<PipelineLibrary::Pipeline> editedPipelines;
};
}; // namespace impl
