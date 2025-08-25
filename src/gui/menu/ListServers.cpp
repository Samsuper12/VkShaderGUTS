#include "guts.hpp"
#include "imgui_util.hpp"
#include <filesystem>
#include <gui.hpp>
#include <imgui.h>

namespace impl {

auto ListServers() -> void {
  ImGui::Begin("Servers");

  // FIXME: fix it on windows. maybe.
  if (!fs::exists(socketDirectory)) {
    util::imgui::CenterText("No sockets found");
    ImGui::End();
    return;
  }

  const auto sockets = fs::directory_iterator(socketDirectory);

  if (ImGui::BeginTable("listServersTable",
                        ImGuiTableFlags_Borders |
                            ImGuiTableFlags_NoSavedSettings)) {


                                
    for (auto &socket : fs::directory_iterator(socketDirectory)) {
      std::array<char, 128> label;
      // TODO: show only file name (it's should be am app name)
      std::format_to(label.begin(), "{}", socket.path().string());

      ImGui::TableNextColumn();
      ImGui::Selectable(label.data(), false,
                        ImGuiSelectableFlags_AllowDoubleClick);

      if (ImGui::IsMouseDoubleClicked(0)) {
        // TODO: open this socket
      }
    }
    ImGui::EndTable();
  }

  ImGui::End();
}

} // namespace impl