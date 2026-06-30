#pragma once

#include "StarLayout.hpp"

namespace Star {

// Super simple, only supports left to right, top to bottom flow layouts
// currently
class FlowLayout;
using FlowLayoutPtr = SharedPtr<FlowLayout>;
class FlowLayout : public Layout {
public:
  explicit FlowLayout(GuiContext& context);
  virtual void update(float dt) override;
  void setSpacing(Vec2I const& spacing);
  void setWrapping(bool wrap);

private:
  Vec2I m_spacing;
  bool m_wrap;
};

}
