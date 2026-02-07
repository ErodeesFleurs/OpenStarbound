#include "StarInteractionTypes.hpp"

#include "StarDataStreamExtra.hpp"

import std;

namespace Star {

auto operator>>(DataStream& ds, InteractRequest& ir) -> DataStream& {
  ds >> ir.sourceId;
  ds >> ir.sourcePosition;
  ds >> ir.targetId;
  ds >> ir.interactPosition;
  return ds;
}

auto operator<<(DataStream& ds, InteractRequest const& ir) -> DataStream& {
  ds << ir.sourceId;
  ds << ir.sourcePosition;
  ds << ir.targetId;
  ds << ir.interactPosition;
  return ds;
}

EnumMap<InteractActionType> const InteractActionTypeNames{{InteractActionType::None, "None"},
                                                          {InteractActionType::OpenContainer, "OpenContainer"},
                                                          {InteractActionType::SitDown, "SitDown"},
                                                          {InteractActionType::OpenCraftingInterface, "OpenCraftingInterface"},
                                                          {InteractActionType::OpenSongbookInterface, "OpenSongbookInterface"},
                                                          {InteractActionType::OpenNpcCraftingInterface, "OpenNpcCraftingInterface"},
                                                          {InteractActionType::OpenMerchantInterface, "OpenMerchantInterface"},
                                                          {InteractActionType::OpenAiInterface, "OpenAiInterface"},
                                                          {InteractActionType::OpenTeleportDialog, "OpenTeleportDialog"},
                                                          {InteractActionType::ShowPopup, "ShowPopup"},
                                                          {InteractActionType::ScriptPane, "ScriptPane"},
                                                          {InteractActionType::Message, "Message"}};

InteractAction::InteractAction() {
  type = InteractActionType::None;
  entityId = NullEntityId;
}

InteractAction::InteractAction(InteractActionType type, EntityId entityId, Json data)
    : type(type), entityId(entityId), data(std::move(data)) {}

InteractAction::InteractAction(String const& typeName, EntityId entityId, Json data)
    : type(InteractActionTypeNames.getLeft(typeName)), entityId(entityId), data(std::move(data)) {}

InteractAction::operator bool() const {
  return type != InteractActionType::None;
}

auto operator>>(DataStream& ds, InteractAction& ir) -> DataStream& {
  ds.read(ir.type);
  ds.read(ir.entityId);
  ds.read(ir.data);
  return ds;
}

auto operator<<(DataStream& ds, InteractAction const& ir) -> DataStream& {
  ds.write(ir.type);
  ds.write(ir.entityId);
  ds.write(ir.data);
  return ds;
}

}// namespace Star
