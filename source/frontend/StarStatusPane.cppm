module;

#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
#include "StarJson.hpp"
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

#include "StarOrderedMap.hpp"




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
import star.pane;
import star.game_timers;
import star.pane_manager;
import star.registered_pane_manager;
import star.main_interface_types;

namespace Star {

STAR_CLASS(Player);
STAR_CLASS(UniverseClient);
STAR_CLASS(StatusPane);

class StatusPane : public Pane {
public:
  StatusPane(MainInterfacePaneManager* paneManager, UniverseClientPtr client);

  virtual PanePtr createTooltip(Vec2I const& screenPosition) override;

protected:
  virtual void renderImpl() override;
  virtual void update(float dt) override;

private:
  struct StatusEffectIndicator {
    String icon;
    Maybe<float> durationPercentage;
    String label;
    RectF screenRect;
  };

  MainInterfacePaneManager* m_paneManager;
  UniverseClientPtr m_client;
  PlayerPtr m_player;

  GuiContext* m_guiContext;
  List<StatusEffectIndicator> m_statusIndicators;
};

}

export module star.status_pane;

export namespace Star {
  using ::Star::Player;
  using ::Star::PlayerPtr;
  using ::Star::PlayerConstPtr;
  using ::Star::PlayerWeakPtr;
  using ::Star::PlayerConstWeakPtr;
  using ::Star::PlayerUPtr;
  using ::Star::PlayerConstUPtr;
  using ::Star::UniverseClient;
  using ::Star::UniverseClientPtr;
  using ::Star::UniverseClientConstPtr;
  using ::Star::UniverseClientWeakPtr;
  using ::Star::UniverseClientConstWeakPtr;
  using ::Star::UniverseClientUPtr;
  using ::Star::UniverseClientConstUPtr;
  using ::Star::StatusPane;
  using ::Star::StatusPanePtr;
  using ::Star::StatusPaneConstPtr;
  using ::Star::StatusPaneWeakPtr;
  using ::Star::StatusPaneConstWeakPtr;
  using ::Star::StatusPaneUPtr;
  using ::Star::StatusPaneConstUPtr;
}
