import std;
import star.json;
import star.id_map;
import star.casting;
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
import star.anchorable_entity;

namespace Star {

bool EntityAnchorState::operator==(EntityAnchorState const& eas) const {
  return std::tie(entityId, positionIndex) == std::tie(eas.entityId, eas.positionIndex);
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
