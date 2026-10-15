module;
import std;
import star.maybe;
import star.json;
import star.id_map;
import star.casting;
import star.vector;
import star.data_stream;
import star.game_types;
import star.strong_typedef;
import star.bi_map;

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;

import star.light_source;

import star.entity;

namespace Star {

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
  std::size_t positionIndex;

  bool operator==(EntityAnchorState const& eas) const;
};

DataStream& operator>>(DataStream& ds, EntityAnchorState& anchorState);
DataStream& operator<<(DataStream& ds, EntityAnchorState const& anchorState);

class AnchorableEntity : public virtual Entity {
public:
  virtual std::size_t anchorCount() const = 0;
  virtual std::shared_ptr<EntityAnchor const> anchor(std::size_t anchorPositionIndex) const = 0;
};

}

export module star.anchorable_entity;

export namespace Star {
  using ::Star::EntityAnchor;
  using ::Star::EntityAnchorState;
  using ::Star::AnchorableEntity;
}
