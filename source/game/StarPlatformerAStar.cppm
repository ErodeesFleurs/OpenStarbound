module;
import std;
import star.format;
import star.json;
import star.id_map;
import star.bi_map;
import star.a_star;
import star.casting;
import star.data_stream;
import star.strong_typedef;
import star.list;
import star.rpc_promise;
import star.game_types;
import star.vector;
import star.rect;

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
import star.lua_root;
import star.force_regions;
import star.world;

import star.physics_entity;
import star.movement_controller;
import star.platformer_astar_types;
import star.anchorable_entity;

import star.game_timers;
import star.actor_movement_controller;

namespace Star {
namespace PlatformerAStar {

  class PathFinder {
  public:
    [[nodiscard]] static std::expected<std::shared_ptr<PathFinder>, StarException> create(World& world,
        Vec2F searchFrom,
        Vec2F searchTo,
        ActorMovementParameters movementParameters,
        Parameters searchParameters = Parameters());

    PathFinder(PathFinder const&) = delete;
    PathFinder& operator=(PathFinder const&) = delete;

    [[nodiscard]] std::expected<std::optional<bool>, StarException> explore(std::optional<unsigned> maxExploreNodes = {});
    std::optional<Path> const& result() const;

  private:
    PathFinder(World& world, Vec2F searchFrom, Vec2F searchTo, ActorMovementParameters movementParameters, Parameters searchParameters);
    enum class BoundBoxKind { Full, Drop, Stand };

    [[nodiscard]] std::expected<void, StarException> initAStar();

    float heuristicCost(Vec2F const& fromPosition, Vec2F const& toPosition) const;
    Edge defaultCostEdge(Action action, Node const& source, Node const& target) const;
    [[nodiscard]] std::expected<void, StarException> neighbors(Node const& node, List<Edge>& neighbors) const;

    [[nodiscard]] std::expected<void, StarException> getDropNeighbors(Node const& node, List<Edge>& neighbors) const; // drop through a platform
    [[nodiscard]] std::expected<void, StarException> getWalkingNeighborsInDirection(Node const& node, List<Edge>& neighbors, float direction) const;
    [[nodiscard]] std::expected<void, StarException> getWalkingNeighbors(Node const& node, List<Edge>& neighbors) const;
    [[nodiscard]] std::expected<void, StarException> getFallingNeighbors(Node const& node, List<Edge>& neighbors) const; // freefall
    [[nodiscard]] std::expected<void, StarException> getJumpingNeighbors(Node const& node, List<Edge>& neighbors) const;
    [[nodiscard]] std::expected<void, StarException> getArcNeighbors(Node const& node, List<Edge>& neighbors) const;
    [[nodiscard]] std::expected<void, StarException> getSwimmingNeighbors(Node const& node, List<Edge>& neighbors) const;
    [[nodiscard]] std::expected<void, StarException> getFlyingNeighbors(Node const& node, List<Edge>& neighbors) const;

    [[nodiscard]] std::expected<void, StarException> forEachArcVelocity(float yVelocity, std::function<std::expected<void, StarException>(Vec2F)> func) const;
    [[nodiscard]] std::expected<void, StarException> forEachArcNeighbor(Node const& node, float yVelocity, std::function<std::expected<void, StarException>(Node, bool)> func) const;
    [[nodiscard]] std::expected<Vec2F, StarException> acceleration(Vec2F pos) const;
    [[nodiscard]] std::expected<Vec2F, StarException> simulateArcCollision(Vec2F position, Vec2F velocity, float dt, bool& collidedX, bool& collidedY) const;
    [[nodiscard]] std::expected<void, StarException> simulateArc(Node const& node, std::function<std::expected<void, StarException>(Node, bool)> func) const;

    [[nodiscard]] std::expected<bool, StarException> validPosition(Vec2F pos, BoundBoxKind boundKind = BoundBoxKind::Full) const;
    [[nodiscard]] std::expected<bool, StarException> onGround(Vec2F pos, BoundBoxKind boundKind = BoundBoxKind::Full) const; // Includes non-solids: platforms, objects, etc.
    [[nodiscard]] std::expected<bool, StarException> onSolidGround(Vec2F pos) const; // Includes only solids
    [[nodiscard]] std::expected<bool, StarException> inLiquid(Vec2F pos) const;

    [[nodiscard]] std::expected<RectF, StarException> boundBox(Vec2F pos, BoundBoxKind boundKind = BoundBoxKind::Full) const;
    // Returns a rect that covers the tiles below the entity's feet if it was at
    // pos
    [[nodiscard]] std::expected<RectI, StarException> groundCollisionRect(Vec2F pos, BoundBoxKind boundKind) const;
    // Returns a rect that covers a 1 tile wide space below the entity's feet at
    // node pos
    [[nodiscard]] std::expected<Vec2I, StarException> groundNodePosition(Vec2F pos) const;

    [[nodiscard]] std::expected<Vec2F, StarException> roundToNode(Vec2F pos) const;
    float distance(Vec2F a, Vec2F b) const;

    std::reference_wrapper<World> m_world;
    Vec2F m_searchFrom;
    Vec2F m_searchTo;
    ActorMovementParameters m_movementParams;
    Parameters m_searchParams;
    std::optional<AStar::Search<Edge, Node>> m_astar;
  };
}
}

export module star.platformer_astar;

export namespace Star::PlatformerAStar {
  using ::Star::PlatformerAStar::PathFinder;
}
