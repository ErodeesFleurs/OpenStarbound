module;
#include "StarJson.hpp"

#include "StarCasting.hpp"
import star.input_event;
import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
import star.rect;
import star.application_controller;
import star.color;
import star.font;
#include "StarVariant.hpp"
#include "StarBiMap.hpp"
import star.ref_ptr;
import star.renderer;
#include "StarList.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarString.hpp"
import star.text;
#include "StarString.hpp"
#include "StarDataStream.hpp"
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
#include "StarThread.hpp"
import star.game_types;
#include "StarSet.hpp"
import star.audio;
#include "StarList.hpp"
#include "StarMap.hpp"
import star.font_texture_group;
import star.anchor_types;
import star.text_painter;
import star.drawable;
import star.asset_texture_group;
import star.drawable_painter;
import star.gui_types;
import star.key_bindings;
import star.mixer;
import star.gui_context;
import star.widget;

namespace Star {

STAR_CLASS(LabelWidget);
class LabelWidget : public Widget {
public:
  LabelWidget();
  LabelWidget(String text,
      Color const& color = Color::White,
      HorizontalAnchor const& hAnchor = HorizontalAnchor::LeftAnchor,
      VerticalAnchor const& vAnchor = VerticalAnchor::BottomAnchor,
      Maybe<unsigned> wrapWidth = {},
      Maybe<float> lineSpacing = {});

  String const& text() const;
  Maybe<unsigned> getTextCharLimit() const;
  void setText(String newText);
  void setFontSize(int fontSize);
  void setFontMode(FontMode fontMode);
  void setColor(Color newColor);
  void setAnchor(HorizontalAnchor hAnchor, VerticalAnchor vAnchor);
  void setWrapWidth(Maybe<unsigned> wrapWidth);
  void setLineSpacing(Maybe<float> lineSpacing);
  void setDirectives(String const& directives);
  void setTextCharLimit(Maybe<unsigned> charLimit);
  void setTextStyle(TextStyle const& style);
  void setFont(String const& font);

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
  Maybe<unsigned> m_wrapWidth;
  Maybe<float> m_lineSpacing;
  Maybe<unsigned> m_textCharLimit;
  RectI m_textRegion;
};

}

export module star.label_widget;

export namespace Star {
  using ::Star::LabelWidget;
  using ::Star::LabelWidgetPtr;
  using ::Star::LabelWidgetConstPtr;
  using ::Star::LabelWidgetWeakPtr;
  using ::Star::LabelWidgetConstWeakPtr;
  using ::Star::LabelWidgetUPtr;
  using ::Star::LabelWidgetConstUPtr;
}
