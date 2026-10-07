module;
#include "StarIdMap.hpp"
#include "StarJson.hpp"
#include "StarBiMap.hpp"
#include "StarAStar.hpp"
#include "StarPoly.hpp"
#include "StarVariant.hpp"
#include "StarCasting.hpp"
#include "StarDataStream.hpp"
#include "StarStrongTypedef.hpp"
#include "StarList.hpp"
#include "StarRpcPromise.hpp"
#include "StarGameTypes.hpp"
#include "StarJson.hpp"
#include "StarMaybe.hpp"
#include "StarNetElementSystem.hpp"
#include "StarVector.hpp"
#include "StarRect.hpp"
#include "StarBiMap.hpp"
#include "StarAStar.hpp"
#include "StarVector.hpp"
#include "StarRect.hpp"
#include "StarBiMap.hpp"
#include "StarAStar.hpp"

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
#include "StarLuaRoot.hpp"
import star.force_regions;
import star.world;

import star.physics_entity;
import star.movement_controller;
import star.platformer_astar_types;
import star.anchorable_entity;

import star.game_timers;
import star.actor_movement_controller;
import star.platformer_astar_types;

namespace Star {
namespace PlatformerAStar {

  STAR_CLASS(PathFinder);

  class PathFinder {
  public:
    PathFinder(World* world,
        Vec2F searchFrom,
        Vec2F searchTo,
        ActorMovementParameters movementParameters,
        Parameters searchParameters = Parameters());

    // Does not preserve current search state.
    PathFinder(PathFinder const& rhs);
    PathFinder& operator=(PathFinder const& rhs);

    Maybe<bool> explore(Maybe<unsigned> maxExploreNodes = {});
    Maybe<Path> const& result() const;

  private:
    enum class BoundBoxKind { Full, Drop, Stand };

    void initAStar();

    float heuristicCost(Vec2F const& fromPosition, Vec2F const& toPosition) const;
    Edge defaultCostEdge(Action action, Node const& source, Node const& target) const;
    void neighbors(Node const& node, List<Edge>& neighbors) const;

    void getDropNeighbors(Node const& node, List<Edge>& neighbors) const; // drop through a platform
    void getWalkingNeighborsInDirection(Node const& node, List<Edge>& neighbors, float direction) const;
    void getWalkingNeighbors(Node const& node, List<Edge>& neighbors) const;
    void getFallingNeighbors(Node const& node, List<Edge>& neighbors) const; // freefall
    void getJumpingNeighbors(Node const& node, List<Edge>& neighbors) const;
    void getArcNeighbors(Node const& node, List<Edge>& neighbors) const;
    void getSwimmingNeighbors(Node const& node, List<Edge>& neighbors) const;
    void getFlyingNeighbors(Node const& node, List<Edge>& neighbors) const;

    void forEachArcVelocity(float yVelocity, function<void(Vec2F)> func) const;
    void forEachArcNeighbor(Node const& node, float yVelocity, function<void(Node, bool)> func) const;
    Vec2F acceleration(Vec2F pos) const;
    Vec2F simulateArcCollision(Vec2F position, Vec2F velocity, float dt, bool& collidedX, bool& collidedY) const;
    void simulateArc(Node const& node, function<void(Node, bool)> func) const;

    bool validPosition(Vec2F pos, BoundBoxKind boundKind = BoundBoxKind::Full) const;
    bool onGround(Vec2F pos, BoundBoxKind boundKind = BoundBoxKind::Full) const; // Includes non-solids: platforms, objects, etc.
    bool onSolidGround(Vec2F pos) const; // Includes only solids
    bool inLiquid(Vec2F pos) const;

    RectF boundBox(Vec2F pos, BoundBoxKind boundKind = BoundBoxKind::Full) const;
    // Returns a rect that covers the tiles below the entity's feet if it was at
    // pos
    RectI groundCollisionRect(Vec2F pos, BoundBoxKind boundKind) const;
    // Returns a rect that covers a 1 tile wide space below the entity's feet at
    // node pos
    Vec2I groundNodePosition(Vec2F pos) const;

    Vec2F roundToNode(Vec2F pos) const;
    float distance(Vec2F a, Vec2F b) const;

    World* m_world;
    Vec2F m_searchFrom;
    Vec2F m_searchTo;
    ActorMovementParameters m_movementParams;
    Parameters m_searchParams;
    Maybe<AStar::Search<Edge, Node>> m_astar;
  };
}
}

export module star.platformer_astar;

export namespace Star::PlatformerAStar {
  using ::Star::PlatformerAStar::PathFinder;
  using ::Star::PlatformerAStar::PathFinderPtr;
  using ::Star::PlatformerAStar::PathFinderConstPtr;
  using ::Star::PlatformerAStar::PathFinderWeakPtr;
  using ::Star::PlatformerAStar::PathFinderConstWeakPtr;
  using ::Star::PlatformerAStar::PathFinderUPtr;
  using ::Star::PlatformerAStar::PathFinderConstUPtr;
}
