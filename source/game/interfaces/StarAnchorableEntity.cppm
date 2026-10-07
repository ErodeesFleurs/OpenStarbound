module;
#include "StarIdMap.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarJson.hpp"
#include "StarGameTypes.hpp"
#include "StarPoly.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;

import star.light_source;

import star.entity;

namespace Star {

STAR_STRUCT(EntityAnchor);

struct EntityAnchor {
  virtual ~EntityAnchor() = default;

  Vec2F position;
  // If set, the entity should place the bottom center of its collision poly on
  // the given position at exit
  Maybe<Vec2F> exitBottomPosition;
  Direction direction;
  float angle;
};

struct EntityAnchorState {
  friend DataStream& operator>>(DataStream& ds, EntityAnchorState& anchorState);
  friend DataStream& operator<<(DataStream& ds, EntityAnchorState const& anchorState);
  EntityId entityId;
  size_t positionIndex;

  bool operator==(EntityAnchorState const& eas) const;
};

DataStream& operator>>(DataStream& ds, EntityAnchorState& anchorState);
DataStream& operator<<(DataStream& ds, EntityAnchorState const& anchorState);

class AnchorableEntity : public virtual Entity {
public:
  virtual size_t anchorCount() const = 0;
  virtual EntityAnchorConstPtr anchor(size_t anchorPositionIndex) const = 0;
};

}

export module star.anchorable_entity;

export namespace Star {
  using ::Star::EntityAnchor;
  using ::Star::EntityAnchorPtr;
  using ::Star::EntityAnchorConstPtr;
  using ::Star::EntityAnchorWeakPtr;
  using ::Star::EntityAnchorConstWeakPtr;
  using ::Star::EntityAnchorUPtr;
  using ::Star::EntityAnchorConstUPtr;
  using ::Star::EntityAnchorState;
  using ::Star::AnchorableEntity;
}
