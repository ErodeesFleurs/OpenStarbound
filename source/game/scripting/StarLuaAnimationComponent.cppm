module;
import std;
import star.json_extra;
import star.json;
import star.vector;
import star.string;
import star.color;
import star.list;
import star.lua;
import star.format;
import star.lua_converters;
import star.lua_game_converters;
import star.lua_components;
import star.drawable;
import star.entity_rendering_types;
import star.mixer;
import star.particle;
import star.assets;
import star.root;
import star.particle_database;
import star.light_source;
import star.algorithm;

namespace Star {

struct LuaAnimationComponentExceptionTag {
  static constexpr char const* name() { return "LuaAnimationComponentException"; }
};
using LuaAnimationComponentException = StarError<LuaAnimationComponentExceptionTag, LuaComponentException>;

// Lua component that allows lua to directly produce drawables, light sources,
// audios, and particles.  Adds a "localAnimation" callback table.
template <typename Base>
class LuaAnimationComponent : public Base {
public:
  explicit LuaAnimationComponent(Root& root);

  List<std::pair<Drawable, std::optional<EntityRenderLayer>>> const& drawables();
  List<LightSource> const& lightSources();

  List<Particle> pullNewParticles();
  List<std::shared_ptr<AudioInstance>> pullNewAudios();

protected:
  // Clears looping audio on context shutdown
  void contextShutdown() override;

private:
  List<Particle> m_pendingParticles;
  List<std::shared_ptr<AudioInstance>> m_pendingAudios;
  List<std::shared_ptr<AudioInstance>> m_activeAudio;

  List<std::pair<Drawable, std::optional<EntityRenderLayer>>> m_drawables;
  List<LightSource> m_lightSources;
};

template <typename Base>
LuaAnimationComponent<Base>::LuaAnimationComponent(Root& root) : Base(root) {
  LuaCallbacks animationCallbacks;
  animationCallbacks.registerCallback("playAudio", [this](String const& sound, std::optional<int> loops, std::optional<float> volume) -> std::shared_ptr<AudioInstance> {
      auto audio = std::make_shared<AudioInstance>(*this->root().assets()->audio(sound));
      audio->setLoops(loops.value_or(0));
      audio->setVolume(volume.value_or(1.0f));
      m_pendingAudios.append(audio);
      m_activeAudio.append(audio);
      return audio;
    });
  animationCallbacks.registerCallback("spawnParticle", [this](Json const& particleConfig, std::optional<Vec2F> const& position) {
      auto particle = this->root().particleDatabase()->particle(particleConfig);
      particle.translate(position.value_or(Vec2F{}));
      m_pendingParticles.append(particle);
    });
  animationCallbacks.registerCallback("clearDrawables", [this]() {
      m_drawables.clear();
    });
  animationCallbacks.registerCallback("addDrawable", [this](Drawable drawable, std::optional<String> renderLayerName) {
      std::optional<EntityRenderLayer> renderLayer;
      if (renderLayerName)
        renderLayer = parseRenderLayer(*renderLayerName);

      if (auto image = drawable.part.ptr<Drawable::ImagePart>())
        image->transformation.scale(0.125f);

      m_drawables.append({std::move(drawable), renderLayer});
    });
  animationCallbacks.registerCallback("addJsonDrawable", [this](Json drawableConfig, std::optional<String> renderLayerName) {
      std::optional<EntityRenderLayer> renderLayer;
      Drawable drawable(*this->root().imageMetadataDatabase(), drawableConfig);
      if (renderLayerName)
        renderLayer = parseRenderLayer(*renderLayerName);

      if (auto image = drawable.part.ptr<Drawable::ImagePart>())
        image->transformation.scale(0.125f);

      m_drawables.append({std::move(drawable), renderLayer});
    });

  animationCallbacks.registerCallback("clearLightSources", [this]() {
      m_lightSources.clear();
    });
  animationCallbacks.registerCallback("addLightSource", [this](LuaTable const& lightSourceTable) {
      m_lightSources.append({
          lightSourceTable.get<Vec2F>("position"),
          lightSourceTable.get<Color>("color").toRgbF(),
          (LightType)lightSourceTable.get<std::optional<bool>>("pointLight").value_or(false),
          lightSourceTable.get<std::optional<float>>("pointBeam").value_or(float{}),
          lightSourceTable.get<std::optional<float>>("beamAngle").value_or(float{}),
          lightSourceTable.get<std::optional<float>>("beamAmbience").value_or(float{})
        });
    });
  Base::addCallbacks("localAnimator", std::move(animationCallbacks));
}

template <typename Base>
List<std::pair<Drawable, std::optional<EntityRenderLayer>>> const& LuaAnimationComponent<Base>::drawables() {
  return m_drawables;
}

template <typename Base>
List<LightSource> const& LuaAnimationComponent<Base>::lightSources() {
  return m_lightSources;
}

template <typename Base>
List<Particle> LuaAnimationComponent<Base>::pullNewParticles() {
  return take(m_pendingParticles);
}

template <typename Base>
List<std::shared_ptr<AudioInstance>> LuaAnimationComponent<Base>::pullNewAudios() {
  eraseWhere(m_activeAudio, [](std::shared_ptr<AudioInstance> const& audio) {
      return audio->finished();
    });
  return take(m_pendingAudios);
}

template <typename Base>
void LuaAnimationComponent<Base>::contextShutdown() {
  for (auto const& audio : m_activeAudio)
    audio->setLoops(0);
  m_activeAudio.clear();
  Base::contextShutdown();
}

}

export module star.lua_animation_component;

export namespace Star {
  using ::Star::LuaAnimationComponentExceptionTag;
  using ::Star::LuaAnimationComponentException;
  using ::Star::LuaAnimationComponent;
}
