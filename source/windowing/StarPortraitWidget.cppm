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

namespace Star {

STAR_CLASS(PortraitEntity);

STAR_CLASS(Player);
STAR_CLASS(PortraitWidget);

class PortraitWidget : public Widget {
public:
  PortraitWidget(PortraitEntityPtr entity, PortraitMode mode = PortraitMode::Full);
  PortraitWidget();
  virtual ~PortraitWidget() {}

  void setEntity(PortraitEntityPtr entity);
  void setMode(PortraitMode mode);
  void setScale(float scale);
  void setIconMode();
  void setRenderHumanoid(bool);
  bool sendEvent(InputEvent const& event);

protected:
  virtual RectI getScissorRect() const;
  virtual void renderImpl();

private:
  void init();
  void updateSize();

  PortraitEntityPtr m_entity;
  PortraitMode m_portraitMode;
  AssetPath m_noEntityImageFull;
  AssetPath m_noEntityImagePart;
  float m_scale;

  bool m_renderHumanoid;
  bool m_iconMode;
  AssetPath m_iconImage;
  Vec2I m_iconOffset;
};

}

export module star.portrait_widget;

export namespace Star {
  using ::Star::PortraitWidget;
  using ::Star::Player;
  using ::Star::PlayerPtr;
  using ::Star::PlayerConstPtr;
  using ::Star::PlayerWeakPtr;
  using ::Star::PlayerConstWeakPtr;
  using ::Star::PlayerUPtr;
  using ::Star::PlayerConstUPtr;
  using ::Star::PortraitWidgetPtr;
  using ::Star::PortraitWidgetConstPtr;
  using ::Star::PortraitWidgetWeakPtr;
  using ::Star::PortraitWidgetConstWeakPtr;
  using ::Star::PortraitWidgetUPtr;
  using ::Star::PortraitWidgetConstUPtr;
  using ::Star::PortraitEntity;
  using ::Star::PortraitEntityPtr;
  using ::Star::PortraitEntityConstPtr;
  using ::Star::PortraitEntityWeakPtr;
  using ::Star::PortraitEntityConstWeakPtr;
  using ::Star::PortraitEntityUPtr;
  using ::Star::PortraitEntityConstUPtr;
}
