#include "gui.hpp"

namespace impl {
auto Gui::DrawPlaybackMenu() -> void {
  ImGui::SetNextWindowSize(ImVec2(180, 75), ImGuiCond_FirstUseEver);
  ImGui::Begin("Playback");
  ImGui::Text("Current frame: %ld", guts.GetFrameCount());

  if (ImGui::Button(state.play ? "Pause" : "Play")) {
    state.play = !state.play;
    guts.SetPlayback(state.play);
  }

  ImGui::SameLine();

  if (ImGui::ArrowButton("button_frameStep", ImGuiDir::ImGuiDir_Right))
    guts.SetPlayStep(1);

  ImGui::SameLine();

  if (ImGui::Button(">>"))
    guts.SetPlayStep(10);

  ImGui::End();
}

} // namespace impl
