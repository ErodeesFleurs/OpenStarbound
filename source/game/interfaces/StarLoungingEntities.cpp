import std;
import star.set;
import star.json;
import star.id_map;
import star.string;
import star.data_stream;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
#include "StarCasting.hpp"
import star.vector;
import star.game_types;
#include "StarStrongTypedef.hpp"
import star.bi_map;
import star.variant;
import star.rpc_promise;
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

extern EnumMap<LoungeOrientation> const LoungeOrientationNames{{LoungeOrientation::None, "none"},
    {LoungeOrientation::Sit, "sit"},
    {LoungeOrientation::Lay, "lay"},
    {LoungeOrientation::Stand, "stand"}};

extern EnumMap<LoungeControl> const LoungeControlNames{{LoungeControl::Left, "Left"},
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

std::shared_ptr<EntityAnchor const> LoungeableEntity::anchor(std::size_t anchorPositionIndex) const {
  return loungeAnchor(anchorPositionIndex);
}

void LoungeableEntity::loungeControl(std::size_t, LoungeControl) {}

void LoungeableEntity::loungeAim(std::size_t, Vec2F const&) {}

Set<EntityId> LoungeableEntity::entitiesLoungingIn(std::size_t positionIndex) const {
  Set<EntityId> loungingInEntities;
  for (auto const& p : entitiesLounging()) {
    if (p.second == positionIndex)
      loungingInEntities.add(p.first);
  }
  return loungingInEntities;
}

Set<std::pair<EntityId, std::size_t>> LoungeableEntity::entitiesLounging() const {
  Set<std::pair<EntityId, std::size_t>> loungingInEntities;
  world()->forEachEntity(metaBoundBox().translated(position()),
      [&](std::shared_ptr<Entity> const& entity) {
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
