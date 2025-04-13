#include "gui.hpp"
#include "imgui/imgui_util.hpp"
#include <imgui.h>

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
auto DrawPipelinesMenuInfo() {

  util::imgui::CenterText("Pipeline Info");
  {
    ImGui::Text("Index:");
    ImGui::SameLine();
    ImGui::Text("Result");
  }

  {
    ImGui::Text("Edited:");
    ImGui::SameLine();
    ImGui::Text("Result");
  }

  {
    ImGui::Text("Type:");
    ImGui::SameLine();
    ImGui::Text("Result");
  }

  {
    ImGui::Text("Result:");
    ImGui::SameLine();
    ImGui::Text("Result");
  }

  {
    ImGui::Text("Build Time:");
    ImGui::SameLine();
    ImGui::Text("Result");
  }

  /* TODO: Put it in the separate tab like InstanceInfo
  {
    ImGui::Text("DynamicState:");
    ImGui::SameLine();
    ImGui::Text("Result");
  }

  {
    ImGui::Text("DynamicPipeline:");
    ImGui::SameLine();
    ImGui::Text("Result");
  }
    */
  ImGui::Separator();

  {
    ImGui::Text("VkGraphicsPipelineCreateInfo:");
  }

  DrawVkGraphicsPipelineCreateInfo(); // or DrawVkComputePipelineCreateInfo
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
        else
          DrawPipelines(allPipelines);

        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Last Frame")) {

        if (state.play)
          util::imgui::CenterText("Wainting for pause.");
        else
          DrawPipelines(lastFramePipelines);

        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Edited")) {

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

    DrawPipelinesMenuInfo();
    // table with shaders here

    ImGui::EndChild();
  }

  ImGui::End();
}

auto Gui::DrawPipelines(const std::ranges::input_range auto &pipelines)
    -> void {
  using PipelineType = PipelineLibrary::Type;

  auto TypeToString = [](PipelineType t) -> std::string {
    switch (t) {
    case PipelineType::Graphics:
      return "Graphics";
    case PipelineType::Compute:
      return "Compute";
    case PipelineType::ShaderObjectEXT:
      return "ShaderObjectEXT";
    }
    std::unreachable();
  };

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
          ImGui::Selectable(text.data(), item_is_selected,
                            ImGuiSelectableFlags_SpanAllColumns |
                                ImGuiSelectableFlags_AllowOverlap);
          std::ranges::fill(text, '\0');
        }

        { // Edited
          ImGui::TableNextColumn();
          ImGui::Text(pipelines[n].data.result ? "" : "X");
        }

        { // Type
          ImGui::TableNextColumn();
          std::format_to(text.begin(), "{}",
                         TypeToString(pipelines[n].data.type));
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