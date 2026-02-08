#pragma once

#include "StarConfig.hpp"
#include "StarObject.hpp"

namespace Star {

class FarmableObject : public Object {
public:
  FarmableObject(ConstPtr<ObjectConfig> config, Json const& parameters);

  void update(float dt, uint64_t currentStep) override;

  auto damageTiles(List<Vec2I> const& position, Vec2F const& sourcePosition, TileDamage const& tileDamage) -> bool override;
  auto interact(InteractRequest const& request) -> InteractAction override;

  auto harvest() -> bool;
  auto stage() const -> int;

protected:
  void readStoredData(Json const& diskStore) override;
  auto writeStoredData() const -> Json override;

private:
  void enterStage(int newStage);

  int m_stage;
  int m_stageAlt;
  double m_stageEnterTime;
  double m_nextStageTime;

  SlidingWindow m_immersion;
  float m_minImmersion;
  float m_maxImmersion;

  bool m_consumeSoilMoisture;

  JsonArray m_stages;
  bool m_finalStage;
};

}// namespace Star
