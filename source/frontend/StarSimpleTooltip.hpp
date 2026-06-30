#pragma once

#include "StarAssets.hpp"
#include "StarString.hpp"

namespace Star {

class Pane;
using PanePtr = SharedPtr<Pane>;

struct SimpleTooltipServices {
  AssetsConstPtr assets;
};

namespace SimpleTooltipBuilder {
  PanePtr buildTooltip(String const& text, SimpleTooltipServices services);
};

}
