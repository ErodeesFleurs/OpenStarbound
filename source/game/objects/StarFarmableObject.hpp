#pragma once

#include "StarObject.hpp"

namespace Star {

class FarmableObject : public Object {
public:
  FarmableObject(ObjectConfigConstPtr config, Json const& parameters);

  void update(float dt, uint64_t currentStep) override;

  [[nodiscard]] bool damageTiles(List<Vec2I> const& position, Vec2F const& sourcePosition, TileDamage const& tileDamage) override;
  [[nodiscard]] InteractAction interact(InteractRequest const& request) override;

  [[nodiscard]] bool harvest();
  [[nodiscard]] int stage() const;

protected:
  void readStoredData(Json const& diskStore) override;
  [[nodiscard]] Json writeStoredData() const override;

private:
  void enterStage(int newStage);

  int m_stage;
  int m_stageAlt = -1;
  double m_stageEnterTime = 0.0;
  double m_nextStageTime = 0.0;

  SlidingWindow m_immersion;
  float m_minImmersion;
  float m_maxImmersion;

  bool m_consumeSoilMoisture;

  JsonArray m_stages;
  bool m_finalStage = false;
};

}
