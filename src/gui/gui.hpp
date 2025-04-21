#pragma once
#include "PipelineLibrary.hpp"
#include "guts.hpp"
#include "util.hpp"
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <format>
#include <imgui.h>
#include <ranges>
#include <string_view>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glbinding/getProcAddress.h>
#include <glbinding/gl/gl.h>

#include <glbinding-aux/debug.h>
#include <glbinding/glbinding.h>

#include "gui/backend/imgui_impl_glfw.h"
#include "gui/backend/imgui_impl_opengl3.h"

#include "gui/imgui_util.hpp"

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
    ShaderGuts::CheckpointType checkpointType;
    ShaderGuts::CheckpointFunction checkpointFunction;
    bool play;
    uint32_t selectedRow;
  };

  Gui() :context(nullptr), window(nullptr) {
    state.play = true;
    bool pauseOnStart = false;
    // FIXME: move this to guts_layer
    //util::envContainsTrue("VK_SHADER_GUTS_GUI_ENABLE", enable);
    util::envContainsTrueOrPair(
        "VK_SHADER_GUTS_GUI_PAUSE", pauseOnStart,
        [&](std::string l, std::string r) {
          if (l.contains("function")) {
            try {
              static const std::map<std::string_view,
                                    ShaderGuts::CheckpointFunction>
                  functionStringToType{
                      {"vkCreateInstance",
                       ShaderGuts::CheckpointFunction::vkCreateInstance},
                      {"vkCreateDevice",
                       ShaderGuts::CheckpointFunction::vkCreateDevice},
                      {"vkCreateGraphicsPipelines",
                       ShaderGuts::CheckpointFunction::
                           vkCreateGraphicsPipelines},
                      {"vkCreateComputePipelines",
                       ShaderGuts::CheckpointFunction::
                           vkCreateComputePipelines},
                      {
                          "vkCmdBindPipeline",
                          ShaderGuts::CheckpointFunction::vkCmdBindPipeline,
                      },
                      {
                          "vkAcquireNextImageKHR",
                          ShaderGuts::CheckpointFunction::vkAcquireNextImageKHR,
                      },
                      {
                          "vkQueuePresentKHR",
                          ShaderGuts::CheckpointFunction::vkQueuePresentKHR,
                      },
                  };

              auto funcType = functionStringToType.at(r);

              // TODO: move that state completely into GUTS.
              state.checkpointType = ShaderGuts::CheckpointType::Function;
              state.checkpointFunction = funcType;
              state.play = false;
              // FIXME:
              // guts.Execute({cmd_t::playback, state.play});
              // guts.Execute({cmd_t::checkpointFunction, funcType});
              // guts.Execute({cmd_t::checkpointType,
              //  ShaderGuts::CheckpointType::Function});

            } catch (std::exception &e) {
              std::clog << "[VK_SHADER_GUTS][GUI][ERR]: bad argument\n ";
            }
          }
        });

    // FIXME
    InitWindow("FIXME");
    InitImgui();
    if (pauseOnStart) {
      state.play = !pauseOnStart;
      state.checkpointFunction =
          ShaderGuts::CheckpointFunction::vkAcquireNextImageKHR;
      // FIXME
      // guts.Execute({cmd_t::checkpointFunction, state.checkpointFunction});
      // guts.Execute({cmd_t::playback, state.play});
    }
  }

  ~Gui() {}
  Gui(const Gui &) = delete;
  Gui &operator=(const Gui &) = delete;
  Gui(Gui &&) = default;

  auto DrawPlaybackMenu() -> void;
  auto DrawPipilinesMenu() -> void;
  auto DrawPipelines(const std::ranges::input_range auto &pipelines) -> void;

  auto Launch() -> void { Draw(); }

private:
  auto Draw() -> void {
    while (!glfwWindowShouldClose(window)) {
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

  auto InitWindow(const std::string &appname) -> void {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    std::array<char, 128> title;

    std::format_to(title.begin(), "{}",
                   appname.empty() ? "VkShaderGUTS"
                                   : ("VkShaderGUTS [" + appname + "]"));

    window = glfwCreateWindow(800, 600, title.data(), nullptr, nullptr);

    if (!window) {
      std::clog << "[VK_SHADER_GUTS][GUI][err]: Failed to create GLFW window\n";
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

    PrepareSaveFile();

    if (!saveConfigPath.empty()) {
      io.IniFilename = nullptr;
      ImGui::LoadIniSettingsFromDisk(saveConfigPath.c_str());
    }

    ImGui::StyleColorsDark();
    auto res1 = ImGui_ImplGlfw_InitForOpenGL(window, true);
    auto res2 = ImGui_ImplOpenGL3_Init(glsl_version);

    if (!(res1 || res2))
      std::clog
          << "[VK_SHADER_GUTS][GUI][err]: Failed to init ImGui context!\n";
  }

private:
  ImGuiContext *context;
  ImVec2 winSize;

  GLFWwindow *window;
  GuiState state{};

  fs::path saveConfigPath;
  std::vector<PipelineLibrary::Pipeline> allPipelines;
  std::vector<PipelineLibrary::Pipeline> lastFramePipelines;
  std::vector<PipelineLibrary::Pipeline> editedPipelines;
};
}; // namespace impl
