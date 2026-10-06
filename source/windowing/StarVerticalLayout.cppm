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
import star.layout;

namespace Star {

STAR_CLASS(VerticalLayout);

class VerticalLayout : public Layout {
public:
  VerticalLayout(VerticalAnchor verticalAnchor = VerticalAnchor::TopAnchor, int verticalSpacing = 0);

  void update(float dt) override;
  Vec2I size() const override;
  RectI relativeBoundRect() const override;

  void setHorizontalAnchor(HorizontalAnchor horizontalAnchor);
  void setVerticalAnchor(VerticalAnchor verticalAnchor);
  void setVerticalSpacing(int verticalSpacing);
  void setFillDown(bool fillDown);

private:
  RectI contentBoundRect() const;

  HorizontalAnchor m_horizontalAnchor;
  VerticalAnchor m_verticalAnchor;
  int m_verticalSpacing;
  bool m_fillDown;
  Vec2I m_size;
};

}

export module star.vertical_layout;

export namespace Star {
  using ::Star::VerticalLayout;
  using ::Star::VerticalLayoutPtr;
  using ::Star::VerticalLayoutConstPtr;
  using ::Star::VerticalLayoutWeakPtr;
  using ::Star::VerticalLayoutConstWeakPtr;
  using ::Star::VerticalLayoutUPtr;
  using ::Star::VerticalLayoutConstUPtr;
}
