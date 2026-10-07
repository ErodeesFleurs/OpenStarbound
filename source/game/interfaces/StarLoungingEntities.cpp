#include "StarIdMap.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarJson.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarGameTypes.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"
#include "StarVariant.hpp"
#include "StarNetElementSystem.hpp"
#include "StarList.hpp"
#include "StarRpcPromise.hpp"
#include "StarLuaRoot.hpp"
import star.drawable;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.anchorable_entity;
import star.entity_rendering_types;
import star.lounging_entities;
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

namespace Star {

EnumMap<LoungeOrientation> const LoungeOrientationNames{{LoungeOrientation::None, "none"},
    {LoungeOrientation::Sit, "sit"},
    {LoungeOrientation::Lay, "lay"},
    {LoungeOrientation::Stand, "stand"}};

EnumMap<LoungeControl> const LoungeControlNames{{LoungeControl::Left, "Left"},
    {LoungeControl::Right, "Right"},
    {LoungeControl::Down, "Down"},
    {LoungeControl::Up, "Up"},
    {LoungeControl::Jump, "Jump"},
    {LoungeControl::PrimaryFire, "PrimaryFire"},
    {LoungeControl::AltFire, "AltFire"},
    {LoungeControl::Special1, "Special1"},
    {LoungeControl::Special2, "Special2"},
    {LoungeControl::Special3, "Special3"},
    {LoungeControl::Walk, "Walk"},
};

EntityAnchorConstPtr LoungeableEntity::anchor(size_t anchorPositionIndex) const {
  return loungeAnchor(anchorPositionIndex);
}

void LoungeableEntity::loungeControl(size_t, LoungeControl) {}

void LoungeableEntity::loungeAim(size_t, Vec2F const&) {}

Set<EntityId> LoungeableEntity::entitiesLoungingIn(size_t positionIndex) const {
  Set<EntityId> loungingInEntities;
  for (auto const& p : entitiesLounging()) {
    if (p.second == positionIndex)
      loungingInEntities.add(p.first);
  }
  return loungingInEntities;
}

Set<pair<EntityId, size_t>> LoungeableEntity::entitiesLounging() const {
  Set<pair<EntityId, size_t>> loungingInEntities;
  world()->forEachEntity(metaBoundBox().translated(position()),
      [&](EntityPtr const& entity) {
        if (auto lounger = as<LoungingEntity>(entity)) {
          if (auto anchorStatus = lounger->loungingIn()) {
            if (anchorStatus->entityId == entityId())
              loungingInEntities.add({entity->entityId(), anchorStatus->positionIndex});
          }
        }
      });
  return loungingInEntities;
}

bool LoungingEntity::inConflictingLoungeAnchor() const {
  if (auto loungeAnchorState = loungingIn()) {
    if (auto loungeableEntity = world()->get<LoungeableEntity>(loungeAnchorState->entityId)) {
      auto entitiesLoungingIn = loungeableEntity->entitiesLoungingIn(loungeAnchorState->positionIndex);
      return entitiesLoungingIn.size() > 1 || !entitiesLoungingIn.contains(entityId());
    }
  }
  return false;
}

}
