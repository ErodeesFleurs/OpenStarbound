#include "StarWorldClientDamageFX.hpp"
#include "StarParticleManager.hpp"
#include "StarWorldTemplate.hpp"
#include "StarWorldClient.hpp"
#include "StarParticleDatabase.hpp"
#include "StarDamageDatabase.hpp"
#include "StarMaterialDatabase.hpp"
#include "StarEffectSourceDatabase.hpp"
#include "StarLogging.hpp"

namespace Star {

bool StarWorldClientDamageFX::DamageNumberKey::operator<(DamageNumberKey const& other) const {
  return tie(sourceEntityId, targetEntityId, damageNumberParticleKind)
      < tie(other.sourceEntityId, other.targetEntityId, other.damageNumberParticleKind);
}

StarWorldClientDamageFX::StarWorldClientDamageFX(WorldClient& worldClient)
  : m_worldClient(worldClient) {}

void StarWorldClientDamageFX::handleDamageNotifications() {
  if (!m_worldClient.inWorld())
    return;

  auto renderParticle = [&](Vec2F position, float amount, String const& damageNumberParticleKind) {
    int displayValue = static_cast<int>(ceil(amount - 0.1f));
    if (displayValue <= 0)
      return;
    Particle particle = m_worldClient.m_particleDatabase->particle(damageNumberParticleKind);
    particle.position += position;
    particle.string = particle.string.replace("$dmg$", toString(displayValue));
    m_worldClient.m_particles->add(particle);
  };

  eraseWhere(m_damageNumbers, [&](std::pair<DamageNumberKey, DamageNumber> const& entry) -> bool {
      if (Time::monotonicTime() - entry.second.timestamp > m_damageNotificationBatchDuration) {
        renderParticle(entry.second.position, entry.second.amount, entry.first.damageNumberParticleKind);
        return true;
      }
      return false;
    });

  for (auto const& damageNotification : m_worldClient.m_damageManager->pullPendingNotifications()) {
    auto damageDatabase = m_worldClient.m_damageDatabase;
    DamageKind const& damageKind = damageDatabase->damageKind(damageNotification.damageSourceKind);
    ElementalType const& elementalType = damageDatabase->elementalType(damageKind.elementalType);

    auto damageNumberParticleKind = elementalType.damageNumberParticles.get(damageNotification.hitType);
    auto damageNumberKey = DamageNumberKey{ damageNumberParticleKind, damageNotification.sourceEntityId, damageNotification.targetEntityId};


    DamageNumber number;
    if (m_damageNumbers.contains(damageNumberKey)) {
      number = m_damageNumbers.take(damageNumberKey);

      if (damageNotification.hitType == HitType::Kill)
        renderParticle(damageNotification.position,
            damageNotification.damageDealt + number.amount,
            damageNumberKey.damageNumberParticleKind);
    } else {
      if (damageNotification.hitType == HitType::Kill)
        renderParticle(damageNotification.position, damageNotification.damageDealt, damageNumberParticleKind);
      number.amount = 0;
      number.timestamp = Time::monotonicTime();
    }

    if (damageNotification.hitType != HitType::Kill) {
      number.position = damageNotification.position;
      number.amount += damageNotification.damageDealt;
      m_damageNumbers[damageNumberKey] = number;
    }

    String material = damageNotification.targetMaterialKind;
    if (!material.empty() && damageKind.effects.contains(material)) {
      HitType effectHitType = damageKind.effects.get(material).contains(damageNotification.hitType) ? damageNotification.hitType : HitType::Hit;
      m_worldClient.m_samples.appendAll(soundsFromDefinition(m_worldClient.m_assets, damageKind.effects.get(material).get(effectHitType).sounds, damageNotification.position));

      auto hitParticles = particlesFromDefinition(damageKind.effects.get(material).get(effectHitType).particles, damageNotification.position, m_worldClient.particleDatabase());

      const List<Directives>* directives = nullptr;
      if (auto& worldTemplate = m_worldClient.m_worldTemplate) {
        if (const auto& parameters = worldTemplate->worldParameters())
          if (auto& globalDirectives = parameters->globalDirectives)
            directives = &globalDirectives.get();
      }
      if (directives) {
        int directiveIndex = unsigned(damageNotification.targetEntityId) % directives->size();
        for (auto& p : hitParticles)
          p.directives.append(directives->get(directiveIndex));
      }

      m_worldClient.m_particles->addParticles(hitParticles);
    }
  }
}

void StarWorldClientDamageFX::sparkDamagedBlocks() {
  if (!m_worldClient.inWorld())
    return;

  auto materialDatabase = m_worldClient.m_materialDatabase;

  for (auto pos : m_worldClient.m_damagedBlocks.values()) {
    if (auto tile = m_worldClient.m_tileArray->modifyTile(pos)) {
      if (tile->backgroundDamage.healthy() && tile->foregroundDamage.healthy())
        m_worldClient.m_damagedBlocks.remove(pos);

      if (isRealMaterial(tile->foreground) && tile->foregroundDamage.damageEffectPercentage() - Random::randf() > 0.0f
          && (Random::randf() < m_blockDamageParticleProbability)) {
        auto particle = m_blockDamageParticle;
        particle.color = materialDatabase->materialParticleColor(tile->foreground, tile->foregroundHueShift);

        if (m_worldClient.isTileProtected(pos))
          particle = m_blockDingParticle;

        particle.position += centerOfTile(pos);
        particle.velocity = particle.velocity.magnitude()
            * vnorm(m_worldClient.m_geometry.diff(tile->foregroundDamage.sourcePosition(), particle.position));
        particle.applyVariance(m_blockDamageParticleVariance);
        m_worldClient.m_particles->add(particle);
      }

      if (isRealMaterial(tile->background) && tile->backgroundDamage.damageEffectPercentage() - Random::randf() > 0.0f
          && (Random::randf() < m_blockDamageParticleProbability)) {
        auto particle = m_blockDamageParticle;
        particle.color = materialDatabase->materialParticleColor(tile->background, tile->backgroundHueShift);

        if (m_worldClient.isTileProtected(pos))
          particle = m_blockDingParticle;

        particle.position += centerOfTile(pos);
        particle.velocity = particle.velocity.magnitude()
            * vnorm(m_worldClient.m_geometry.diff(tile->backgroundDamage.sourcePosition(), particle.position));
        particle.applyVariance(m_blockDamageParticleVariance);
        m_worldClient.m_particles->add(particle);
      }
    }
  }
}

}
