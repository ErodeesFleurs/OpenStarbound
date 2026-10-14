module;
import std;
import star.string;
import star.list;
import star.set;
import star.json;

import star.vector;
import star.game_types;

import star.net_element_basic_fields;
import star.net_element_group;

namespace Star {

class RenderCallback;
using RenderCallbackPtr = std::shared_ptr<RenderCallback>;
using RenderCallbackConstPtr = std::shared_ptr<const RenderCallback>;
using RenderCallbackWeakPtr = std::weak_ptr<RenderCallback>;
using RenderCallbackConstWeakPtr = std::weak_ptr<const RenderCallback>;
using RenderCallbackUPtr = std::unique_ptr<RenderCallback>;
using RenderCallbackConstUPtr = std::unique_ptr<const RenderCallback>;
class EffectEmitter;
using EffectEmitterPtr = std::shared_ptr<EffectEmitter>;
using EffectEmitterConstPtr = std::shared_ptr<const EffectEmitter>;
using EffectEmitterWeakPtr = std::weak_ptr<EffectEmitter>;
using EffectEmitterConstWeakPtr = std::weak_ptr<const EffectEmitter>;
using EffectEmitterUPtr = std::unique_ptr<EffectEmitter>;
using EffectEmitterConstUPtr = std::unique_ptr<const EffectEmitter>;
class EffectSource;
using EffectSourcePtr = std::shared_ptr<EffectSource>;
using EffectSourceConstPtr = std::shared_ptr<const EffectSource>;
using EffectSourceWeakPtr = std::weak_ptr<EffectSource>;
using EffectSourceConstWeakPtr = std::weak_ptr<const EffectSource>;
using EffectSourceUPtr = std::unique_ptr<EffectSource>;
using EffectSourceConstUPtr = std::unique_ptr<const EffectSource>;

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
  Set<std::pair<String, String>> m_newSources;
  List<EffectSourcePtr> m_sources;
  NetElementData<Set<std::pair<String, String>>> m_activeSources;

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
