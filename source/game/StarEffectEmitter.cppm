module;

#include "StarNetElementSystem.hpp"
#include "StarGameTypes.hpp"

namespace Star {

STAR_CLASS(RenderCallback);
STAR_CLASS(EffectEmitter);
STAR_CLASS(EffectSource);

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

export module star.effect_emitter;

export namespace Star {
using ::Star::RenderCallback;
using ::Star::RenderCallbackPtr;
using ::Star::RenderCallbackConstPtr;
using ::Star::RenderCallbackWeakPtr;
using ::Star::RenderCallbackConstWeakPtr;
using ::Star::RenderCallbackUPtr;
using ::Star::RenderCallbackConstUPtr;
using ::Star::EffectEmitter;
using ::Star::EffectEmitterPtr;
using ::Star::EffectEmitterConstPtr;
using ::Star::EffectEmitterWeakPtr;
using ::Star::EffectEmitterConstWeakPtr;
using ::Star::EffectEmitterUPtr;
using ::Star::EffectEmitterConstUPtr;
using ::Star::EffectSource;
using ::Star::EffectSourcePtr;
using ::Star::EffectSourceConstPtr;
using ::Star::EffectSourceWeakPtr;
using ::Star::EffectSourceConstWeakPtr;
using ::Star::EffectSourceUPtr;
using ::Star::EffectSourceConstUPtr;
}
