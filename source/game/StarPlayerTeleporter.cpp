#include "StarPlayerTeleporter.hpp"
#include "StarPlayer.hpp"
#include "StarPlayerChatAndEmotes.hpp"
#include "StarPlayerFactory.hpp"
#include "StarRoot.hpp"
#include "StarAssets.hpp"
#include "StarStatusController.hpp"
#include "StarActorMovementController.hpp"
#include "StarPlayerDeployment.hpp"
#include "StarTechController.hpp"
#include "StarPlayerInventory.hpp"
#include "StarEffectEmitter.hpp"

namespace Star {

PlayerTeleporter::PlayerTeleporter(Player* player)
  : m_player(player), m_teleportTimer(0.0f), m_teleportAnimationType("default") {}

void PlayerTeleporter::init() {
  m_teleportTimer = 0.0f;
  m_teleportAnimationType = "default";
}

void PlayerTeleporter::teleportOut(String const& animationType, bool deploy) {
  m_player->m_state = Player::State::TeleportOut;
  m_teleportAnimationType = animationType;
  m_player->m_effectsAnimator->setState("teleport", m_teleportAnimationType + "Out");
  m_player->m_deployment->setDeploying(deploy);
  m_player->m_deployment->teleportOut();
  m_teleportTimer = deploy ? m_player->m_config->deployOutTime : m_player->m_config->teleportOutTime;
}

void PlayerTeleporter::teleportIn() {
  m_player->m_state = Player::State::TeleportIn;
  m_player->m_effectsAnimator->setState("teleport", m_teleportAnimationType + "In");
  m_teleportTimer = m_player->m_deployment->isDeployed() ? m_player->m_config->deployInTime : m_player->m_config->teleportInTime;

  auto statusEffects = Root::singleton().assets()->json("/player.config:teleportInStatusEffects").toArray().transformed(jsonToEphemeralStatusEffect);
  m_player->m_statusController->addEphemeralEffects(statusEffects);
}

void PlayerTeleporter::teleportAbort() {
  m_player->m_state = Player::State::TeleportIn;
  m_player->m_effectsAnimator->setState("teleport", "abort");
  m_player->m_deployment->setDeploying(m_player->m_deployment->isDeployed());
  m_teleportTimer = m_player->m_config->teleportInTime;
}

bool PlayerTeleporter::isTeleporting() const {
  return (m_player->m_state == Player::State::TeleportIn) || (m_player->m_state == Player::State::TeleportOut);
}

bool PlayerTeleporter::isTeleportingOut() const {
  return m_player->inWorld() && (m_player->m_state == Player::State::TeleportOut) && m_teleportTimer >= 0.0f;
}

bool PlayerTeleporter::canDeploy() {
  return m_player->m_deployment->canDeploy();
}

void PlayerTeleporter::deployAbort(String const& animationType) {
  m_teleportAnimationType = animationType;
  m_player->m_deployment->setDeploying(false);
}

bool PlayerTeleporter::isDeploying() const {
  return m_player->m_deployment->isDeploying();
}

bool PlayerTeleporter::isDeployed() const {
  return m_player->m_deployment->isDeployed();
}

void PlayerTeleporter::setBusyState(PlayerBusyState busyState) {
  m_player->m_effectsAnimator->setState("busy", PlayerBusyStateNames.getRight(busyState));
}

void PlayerTeleporter::moveTo(Vec2F const& footPosition) {
  m_player->m_movementController->setPosition(footPosition - m_player->feetOffset());
  m_player->m_movementController->setVelocity(Vec2F());
}

void PlayerTeleporter::revive(Vec2F const& footPosition) {
  if (!m_player->isDead())
    return;

  m_player->m_state = Player::State::Idle;
  m_player->m_chatAndEmotes->setEmoteState(HumanoidEmote::Idle);

  m_player->m_statusController->setPersistentEffects("armor", m_player->m_armor->statusEffects());
  m_player->m_statusController->setPersistentEffects("tools", m_player->m_tools->statusEffects());
  m_player->m_statusController->resetAllResources();

  m_player->m_statusController->clearEphemeralEffects();

  m_player->endPrimaryFire();
  m_player->endAltFire();
  m_player->endTrigger();

  m_player->m_effectEmitter->reset();
  m_player->m_movementController->setPosition(footPosition - m_player->feetOffset());
  m_player->m_movementController->setVelocity(Vec2F());

  m_player->m_techController->reloadTech();

  float moneyCost = m_player->m_inventory->currency("money") * m_player->modeConfig().reviveCostPercentile;
  m_player->m_inventory->consumeCurrency("money", min(static_cast<uint64_t>(round(moneyCost)), m_player->m_inventory->currency("money")));
}

void PlayerTeleporter::tick(float dt) {
  if (isTeleporting()) {
    m_teleportTimer -= dt;
    if (m_teleportTimer <= 0 && m_player->m_state == Player::State::TeleportIn) {
      m_player->m_state = Player::State::Idle;
      m_player->m_effectsAnimator->burstParticleEmitter(m_teleportAnimationType + "Burst");
    }
  }
}

}

