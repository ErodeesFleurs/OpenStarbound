#pragma once

// Visual/rendering ECS components for OpenStarbound
// These components handle sprites, animations, lights, and particles

#include "StarVector.hpp"
#include "StarString.hpp"
#include "StarList.hpp"
#include "StarColor.hpp"
#include "StarDrawable.hpp"
#include "StarLightSource.hpp"
#include "StarParticle.hpp"

namespace Star {
namespace ECS {

//=============================================================================
// Visual Components
//=============================================================================

// Sprite component for simple sprite rendering
struct SpriteComponent {
  String imagePath = "";
  Directives directives = {};
  Vec2F offset = {};
  float zLevel = 0.0f;
  bool visible = true;
  bool fullbright = false;
  bool centered = true;
  float scale = 1.0f;
  Color color = Color::White;
};

// Animation component for animated entities
struct AnimationComponent {
  String animationState = "idle";
  float animationTime = 0.0f;
  float animationSpeed = 1.0f;
  bool animationLooping = true;
  bool animationFinished = false;
  String nextState = "";
  
  void setState(String const& state, bool loop = true) {
    if (animationState != state) {
      animationState = state;
      animationTime = 0.0f;
      animationLooping = loop;
      animationFinished = false;
    }
  }
  
  void update(float dt) {
    animationTime += dt * animationSpeed;
  }
};

// Networked animator component (for complex animations)
struct NetworkedAnimatorComponent {
  String animatorConfig = "";
  StringMap<String> animatorStates = {};
  StringMap<float> animatorParameters = {};
  StringMap<bool> animatorPartsEnabled = {};
  bool flipped = false;
  
  void setGlobalTag(String const& tag, String const& value) {
    animatorStates[tag] = value;
  }
  
  void setParameter(String const& param, float value) {
    animatorParameters[param] = value;
  }
};

// Light sources
struct LightSourceComponent {
  List<LightSource> sources = {};
  
  void addLight(LightSource source) {
    sources.append(std::move(source));
  }
  
  void addPointLight(Vec2F position, Color color, float intensity = 1.0f) {
    LightSource light;
    light.position = position;
    light.color = color.toRgb();
    light.pointLight = true;
    sources.append(std::move(light));
  }
  
  void clearLights() {
    sources.clear();
  }
};

// Particle emitter
struct ParticleEmitterComponent {
  List<Particle> pendingParticles = {};
  String particleConfig = "";
  float emissionRate = 0.0f;
  float emissionTimer = 0.0f;
  bool emitting = true;
  Vec2F emissionOffset = {};
  
  void emit(Particle particle) {
    pendingParticles.append(std::move(particle));
  }
  
  List<Particle> pullParticles() {
    List<Particle> result;
    std::swap(result, pendingParticles);
    return result;
  }
};

// Audio/sound source
struct AudioSourceComponent {
  List<String> pendingSounds = {};
  String ambientSound = "";
  float volume = 1.0f;
  float pitch = 1.0f;
  
  void playSound(String const& sound) {
    pendingSounds.append(sound);
  }
  
  List<String> pullSounds() {
    List<String> result;
    std::swap(result, pendingSounds);
    return result;
  }
};

// Effect emitter (visual effects like sparks, smoke)
struct EffectEmitterComponent {
  StringSet activeEffects = {};
  
  void addEffect(String const& effect) {
    activeEffects.add(effect);
  }
  
  void removeEffect(String const& effect) {
    activeEffects.remove(effect);
  }
  
  void clearEffects() {
    activeEffects.clear();
  }
};

// Drawable override (for custom drawing)
struct DrawableOverrideComponent {
  List<Drawable> drawables = {};
  bool replaceDefault = false;
  
  void setDrawables(List<Drawable> newDrawables) {
    drawables = std::move(newDrawables);
  }
  
  void clearDrawables() {
    drawables.clear();
  }
};

} // namespace ECS
} // namespace Star
