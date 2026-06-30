#pragma once

#include "StarAssets.hpp"
#include "StarString.hpp"

namespace Star {

class Pane;
using PanePtr = SharedPtr<Pane>;
class GuiContext;

struct SimpleTooltipServices {
  AssetsConstPtr assets;
  GuiContext& guiContext;
};

namespace SimpleTooltipBuilder {
  [[nodiscard]] UniquePtr<Pane> buildTooltip(String const& text, SimpleTooltipServices services);
};

}
