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
  ScriptPane(UniverseClientPtr client, Json config, EntityId sourceEntityId, BaseScriptPaneServices services);

  void displayed() override;
  void dismissed() override;

  void tick(float dt) override;

  [[nodiscard]] PanePtr createTooltip(Vec2I const& screenPosition) override;

  [[nodiscard]] bool openWithInventory() const;
  [[nodiscard]] bool closeWithInventory() const;

  [[nodiscard]] EntityId sourceEntityId() const;

  [[nodiscard]] LuaCallbacks makePaneCallbacks() override;
private:
  UniverseClientPtr m_client;
  EntityId m_sourceEntityId;
};

}
