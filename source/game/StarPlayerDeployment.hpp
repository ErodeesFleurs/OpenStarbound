#pragma once

#include "StarObserverPtr.hpp"
#include "StarLuaComponents.hpp"
#include "StarLuaAnimationComponent.hpp"
#include "StarWorld.hpp"

namespace Star {

class RenderCallback;
class Player;

class PlayerDeployment;
using PlayerDeploymentPtr = SharedPtr<PlayerDeployment>;

class PlayerDeployment {
public:
  PlayerDeployment(Json const& config, AssetsConstPtr assets);

  void diskLoad(Json const& diskStore);
  [[nodiscard]] Json diskStore() const;

  [[nodiscard]] bool canDeploy();
  void setDeploying(bool deploying);
  [[nodiscard]] bool isDeploying() const;
  [[nodiscard]] bool isDeployed() const;

  void init(Player& player, World& world);
  void uninit();

  void teleportOut();
  [[nodiscard]] Maybe<Json> receiveMessage(String const& message, bool localMessage, JsonArray const& args = {});
  void update(float dt);

  void render(RenderCallback* renderCallback, Vec2F const& position);

  void renderLightSources(RenderCallback* renderCallback);
private:
  observer_ptr<World> m_world;
  Json m_config;

  bool m_deploying;
  bool m_deployed;
  LuaAnimationComponent<LuaMessageHandlingComponent<LuaStorableComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>>>> m_scriptComponent;
};

}
