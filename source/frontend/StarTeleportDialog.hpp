#pragma once

#include "StarAssets.hpp"
#include "StarPane.hpp"
#include "StarWarping.hpp"
#include "StarPlayerUniverseMap.hpp"
#include "StarBookmarkInterface.hpp"

namespace Star {

class UniverseClient;
using UniverseClientPtr = SharedPtr<UniverseClient>;
class PaneManager;
using PaneManagerPtr = SharedPtr<PaneManager>;

class TeleportDialog;
using TeleportDialogPtr = SharedPtr<TeleportDialog>;

class TeleportDialog : public Pane {
public:
  struct Services {
    AssetsConstPtr assets;
    GuiContext& guiContext;
  };

  TeleportDialog(UniverseClientPtr client,
      PaneManager& paneManager,
      Json config,
      EntityId sourceEntityId,
      TeleportBookmark currentLocation,
      Services services);

  void tick(float dt) override;

  void selectDestination();
  void teleport();
  void editBookmark();

private:
  EntityId m_sourceEntityId;
  UniverseClientPtr m_client;
  PaneManager& m_paneManager;
  AssetsConstPtr m_assets;
  List<pair<WarpAction, bool>> m_destinations;
  TeleportBookmark m_currentLocation;
};

}
