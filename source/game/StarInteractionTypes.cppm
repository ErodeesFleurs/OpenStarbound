module;

#include "StarGameTypes.hpp"
#include "StarJson.hpp"

namespace Star {

struct InteractActionExceptionTag {
  static constexpr char const* name() { return "InteractActionException"; }
};
using InteractActionException = StarError<InteractActionExceptionTag, StarException>;

struct InteractRequest {
  friend DataStream& operator>>(DataStream& ds, InteractRequest& ir);
  friend DataStream& operator<<(DataStream& ds, InteractRequest const& ir);
  EntityId sourceId;
  Vec2F sourcePosition;
  EntityId targetId;
  Vec2F interactPosition;
};

DataStream& operator>>(DataStream& ds, InteractRequest& ir);
DataStream& operator<<(DataStream& ds, InteractRequest const& ir);

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
  friend DataStream& operator>>(DataStream& ds, InteractAction& ir);
  friend DataStream& operator<<(DataStream& ds, InteractAction const& ir);
  InteractAction();
  InteractAction(InteractActionType type, EntityId entityId, Json data);
  InteractAction(String const& typeName, EntityId entityId, Json data);

  explicit operator bool() const;

  InteractActionType type;
  EntityId entityId;
  Json data;
};

DataStream& operator>>(DataStream& ds, InteractAction& ir);
DataStream& operator<<(DataStream& ds, InteractAction const& ir);

}

export module star.interaction_types;

export namespace Star {
  using ::Star::InteractActionExceptionTag;
  using ::Star::InteractActionException;
  using ::Star::InteractRequest;
  using ::Star::InteractActionType;
  using ::Star::InteractActionTypeNames;
  using ::Star::InteractAction;
}
