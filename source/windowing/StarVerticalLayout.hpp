#pragma once

#include "StarLayout.hpp"

namespace Star {

class VerticalLayout;
using VerticalLayoutPtr = SharedPtr<VerticalLayout>;

class VerticalLayout : public Layout {
public:
  VerticalLayout(GuiContext& context, VerticalAnchor verticalAnchor = VerticalAnchor::TopAnchor, int verticalSpacing = 0);

  void update(float dt) override;
  [[nodiscard]] Vec2I size() const override;
  [[nodiscard]] RectI relativeBoundRect() const override;

  void setHorizontalAnchor(HorizontalAnchor horizontalAnchor);
  void setVerticalAnchor(VerticalAnchor verticalAnchor);
  void setVerticalSpacing(int verticalSpacing);
  void setFillDown(bool fillDown);

private:
  [[nodiscard]] RectI contentBoundRect() const;

  HorizontalAnchor m_horizontalAnchor;
  VerticalAnchor m_verticalAnchor;
  int m_verticalSpacing;
  bool m_fillDown;
  Vec2I m_size;
};

}
