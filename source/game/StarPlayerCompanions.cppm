module;
import std;
import star.lua;
import star.maybe;
import star.json;
import star.id_map;
import star.data_stream;
import star.string;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.variant;
#include "StarCasting.hpp"
import star.game_types;
#include "StarStrongTypedef.hpp"
import star.bi_map;
import star.rpc_promise;

import star.uuid;
import star.drawable;
#include "StarLuaComponents.hpp"
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

namespace Star {

class Companion;
using CompanionPtr = std::shared_ptr<Companion>;
using CompanionConstPtr = std::shared_ptr<const Companion>;
using CompanionWeakPtr = std::weak_ptr<Companion>;
using CompanionConstWeakPtr = std::weak_ptr<const Companion>;
using CompanionUPtr = std::unique_ptr<Companion>;
using CompanionConstUPtr = std::unique_ptr<const Companion>;
class PlayerCompanions;
using PlayerCompanionsPtr = std::shared_ptr<PlayerCompanions>;
using PlayerCompanionsConstPtr = std::shared_ptr<const PlayerCompanions>;
using PlayerCompanionsWeakPtr = std::weak_ptr<PlayerCompanions>;
using PlayerCompanionsConstWeakPtr = std::weak_ptr<const PlayerCompanions>;
using PlayerCompanionsUPtr = std::unique_ptr<PlayerCompanions>;
using PlayerCompanionsConstUPtr = std::unique_ptr<const PlayerCompanions>;

class Companion {
public:
  Companion(Json const& json);
  Json toJson() const;

  Uuid podUuid() const;
  Maybe<String> name() const;
  Maybe<String> description() const;

  List<Drawable> portrait() const;

  Maybe<float> resource(String const& resourceName) const;
  Maybe<float> resourceMax(String const& resourceName) const;

  Maybe<float> stat(String const& statName) const;

private:
  Json m_json;
  List<Drawable> m_portrait;
};

class PlayerCompanions {
public:
  PlayerCompanions(Json const& config);

  void diskLoad(Json const& diskStore);
  Json diskStore() const;

  List<CompanionPtr> getCompanions(String const& category) const;

  void init(Entity* player, World* world);
  void uninit();

  void dismissCompanion(String const& category, Uuid const& podUuid);

  Maybe<ChainableJsonMessageResponse> receiveMessage(String const& message, bool localMessage, JsonArray const& args = {});
  void update(float dt);

private:
  LuaCallbacks makeCompanionsCallbacks();

  World* m_world;
  Json m_config;
  StringMap<List<CompanionPtr>> m_companions;

  LuaMessageHandlingComponent<LuaStorableComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>>>
      m_scriptComponent;
};

}

export module star.player_companions;

export namespace Star {
using ::Star::Companion;
using ::Star::CompanionPtr;
using ::Star::CompanionConstPtr;
using ::Star::CompanionWeakPtr;
using ::Star::CompanionConstWeakPtr;
using ::Star::CompanionUPtr;
using ::Star::CompanionConstUPtr;
using ::Star::PlayerCompanions;
using ::Star::PlayerCompanionsPtr;
using ::Star::PlayerCompanionsConstPtr;
using ::Star::PlayerCompanionsWeakPtr;
using ::Star::PlayerCompanionsConstWeakPtr;
using ::Star::PlayerCompanionsUPtr;
using ::Star::PlayerCompanionsConstUPtr;
}
