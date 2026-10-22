module;
import std;
import star.string;
import star.data_stream;
import star.format;
import star.json;

import star.bi_map;
import star.vector;
import star.game_types;

namespace Star {

struct InteractActionExceptionTag {
  static constexpr char const* name() { return "InteractActionException"; }
};
using InteractActionException = StarError<InteractActionExceptionTag, StarException>;

struct InteractRequest {
  friend std::expected<DataStream*, StarException> operator>>(DataStream& ds, InteractRequest& ir);
  friend std::expected<DataStream*, StarException> operator<<(DataStream& ds, InteractRequest const& ir);
  EntityId sourceId;
  Vec2F sourcePosition;
  EntityId targetId;
  Vec2F interactPosition;
};

[[nodiscard]] std::expected<DataStream*, StarException> operator>>(DataStream& ds, InteractRequest& ir);
[[nodiscard]] std::expected<DataStream*, StarException> operator<<(DataStream& ds, InteractRequest const& ir);

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
extern std::expected<EnumMap<InteractActionType>, StarException> const InteractActionTypeNames;

struct InteractAction {
  friend std::expected<DataStream*, StarException> operator>>(DataStream& ds, InteractAction& ir);
  friend std::expected<DataStream*, StarException> operator<<(DataStream& ds, InteractAction const& ir);
  InteractAction();
  InteractAction(InteractActionType type, EntityId entityId, Json data);
  [[nodiscard]] static std::expected<InteractAction, StarException> create(String const& typeName, EntityId entityId, Json data);

  explicit operator bool() const;

  InteractActionType type;
  EntityId entityId;
  Json data;
};

[[nodiscard]] std::expected<DataStream*, StarException> operator>>(DataStream& ds, InteractAction& ir);
[[nodiscard]] std::expected<DataStream*, StarException> operator<<(DataStream& ds, InteractAction const& ir);

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
