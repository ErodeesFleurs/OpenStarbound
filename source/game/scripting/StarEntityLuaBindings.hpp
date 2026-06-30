#pragma once

#include "StarLua.hpp"
#include "StarEntity.hpp"

namespace Star {

namespace LuaBindings {
  [[nodiscard]] LuaCallbacks makeEntityCallbacks(Entity const& entity);

  namespace EntityCallbacks {
    [[nodiscard]] EntityId id(Entity const& entity);
    [[nodiscard]] LuaTable damageTeam(Entity const& entity, LuaEngine& engine);
    [[nodiscard]] bool isValidTarget(Entity const& entity, EntityId entityId);
    [[nodiscard]] Vec2F distanceToEntity(Entity const& entity, EntityId entityId);
    [[nodiscard]] bool entityInSight(Entity const& entity, EntityId entityId);
  }
}
}
