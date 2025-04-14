#include "PipelineLibrary.hpp"
#include "gui.hpp"

namespace impl {

auto DrawVkGraphicsPipelineCreateInfo() {

  {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);

    ImGui::Text("flags:");
    ImGui::SameLine();
    ImGui::Text("Result");
  }

  {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);

    ImGui::Text("pStages:");
    ImGui::SameLine();
    // table or none text
    ImGui::Text("Result");
  }

  {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);

    ImGui::Text("pVertexInputState:");
    ImGui::SameLine();
    ImGui::Text("Result");
  }

  {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);

    ImGui::Text("pInputAssemblyState:");
    ImGui::SameLine();
    ImGui::Text("Result");
  }

  {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);

    ImGui::Text("pTesselationInputState:");
    ImGui::SameLine();
    ImGui::Text("Result");
  }

  {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);

    ImGui::Text("pViewportInputState:");
    ImGui::SameLine();
    ImGui::Text("Result");
  }

  {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);

    ImGui::Text("pRasterizationState:");
    ImGui::SameLine();
    ImGui::Text("Result");
  }

  {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);

    ImGui::Text("pMultisampleState:");
    ImGui::SameLine();
    ImGui::Text("Result");
  }

  {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);

    ImGui::Text("pDepthStencilState:");
    ImGui::SameLine();
    ImGui::Text("Result");
  }

  {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);

    ImGui::Text("pColorBlendState:");
    ImGui::SameLine();
    ImGui::Text("Result");
  }

  {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);

    ImGui::Text("pDynamicState:");
    ImGui::SameLine();
    ImGui::Text("Result");
  }

  {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);

    ImGui::Text("subpass:");
    ImGui::SameLine();
    ImGui::Text("Result");
  }

  {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);

    ImGui::Text("basePipelineIndex:");
    ImGui::SameLine();
    ImGui::Text("Result");
  }
}

auto DrawPipelinesMenuInfo(bool empty,
                           const PipelineLibrary::Pipeline &pipeline) {

  util::imgui::CenterText("Pipeline Info");

  const char *emptyLine = "";
  std::array<char, 128> valueLine;

  {
    ImGui::Text("Index:");
    ImGui::SameLine();

    if (!empty) {
      std::ranges::fill(valueLine, '\0');
      std::format_to(valueLine.begin(), "{}", pipeline.data.index);
    }

    ImGui::Text(empty ? emptyLine : valueLine.data());
  }

  {
    ImGui::Text("Edited:");
    ImGui::SameLine();

    if (!empty) {
      std::ranges::fill(valueLine, '\0');
      std::format_to(valueLine.begin(), "{}",
                     pipeline.data.edited ? "Yes" : "No");
    }
    ImGui::Text(empty ? emptyLine : valueLine.data());
  }

  {
    ImGui::Text("Type:");
    ImGui::SameLine();
    if (!empty) {
      std::ranges::fill(valueLine, '\0');
      std::format_to(valueLine.begin(), "{}",
                     PipelineLibrary::TypeToString(pipeline.data.type));
    }
    ImGui::Text(empty ? emptyLine : valueLine.data());
  }

  {
    ImGui::Text("Result:");
    ImGui::SameLine();
    if (!empty) {
      std::ranges::fill(valueLine, '\0');
      std::format_to(valueLine.begin(), "{}",
                     pipeline.data.result == VK_SUCCESS ? "Success" : "Failed");
    }
    ImGui::Text(empty ? emptyLine : valueLine.data());
  }

  {
    ImGui::Text("Build Time:");
    ImGui::SameLine();
    if (!empty) {
      std::ranges::fill(valueLine, '\0');
      std::format_to(valueLine.begin(), "{:.3f}ms",
                     pipeline.data.compileDuration);
    }
    ImGui::Text(empty ? emptyLine : valueLine.data());
  }

  ImGui::Separator();
  ImGui::Text("VkGraphicsPipelineCreateInfo:");

  /*
    switch (pipeline.data.type) {
    case PipelineLibrary::Type::Graphics:
      DrawVkGraphicsPipelineCreateInfo();
      break;
    case PipelineLibrary::Type::Compute:
      // DrawVkComputePipelineCreateInfo();
      break;
    case PipelineLibrary::Type::ShaderObjectEXT:
      // DrawShaderObjectEXT();
      break;
    }
      */
}

auto Gui::DrawPipilinesMenu() -> void {
  ImGui::SetNextWindowSize(ImVec2(700, 500), ImGuiCond_FirstUseEver);

  ImGui::Begin("Pipelines");

  if (pipeLibrary.ReadyToPull()) {
    allPipelines = pipeLibrary.GetAllPipelines();
    lastFramePipelines = pipeLibrary.GetLastFramePipelines();
  }

  { // list
    ImGui::BeginChild("ChildL", ImVec2(350, ImGui::GetContentRegionAvail().y),
                      ImGuiChildFlags_Borders);

    if (ImGui::BeginTabBar(
            "PipilinesTab",
            ImGuiTabBarFlags_Reorderable |
                ImGuiTabBarFlags_NoCloseWithMiddleMouseButton |
                ImGuiTabBarFlags_DrawSelectedOverline |
                ImGuiTabItemFlags_NoCloseWithMiddleMouseButton)) {

      if (ImGui::BeginTabItem("All")) {

        if (state.play)
          util::imgui::CenterText("Wainting for pause.");
        else {
          DrawPipelines(allPipelines);
          state.currentPipelineTab = GuiState::PipelineTabs::all;
        }

        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Last Frame")) {

        if (state.play)
          util::imgui::CenterText("Wainting for pause.");
        else {
          DrawPipelines(lastFramePipelines);
          state.currentPipelineTab = GuiState::PipelineTabs::lastFrame;
        }

        ImGui::EndTabItem();
      }

      // TODO
      if (ImGui::BeginTabItem("Edited")) {

        if (false) {
          state.currentPipelineTab = GuiState::PipelineTabs::edited;
        }

        util::imgui::CenterText("TODO");
        ImGui::EndTabItem();
      }

      ImGui::EndTabBar();
    }
    ImGui::EndChild();
  }

  ImGui::SameLine();

  { // info about list item
    ImGui::BeginChild("ChildR", ImVec2(0, ImGui::GetContentRegionAvail().y),
                      ImGuiChildFlags_None);

    const auto &tab =
        state.currentPipelineTab == GuiState::PipelineTabs::all ? allPipelines
        : state.currentPipelineTab == GuiState::PipelineTabs::lastFrame
            ? lastFramePipelines
            : editedPipelines;

    auto drawEmpty = !(state.selectedRow <= tab.size() && !tab.empty());
    // table with shaders here

    DrawPipelinesMenuInfo(drawEmpty, tab[state.selectedRow]);

    ImGui::EndChild();
  }

  ImGui::End();
}

auto Gui::DrawPipelines(const std::ranges::input_range auto &pipelines)
    -> void {
  if (ImGui::BeginTable("##Basket", 4,
                        ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg |
                            ImGuiTableFlags_BordersOuter |
                            ImGuiTableFlags_SizingFixedFit)) {

    static ImGuiSelectionBasicStorage selection;
    ImGui::TableSetupColumn("Pipeline");
    ImGui::TableSetupColumn("Failed");
    ImGui::TableSetupColumn("Type");
    ImGui::TableSetupColumn("Compile");
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableHeadersRow();

    ImGuiMultiSelectFlags flags =
        ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_BoxSelect1d;
    ImGuiMultiSelectIO *ms_io =
        ImGui::BeginMultiSelect(flags, selection.Size, pipelines.size());
    selection.ApplyRequests(ms_io);

    ImGuiListClipper clipper;
    clipper.Begin(pipelines.size());
    if (ms_io->RangeSrcItem != -1)
      clipper.IncludeItemByIndex(
          (int)ms_io->RangeSrcItem); // Ensure RangeSrc item is not clipped.

    while (clipper.Step()) {
      for (int n = clipper.DisplayStart; n < clipper.DisplayEnd; n++) {
        std::array<char, 128> text;
        ImGui::TableNextRow();

        { // Pipeline
          ImGui::TableNextColumn();
          std::format_to(text.begin(), "VkPipeline{}", pipelines[n].data.index);
          bool item_is_selected = selection.Contains((ImGuiID)n);
          ImGui::SetNextItemSelectionUserData(n);
          if (ImGui::Selectable(text.data(), item_is_selected,
                                ImGuiSelectableFlags_SpanAllColumns |
                                    ImGuiSelectableFlags_AllowOverlap)) {
            state.selectedRow = n;
          }
          std::ranges::fill(text, '\0');
        }

        { // Edited
          ImGui::TableNextColumn();
          ImGui::Text(pipelines[n].data.result ? "" : "X");
        }

        { // Type
          ImGui::TableNextColumn();
          std::format_to(text.begin(), "{}",
                         PipelineLibrary::TypeToString(pipelines[n].data.type));
          ImGui::Text(text.data());
          std::ranges::fill(text, '\0');
        }

        { // Compile duration
          ImGui::TableNextColumn();
          std::format_to(text.begin(), "{:.3f}ms",
                         pipelines[n].data.compileDuration);
          ImGui::Text(text.data());
          std::ranges::fill(text, '\0');
        }
      }
    }
    ms_io = ImGui::EndMultiSelect();
    selection.ApplyRequests(ms_io);
    ImGui::EndTable();
  }
}
} // namespace impl