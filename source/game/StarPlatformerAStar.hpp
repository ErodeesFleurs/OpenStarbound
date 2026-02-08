#pragma once

#include "StarAStar.hpp"
#include "StarActorMovementController.hpp"
#include "StarPlatformerAStarTypes.hpp"
#include "StarWorld.hpp"

import std;

namespace Star::PlatformerAStar {

class PathFinder {
public:
  PathFinder(World* world,
             Vec2F searchFrom,
             Vec2F searchTo,
             ActorMovementParameters movementParameters,
             Parameters searchParameters = Parameters());

  // Does not preserve current search state.
  PathFinder(PathFinder const& rhs);
  auto operator=(PathFinder const& rhs) -> PathFinder&;

  auto explore(std::optional<unsigned> maxExploreNodes = {}) -> std::optional<bool>;
  [[nodiscard]] auto result() const -> std::optional<Path> const&;

private:
  enum class BoundBoxKind { Full,
                            Drop,
                            Stand };

  void initAStar();

  [[nodiscard]] auto heuristicCost(Vec2F const& fromPosition, Vec2F const& toPosition) const -> float;
  [[nodiscard]] auto defaultCostEdge(Action action, Node const& source, Node const& target) const -> Edge;
  void neighbors(Node const& node, List<Edge>& neighbors) const;

  void getDropNeighbors(Node const& node, List<Edge>& neighbors) const;// drop through a platform
  void getWalkingNeighborsInDirection(Node const& node, List<Edge>& neighbors, float direction) const;
  void getWalkingNeighbors(Node const& node, List<Edge>& neighbors) const;
  void getFallingNeighbors(Node const& node, List<Edge>& neighbors) const;// freefall
  void getJumpingNeighbors(Node const& node, List<Edge>& neighbors) const;
  void getArcNeighbors(Node const& node, List<Edge>& neighbors) const;
  void getSwimmingNeighbors(Node const& node, List<Edge>& neighbors) const;
  void getFlyingNeighbors(Node const& node, List<Edge>& neighbors) const;

  void forEachArcVelocity(float yVelocity, std::function<void(Vec2F)> func) const;
  void forEachArcNeighbor(Node const& node, float yVelocity, std::function<void(Node, bool)> func) const;
  [[nodiscard]] auto acceleration(Vec2F pos) const -> Vec2F;
  auto simulateArcCollision(Vec2F position, Vec2F velocity, float dt, bool& collidedX, bool& collidedY) const -> Vec2F;
  void simulateArc(Node const& node, std::function<void(Node, bool)> func) const;

  [[nodiscard]] auto validPosition(Vec2F pos, BoundBoxKind boundKind = BoundBoxKind::Full) const -> bool;
  [[nodiscard]] auto onGround(Vec2F pos, BoundBoxKind boundKind = BoundBoxKind::Full) const -> bool;// Includes non-solids: platforms, objects, etc.
  [[nodiscard]] auto onSolidGround(Vec2F pos) const -> bool;                                        // Includes only solids
  [[nodiscard]] auto inLiquid(Vec2F pos) const -> bool;

  [[nodiscard]] auto boundBox(Vec2F pos, BoundBoxKind boundKind = BoundBoxKind::Full) const -> RectF;
  // Returns a rect that covers the tiles below the entity's feet if it was at
  // pos
  [[nodiscard]] auto groundCollisionRect(Vec2F pos, BoundBoxKind boundKind) const -> RectI;
  // Returns a rect that covers a 1 tile wide space below the entity's feet at
  // node pos
  [[nodiscard]] auto groundNodePosition(Vec2F pos) const -> Vec2I;

  [[nodiscard]] auto roundToNode(Vec2F pos) const -> Vec2F;
  [[nodiscard]] auto distance(Vec2F a, Vec2F b) const -> float;

  World* m_world;
  Vec2F m_searchFrom;
  Vec2F m_searchTo;
  ActorMovementParameters m_movementParams;
  Parameters m_searchParams;
  std::optional<AStar::Search<Edge, Node>> m_astar;
};
}// namespace Star::PlatformerAStar
