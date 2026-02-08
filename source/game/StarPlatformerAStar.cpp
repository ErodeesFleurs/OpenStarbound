#include "StarPlatformerAStar.hpp"

#include "StarWorld.hpp"
#include "StarLiquidTypes.hpp"

import std;

namespace Star::PlatformerAStar {

  // The desired spacing between nodes:
  float const NodeGranularity = 1.f;
  float const SimulateArcGranularity = 0.5f;

  float const DefaultMaxDistance = 50.0f;
  float const DefaultSmallJumpMultiplier = 0.75f;
  float const DefaultJumpDropXMultiplier = 0.125f;

  float const DefaultSwimCost = 40.0f;
  float const DefaultJumpCost = 3.0f;
  float const DefaultLiquidJumpCost = 10.0f;
  float const DefaultDropCost = 3.0f;

  float const DefaultMaxLandingVelocity = -5.0f;

  // Bounding boxes are shrunk slightly to work around floating point rounding
  // errors.
  float const BoundBoxRoundingErrorScaling = 0.99f;

  CollisionSet const CollisionSolid{CollisionKind::Null, CollisionKind::Slippery, CollisionKind::Block, CollisionKind::Slippery};

  CollisionSet const CollisionFloorOnly{CollisionKind::Null, CollisionKind::Block, CollisionKind::Slippery, CollisionKind::Platform};

  CollisionSet const CollisionDynamic{CollisionKind::Dynamic};

  CollisionSet const CollisionAny{CollisionKind::Null,
      CollisionKind::Platform,
      CollisionKind::Dynamic,
      CollisionKind::Slippery,
      CollisionKind::Block};

  PathFinder::PathFinder(World* world,
      Vec2F searchFrom,
      Vec2F searchTo,
      ActorMovementParameters movementParameters,
      Parameters searchParameters)
    : m_world(world),
      m_searchFrom(searchFrom),
      m_searchTo(searchTo),
      m_movementParams(std::move(movementParameters)),
      m_searchParams(std::move(searchParameters)) {
    initAStar();
  }

  PathFinder::PathFinder(PathFinder const& rhs) {
    operator=(rhs);
  }

  auto PathFinder::operator=(PathFinder const& rhs) -> PathFinder& {
    m_world = rhs.m_world;
    m_searchFrom = rhs.m_searchFrom;
    m_searchTo = rhs.m_searchTo;
    m_movementParams = rhs.m_movementParams;
    m_searchParams = rhs.m_searchParams;
    initAStar();
    return *this;
  }

  auto PathFinder::explore(std::optional<unsigned> maxExploreNodes) -> std::optional<bool> {
    return m_astar->explore(maxExploreNodes);
  }

  auto PathFinder::result() const -> std::optional<Path> const& {
    return m_astar->result();
  }

  void PathFinder::initAStar() {
    auto heuristicCostFn = [this](Node const& fromNode, Node const& toNode) -> float {
      return heuristicCost(fromNode.position, toNode.position);
    };
    auto goalReachedFn = [this](Node const& node) -> bool {
      if (m_searchParams.mustEndOnGround && (!onGround(node.position) || node.velocity.has_value()))
        return false;
      return distance(node.position, m_searchTo) < NodeGranularity;
    };
    auto neighborsFn = [this](Node const& node, List<Edge>& result) -> void {
      auto neighborFilter = [this](Edge const& edge) -> bool {
        return distance(edge.source.position, m_searchFrom) <= m_searchParams.maxDistance.value_or(DefaultMaxDistance);
      };
      neighbors(node, result);
      result.filter(neighborFilter);
    };
    auto validateEndFn = [this](Edge const& edge) -> bool {
      if (!m_searchParams.mustEndOnGround)
        return true;
      return onGround(edge.target.position) && edge.action != Action::Jump;
    };

    Vec2F roundedFrom = roundToNode(m_searchFrom);
    Vec2F roundedTo = roundToNode(m_searchTo);

    m_astar = AStar::Search<Edge, Node>(heuristicCostFn,
        neighborsFn,
        goalReachedFn,
        m_searchParams.returnBest,
        {validateEndFn},
        m_searchParams.maxFScore,
        m_searchParams.maxNodesToSearch);
    m_astar->start(Node{.position=roundedFrom, .velocity={}}, Node{.position=roundedTo, .velocity={}});
  }

  auto PathFinder::heuristicCost(Vec2F const& fromPosition, Vec2F const& toPosition) const -> float {
    // This function is used to estimate the cost of travel between two nodes.
    // Underestimating the actual cost results in A* giving the optimal path.
    // Overestimating results in A* finding a non-optimal path, but terminating
    // more quickly when there is a route to the target.
    // We don't really care all that much about getting the optimal path as long
    // as we get one that looks feasible, so we deliberately overestimate here.
    Vec2F diff = m_world->geometry().diff(fromPosition, toPosition);
    // Manhattan distance * 2:
    return 2.0f * (std::abs(diff[0]) + std::abs(diff[1]));
  }

  auto PathFinder::defaultCostEdge(Action action, Node const& source, Node const& target) const -> Edge {
    return Edge{.cost=distance(source.position, target.position), .action=action, .jumpVelocity=Vec2F(0, 0), .source=source, .target=target};
  }

  void PathFinder::neighbors(Node const& node, List<Edge>& neighbors) const {
    if (node.velocity.has_value()) {
      // Follow the current trajectory. Most of the time, this will only produce
      // one neighbor to avoid massive search space explosion, however one
      // change of X velocity is allowed at the peak of a jump.
      getArcNeighbors(node, neighbors);

    } else if (inLiquid(node.position)) {
      getSwimmingNeighbors(node, neighbors);

    } else if (acceleration(node.position)[1] == 0.0f) {
      getFlyingNeighbors(node, neighbors);

    } else if (onGround(node.position)) {
      getWalkingNeighbors(node, neighbors);

      if (!onSolidGround(node.position)) {
        // Add a node for dropping through a platform.
        // When that node is explored, if it's not onGround, its neighbors will
        // be falling to the ground.
        getDropNeighbors(node, neighbors);
      }

      getJumpingNeighbors(node, neighbors);
    } else {
      // We're in the air, and can only fall now
      getFallingNeighbors(node, neighbors);
    }
  }

  void PathFinder::getDropNeighbors(Node const& node, List<Edge>& neighbors) const {
    auto dropPosition = node.position + Vec2F(0, -1);
    // The physics of platforms don't allow us to drop through platforms resting
    // directly on solid surfaces. So if there is solid ground below the
    // platform, don't allow dropping through the platform:
    if (!onSolidGround(dropPosition)) {
      float dropCost = m_searchParams.dropCost.value_or(DefaultDropCost);
      float acc = acceleration(node.position)[1];
      float dropSpeed = acc * std::sqrt(2.0 / std::abs(acc));
      neighbors.append(Edge{.cost=dropCost, .action=Action::Drop, .jumpVelocity=Vec2F(0, 0), .source=node, .target=Node{.position=dropPosition, .velocity=Vec2F(0, dropSpeed)}});
    }
  }

  void PathFinder::getWalkingNeighborsInDirection(Node const& node, List<Edge>& neighbors, float direction) const {
    auto addNode = [this, &node, &neighbors](Node const& target) -> void {
      neighbors.append(defaultCostEdge(Action::Walk, node, target));
    };

    Vec2F forward = node.position + Vec2F(direction, 0);
    Vec2F forwardAndUp = node.position + Vec2F(direction, 1);
    Vec2F forwardAndDown = node.position + Vec2F(direction, -1);

    RectF bounds = boundBox(node.position);

    bool slopeDown = false;
    bool slopeUp = false;
    Vec2F forwardGroundPos = direction > 0 ? Vec2F(bounds.xMax(), bounds.yMin()) : Vec2F(bounds.xMin(), bounds.yMin());
    Vec2F backGroundPos = direction < 0 ? Vec2F(bounds.xMax(), bounds.yMin()) : Vec2F(bounds.xMin(), bounds.yMin());
    m_world->forEachCollisionBlock(groundCollisionRect(node.position, BoundBoxKind::Full).padded(1), [&](CollisionBlock const& block) -> void {
      if (slopeUp || slopeDown) return;
      for (size_t i = 0; i < block.poly.sides(); ++i) {
        auto side = block.poly.side(i);
        auto sideDir = side.direction();

        auto lower = side.min()[1] < side.max()[1] ? side.min() : side.max();
        auto upper = side.min()[1] > side.max()[1] ? side.min() : side.max();
        if (sideDir[0] != 0 && sideDir[1] != 0 && (lower[1] == std::round(forwardGroundPos[1]) || upper[1] == std::round(forwardGroundPos[1]))) {
          float yDir = (sideDir[1] / sideDir[0]) * direction;
          if (std::abs(m_world->geometry().diff(forwardGroundPos, lower)[0]) < 0.5 && yDir > 0)
            slopeUp = true;
          else if (std::abs(m_world->geometry().diff(backGroundPos, upper)[0]) < 0.5 && yDir < 0)
            slopeDown = true;

          if (slopeUp || slopeDown) break;
        }
      }
    });

    std::optional<float> walkSpeed = m_movementParams.walkSpeed;
    std::optional<float> runSpeed = m_movementParams.runSpeed;

    // Check if it's possible to walk up a block like a ramp first
    if (slopeUp && onGround(forwardAndUp) && validPosition(forwardAndUp)) {
      // Walk up a slope
      addNode(Node{.position=forwardAndUp, .velocity={}});
    } else if (validPosition(forward) && onGround(forward)) {
      // Walk along a flat plane
      addNode(Node{.position=forward, .velocity={}});
    } else if (slopeDown && validPosition(forward) && validPosition(forwardAndDown) && onGround(forwardAndDown)) {
      // Walk down a slope
      addNode(Node{.position=forwardAndDown, .velocity={}});
    } else if (validPosition(forward)) {
      // Fall off a ledge
      bounds = m_movementParams.standingPoly->boundBox();
      float back = direction > 0 ? bounds.xMin() : bounds.xMax();
      forward[0] -= (1 - std::fmod(std::abs(back), 1.0f)) * direction;
      if (walkSpeed.has_value())
        addNode(Node{.position=forward, .velocity=Vec2F{std::copysign(*walkSpeed, direction), 0.0f}});
      if (runSpeed.has_value())
        addNode(Node{.position=forward, .velocity=Vec2F{std::copysign(*runSpeed, direction), 0.0f}});
    }
  }

  void PathFinder::getWalkingNeighbors(Node const& node, List<Edge>& neighbors) const {
    getWalkingNeighborsInDirection(node, neighbors, NodeGranularity);
    getWalkingNeighborsInDirection(node, neighbors, -NodeGranularity);
  }

  void PathFinder::getFallingNeighbors(Node const& node, List<Edge>& neighbors) const {
    forEachArcNeighbor(node,  0.0f, [this, &node, &neighbors](Node const& target, bool landed) -> void {
      neighbors.append(defaultCostEdge(Action::Arc, node, target));
      if (landed) {
        neighbors.append(defaultCostEdge(Action::Land, target, Node{.position=target.position, .velocity={}}));
      }
    });
  }

  void PathFinder::getJumpingNeighbors(Node const& node, List<Edge>& neighbors) const {
    if (auto jumpSpeed = m_movementParams.airJumpProfile.jumpSpeed) {
      float jumpCost = m_searchParams.jumpCost.value_or(DefaultJumpCost);
      if (inLiquid(node.position))
        jumpCost = m_searchParams.liquidJumpCost.value_or(DefaultLiquidJumpCost);

      auto addVel = [jumpCost, &node, &neighbors](Vec2F const& vel) -> void {
        neighbors.append(Edge{.cost=jumpCost, .action=Action::Jump, .jumpVelocity=vel, .source=node, .target=node.withVelocity(vel)});
      };

      forEachArcVelocity(*jumpSpeed, addVel);
      forEachArcVelocity(*jumpSpeed * m_searchParams.smallJumpMultiplier.value_or(DefaultSmallJumpMultiplier), addVel);
    }
  }

  void PathFinder::getSwimmingNeighbors(Node const& node, List<Edge>& neighbors) const {
    // TODO avoid damaging liquids, e.g. lava

    // We assume when we're swimming we can move freely against gravity
    getFlyingNeighbors(node, neighbors);

    // Also allow jumping out of the water if we're at the surface:
    RectF box = boundBox(node.position);
    if (acceleration(node.position)[1] != 0.0f && m_world->liquidLevel(box).level < 1.0f)
      getJumpingNeighbors(node, neighbors);

    neighbors.filter([this](Edge& edge) -> bool {
      return inLiquid(edge.target.position);
    });
    neighbors.transform([this](Edge& edge) -> Edge& {
      if (edge.action == Action::Fly)
        edge.action = Action::Swim;
      edge.cost *= m_searchParams.swimCost.value_or(DefaultSwimCost);
      return edge;
    });
  }

  void PathFinder::getFlyingNeighbors(Node const& node, List<Edge>& neighbors) const {
    auto addNode = [this, &node, &neighbors](
        Node const& target) -> void { neighbors.append(defaultCostEdge(Action::Fly, node, target)); };

    Vec2F roundedPosition = roundToNode(node.position);
    for (int dx = -1; dx < 2; ++dx) {
      for (int dy = -1; dy < 2; ++dy) {
        Vec2F newPosition = roundedPosition + Vec2F(dx, dy) * NodeGranularity;
        if (validPosition(newPosition)) {
          addNode(Node{.position=newPosition, .velocity={}});
        }
      }
    }
  }

  void PathFinder::getArcNeighbors(Node const& node, List<Edge>& neighbors) const {
    auto addNode = [this, &node, &neighbors](Node const& target, bool landed) -> void {
      neighbors.append(defaultCostEdge(Action::Arc, node, target));
      if (landed) {
        neighbors.append(defaultCostEdge(Action::Land, target, Node{.position=target.position, .velocity={}}));
      }
    };

    simulateArc(node, addNode);
  }

  void PathFinder::forEachArcVelocity(float yVelocity, std::function<void(Vec2F)> func) const {
    std::optional<float> walkSpeed = m_movementParams.walkSpeed;
    std::optional<float> runSpeed = m_movementParams.runSpeed;

    func(Vec2F(0, yVelocity));
    if (m_searchParams.enableWalkSpeedJumps && walkSpeed.has_value()) {
      func(Vec2F(*walkSpeed, yVelocity));
      func(Vec2F(-*walkSpeed, yVelocity));
    }
    if (runSpeed.has_value()) {
      func(Vec2F(*runSpeed, yVelocity));
      func(Vec2F(-*runSpeed, yVelocity));
    }
  }

  void PathFinder::forEachArcNeighbor(Node const& node, float yVelocity, std::function<void(Node, bool)> func) const {
    Vec2F position = roundToNode(node.position);
    forEachArcVelocity(yVelocity,
        [this, &position, &func](Vec2F const& vel) -> void {
          simulateArc(Node{.position=position, .velocity=vel}, func);
        });
  }

  auto PathFinder::acceleration(Vec2F pos) const -> Vec2F {
    auto const& parameters = m_movementParams;
    float gravity = m_world->gravity(pos) * parameters.gravityMultiplier.value_or(1.0f);
    if (!parameters.gravityEnabled.value_or(true) || parameters.mass.value_or(0.0f) == 0.0f)
      gravity = 0.0f;
    float buoyancy = parameters.airBuoyancy.value_or(0.0f);
    return {0, -gravity * (1.0f - buoyancy)};
  }

  auto PathFinder::simulateArcCollision(Vec2F position, Vec2F velocity, float dt, bool& collidedX, bool& collidedY) const -> Vec2F {
    // Returns the new position and whether a collision in the Y axis occurred.
    // We avoid actual collision detection / resolution as that would make
    // pathfinding very expensive.

    Vec2F newPosition = position + velocity * dt;
    if (validPosition(newPosition)) {
      collidedX = collidedY = false;
      return newPosition;
    } else {
      collidedX = collidedY = true;

      if (validPosition(Vec2F(newPosition[0], position[1]))) {
        collidedX = false;
        position[0] = newPosition[0];
      } else if (validPosition(Vec2F(position[0], newPosition[1]), BoundBoxKind::Stand)) {
        collidedY = false;
        position[1] = newPosition[1];
      }
    }

    return position;
  }

  void PathFinder::simulateArc(Node const& node, std::function<void(Node, bool)> func) const {
    Vec2F position = node.position;
    Vec2F velocity = *node.velocity;
    bool jumping = velocity[1] > 0.0f;
    float maxLandingVelocity = m_searchParams.maxLandingVelocity.value_or(DefaultMaxLandingVelocity);

    Vec2F acc = acceleration(position);
    if (acc[1] == 0.0f)
      return;

    // Simulate until we're roughly NodeGranularity distance from the previous
    // node
    Vec2F start = roundToNode(node.position);
    Vec2F rounded = start;
    while (rounded == start) {
      float speed = velocity.magnitude();
      float dt = std::fmin(0.2f, speed != 0.0f ? SimulateArcGranularity / speed : std::sqrt(SimulateArcGranularity * 2.0 * std::abs(acc[1])));

      bool collidedX = false, collidedY = false;
      position = simulateArcCollision(position, velocity, dt, collidedX, collidedY);
      rounded = roundToNode(position);

      if (collidedY) {
        // We've either landed or hit our head on the ceiling
        if (!jumping) {
          // Landed
          if (velocity[1] < maxLandingVelocity)
            func(Node{.position=rounded, .velocity=velocity}, true);
          return;
        } else if (onGround(rounded, BoundBoxKind::Stand)) {
          // Simultaneously hit head and landed -- this is a gap we can *just*
          // fit
          // through. No checking of the maxLandingVelocity, since the tiles'
          // polygons are rounded, making this an easier target to hit than it
          // seems.
          func(Node{.position=rounded, .velocity=velocity}, true);
          return;
        }
        // Hit ceiling. Remove y velocity
        velocity[1] = 0.0f;

      } else if (collidedX) {
        // Hit a wall, just fall down
        velocity[0] = 0.0f;
        if (jumping) {
          velocity[1] = 0.0f;
          jumping = false;
        }
      }

      velocity += acc * dt;
      if (jumping && velocity[1] <= 0.0f) {
        // We've reached a peak in the jump and the entity can now choose to
        // change direction.
        std::optional<float> runSpeed = m_movementParams.runSpeed;
        std::optional<float> walkSpeed = m_movementParams.walkSpeed;
        float crawlMultiplier = m_searchParams.jumpDropXMultiplier.value_or(DefaultJumpDropXMultiplier);

        if ((*node.velocity)[0] != 0.0f || m_searchParams.enableVerticalJumpAirControl) {
          if (runSpeed.has_value())
            func(Node{.position=position, .velocity=Vec2F{std::copysign(*runSpeed, velocity[0]), 0.0f}}, false);
          if (m_searchParams.enableWalkSpeedJumps && walkSpeed.has_value()) {
            func(Node{.position=position, .velocity=Vec2F{std::copysign(*walkSpeed, velocity[0]), 0.0f}}, false);
            func(Node{.position=position, .velocity=Vec2F{std::copysign(*walkSpeed * crawlMultiplier, velocity[0]), 0.0f}}, false);
          }
        }
        // Only fall straight down if we were going straight up originally.
        // Going from an arc to falling straight down looks unnatural.
        if ((*node.velocity)[0] == 0.0f) {
          func(Node{.position=position, .velocity=Vec2F(0.0f, 0.0f)}, false);
        }
        return;
      }
    }

    if (!jumping) {
      if (velocity[1] < maxLandingVelocity) {
        if (onGround(rounded, BoundBoxKind::Stand) || inLiquid(rounded)) {
          // Collision with platform
          func(Node{.position=rounded, .velocity=velocity}, true);
          return;
        }
      }
    }

    func(Node{.position=position, .velocity=velocity}, false);
    return;
  }

  auto PathFinder::validPosition(Vec2F pos, BoundBoxKind boundKind) const -> bool {
    return !m_world->rectTileCollision(RectI::integral(boundBox(pos, boundKind)), CollisionSolid);
  }

  auto PathFinder::onGround(Vec2F pos, BoundBoxKind boundKind) const -> bool {
    auto groundRect = groundCollisionRect(pos, boundKind);
    // Check there is something under the feet.
    // We allow walking over the tops of objects (e.g. trapdoors) without being
    // able to float inside objects.
    if (m_world->rectTileCollision(RectI::integral(boundBox(pos, boundKind)), CollisionDynamic))
      // We're inside an object. Don't collide with object directly below our
      // feet:
      return m_world->rectTileCollision(groundRect, CollisionFloorOnly);
    // Not inside an object, allow colliding with objects below our feet:
    // We need to be for sure above platforms, but can be up to a full tile
    // below the top of solid blocks because rounded collision polys
    return m_world->rectTileCollision(groundRect, CollisionAny) || m_world->rectTileCollision(groundRect.translated(Vec2I(0, 1)), CollisionSolid);
  }

  auto PathFinder::onSolidGround(Vec2F pos) const -> bool {
    return m_world->rectTileCollision(groundCollisionRect(pos, BoundBoxKind::Drop), CollisionSolid);
  }

  auto PathFinder::inLiquid(Vec2F pos) const -> bool {
    RectF box = boundBox(pos);
    return m_world->liquidLevel(box).level >= m_movementParams.minimumLiquidPercentage.value_or(0.5f);
  }

  auto PathFinder::boundBox(Vec2F pos, BoundBoxKind boundKind) const -> RectF {
    RectF boundBox;
    if (boundKind == BoundBoxKind::Drop && m_searchParams.droppingBoundBox) {
      boundBox = *m_searchParams.droppingBoundBox;
    } else if (boundKind == BoundBoxKind::Stand && m_searchParams.standingBoundBox) {
      boundBox = *m_searchParams.standingBoundBox;
    } else if (m_searchParams.boundBox.has_value()) {
      boundBox = *m_searchParams.boundBox;
    } else {
      boundBox = m_movementParams.standingPoly->boundBox();
    }

    boundBox.scale(BoundBoxRoundingErrorScaling);
    boundBox.translate(pos);
    return boundBox;
  }

  auto PathFinder::groundCollisionRect(Vec2F pos, BoundBoxKind boundKind) const -> RectI {
    RectI bounds = RectI::integral(boundBox(pos, boundKind));

    Vec2I min = Vec2I(bounds.xMin(), bounds.yMin() - 1);
    Vec2I max = Vec2I(bounds.xMax(), bounds.yMin());
    // Return a 1-tile-thick rectangle below the 'feet' of the entity
    return {min, max};
  }

  auto PathFinder::groundNodePosition(Vec2F pos) const -> Vec2I {
    RectI bounds = RectI::integral(boundBox(pos));

    return {static_cast<int>(std::floor(pos[0])), bounds.yMin() - 1};
  }

  auto PathFinder::roundToNode(Vec2F pos) const -> Vec2F {
    // Round pos to the nearest node.

    // Work out the distance from the entity's origin to the bottom of its
    // feet. We round Y relative to this so that we ensure we're able to
    // generate
    // paths through gaps that are *just* tall enough for the entity to fit
    // through.
    RectF boundBox;
    if (m_searchParams.boundBox.has_value()) {
      boundBox = *m_searchParams.boundBox;
    } else {
      boundBox = m_movementParams.standingPoly->boundBox();
    }
    float bottom = boundBox.yMin();

    float x = std::round(pos[0] / NodeGranularity) * NodeGranularity;
    float y = std::round((pos[1] + bottom) / NodeGranularity) * NodeGranularity - bottom;
    return {x, y};
  }

  auto PathFinder::distance(Vec2F a, Vec2F b) const -> float {
    return m_world->geometry().diff(a, b).magnitude();
  }
}
