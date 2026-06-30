#pragma once

#include "StarWidget.hpp"

namespace Star {

class ImageWidget;
using ImageWidgetPtr = SharedPtr<ImageWidget>;
class ImageWidget : public Widget {
public:
  ImageWidget(GuiContext& context, String const& image = {});

  bool interactive() const override;
  void setImage(String const& image);
  void setScale(float scale);
  void setRotation(float rotation);
  String image() const;

  void setDrawables(List<Drawable> drawables);
  Vec2I offset();
  void setOffset(Vec2I const& offset);
  bool centered();
  void setCentered(bool centered);
  bool trim();
  void setTrim(bool trim);

  void setMaxSize(Vec2I const& size);
  void setMinSize(Vec2I const& size);

  RectI screenBoundRect() const override;

protected:
  void renderImpl() override;

private:
  void transformDrawables();

  List<Drawable> m_baseDrawables;
  List<Drawable> m_drawables;
  bool m_centered = false;
  bool m_trim = false;
  float m_scale = 1.0f;
  float m_rotation = 0.0f;
  Vec2I m_offset = {0, 0};
  Vec2I m_maxSize = {4096, 4096};
  Vec2I m_minSize = {0, 0};
};

}
