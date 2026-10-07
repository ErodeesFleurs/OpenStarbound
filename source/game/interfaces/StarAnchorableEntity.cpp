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
import star.anchorable_entity;

namespace Star {

bool EntityAnchorState::operator==(EntityAnchorState const& eas) const {
  return tie(entityId, positionIndex) == tie(eas.entityId, eas.positionIndex);
}

DataStream& operator>>(DataStream& ds, EntityAnchorState& anchorState) {
  ds.read(anchorState.entityId);
  ds.readVlqS(anchorState.positionIndex);
  return ds;
}

DataStream& operator<<(DataStream& ds, EntityAnchorState const& anchorState) {
  ds.write(anchorState.entityId);
  ds.writeVlqS(anchorState.positionIndex);
  return ds;
}

}
