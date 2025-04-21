#include "gui.hpp"

auto main(int argc, char *argv[]) -> int {
  impl::Gui gui;
  gui.Launch();
  return 0;
}