#include "StarJson.hpp"
#include "StarIdMap.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarGameTypes.hpp"
#include "StarPoly.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"
#include "StarNetElementSystem.hpp"
#include "StarList.hpp"
#include "StarVariant.hpp"
#include "StarRpcPromise.hpp"
#include "StarDataStreamExtra.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarThread.hpp"
#include "StarIODevice.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
import star.listener;
#include "StarConfig.hpp"
import star.version;

#include "StarLuaRoot.hpp"
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
import star.force_regions;
import star.world;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

import star.liquids_database;

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

Vec2F TileEntity::position() const {
  return Vec2F(tilePosition());
}

List<Vec2I> TileEntity::spaces() const {
  return {};
}

List<Vec2I> TileEntity::roots() const {
  return {};
}

List<MaterialSpace> TileEntity::materialSpaces() const {
  return {};
}

bool TileEntity::damageTiles(List<Vec2I> const&, Vec2F const&, TileDamage const&) {
  return false;
}

bool TileEntity::canBeDamaged() const {
  return true;
}

bool TileEntity::isInteractive() const {
  return false;
}

List<Vec2I> TileEntity::interactiveSpaces() const {
  return spaces();
}

InteractAction TileEntity::interact(InteractRequest const& request) {
  _unused(request);
  return InteractAction();
}

List<QuestArcDescriptor> TileEntity::offeredQuests() const {
  return {};
}

StringSet TileEntity::turnInQuests() const {
  return StringSet();
}

Vec2F TileEntity::questIndicatorPosition() const {
  return position();
}

bool TileEntity::anySpacesOccupied(List<Vec2I> const& spaces) const {
  Vec2I tp = tilePosition();
  for (auto pos : spaces) {
    pos += tp;
    if (isConnectableMaterial(world()->material(pos, TileLayer::Foreground)))
      return true;
  }

  return false;
}

bool TileEntity::allSpacesOccupied(List<Vec2I> const& spaces) const {
  Vec2I tp = tilePosition();
  for (auto pos : spaces) {
    pos += tp;
    if (!isConnectableMaterial(world()->material(pos, TileLayer::Foreground)))
      return false;
  }

  return true;
}

float TileEntity::spacesLiquidFillLevel(List<Vec2I> const& relativeSpaces) const {
  float total = 0.0f;
  for (auto pos : relativeSpaces) {
    pos += tilePosition();
    total += world()->liquidLevel(pos).level;
  }
  return total / relativeSpaces.size();
}

}
