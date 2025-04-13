#pragma once

#include <imgui.h>

namespace util::imgui {
inline auto CenterText(const char *text) -> void {
  float textWidth = ImGui::CalcTextSize(text).x;
  float availWidth = ImGui::GetContentRegionAvail().x;

  float offset = (availWidth - textWidth) * 0.5f;

  if (offset > 0.0f)
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

  ImGui::TextUnformatted(text);
}
} // namespace util::imgui