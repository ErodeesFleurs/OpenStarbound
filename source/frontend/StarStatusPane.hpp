#pragma once

#include "StarAssets.hpp"
#include "StarImageMetadataDatabase.hpp"
#include "StarPane.hpp"
#include "StarMainInterfaceTypes.hpp"
#include "StarStatusEffectDatabase.hpp"

namespace Star {

class Player;
using PlayerPtr = SharedPtr<Player>;
class UniverseClient;
using UniverseClientPtr = SharedPtr<UniverseClient>;
class StatusPane;
using StatusPanePtr = SharedPtr<StatusPane>;

struct StatusPaneServices {
  AssetsConstPtr assets;
  ImageMetadataDatabaseConstPtr imageMetadataDatabase;
  StatusEffectDatabaseConstPtr statusEffectDatabase;
  GuiContext& guiContext;
};

class StatusPane : public Pane {
public:
  StatusPane(UniverseClientPtr client, StatusPaneServices services);

  PanePtr createTooltip(Vec2I const& screenPosition) override;

protected:
  void renderImpl() override;
  void update(float dt) override;

private:
  struct StatusEffectIndicator {
    String icon;
    Maybe<float> durationPercentage;
    String label;
    RectF screenRect;
  };

  UniverseClientPtr m_client;
  PlayerPtr m_player;
  AssetsConstPtr m_assets;
  ImageMetadataDatabaseConstPtr m_imageMetadataDatabase;
  StatusEffectDatabaseConstPtr m_statusEffectDatabase;

  GuiContext& m_guiContext;
  List<StatusEffectIndicator> m_statusIndicators;
};

}
