#include "gui.hpp"
#include "guts.hpp"
#include "imgui/imgui_util.hpp"
#include <cstdint>
#include <imgui.h>
#include <string_view>

namespace impl {
auto Gui::DrawPlaybackMenu() -> void {

  static const std::map<uint32_t,
                        std::pair<ShaderGuts::CheckpointType, std::string_view>>
      indexToCheckpointTypeString{
          {0, {ShaderGuts::CheckpointType::Function, "Function"}},
          // {2, {GuiState::CheckpointType::Pipeline, "Pipeline"}}
      };

  static const std::map<ShaderGuts::CheckpointType, std::string_view>
      checkpointTypeToString{
          {ShaderGuts::CheckpointType::Function, "Function"},
          //  {GuiState::CheckpointType::Pipeline, "Pipeline"}
      };

  static const std::map<
      uint32_t, std::pair<ShaderGuts::CheckpointFunction, std::string_view>>
      indexToFunctionString{
          {0,
           {ShaderGuts::CheckpointFunction::vkCreateInstance,
            "vkCreateInstance"}},
          {1,
           {ShaderGuts::CheckpointFunction::vkCreateDevice, "vkCreateDevice"}},
          {2,
           {ShaderGuts::CheckpointFunction::vkCreateGraphicsPipelines,
            "vkCreateGraphicsPipelines"}},
          {3,
           {ShaderGuts::CheckpointFunction::vkCreateComputePipelines,
            "vkCreateComputePipelines"}},
          {4,
           {ShaderGuts::CheckpointFunction::vkCmdBindPipeline,
            "vkCmdBindPipeline"}},
          {5,
           {ShaderGuts::CheckpointFunction::vkAcquireNextImageKHR,
            "vkAcquireNextImageKHR"}},
          {6,
           {ShaderGuts::CheckpointFunction::vkQueuePresentKHR,
            "vkQueuePresentKHR"}}};

  static const std::map<ShaderGuts::CheckpointFunction, std::string_view>
      functionTypeToString{
          {ShaderGuts::CheckpointFunction::vkCreateInstance,
           "vkCreateInstance"},
          {ShaderGuts::CheckpointFunction::vkCreateDevice, "vkCreateDevice"},
          {ShaderGuts::CheckpointFunction::vkCreateGraphicsPipelines,
           "vkCreateGraphicsPipelines"},
          {ShaderGuts::CheckpointFunction::vkCreateComputePipelines,
           "vkCreateComputePipelines"},
          {ShaderGuts::CheckpointFunction::vkCmdBindPipeline,
           "vkCmdBindPipeline"},
          {ShaderGuts::CheckpointFunction::vkAcquireNextImageKHR,
           "vkAcquireNextImageKHR"},
          {ShaderGuts::CheckpointFunction::vkQueuePresentKHR,
           "vkQueuePresentKHR"}};

  // ImGui::SetNextWindowSize(ImVec2(180, 75), ImGuiCond_FirstUseEver);
  ImGui::Begin("Playback");

  if (ImGui::TreeNode("Settings")) {

    if (ImGui::BeginCombo(
            "Checkpoint",
            checkpointTypeToString.at(state.checkpointType).data(),
            ImGuiComboFlags_WidthFitPreview)) {
      for (size_t n = 0; n < indexToCheckpointTypeString.size(); n++) {
        const auto item = indexToCheckpointTypeString.at(n);
        const bool isSelected = (item.first == state.checkpointType);
        if (ImGui::Selectable(item.second.data(), isSelected)) {
          state.checkpointType = item.first;
          guts.Execute({cmd_t::checkpointType, state.checkpointType});
        }

        if (isSelected)
          ImGui::SetItemDefaultFocus();
      }
      ImGui::EndCombo();
    }

    ImGui::SameLine();

    util::imgui::HelpMarker(
        "Checkpoint types:\n"
        "Frame - Pause at start of a new frame (vkAcquireNextImageKHR)\n"
        "Function - Pause on each call of the selected function");

    if (state.checkpointType == ShaderGuts::CheckpointType::Function) {
      if (ImGui::BeginCombo(
              "Function",
              functionTypeToString.at(state.checkpointFunction).data(),
              ImGuiComboFlags_WidthFitPreview)) {

        for (size_t n = 0; n < indexToFunctionString.size(); n++) {
          const auto item = indexToFunctionString.at(n);
          const bool isSelected = (item.first == state.checkpointFunction);

          if (ImGui::Selectable(item.second.data(), isSelected)) {
            state.checkpointFunction = item.first;
            guts.Execute({cmd_t::checkpointFunction, state.checkpointFunction});
          }

          if (isSelected)
            ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
    }

    ImGui::TreePop();
  }

  ImGui::Text("Current frame: %ld", guts.GetFrameCount());

  if (ImGui::Button(state.play ? "Pause" : "Play")) {
    state.play = !state.play;

    guts.Execute({cmd_t::playback, state.play});
  }

  ImGui::SameLine();

  if (ImGui::ArrowButton("button_frameStep", ImGuiDir::ImGuiDir_Right))
    guts.Execute({cmd_t::playstep, 1});

  ImGui::SameLine();

  if (ImGui::Button(">>"))
    guts.Execute({cmd_t::playstep, 10});

  ImGui::End();
}

} // namespace impl
