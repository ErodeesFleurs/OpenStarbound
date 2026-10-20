import std;
import star.bi_map;
import star.math_common;
import star.format;
import star.game_types;

namespace Star {


extern EnumMap<Direction> const DirectionNames{
  {Direction::Left, "left"},
  {Direction::Right, "right"},
};

extern EnumMap<Gender> const GenderNames{
  {Gender::Male, "male"},
  {Gender::Female, "female"},
};

extern EnumMap<FireMode> const FireModeNames{
  {FireMode::None, "none"},
  {FireMode::Primary, "primary"},
  {FireMode::Alt, "alt"}
};

extern EnumMap<ToolHand> const ToolHandNames{
  {ToolHand::Primary, "primary"},
  {ToolHand::Alt, "alt"}
};

extern EnumMap<TileLayer> const TileLayerNames{
  {TileLayer::Foreground, "foreground"},
  {TileLayer::Background, "background"}
};

extern EnumMap<MoveControlType> const MoveControlTypeNames{
  {MoveControlType::Left, "left"},
  {MoveControlType::Right, "right"},
  {MoveControlType::Down, "down"},
  {MoveControlType::Up, "up"},
  {MoveControlType::Jump, "jump"}
};

extern EnumMap<PortraitMode> const PortraitModeNames{
  {PortraitMode::Head, "head"},
  {PortraitMode::Bust, "bust"},
  {PortraitMode::Full, "full"},
  {PortraitMode::FullNeutral, "fullneutral"},
  {PortraitMode::FullNude, "fullnude"},
  {PortraitMode::FullNeutralNude, "fullneutralnude"}
};

extern EnumMap<Rarity> const RarityNames{
  {Rarity::Common, "common"},
  {Rarity::Uncommon, "uncommon"},
  {Rarity::Rare, "rare"},
  {Rarity::Legendary, "legendary"},
  {Rarity::Essential, "essential"}
};

std::pair<EntityId, EntityId> connectionEntitySpace(ConnectionId connectionId) {
  if (connectionId == ServerConnectionId) {
    return {MinServerEntityId, MaxServerEntityId};
  } else if (connectionId >= MinClientConnectionId && connectionId <= MaxClientSubWorldConnectionId) {
    EntityId beginIdSpace = (EntityId)connectionId * -65536;
    EntityId endIdSpace = beginIdSpace + 65535;
    return {beginIdSpace, endIdSpace};
  } else {
    throw StarException::format("Invalid connection id in clientEntitySpace({})", connectionId);
  }
}

bool entityIdInSpace(EntityId entityId, ConnectionId connectionId) {
  auto pair = connectionEntitySpace(connectionId);
  return entityId >= pair.first && entityId <= pair.second;
}

ConnectionId connectionForEntity(EntityId entityId) {
  if (entityId > 0)
    return ServerConnectionId;
  else
    return (-entityId - 1) / 65536 + 1;
}

std::pair<float, Direction> getAngleSide(float angle, bool ccRotation) {
  angle = constrainAngle(angle);
  Direction direction = Direction::Right;
  if (angle > std::numbers::pi / 2) {
    direction = Direction::Left;
    angle = std::numbers::pi - angle;
  } else if (angle < -std::numbers::pi / 2) {
    direction = Direction::Left;
    angle = -std::numbers::pi - angle;
  }

  if (direction == Direction::Left && ccRotation)
    angle *= -1;

  return std::make_pair(angle, direction);
}

}
