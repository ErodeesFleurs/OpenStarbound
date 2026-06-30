#include "StarTileEntity.hpp"
#include "StarWorld.hpp"
#include "StarRoot.hpp"
#include "StarLiquidsDatabase.hpp"
#include "StarDataStreamExtra.hpp"

namespace Star {

DataStream& operator<<(DataStream& ds, MaterialSpace const& materialSpace) {
  ds.write(materialSpace.space);
  ds.write(materialSpace.material);
  return ds;
}

DataStream& operator>>(DataStream& ds, MaterialSpace& materialSpace) {
  ds.read(materialSpace.space);
  ds.read(materialSpace.material);
  return ds;
}

TileEntity::TileEntity() {
  setPersistent(true);
}

[[nodiscard]] Vec2F TileEntity::position() const {
  return Vec2F(tilePosition());
}

[[nodiscard]] List<Vec2I> TileEntity::spaces() const {
  return {};
}

[[nodiscard]] List<Vec2I> TileEntity::roots() const {
  return {};
}

[[nodiscard]] List<MaterialSpace> TileEntity::materialSpaces() const {
  return {};
}

bool TileEntity::damageTiles(List<Vec2I> const&, Vec2F const&, TileDamage const&) {
  return false;
}

[[nodiscard]] bool TileEntity::canBeDamaged() const {
  return true;
}

[[nodiscard]] bool TileEntity::isInteractive() const {
  return false;
}

[[nodiscard]] List<Vec2I> TileEntity::interactiveSpaces() const {
  return spaces();
}

InteractAction TileEntity::interact([[maybe_unused]] InteractRequest const& request) {
  return InteractAction();
}

[[nodiscard]] List<QuestArcDescriptor> TileEntity::offeredQuests() const {
  return {};
}

[[nodiscard]] StringSet TileEntity::turnInQuests() const {
  return StringSet();
}

[[nodiscard]] Vec2F TileEntity::questIndicatorPosition() const {
  return position();
}

[[nodiscard]] bool TileEntity::anySpacesOccupied(List<Vec2I> const& spaces) const {
  Vec2I tp = tilePosition();
  for (auto pos : spaces) {
    pos += tp;
    if (isConnectableMaterial(world().material(pos, TileLayer::Foreground)))
      return true;
  }

  return false;
}

[[nodiscard]] bool TileEntity::allSpacesOccupied(List<Vec2I> const& spaces) const {
  Vec2I tp = tilePosition();
  for (auto pos : spaces) {
    pos += tp;
    if (!isConnectableMaterial(world().material(pos, TileLayer::Foreground)))
      return false;
  }

  return true;
}

[[nodiscard]] float TileEntity::spacesLiquidFillLevel(List<Vec2I> const& relativeSpaces) const {
  float total = 0.0f;
  for (auto pos : relativeSpaces) {
    pos += tilePosition();
    total += world().liquidLevel(pos).level;
  }
  return total / relativeSpaces.size();
}

}
