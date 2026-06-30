#pragma once

#include "StarNetElementSystem.hpp"
#include "StarEffectSourceDatabase.hpp"
#include "StarGameTypes.hpp"

namespace Star {

class RenderCallback;
class EffectEmitter;
using EffectEmitterPtr = SharedPtr<EffectEmitter>;

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

  Json toJson() const;
  void fromJson(Json const& diskStore);

private:
  Set<pair<String, String>> m_newSources;
  List<EffectSourcePtr> m_sources;
  NetElementData<Set<pair<String, String>>> m_activeSources;

  StringMap<Vec2F> m_positions;
  Direction m_direction;
  Vec2F m_baseVelocity;

  bool m_renders;
};

}
