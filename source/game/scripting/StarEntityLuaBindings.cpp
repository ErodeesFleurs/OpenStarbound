#include "StarEntityLuaBindings.hpp"
#include "StarJsonExtra.hpp"
#include "StarLuaGameConverters.hpp"
#include "StarPlayer.hpp"
#include "StarMonster.hpp"
#include "StarNpc.hpp"
#include "StarWorld.hpp"

namespace Star {

LuaCallbacks LuaBindings::makeEntityCallbacks(Entity const* entity) {
  LuaCallbacks callbacks;

  callbacks.registerCallbackWithSignature<EntityId>("id", [entity]() { return EntityCallbacks::id(entity); });
  callbacks.registerCallbackWithSignature<LuaTable, LuaEngine&>(
      "damageTeam", [entity](LuaEngine& engine) { return EntityCallbacks::damageTeam(entity, engine); });
  callbacks.registerCallbackWithSignature<bool, EntityId>(
      "isValidTarget", [entity](EntityId targetId) { return EntityCallbacks::isValidTarget(entity, targetId); });
  callbacks.registerCallbackWithSignature<Vec2F, EntityId>(
      "distanceToEntity", [entity](EntityId targetId) { return EntityCallbacks::distanceToEntity(entity, targetId); });
  callbacks.registerCallbackWithSignature<bool, EntityId>(
      "entityInSight", [entity](EntityId targetId) { return EntityCallbacks::entityInSight(entity, targetId); });

  callbacks.registerCallback("position", [entity]() { return entity->position(); });
  callbacks.registerCallback("entityType", [entity]() { return EntityTypeNames.getRight(entity->entityType()); });
  callbacks.registerCallback("uniqueId", [entity]() { return entity->uniqueId(); });
  callbacks.registerCallback("persistent", [entity]() { return entity->persistent(); });

  return callbacks;
}

EntityId LuaBindings::EntityCallbacks::id(Entity const* entity) {
  return entity->entityId();
}

LuaTable LuaBindings::EntityCallbacks::damageTeam(Entity const* entity, LuaEngine& engine) {
  auto table = engine.createTable();
  auto team = entity->getTeam();
  table.set("type", TeamTypeNames.getRight(team.type));
  table.set("team", team.team);
  return table;
}

bool LuaBindings::EntityCallbacks::isValidTarget(Entity const* entity, EntityId entityId) {
  auto target = entity->world()->entity(entityId);

  if (!target || !entity->getTeam().canDamage(target->getTeam(), false))
    return false;

  if (auto monster = as<Monster>(target))
    return monster->aggressive();

  if (auto npc = as<Npc>(target)) {
    if (auto attackerNpc = as<Npc>(entity))
      return npc->aggressive() || attackerNpc->aggressive();
    return true;
  }

  return is<Player>(target);
}

Vec2F LuaBindings::EntityCallbacks::distanceToEntity(Entity const* entity, EntityId entityId) {
  Vec2F dist;
  if (auto target = entity->world()->entity(entityId))
    dist = entity->world()->geometry().diff(target->position(), entity->position());

  return dist;
}

bool LuaBindings::EntityCallbacks::entityInSight(Entity const* entity, EntityId entityId) {
  if (auto target = entity->world()->entity(entityId))
    return !entity->world()->lineTileCollision(target->position(), entity->position());
  else
    return false;
}

}
