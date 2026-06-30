#pragma once

#include "StarGameTypes.hpp"
#include "StarPlayerTypes.hpp"

namespace Star {

class Player;

class PlayerTeleporter {
public:
  explicit PlayerTeleporter(Player& player);
  void init();

  void teleportOut(String const& animationType = "default", bool deploy = false);
  void teleportIn();
  void teleportAbort();
  [[nodiscard]] bool isTeleporting() const;
  [[nodiscard]] bool isTeleportingOut() const;
  [[nodiscard]] bool canDeploy();
  void deployAbort(String const& animationType = "default");
  [[nodiscard]] bool isDeploying() const;
  [[nodiscard]] bool isDeployed() const;
  void setBusyState(PlayerBusyState busyState);

  void moveTo(Vec2F const& footPosition);
  void revive(Vec2F const& footPosition);

  void tick(float dt);

private:
  Player& m_player;

  float m_teleportTimer = 0.0f;
  String m_teleportAnimationType = "default";
};

}
