#pragma once

#include "StarDamageManager.hpp"
#include "StarEntity.hpp"
#include "StarGameTypes.hpp"

namespace Star {

class Player;

class PlayerDamagePipeline {
public:
  explicit PlayerDamagePipeline(Player* player);

  void init();

  Maybe<HitType> queryHit(DamageSource const& source) const;
  Maybe<PolyF> hitPoly() const;
  List<DamageNotification> applyDamage(DamageRequest const& request);
  List<DamageNotification> selfDamageNotifications();
  void hitOther(EntityId targetEntityId, DamageRequest const& damageRequest);
  void damagedOther(DamageNotification const& damage);
  List<DamageSource> damageSources() const;

  void tick(float dt);
  void tickBuildSources();

  float timeSinceLastGaveDamage() const;
  EntityId lastDamagedTarget() const;

private:
  Player* m_player;

  List<DamageSource> m_damageSources;
  float m_lastDamagedOtherTimer;
  EntityId m_lastDamagedTarget;
};

}

