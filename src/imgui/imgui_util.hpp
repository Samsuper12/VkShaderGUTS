#pragma once

#include <imgui.h>
#include <string>

namespace util::imgui {
inline auto CenterText(const char *text) -> void {
  float textWidth = ImGui::CalcTextSize(text).x;
  float availWidth = ImGui::GetContentRegionAvail().x;

  float offset = (availWidth - textWidth) * 0.5f;

  if (offset > 0.0f)
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

  ImGui::TextUnformatted(text);
}

inline void HelpMarker(const std::string &desc) {
  ImGui::TextDisabled("(?)");
  if (ImGui::BeginItemTooltip()) {
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
    ImGui::TextUnformatted(desc.c_str());
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
  }
}
} // namespace util::imgui