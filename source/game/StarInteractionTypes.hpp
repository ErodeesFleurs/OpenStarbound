#pragma once

#include "StarException.hpp"
#include "StarGameTypes.hpp"
#include "StarJson.hpp"

namespace Star {

using InteractActionException = ExceptionDerived<"InteractActionException">;

struct InteractRequest {
  EntityId sourceId;
  Vec2F sourcePosition;
  EntityId targetId;
  Vec2F interactPosition;
};

auto operator>>(DataStream& ds, InteractRequest& ir) -> DataStream&;
auto operator<<(DataStream& ds, InteractRequest const& ir) -> DataStream&;

enum class InteractActionType {
  None,
  OpenContainer,
  SitDown,
  OpenCraftingInterface,
  OpenSongbookInterface,
  OpenNpcCraftingInterface,
  OpenMerchantInterface,
  OpenAiInterface,
  OpenTeleportDialog,
  ShowPopup,
  ScriptPane,
  Message
};
extern EnumMap<InteractActionType> const InteractActionTypeNames;

struct InteractAction {
  InteractAction();
  InteractAction(InteractActionType type, EntityId entityId, Json data);
  InteractAction(String const& typeName, EntityId entityId, Json data);

  explicit operator bool() const;

  InteractActionType type;
  EntityId entityId;
  Json data;
};

auto operator>>(DataStream& ds, InteractAction& ir) -> DataStream&;
auto operator<<(DataStream& ds, InteractAction const& ir) -> DataStream&;

}
