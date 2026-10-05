module;

#include "StarJson.hpp"
#include "StarJsonExtra.hpp"
#include "StarLuaComponents.hpp"
#include "StarLuaAnimationComponent.hpp"
#include "StarWorld.hpp"

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
