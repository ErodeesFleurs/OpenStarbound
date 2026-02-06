#pragma once

#include <optional>

#include "StarWidget.hpp"

namespace Star {

STAR_CLASS(LabelWidget);
class LabelWidget : public Widget {
public:
  LabelWidget(String text = String(),
      Color const& color = Color::White,
      HorizontalAnchor const& hAnchor = HorizontalAnchor::LeftAnchor,
      VerticalAnchor const& vAnchor = VerticalAnchor::BottomAnchor,
      std::optional<unsigned> wrapWidth = {},
      std::optional<float> lineSpacing = {});

  String const& text() const;
  std::optional<unsigned> getTextCharLimit() const;
  void setText(String newText);
  void setFontSize(int fontSize);
  void setFontMode(FontMode fontMode);
  void setColor(Color newColor);
  void setAnchor(HorizontalAnchor hAnchor, VerticalAnchor vAnchor);
  void setWrapWidth(std::optional<unsigned> wrapWidth);
  void setLineSpacing(std::optional<float> lineSpacing);
  void setDirectives(String const& directives);
  void setTextCharLimit(std::optional<unsigned> charLimit);
  void setTextStyle(TextStyle const& style);

  RectI relativeBoundRect() const override;

protected:
  virtual RectI getScissorRect() const override;
  virtual void renderImpl() override;

private:
  void updateTextRegion();

  String m_text;
  TextStyle m_style;
  HorizontalAnchor m_hAnchor;
  VerticalAnchor m_vAnchor;
  std::optional<unsigned> m_wrapWidth;
  std::optional<float> m_lineSpacing;
  std::optional<unsigned> m_textCharLimit;
  RectI m_textRegion;
};

}
