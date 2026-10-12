module;
#include "StarJson.hpp"
#include "StarCasting.hpp"
import star.input_event;
import star.color;
#include "StarList.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
#include "StarBiMap.hpp"
#include "StarString.hpp"
import star.text;
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
import star.game_types;
#include "StarSet.hpp"
import star.audio;
#include "StarMap.hpp"
#include "StarStrongTypedef.hpp"
#include "StarVariant.hpp"
import star.random;
import star.weighted_pool;
import star.either;



// Parse foundations before importing global bookmark value types on GCC.
import star.sky_types;
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.sky_parameters;
import star.system_world;




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
import star.uuid;
import star.celestial_coordinate;
import star.warping;
import star.player_universe_map;
import star.bookmark_interface;

namespace Star {

STAR_CLASS(UniverseClient);
STAR_CLASS(PaneManager);

class TeleportDialog : public Pane {
public:
  TeleportDialog(UniverseClientPtr client,
      PaneManager* paneManager,
      Json config,
      EntityId sourceEntityId,
      TeleportBookmark currentLocation);

  void tick(float dt) override;

  void selectDestination();
  void teleport();
  void editBookmark();

private:
  EntityId m_sourceEntityId;
  UniverseClientPtr m_client;
  PaneManager* m_paneManager;
  List<pair<WarpAction, bool>> m_destinations;
  TeleportBookmark m_currentLocation;
};

}

export module star.teleport_dialog;

export namespace Star {
  using ::Star::UniverseClient;
  using ::Star::UniverseClientPtr;
  using ::Star::UniverseClientConstPtr;
  using ::Star::UniverseClientWeakPtr;
  using ::Star::UniverseClientConstWeakPtr;
  using ::Star::UniverseClientUPtr;
  using ::Star::UniverseClientConstUPtr;
  using ::Star::PaneManager;
  using ::Star::PaneManagerPtr;
  using ::Star::PaneManagerConstPtr;
  using ::Star::PaneManagerWeakPtr;
  using ::Star::PaneManagerConstWeakPtr;
  using ::Star::PaneManagerUPtr;
  using ::Star::PaneManagerConstUPtr;
  using ::Star::TeleportDialog;
}
