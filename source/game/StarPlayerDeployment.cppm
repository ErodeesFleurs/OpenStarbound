module;
#include "StarIdMap.hpp"
#include "StarJson.hpp"
#include "StarJsonExtra.hpp"
#include "StarPoly.hpp"
#include "StarVariant.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarGameTypes.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"
#include "StarNetElementSystem.hpp"
#include "StarList.hpp"
#include "StarRpcPromise.hpp"

#include "StarLuaComponents.hpp"
#include "StarLuaAnimationComponent.hpp"
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

STAR_CLASS(RenderCallback);

STAR_CLASS(PlayerDeployment);

class PlayerDeployment {
public:
  PlayerDeployment(Json const& config);

  void diskLoad(Json const& diskStore);
  Json diskStore() const;

  bool canDeploy();
  void setDeploying(bool deploying);
  bool isDeploying() const;
  bool isDeployed() const;

  void init(Entity* player, World* world);
  void uninit();

  void teleportOut();
  Maybe<ChainableJsonMessageResponse> receiveMessage(String const& message, bool localMessage, JsonArray const& args = {});
  void update(float dt);

  void render(RenderCallback* renderCallback, Vec2F const& position);

  void renderLightSources(RenderCallback* renderCallback);
private:
  World* m_world;
  Json m_config;

  bool m_deploying;
  bool m_deployed;
  LuaAnimationComponent<LuaMessageHandlingComponent<LuaStorableComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>>>> m_scriptComponent;
};

}

export module star.player_deployment;

export namespace Star {
using ::Star::RenderCallback;
using ::Star::RenderCallbackPtr;
using ::Star::RenderCallbackConstPtr;
using ::Star::RenderCallbackWeakPtr;
using ::Star::RenderCallbackConstWeakPtr;
using ::Star::RenderCallbackUPtr;
using ::Star::RenderCallbackConstUPtr;
using ::Star::PlayerDeployment;
using ::Star::PlayerDeploymentPtr;
using ::Star::PlayerDeploymentConstPtr;
using ::Star::PlayerDeploymentWeakPtr;
using ::Star::PlayerDeploymentConstWeakPtr;
using ::Star::PlayerDeploymentUPtr;
using ::Star::PlayerDeploymentConstUPtr;
}
