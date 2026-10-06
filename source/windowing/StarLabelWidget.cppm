module;

#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarApplicationController.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarRenderer.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
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
  LabelWidget(String text = String(),
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
