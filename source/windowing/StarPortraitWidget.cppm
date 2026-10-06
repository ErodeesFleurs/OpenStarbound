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
