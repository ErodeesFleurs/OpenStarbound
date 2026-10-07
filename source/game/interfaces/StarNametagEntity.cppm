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

STAR_CLASS(NametagEntity);

class NametagEntity : public virtual Entity {
public:
  virtual String nametag() const = 0;
  virtual Maybe<String> statusText() const = 0;
  virtual bool displayNametag() const = 0;
  virtual Vec3B nametagColor() const = 0;
  virtual Vec2F nametagOrigin() const = 0;
};

}

export module star.nametag_entity;

export namespace Star {
  using ::Star::NametagEntity;
  using ::Star::NametagEntityPtr;
  using ::Star::NametagEntityConstPtr;
  using ::Star::NametagEntityWeakPtr;
  using ::Star::NametagEntityConstWeakPtr;
  using ::Star::NametagEntityUPtr;
  using ::Star::NametagEntityConstUPtr;
}
