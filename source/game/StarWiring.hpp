#pragma once

#include "StarDataStream.hpp"
#include "StarGameTypes.hpp"
#include "StarWorldGeometry.hpp"

import std;

namespace Star {

enum class WireDirection {
  Input,
  Output
};

auto otherWireDirection(WireDirection direction) -> WireDirection;

// Identifier for a specific WireNode in a WireEntity, node indexes for input
// and output nodes are separate.
struct WireNode {
  WireDirection direction;
  std::size_t nodeIndex;
};

auto operator>>(DataStream& ds, WireNode& wireNode) -> DataStream&;
auto operator<<(DataStream& ds, WireNode const& wireNode) -> DataStream&;

// Connection from a given WireNode to another WireNode, the direction must be
// implied based on the context.
struct WireConnection {
  Vec2I entityLocation;
  std::size_t nodeIndex;

  auto operator==(WireConnection const& wireConnection) const -> bool;
};

template <>
struct hash<WireConnection> {
  auto operator()(WireConnection const& wireConnection) const -> std::size_t;
};

auto operator>>(DataStream& ds, WireConnection& wireConnection) -> DataStream&;
auto operator<<(DataStream& ds, WireConnection const& wireConnection) -> DataStream&;

class WireCoordinator {
public:
  virtual ~WireCoordinator() = default;

  virtual auto readInputConnection(WireConnection const& connection) -> bool = 0;
};

class WireConnector {
public:
  enum SwingResult {
    Connect,
    Mismatch,
    Protected,
    Nothing
  };

  virtual ~WireConnector() = default;

  virtual auto swing(WorldGeometry const& geometry, Vec2F position, FireMode mode) -> SwingResult = 0;
  virtual auto connecting() -> bool = 0;
};

}// namespace Star
