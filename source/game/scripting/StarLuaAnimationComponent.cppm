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

// Produces local drawables, lights, audio and particles from script callbacks.
template <typename Base>
class LuaAnimationComponent : public Base {
public:
  using Base::Base;
  List<std::pair<Drawable, std::optional<EntityRenderLayer>>> const& drawables();
  List<LightSource> const& lightSources();
  List<Particle> pullNewParticles();
  List<std::shared_ptr<AudioInstance>> pullNewAudios();
protected:
  [[nodiscard]] std::expected<void, StarException> contextSetup() override;
  [[nodiscard]] std::expected<void, StarException> contextShutdown() override;
private:
  List<Particle> m_pendingParticles;
  List<std::shared_ptr<AudioInstance>> m_pendingAudios;
  List<std::shared_ptr<AudioInstance>> m_activeAudio;
  List<std::pair<Drawable, std::optional<EntityRenderLayer>>> m_drawables;
  List<LightSource> m_lightSources;
};

template <typename Base>
[[nodiscard]] std::expected<void, StarException> LuaAnimationComponent<Base>::contextSetup() {
  if (auto result = Base::contextSetup(); !result)
    return std::unexpected(std::move(result.error()));
  if (Base::hasCallbacks("localAnimator"))
    return {};
  LuaCallbacks callbacks;
  if (auto result = callbacks.registerCallback("playAudio", [this](String const& sound, std::optional<int> loops, std::optional<float> volume) -> std::expected<std::shared_ptr<AudioInstance>, StarException> {
    auto assets = this->root().assets();
    if (!assets)
      return std::unexpected(std::move(assets.error()));
    auto source = (*assets)->audio(sound);
    if (!source)
      return std::unexpected(std::move(source.error()));
    auto audio = AudioInstance::create(**source);
    if (!audio)
      return std::unexpected(std::move(audio.error()));
    (*audio)->setLoops(loops.value_or(0));
    (*audio)->setVolume(volume.value_or(1.0f));
    m_pendingAudios.append(*audio);
    m_activeAudio.append(*audio);
    return audio;
  }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("spawnParticle", [this](Json const& config, std::optional<Vec2F> const& position) -> std::expected<void, StarException> {
    auto database = this->root().particleDatabase();
    if (!database)
      return std::unexpected(std::move(database.error()));
    auto particle = (*database)->particle(config);
    if (!particle)
      return std::unexpected(std::move(particle.error()));
    particle->translate(position.value_or(Vec2F{}));
    m_pendingParticles.append(std::move(*particle));
    return {};
  }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("clearDrawables", [this]() { m_drawables.clear(); }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("addDrawable", [this](Drawable drawable, std::optional<String> layerName) -> std::expected<void, StarException> {
    std::optional<EntityRenderLayer> layer;
    if (layerName) {
      auto parsed = parseRenderLayer(*layerName);
      if (!parsed)
        return std::unexpected(std::move(parsed.error()));
      layer = *parsed;
    }
    if (auto image = std::get_if<Drawable::ImagePart>(&drawable.part))
      image->transformation.scale(0.125f);
    m_drawables.append({std::move(drawable), layer});
    return {};
  }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("addJsonDrawable", [this](Json config, std::optional<String> layerName) -> std::expected<void, StarException> {
    auto metadata = this->root().imageMetadataDatabase();
    if (!metadata)
      return std::unexpected(std::move(metadata.error()));
    auto drawable = Drawable::create(**metadata, config);
    if (!drawable)
      return std::unexpected(std::move(drawable.error()));
    std::optional<EntityRenderLayer> layer;
    if (layerName) {
      auto parsed = parseRenderLayer(*layerName);
      if (!parsed)
        return std::unexpected(std::move(parsed.error()));
      layer = *parsed;
    }
    if (auto image = std::get_if<Drawable::ImagePart>(&drawable->part))
      image->transformation.scale(0.125f);
    m_drawables.append({std::move(*drawable), layer});
    return {};
  }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("clearLightSources", [this]() { m_lightSources.clear(); }); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("addLightSource", [this](LuaTable const& table) -> std::expected<void, StarException> {
    auto position = table.get<Vec2F>("position");
    if (!position)
      return std::unexpected(std::move(position.error()));
    auto color = table.get<Color>("color");
    if (!color)
      return std::unexpected(std::move(color.error()));
    auto point = table.get<std::optional<bool>>("pointLight");
    if (!point)
      return std::unexpected(std::move(point.error()));
    auto beam = table.get<std::optional<float>>("pointBeam");
    if (!beam)
      return std::unexpected(std::move(beam.error()));
    auto angle = table.get<std::optional<float>>("beamAngle");
    if (!angle)
      return std::unexpected(std::move(angle.error()));
    auto ambience = table.get<std::optional<float>>("beamAmbience");
    if (!ambience)
      return std::unexpected(std::move(ambience.error()));
    m_lightSources.append({*position, color->toRgbF(), (LightType)point->value_or(false), beam->value_or(float{}), angle->value_or(float{}), ambience->value_or(float{})});
    return {};
  }); !result)
    return std::unexpected(std::move(result.error()));
  return Base::addCallbacks("localAnimator", std::move(callbacks));
}

template <typename Base>
List<std::pair<Drawable, std::optional<EntityRenderLayer>>> const& LuaAnimationComponent<Base>::drawables() { return m_drawables; }

template <typename Base>
List<LightSource> const& LuaAnimationComponent<Base>::lightSources() { return m_lightSources; }

template <typename Base>
List<Particle> LuaAnimationComponent<Base>::pullNewParticles() { return take(m_pendingParticles); }

template <typename Base>
List<std::shared_ptr<AudioInstance>> LuaAnimationComponent<Base>::pullNewAudios() {
  eraseWhere(m_activeAudio, [](std::shared_ptr<AudioInstance> const& audio) { return audio->finished(); });
  return take(m_pendingAudios);
}

template <typename Base>
[[nodiscard]] std::expected<void, StarException> LuaAnimationComponent<Base>::contextShutdown() {
  for (auto const& audio : m_activeAudio)
    audio->setLoops(0);
  m_activeAudio.clear();
  return Base::contextShutdown();
}

}

export module star.lua_animation_component;

export namespace Star {
  using ::Star::LuaAnimationComponentExceptionTag;
  using ::Star::LuaAnimationComponentException;
  using ::Star::LuaAnimationComponent;
}
