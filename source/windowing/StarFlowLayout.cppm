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

// Super simple, only supports left to right, top to bottom flow layouts
// currently
STAR_CLASS(FlowLayout);
class FlowLayout : public Layout {
public:
  FlowLayout();
  virtual void update(float dt) override;
  void setSpacing(Vec2I const& spacing);
  void setWrapping(bool wrap);

private:
  Vec2I m_spacing;
  bool m_wrap;
};

}

export module star.flow_layout;

export namespace Star {
  using ::Star::FlowLayout;
  using ::Star::FlowLayoutPtr;
  using ::Star::FlowLayoutConstPtr;
  using ::Star::FlowLayoutWeakPtr;
  using ::Star::FlowLayoutConstWeakPtr;
  using ::Star::FlowLayoutUPtr;
  using ::Star::FlowLayoutConstUPtr;
}
