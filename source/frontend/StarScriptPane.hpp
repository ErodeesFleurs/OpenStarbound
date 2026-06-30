#pragma once

#include "StarBaseScriptPane.hpp"

namespace Star {

class CanvasWidget;
using CanvasWidgetPtr = SharedPtr<CanvasWidget>;
class ScriptPane;
using ScriptPanePtr = SharedPtr<ScriptPane>;
class UniverseClient;
using UniverseClientPtr = SharedPtr<UniverseClient>;

class ScriptPane : public BaseScriptPane {
public:
  ScriptPane(UniverseClientPtr client, Json config, EntityId sourceEntityId = NullEntityId);

  void displayed() override;
  void dismissed() override;

  void tick(float dt) override;

  PanePtr createTooltip(Vec2I const& screenPosition) override;

  bool openWithInventory() const;
  bool closeWithInventory() const;

  EntityId sourceEntityId() const;

  LuaCallbacks makePaneCallbacks() override;
private:
  UniverseClientPtr m_client;
  EntityId m_sourceEntityId;
};

}
