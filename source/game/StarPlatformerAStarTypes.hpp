#pragma once

#include "StarAStar.hpp"
#include "StarBiMap.hpp"
#include "StarFormat.hpp"
#include "StarRect.hpp"
#include "StarVector.hpp"

import std;

namespace Star::PlatformerAStar {

struct Node {
  [[nodiscard]] auto withVelocity(Vec2F velocity) const -> Node;

  auto operator<(Node const& other) const -> bool;

  Vec2F position;
  std::optional<Vec2F> velocity;// Only valid when jumping/falling
};

enum class Action { Walk,
                    Jump,
                    Arc,
                    Drop,
                    Swim,
                    Fly,
                    Land };
extern EnumMap<Action> const ActionNames;

struct Edge {
  float cost;
  Action action;
  Vec2F jumpVelocity;
  Node source;
  Node target;
};

using Path = AStar::Path<Edge>;

struct Parameters {
  // Maximum distance from the start node to search for a path to the target
  // node
  std::optional<float> maxDistance;
  // If true, returns the path to the closest node to the target found, if a
  // path to the target itself could not be found.
  // Otherwise, findPath will return a None value.
  bool returnBest;
  // If true, end the path only on ground
  bool mustEndOnGround;
  // If true, allows jumps to have the entity's walk speed as horizontal
  // velocity
  bool enableWalkSpeedJumps;
  // if true, allows perfectly vertical jumps to change horizontal velocity at
  // the peak
  bool enableVerticalJumpAirControl;
  // Multiplies the cost of edges going through liquids. Can be used to
  // penalize or promote paths involving swiming.
  std::optional<float> swimCost;
  // The cost of jump edges.
  std::optional<float> jumpCost;
  // The cost of jump edges that start in liquids.
  std::optional<float> liquidJumpCost;
  // The cost of dropping through a platform.
  std::optional<float> dropCost;
  // If set, will be the default bounding box, otherwise will use
  // movementParameters.standingPoly.
  std::optional<RectF> boundBox;
  // The bound box used for checking if the entity can stand at a position
  // Should be thinner than the full bound box
  std::optional<RectF> standingBoundBox;
  // The bound box used for checking if the entity can drop at a position
  // Should be wider than the full bound box
  std::optional<RectF> droppingBoundBox;
  // Pathing simulates jump arcs for two Y velocities: 1.0 * jumpSpeed and
  // smallJumpMultiplier * jumpSpeed. This value should be in the range
  // 0 < smallJumpMultiplier < 1.0
  std::optional<float> smallJumpMultiplier;
  // Mid-jump, at the peak, entities can choose to change horizontal velocity.
  // The velocities they can switch to are runSpeed, walkSpeed, and
  // (walkSpeed * jumpDropXMultiplier). The purpose of the latter option is to
  // make a vertical drop (if 0) or disable dropping (if 1). Inbetween values
  // can be used to make less angular-looking arcs.
  std::optional<float> jumpDropXMultiplier;
  // If provided, the following fields can be supplied to put a limit on how
  // long findPath calls can take:
  std::optional<double> maxFScore;
  std::optional<unsigned> maxNodesToSearch;
  // Upper bound on the (negative) velocity that entities can land on
  // platforms
  // and ledges with. This is used to ensure there is a small amount of
  // clearance
  // over ledges to improve the scripts' chances of landing the same way we
  // simulated the jump.
  std::optional<float> maxLandingVelocity;
};

auto operator==(Parameters const& lhs, Parameters const& rhs) -> bool;
auto operator!=(Parameters const& lhs, Parameters const& rhs) -> bool;

inline auto operator==(Node const& a, Node const& b) -> bool {
  return a.position == b.position && a.velocity == b.velocity;
}

inline auto operator<<(std::ostream& os, Node const& node) -> std::ostream& {
  return os << strf("Node{{position = {}, velocity = {}}}", node.position, node.velocity);
}
}// namespace Star::PlatformerAStar

template <>
struct std::formatter<Star::PlatformerAStar::Node> : Star::ostream_formatter {};

namespace Star::PlatformerAStar {

inline auto operator<<(std::ostream& os, Action action) -> std::ostream& {
  return os << ActionNames.getRight(action);
}
}// namespace Star::PlatformerAStar

template <>
struct std::formatter<Star::PlatformerAStar::Action> : Star::ostream_formatter {};

namespace Star::PlatformerAStar {

inline auto operator<<(std::ostream& os, Edge const& edge) -> std::ostream& {
  return os << strf("Edge{{cost = {}, action = {}, jumpVelocity = {}, source = {}, target = {}}}",
                    edge.cost,
                    edge.action,
                    edge.jumpVelocity,
                    edge.source,
                    edge.target);
}
}// namespace Star::PlatformerAStar

template <>
struct std::formatter<Star::PlatformerAStar::Edge> : Star::ostream_formatter {};

namespace Star::PlatformerAStar {

inline auto Node::operator<(Node const& other) const -> bool {
  if (position == other.position)
    return velocity < other.velocity;
  return position < other.position;
}

inline auto operator==(Parameters const& lhs, Parameters const& rhs) -> bool {
  return lhs.maxDistance == rhs.maxDistance
    && lhs.returnBest == rhs.returnBest
    && lhs.mustEndOnGround == rhs.mustEndOnGround
    && lhs.enableWalkSpeedJumps == rhs.enableWalkSpeedJumps
    && lhs.enableVerticalJumpAirControl == rhs.enableVerticalJumpAirControl
    && lhs.swimCost == rhs.swimCost
    && lhs.jumpCost == rhs.jumpCost
    && lhs.liquidJumpCost == rhs.liquidJumpCost
    && lhs.dropCost == rhs.dropCost
    && lhs.boundBox == rhs.boundBox
    && lhs.standingBoundBox == rhs.standingBoundBox
    && lhs.droppingBoundBox == rhs.droppingBoundBox
    && lhs.smallJumpMultiplier == rhs.smallJumpMultiplier
    && lhs.jumpDropXMultiplier == rhs.jumpDropXMultiplier
    && lhs.maxFScore == rhs.maxFScore
    && lhs.maxNodesToSearch == rhs.maxNodesToSearch
    && lhs.maxLandingVelocity == rhs.maxLandingVelocity;
}

inline auto operator!=(Parameters const& lhs, Parameters const& rhs) -> bool {
  return !(lhs == rhs);
}

}// namespace Star::PlatformerAStar
