module;
#include "StarIdMap.hpp"
#include "StarGameTypes.hpp"
#include "StarPoly.hpp"
#include "StarDataStream.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarJson.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"
#include "StarNetElementSystem.hpp"
#include "StarList.hpp"

import star.world_geometry;
import star.wiring;

import star.damage_types;
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

namespace Star {

STAR_CLASS(WireEntity);

class WireEntity : public virtual TileEntity {
public:
  virtual ~WireEntity() {}

  virtual size_t nodeCount(WireDirection direction) const = 0;
  virtual Vec2I nodePosition(WireNode wireNode) const = 0;
  virtual List<WireConnection> connectionsForNode(WireNode wireNode) const = 0;
  virtual bool nodeState(WireNode wireNode) const = 0;

  virtual Color nodeColor(WireNode wireNode) const = 0;
  virtual String nodeIcon(WireNode wireNode) const = 0;

  virtual void addNodeConnection(WireNode wireNode, WireConnection nodeConnection) = 0;
  virtual void removeNodeConnection(WireNode wireNode, WireConnection nodeConnection) = 0;

  virtual void evaluate(WireCoordinator* coordinator) = 0;
};

}

export module star.wire_entity;

export namespace Star {
  using ::Star::WireEntity;
  using ::Star::WireEntityPtr;
  using ::Star::WireEntityConstPtr;
  using ::Star::WireEntityWeakPtr;
  using ::Star::WireEntityConstWeakPtr;
  using ::Star::WireEntityUPtr;
  using ::Star::WireEntityConstUPtr;
}
