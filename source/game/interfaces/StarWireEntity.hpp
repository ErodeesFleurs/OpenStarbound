#pragma once

#include "StarTileEntity.hpp"
#include "StarWiring.hpp"

import std;

namespace Star {

class WireEntity : public virtual TileEntity {
public:
  ~WireEntity() override = default;

  [[nodiscard]] virtual auto nodeCount(WireDirection direction) const -> std::size_t = 0;
  [[nodiscard]] virtual auto nodePosition(WireNode wireNode) const -> Vec2I = 0;
  [[nodiscard]] virtual auto connectionsForNode(WireNode wireNode) const -> List<WireConnection> = 0;
  [[nodiscard]] virtual auto nodeState(WireNode wireNode) const -> bool = 0;

  [[nodiscard]] virtual auto nodeColor(WireNode wireNode) const -> Color = 0;
  [[nodiscard]] virtual auto nodeIcon(WireNode wireNode) const -> String = 0;

  virtual void addNodeConnection(WireNode wireNode, WireConnection nodeConnection) = 0;
  virtual void removeNodeConnection(WireNode wireNode, WireConnection nodeConnection) = 0;

  virtual void evaluate(WireCoordinator* coordinator) = 0;
};

}// namespace Star
