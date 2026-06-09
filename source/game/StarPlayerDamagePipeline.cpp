#include "StarPlayerDamagePipeline.hpp"
#include "StarPlayer.hpp"
#include "StarStatusController.hpp"
#include "StarWorld.hpp"

namespace Star {

PlayerDamagePipeline::PlayerDamagePipeline(Player* player)
  : m_player(player), m_lastDamagedOtherTimer(0), m_lastDamagedTarget(NullEntityId) {}

void PlayerDamagePipeline::init() {
  m_lastDamagedOtherTimer = 0;
  m_lastDamagedTarget = NullEntityId;
}

Maybe<HitType> PlayerDamagePipeline::queryHit(DamageSource const& source) const {
  if (!m_player->inWorld() || m_player->isDead() || m_player->isAdmin() || m_player->isTeleporting() || m_player->statusController()->statPositive("invulnerable"))
    return {};

  if (m_player->m_tools->queryShieldHit(source))
    return HitType::ShieldHit;

  if (source.intersectsWithPoly(m_player->world()->geometry(), m_player->movementController()->collisionBody()))
    return HitType::Hit;

  return {};
}

Maybe<PolyF> PlayerDamagePipeline::hitPoly() const {
  return m_player->movementController()->collisionBody();
}

List<DamageNotification> PlayerDamagePipeline::applyDamage(DamageRequest const& request) {
  if (!m_player->inWorld() || m_player->isDead() || m_player->isAdmin())
    return {};

  return m_player->statusController()->applyDamageRequest(request);
}

List<DamageNotification> PlayerDamagePipeline::selfDamageNotifications() {
  return m_player->statusController()->pullSelfDamageNotifications();
}

void PlayerDamagePipeline::hitOther(EntityId targetEntityId, DamageRequest const& damageRequest) {
  if (!m_player->isMaster())
    return;

  m_player->statusController()->hitOther(targetEntityId, damageRequest);
  if (as<DamageBarEntity>(m_player->world()->entity(targetEntityId))) {
    m_lastDamagedOtherTimer = 0;
    m_lastDamagedTarget = targetEntityId;
  }
}

void PlayerDamagePipeline::damagedOther(DamageNotification const& damage) {
  if (!m_player->isMaster())
    return;

  m_player->statusController()->damagedOther(damage);
}

List<DamageSource> PlayerDamagePipeline::damageSources() const {
  return m_damageSources;
}

void PlayerDamagePipeline::tick(float dt) {
  m_lastDamagedOtherTimer += dt;
}

void PlayerDamagePipeline::tickBuildSources() {
  m_damageSources = m_player->m_tools->damageSources();
  for (auto& damageSource : m_damageSources) {
    damageSource.sourceEntityId = m_player->entityId();
    damageSource.team = m_player->getTeam();
  }
}

float PlayerDamagePipeline::timeSinceLastGaveDamage() const {
  return m_lastDamagedOtherTimer;
}

EntityId PlayerDamagePipeline::lastDamagedTarget() const {
  return m_lastDamagedTarget;
}

}

