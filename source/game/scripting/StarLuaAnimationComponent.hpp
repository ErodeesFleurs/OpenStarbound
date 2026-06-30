#pragma once

#include "StarAssets.hpp"
#include "StarDrawable.hpp"
#include "StarEntityRenderingTypes.hpp"
#include "StarJsonExtra.hpp"
#include "StarLightSource.hpp"
#include "StarLuaComponents.hpp"
#include "StarLuaConverters.hpp"
#include "StarMixer.hpp"
#include "StarParticle.hpp"
#include "StarParticleDatabase.hpp"
#include "StarWorld.hpp"

namespace Star {

struct LuaAnimationComponentExceptionTag {
  static constexpr char const* typeName = "LuaAnimationComponentException";
};
using LuaAnimationComponentException = TypedException<LuaComponentException, LuaAnimationComponentExceptionTag>;

// Lua component that allows lua to directly produce drawables, light sources,
// audios, and particles.  Adds a "localAnimation" callback table.
template <typename Base>
class LuaAnimationComponent : public Base {
public:
  LuaAnimationComponent(AssetsConstPtr assets = {});
  void init(World& world);
  void uninit();

  List<pair<Drawable, Maybe<EntityRenderLayer>>> const& drawables();
  List<LightSource> const& lightSources();

  List<Particle> pullNewParticles();
  List<AudioInstancePtr> pullNewAudios();

protected:
  // Clears looping audio on context shutdown
  void contextShutdown() override;

private:
  AssetsConstPtr m_assets;
  ParticleDatabaseConstPtr m_particleDatabase;

  List<Particle> m_pendingParticles;
  List<AudioInstancePtr> m_pendingAudios;
  List<AudioInstancePtr> m_activeAudio;

  List<pair<Drawable, Maybe<EntityRenderLayer>>> m_drawables;
  List<LightSource> m_lightSources;
};

template <typename Base>
LuaAnimationComponent<Base>::LuaAnimationComponent(AssetsConstPtr assets) {
  if (!assets)
    throw LuaAnimationComponentException("LuaAnimationComponent requires assets service");
  m_assets = std::move(assets);

  LuaCallbacks animationCallbacks;
  animationCallbacks.registerCallback("playAudio", [this](String const& sound, Maybe<int> loops, Maybe<float> volume) {
    auto audio = make_shared<AudioInstance>(*m_assets->audio(sound));
    audio->setLoops(loops.value(0));
    audio->setVolume(volume.value(1.0f));
    m_pendingAudios.append(audio);
    m_activeAudio.append(audio);
  });
  animationCallbacks.registerCallback("spawnParticle", [this](Json const& particleConfig, Maybe<Vec2F> const& position) {
    if (!m_particleDatabase)
      throw LuaAnimationComponentException("LuaAnimationComponent requires initialized particle database service");
    auto particle = m_particleDatabase->particle(particleConfig);
    particle.translate(position.value());
    m_pendingParticles.append(particle);
  });
  animationCallbacks.registerCallback("clearDrawables", [this]() {
    m_drawables.clear();
  });
  animationCallbacks.registerCallback("addDrawable", [this](Drawable drawable, Maybe<String> renderLayerName) {
    Maybe<EntityRenderLayer> renderLayer;
    if (renderLayerName)
      renderLayer = parseRenderLayer(*renderLayerName);

    if (auto image = drawable.part.ptr<Drawable::ImagePart>())
      image->transformation.scale(0.125f);

    m_drawables.append({std::move(drawable), renderLayer});
  });
  animationCallbacks.registerCallback("addJsonDrawable", [this](Json drawableConfig, Maybe<String> renderLayerName) {
    Maybe<EntityRenderLayer> renderLayer;
    Drawable drawable(drawableConfig);
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
    m_lightSources.append({lightSourceTable.get<Vec2F>("position"),
                           lightSourceTable.get<Color>("color").toRgbF(),
                           static_cast<LightType>(lightSourceTable.get<Maybe<bool>>("pointLight").value()),
                           lightSourceTable.get<Maybe<float>>("pointBeam").value(),
                           lightSourceTable.get<Maybe<float>>("beamAngle").value(),
                           lightSourceTable.get<Maybe<float>>("beamAmbience").value()});
  });
  Base::addCallbacks("localAnimator", std::move(animationCallbacks));
}

template <typename Base>
void LuaAnimationComponent<Base>::init(World& world) {
  m_particleDatabase = world.particleDatabase();
  Base::init(world);
}

template <typename Base>
void LuaAnimationComponent<Base>::uninit() {
  m_particleDatabase.reset();
  Base::uninit();
}

template <typename Base>
List<pair<Drawable, Maybe<EntityRenderLayer>>> const& LuaAnimationComponent<Base>::drawables() {
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
List<AudioInstancePtr> LuaAnimationComponent<Base>::pullNewAudios() {
  eraseWhere(m_activeAudio, [](AudioInstancePtr const& audio) {
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

}// namespace Star
