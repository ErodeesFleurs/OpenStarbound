#pragma once

#include "StarParticle.hpp"
#include "StarEntity.hpp"

namespace Star {

class WorldClient;

class StarWorldClientDamageFX {
public:
  friend class WorldClient;

  struct DamageNumber {
    float amount;
    Vec2F position;
    double timestamp;
  };

  struct DamageNumberKey {
    String damageNumberParticleKind;
    EntityId sourceEntityId;
    EntityId targetEntityId;

    bool operator<(DamageNumberKey const& other) const;
  };

  explicit StarWorldClientDamageFX(WorldClient& worldClient);

  void handleDamageNotifications();
  void sparkDamagedBlocks();

private:
  WorldClient& m_worldClient;

  Map<DamageNumberKey, DamageNumber> m_damageNumbers;
  float m_damageNotificationBatchDuration = 0.0f;

  Particle m_blockDamageParticle;
  Particle m_blockDamageParticleVariance;
  float m_blockDamageParticleProbability = 0.0f;

  Particle m_blockDingParticle;
  Particle m_blockDingParticleVariance;
  float m_blockDingParticleProbability = 0.0f;
};

}
