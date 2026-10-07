module;
#include "StarJson.hpp"
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarStrongTypedef.hpp"
#include "StarArray.hpp"
#include "StarVariant.hpp"
#include "StarWeightedPool.hpp"
#include "StarEither.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarNetElementSystem.hpp"



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
