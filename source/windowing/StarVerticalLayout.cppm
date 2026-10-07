module;

#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
import star.application;
#include "StarStatisticsService.hpp"
#include "StarP2PNetworkingService.hpp"
#include "StarUserGeneratedContentService.hpp"
#include "StarDesktopService.hpp"
#include "StarImage.hpp"
#include "StarRect.hpp"
import star.application_controller;
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarVariant.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
#include "StarBiMap.hpp"
#include "StarRefPtr.hpp"
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
#include "StarPoly.hpp"
#include "StarJson.hpp"
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
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
