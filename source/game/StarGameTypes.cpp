import std;
import star.bi_map;
import star.math_common;
import star.format;
import star.game_types;

namespace Star {


extern std::expected<EnumMap<Direction>, StarException> const DirectionNames = EnumMap<Direction>::create({
  {Direction::Left, "left"},
  {Direction::Right, "right"},
});

extern std::expected<EnumMap<Gender>, StarException> const GenderNames = EnumMap<Gender>::create({
  {Gender::Male, "male"},
  {Gender::Female, "female"},
});

extern std::expected<EnumMap<FireMode>, StarException> const FireModeNames = EnumMap<FireMode>::create({
  {FireMode::None, "none"},
  {FireMode::Primary, "primary"},
  {FireMode::Alt, "alt"}
});

extern std::expected<EnumMap<ToolHand>, StarException> const ToolHandNames = EnumMap<ToolHand>::create({
  {ToolHand::Primary, "primary"},
  {ToolHand::Alt, "alt"}
});

extern std::expected<EnumMap<TileLayer>, StarException> const TileLayerNames = EnumMap<TileLayer>::create({
  {TileLayer::Foreground, "foreground"},
  {TileLayer::Background, "background"}
});

extern std::expected<EnumMap<MoveControlType>, StarException> const MoveControlTypeNames = EnumMap<MoveControlType>::create({
  {MoveControlType::Left, "left"},
  {MoveControlType::Right, "right"},
  {MoveControlType::Down, "down"},
  {MoveControlType::Up, "up"},
  {MoveControlType::Jump, "jump"}
});

extern std::expected<EnumMap<PortraitMode>, StarException> const PortraitModeNames = EnumMap<PortraitMode>::create({
  {PortraitMode::Head, "head"},
  {PortraitMode::Bust, "bust"},
  {PortraitMode::Full, "full"},
  {PortraitMode::FullNeutral, "fullneutral"},
  {PortraitMode::FullNude, "fullnude"},
  {PortraitMode::FullNeutralNude, "fullneutralnude"}
});

extern std::expected<EnumMap<Rarity>, StarException> const RarityNames = EnumMap<Rarity>::create({
  {Rarity::Common, "common"},
  {Rarity::Uncommon, "uncommon"},
  {Rarity::Rare, "rare"},
  {Rarity::Legendary, "legendary"},
  {Rarity::Essential, "essential"}
});

std::expected<std::pair<EntityId, EntityId>, StarException> connectionEntitySpace(ConnectionId connectionId) {
  if (connectionId == ServerConnectionId) {
    return std::pair{MinServerEntityId, MaxServerEntityId};
  } else if (connectionId >= MinClientConnectionId && connectionId <= MaxClientSubWorldConnectionId) {
    EntityId beginIdSpace = (EntityId)connectionId * -65536;
    EntityId endIdSpace = beginIdSpace + 65535;
    return std::pair{beginIdSpace, endIdSpace};
  } else {
    return std::unexpected(StarException::format("Invalid connection id in clientEntitySpace({})", connectionId));
  }
}

std::expected<bool, StarException> entityIdInSpace(EntityId entityId, ConnectionId connectionId) {
  auto pair = connectionEntitySpace(connectionId);
  if (!pair) return std::unexpected(std::move(pair.error()));
  return entityId >= pair->first && entityId <= pair->second;
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
