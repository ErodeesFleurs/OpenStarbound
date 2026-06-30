#pragma once

#include "StarWidget.hpp"

namespace Star {

// VERY simple base class for a layout container object.
class Layout : public Widget {
public:
  explicit Layout(GuiContext& context);
  void update(float dt) override;
};

}
