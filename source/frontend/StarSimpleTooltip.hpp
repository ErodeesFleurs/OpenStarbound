#pragma once

#include "StarString.hpp"

namespace Star {

class Pane;
using PanePtr = SharedPtr<Pane>;

namespace SimpleTooltipBuilder {
  PanePtr buildTooltip(String const& text);
};

}
