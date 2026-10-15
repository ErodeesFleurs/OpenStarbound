module;
import std;
import star.hash;

import star.vector;
import star.game_types;
import star.world_geometry;
import star.data_stream;

namespace Star {

enum class WireDirection {
  Input,
  Output
};

WireDirection otherWireDirection(WireDirection direction);

// Identifier for a specific WireNode in a WireEntity, node indexes for input
// and output nodes are separate.
struct WireNode {
  friend DataStream& operator>>(DataStream& ds, WireNode& value);
  friend DataStream& operator<<(DataStream& ds, WireNode const& value);
  WireDirection direction;
  std::size_t nodeIndex;
};

DataStream& operator>>(DataStream& ds, WireNode& wireNode);
DataStream& operator<<(DataStream& ds, WireNode const& wireNode);

// Connection from a given WireNode to another WireNode, the direction must be
// implied based on the context.
struct WireConnection {
  friend DataStream& operator>>(DataStream& ds, WireConnection& value);
  friend DataStream& operator<<(DataStream& ds, WireConnection const& value);
  Vec2I entityLocation;
  std::size_t nodeIndex;

  bool operator==(WireConnection const& wireConnection) const;
};

template <>
struct hash<WireConnection> {
  std::size_t operator()(WireConnection const& wireConnection) const;
};

DataStream& operator>>(DataStream& ds, WireConnection& wireConnection);
DataStream& operator<<(DataStream& ds, WireConnection const& wireConnection);

class WireCoordinator {
public:
  virtual ~WireCoordinator() {}

  virtual bool readInputConnection(WireConnection const& connection) = 0;
};

class WireConnector {
public:
  enum SwingResult {
    Connect,
    Mismatch,
    Protected,
    Nothing
  };

  virtual ~WireConnector() {}

  virtual SwingResult swing(WorldGeometry const& geometry, Vec2F position, FireMode mode) = 0;
  virtual bool connecting() = 0;
};

}

export module star.wiring;

export namespace Star {
  template <>
  struct hash<WireConnection>;
}


export namespace Star {
  using ::Star::WireCoordinator;
  using ::Star::WireConnector;
  using ::Star::WireDirection;
  using ::Star::otherWireDirection;
  using ::Star::WireNode;
  using ::Star::WireConnection;
  using ::Star::hash;
}
