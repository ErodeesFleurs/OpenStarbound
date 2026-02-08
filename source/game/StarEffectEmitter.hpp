#pragma once

#include "StarConfig.hpp"
#include "StarEffectSourceDatabase.hpp"
#include "StarGameTypes.hpp"
#include "StarNetElementBasicFields.hpp"
#include "StarNetElementGroup.hpp"

import std;

namespace Star {

class RenderCallback;

class EffectEmitter : public NetElementGroup {
public:
  EffectEmitter();

  void addEffectSources(String const& position, StringSet effectSources);
  void setSourcePosition(String name, Vec2F const& position);
  void setDirection(Direction direction);
  void setBaseVelocity(Vec2F const& velocity);

  void tick(float dt, EntityMode mode);
  void reset();

  void render(RenderCallback* renderCallback);

  auto toJson() const -> Json;
  void fromJson(Json const& diskStore);

private:
  Set<std::pair<String, String>> m_newSources;
  List<Ptr<EffectSource>> m_sources;
  NetElementData<Set<std::pair<String, String>>> m_activeSources;

  StringMap<Vec2F> m_positions;
  Direction m_direction;
  Vec2F m_baseVelocity;

  bool m_renders;
};

}// namespace Star
