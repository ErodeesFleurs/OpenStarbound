#include "StarAnchorableEntity.hpp"

import std;

namespace Star {

auto EntityAnchorState::operator==(EntityAnchorState const& eas) const -> bool {
  return std::tie(entityId, positionIndex) == std::tie(eas.entityId, eas.positionIndex);
}

auto operator>>(DataStream& ds, EntityAnchorState& anchorState) -> DataStream& {
  ds.read(anchorState.entityId);
  ds.readVlqS(anchorState.positionIndex);
  return ds;
}

auto operator<<(DataStream& ds, EntityAnchorState const& anchorState) -> DataStream& {
  ds.write(anchorState.entityId);
  ds.writeVlqS(anchorState.positionIndex);
  return ds;
}

}// namespace Star
