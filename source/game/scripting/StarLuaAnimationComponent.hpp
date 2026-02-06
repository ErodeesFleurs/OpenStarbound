#pragma once

#include "StarLuaComponents.hpp"
#include "StarJsonExtra.hpp"
#include "StarLightSource.hpp"
#include "StarDrawable.hpp"
#include "StarEntityRenderingTypes.hpp"
#include "StarMixer.hpp"
#include "StarParticleDatabase.hpp"
#include "StarParticle.hpp"
#include "StarRoot.hpp"
#include "StarAssets.hpp"
#include "StarLuaConverters.hpp"

#include <optional>

namespace Star {

STAR_EXCEPTION(LuaAnimationComponentException, LuaComponentException);

// Lua component that allows lua to directly produce drawables, light sources,
// audios, and particles.  Adds a "localAnimation" callback table.
template <typename Base>
class LuaAnimationComponent : public Base {
public:
  LuaAnimationComponent();

  List<pair<Drawable, std::optional<EntityRenderLayer>>> const& drawables();
  List<LightSource> const& lightSources();

  List<Particle> pullNewParticles();
  List<AudioInstancePtr> pullNewAudios();

protected:
  // Clears looping audio on context shutdown
  void contextShutdown() override;

private:
  List<Particle> m_pendingParticles;
  List<AudioInstancePtr> m_pendingAudios;
  List<AudioInstancePtr> m_activeAudio;

  List<pair<Drawable, std::optional<EntityRenderLayer>>> m_drawables;
  List<LightSource> m_lightSources;
};

template <typename Base>
LuaAnimationComponent<Base>::LuaAnimationComponent() {
  LuaCallbacks animationCallbacks;
  animationCallbacks.registerCallback("playAudio", [this](String const& sound, std::optional<int> loops, std::optional<float> volume) {
      auto audio = make_shared<AudioInstance>(*Root::singleton().assets()->audio(sound));
      audio->setLoops(loops.value_or(0));
      audio->setVolume(volume.value_or(1.0f));
      m_pendingAudios.append(audio);
      m_activeAudio.append(audio);
    });
  animationCallbacks.registerCallback("spawnParticle", [this](Json const& particleConfig, std::optional<Vec2F> const& position) {
      auto particle = Root::singleton().particleDatabase()->particle(particleConfig);
      particle.translate(position.value_or(Vec2F()));
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
      m_lightSources.append({
          lightSourceTable.get<Vec2F>("position"),
          lightSourceTable.get<Color>("color").toRgbF(),
          (LightType)lightSourceTable.get<std::optional<bool>>("pointLight").value_or(false),
          lightSourceTable.get<std::optional<float>>("pointBeam").value_or(0.0f),
          lightSourceTable.get<std::optional<float>>("beamAngle").value_or(0.0f),
          lightSourceTable.get<std::optional<float>>("beamAmbience").value_or(0.0f)
        });
    });
  Base::addCallbacks("localAnimator", std::move(animationCallbacks));
}

template <typename Base>
List<pair<Drawable, std::optional<EntityRenderLayer>>> const& LuaAnimationComponent<Base>::drawables() {
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

}
