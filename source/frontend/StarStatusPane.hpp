#pragma once

#include "StarIAssets.hpp"
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
  IAssetsConstPtr assets;
  ImageMetadataDatabaseConstPtr imageMetadataDatabase;
  StatusEffectDatabaseConstPtr statusEffectDatabase;
};

class StatusPane : public Pane {
public:
  StatusPane(UniverseClientPtr client, StatusPaneServices services);

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

  UniverseClientPtr m_client;
  PlayerPtr m_player;
  IAssetsConstPtr m_assets;
  ImageMetadataDatabaseConstPtr m_imageMetadataDatabase;
  StatusEffectDatabaseConstPtr m_statusEffectDatabase;

  GuiContext* m_guiContext;
  List<StatusEffectIndicator> m_statusIndicators;
};

}
