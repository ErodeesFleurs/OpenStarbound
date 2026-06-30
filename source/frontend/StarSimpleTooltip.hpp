#pragma once

#include "StarIAssets.hpp"
#include "StarString.hpp"

namespace Star {

class Pane;
using PanePtr = SharedPtr<Pane>;

struct SimpleTooltipServices {
  IAssetsConstPtr assets;
};

namespace SimpleTooltipBuilder {
  PanePtr buildTooltip(String const& text, SimpleTooltipServices services = {});
};

}
