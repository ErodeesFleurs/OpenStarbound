module;
#include "StarIdMap.hpp"
#include "StarJson.hpp"
#include "StarArray.hpp"
#include "StarDataStream.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
#include "StarVariant.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarGameTypes.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"
#include "StarNetElementSystem.hpp"
#include "StarList.hpp"
#include "StarRpcPromise.hpp"

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

STAR_CLASS(Companion);
STAR_CLASS(PlayerCompanions);

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
