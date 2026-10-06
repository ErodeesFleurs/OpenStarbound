#pragma once

#include "StarPane.hpp"
#include "StarStrongTypedef.hpp"
#include "StarUuid.hpp"
#include "StarJson.hpp"
#include "StarVector.hpp"
import star.celestial_coordinate;
#include "StarGameTypes.hpp"
import star.warping;
#include "StarBookmarkInterface.hpp"
import star.player_universe_map;

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
