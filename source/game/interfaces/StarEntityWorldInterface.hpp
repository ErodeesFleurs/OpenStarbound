#pragma once

#include "StarTileEntity.hpp"
#include "StarInteractionTypes.hpp"
#include "StarRpcPromise.hpp"

namespace Star {

// Interface for entity queries and lifecycle.
// Extracted from World for use where only entity-level operations are needed.
class EntityWorldInterface {
public:
  virtual ~EntityWorldInterface() = default;

  virtual EntityPtr entity(EntityId entityId) const = 0;
  virtual void addEntity(EntityPtr const& entity, EntityId entityId = NullEntityId) = 0;

  virtual EntityPtr closestEntity(Vec2F const& center, float radius, EntityFilter selector = {}) const = 0;

  virtual void forAllEntities(EntityCallback entityCallback) const = 0;
  virtual void forEachEntity(RectF const& boundBox, EntityCallback entityCallback) const = 0;
  virtual void forEachEntityLine(Vec2F const& begin, Vec2F const& end, EntityCallback entityCallback) const = 0;
  virtual void forEachEntityAtTile(Vec2I const& pos, EntityCallbackOf<TileEntity> entityCallback) const = 0;

  virtual EntityPtr findEntity(RectF const& boundBox, EntityFilter entityFilter) const = 0;
  virtual EntityPtr findEntityLine(Vec2F const& begin, Vec2F const& end, EntityFilter entityFilter) const = 0;
  virtual EntityPtr findEntityAtTile(Vec2I const& pos, EntityFilterOf<TileEntity> entityFilter) const = 0;

  virtual InteractiveEntityPtr getInteractiveInRange(Vec2F const& targetPosition, Vec2F const& sourcePosition, float maxRange) const = 0;
  virtual bool canReachEntity(Vec2F const& position, float radius, EntityId targetEntity, bool preferInteractive = true) const = 0;
  virtual RpcPromise<InteractAction> interact(InteractRequest const& request) = 0;
};

}
