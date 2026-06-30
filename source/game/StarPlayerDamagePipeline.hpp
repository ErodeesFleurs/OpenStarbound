#pragma once

#include "StarDamageManager.hpp"
#include "StarEntity.hpp"
#include "StarGameTypes.hpp"

namespace Star {

class Player;

class PlayerDamagePipeline {
public:
  explicit PlayerDamagePipeline(Player& player);

  void init();

  [[nodiscard]] Maybe<HitType> queryHit(DamageSource const& source) const;
  [[nodiscard]] Maybe<PolyF> hitPoly() const;
  [[nodiscard]] List<DamageNotification> applyDamage(DamageRequest const& request);
  [[nodiscard]] List<DamageNotification> selfDamageNotifications();
  void hitOther(EntityId targetEntityId, DamageRequest const& damageRequest);
  void damagedOther(DamageNotification const& damage);
  [[nodiscard]] List<DamageSource> damageSources() const;

  void tick(float dt);
  void tickBuildSources();

  [[nodiscard]] float timeSinceLastGaveDamage() const;
  [[nodiscard]] EntityId lastDamagedTarget() const;

private:
  Player& m_player;

  List<DamageSource> m_damageSources;
  float m_lastDamagedOtherTimer = 0;
  EntityId m_lastDamagedTarget = NullEntityId;
};

}
