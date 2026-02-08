#include "StarWiring.hpp"

import std;

namespace Star {

auto otherWireDirection(WireDirection direction) -> WireDirection {
  return direction == WireDirection::Input ? WireDirection::Output : WireDirection::Input;
}

auto WireConnection::operator==(WireConnection const& wireConnection) const -> bool {
  return tie(entityLocation, nodeIndex) == tie(wireConnection.entityLocation, wireConnection.nodeIndex);
}

auto hash<WireConnection>::operator()(WireConnection const& wireConnection) const -> std::size_t {
  return hashOf(wireConnection.entityLocation, wireConnection.nodeIndex);
}

auto operator>>(DataStream& ds, WireConnection& wireConnection) -> DataStream& {
  ds.viread(wireConnection.entityLocation[0]);
  ds.viread(wireConnection.entityLocation[1]);
  ds.vuread(wireConnection.nodeIndex);
  return ds;
}

auto operator<<(DataStream& ds, WireConnection const& wireConnection) -> DataStream& {
  ds.viwrite(wireConnection.entityLocation[0]);
  ds.viwrite(wireConnection.entityLocation[1]);
  ds.vuwrite(wireConnection.nodeIndex);
  return ds;
}

auto operator>>(DataStream& ds, WireNode& wireNode) -> DataStream& {
  ds.read(wireNode.direction);
  ds.vuread(wireNode.nodeIndex);
  return ds;
}

auto operator<<(DataStream& ds, WireNode const& wireNode) -> DataStream& {
  ds.write(wireNode.direction);
  ds.vuwrite(wireNode.nodeIndex);
  return ds;
}

}// namespace Star
