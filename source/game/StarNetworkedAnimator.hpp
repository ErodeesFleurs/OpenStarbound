#pragma once

#include "StarAnimatedPartSet.hpp"
#include "StarConfig.hpp"
#include "StarDrawable.hpp"
#include "StarException.hpp"
#include "StarLightSource.hpp"
#include "StarMixer.hpp"
#include "StarNetElementBasicFields.hpp"
#include "StarNetElementContainers.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarNetElementSignal.hpp"
#include "StarNetElementSyncGroup.hpp"
#include "StarParticle.hpp"
#include "StarPeriodicFunction.hpp"

import std;

namespace Star {

using NetworkedAnimatorException = ExceptionDerived<"NetworkedAnimatorException">;

// Wraps an AnimatedPartSet with a set of optional light sources and particle
// emitters to produce a network capable animation system.
class NetworkedAnimator : public NetElementSyncGroup {
public:
  // Target for dynamic render data such as sounds and particles that are not
  // persistent and are instead produced during a call to update, and may need
  // to be tracked over time.
  class DynamicTarget {
  public:
    // Calls stopAudio()
    ~DynamicTarget();

    auto pullNewAudios() -> List<Ptr<AudioInstance>>;
    auto pullNewParticles() -> List<Particle>;

    // Stops all looping audio immediately and lets non-looping audio finish
    // normally
    void stopAudio();

    // Updates the base position of all un-pulled particles and all active
    // audio.  Not necessary to call, but if not called all pulled data will be
    // relative to (0, 0).
    void updatePosition(Vec2F const& position);

  private:
    friend class NetworkedAnimator;

    void clearFinishedAudio();

    struct PersistentSound {
      Json sound;
      Ptr<AudioInstance> audio;
      float stopRampTime;
    };

    struct ImmediateSound {
      Json sound;
      Ptr<AudioInstance> audio;
    };

    Vec2F position;
    List<Ptr<AudioInstance>> pendingAudios;
    List<Particle> pendingParticles;
    StringMap<PersistentSound> statePersistentSounds;
    StringMap<ImmediateSound> stateImmediateSounds;
    StringMap<List<Ptr<AudioInstance>>> independentSounds;
    HashMap<Ptr<AudioInstance>, Vec2F> currentAudioBasePositions;
  };

  NetworkedAnimator();
  // If passed a string as config, NetworkedAnimator will interpret this as a
  // config path, otherwise it is interpreted as the literal config.
  NetworkedAnimator(Json config, String relativePath = String());

  NetworkedAnimator(NetworkedAnimator&& animator);
  NetworkedAnimator(NetworkedAnimator const& animator);

  auto operator=(NetworkedAnimator&& animator) -> NetworkedAnimator&;
  auto operator=(NetworkedAnimator const& animator) -> NetworkedAnimator&;

  auto stateTypes() const -> StringList;
  auto states(String const& stateType) const -> StringList;

  // Returns whether a state change occurred.  If startNew is true, always
  // forces a state change and starts the state off at the beginning even if
  // this state is already the current state.
  auto setState(String const& stateType, String const& state, bool startNew = false, bool reverse = false) -> bool;
  auto setLocalState(String const& stateType, String const& state, bool startNew = false, bool reverse = false) -> bool;
  auto state(String const& stateType) const -> String;
  auto stateFrame(String const& stateType) const -> int;
  auto stateNextFrame(String const& stateType) const -> int;
  auto stateFrameProgress(String const& stateType) const -> float;
  auto stateTimer(String const& stateType) const -> float;
  auto stateReverse(String const& stateType) const -> bool;

  auto stateCycle(String const& stateType, std::optional<String> state) const -> float;
  auto stateFrames(String const& stateType, std::optional<String> state) const -> int;

  auto hasState(String const& stateType, std::optional<String> const& state = std::nullopt) const -> bool;

  auto constParts() const -> StringMap<AnimatedPartSet::Part> const&;
  auto parts() -> StringMap<AnimatedPartSet::Part>&;
  auto partNames() const -> StringList;

  // Queries, if it exists, a property value from the underlying
  // AnimatedPartSet for the given state or part.  If the property does not
  // exist, returns null.
  auto stateProperty(String const& stateType, String const& propertyName, std::optional<String> state = std::nullopt, std::optional<int> frame = std::nullopt) const -> Json;
  auto stateNextProperty(String const& stateType, String const& propertyName) const -> Json;
  auto partProperty(String const& partName, String const& propertyName, std::optional<String> stateType = std::nullopt, std::optional<String> state = std::nullopt, std::optional<int> frame = std::nullopt) const -> Json;
  auto partNextProperty(String const& partName, String const& propertyName) const -> Json;

  // Returns the transformation from flipping and zooming that is applied to
  // all parts in the NetworkedAnimator.
  auto globalTransformation() const -> Mat3F;
  // The transformation applied from the given set of transformation groups
  auto groupTransformation(StringList const& transformationGroups) const -> Mat3F;
  // The transformation that is applied to the given part NOT including the
  // global transformation
  auto partTransformation(String const& partName) const -> Mat3F;
  // Returns the total transformation for the given part, which includes the
  // globalTransformation, as well as the part rotation, scaling, and
  // translation.
  auto finalPartTransformation(String const& partName) const -> Mat3F;

  // partPoint / partPoly takes a propertyName and looks up the associated part
  // property and interprets is a Vec2F or a PolyF, then applies the final part
  // transformation and returns it.
  auto partPoint(String const& partName, String const& propertyName) const -> std::optional<Vec2F>;
  auto partPoly(String const& partName, String const& propertyName) const -> std::optional<PolyF>;

  // Every part image can have one or more <tag> directives in it, which if set
  // here will be replaced by the tag value when constructing Drawables.  All
  // Drawables can also have a <frame> tag which will be set to whatever the
  // current state frame is (1 indexed, so the first frame is 1).
  void setGlobalTag(String tagName, std::optional<String> tagValue = std::nullopt);
  void removeGlobalTag(String const& tagName);
  auto globalTagPtr(String const& tagName) const -> String const*;
  void setPartTag(String const& partType, String tagName, std::optional<String> tagValue = std::nullopt);
  void setLocalTag(String tagName, std::optional<String> tagValue = std::nullopt);

  void setPartDrawables(String const& partName, List<Drawable> drawables);
  void addPartDrawables(String const& partName, List<Drawable> drawables);

  auto applyPartTags(String const& partName, String apply) const -> String;

  void setProcessingDirectives(Directives const& directives);
  void setZoom(float zoom);
  auto flipped() const -> bool;
  auto flippedRelativeCenterLine() const -> float;
  void setFlipped(bool flipped, float relativeCenterLine = 0.0f);

  // Animation rate defaults to 1.0, which means normal animation speed.  This
  // can be used to globally speed up or slow down all components of
  // NetworkedAnimator together.
  void setAnimationRate(float rate);
  auto animationRate() -> float;

  // Given angle is an absolute angle.  Will rotate over time at the configured
  // angular velocity unless the immediate flag is set.
  auto hasRotationGroup(String const& rotationGroup) const -> bool;
  void rotateGroup(String const& rotationGroup, float targetAngle, bool immediate = false);
  auto currentRotationAngle(String const& rotationGroup) const -> float;

  // Transformation groups can be used for arbitrary part transforamtions.
  // They apply immediately, and are optionally interpolated on slaves.
  auto hasTransformationGroup(String const& transformationGroup) const -> bool;
  void translateTransformationGroup(String const& transformationGroup, Vec2F const& translation);
  void rotateTransformationGroup(String const& transformationGroup, float rotation, Vec2F const& rotationCenter = Vec2F());
  void scaleTransformationGroup(String const& transformationGroup, float scale, Vec2F const& scaleCenter = Vec2F());
  void scaleTransformationGroup(String const& transformationGroup, Vec2F const& scale, Vec2F const& scaleCenter = Vec2F());
  void transformTransformationGroup(String const& transformationGroup, float a, float b, float c, float d, float tx, float ty);
  void resetTransformationGroup(String const& transformationGroup);
  void setTransformationGroup(String const& transformationGroup, Mat3F transform);
  auto getTransformationGroup(String const& transformationGroup) -> Mat3F;

  void translateLocalTransformationGroup(String const& transformationGroup, Vec2F const& translation);
  void rotateLocalTransformationGroup(String const& transformationGroup, float rotation, Vec2F const& rotationCenter = Vec2F());
  void scaleLocalTransformationGroup(String const& transformationGroup, float scale, Vec2F const& scaleCenter = Vec2F());
  void scaleLocalTransformationGroup(String const& transformationGroup, Vec2F const& scale, Vec2F const& scaleCenter = Vec2F());
  void transformLocalTransformationGroup(String const& transformationGroup, float a, float b, float c, float d, float tx, float ty);
  void resetLocalTransformationGroup(String const& transformationGroup);
  void setLocalTransformationGroup(String const& transformationGroup, Mat3F transform);
  auto getLocalTransformationGroup(String const& transformationGroup) -> Mat3F;

  auto hasParticleEmitter(String const& emitterName) const -> bool;
  // Active particle emitters emit over time based on emission rate/variance.
  void setParticleEmitterActive(String const& emitterName, bool active);
  // Set the emission rate in particles / sec for a given emitter
  void setParticleEmitterEmissionRate(String const& emitterName, float emissionRate);
  // Set the optional particle emitter offset region, which particles will be
  // spread around randomly before being spawned
  void setParticleEmitterOffsetRegion(String const& emitterName, RectF const& offsetRegion);

  // Number of times to cycle when emitting a burst of particles.
  void setParticleEmitterBurstCount(String const& emitterName, unsigned burstCount);

  // Cause one time burst of all types of particles in an emitter looping around
  // burstCount times
  void burstParticleEmitter(String const& emitterName);

  auto hasLight(String const& lightName) const -> bool;
  void setLightActive(String const& lightName, bool active);
  void setLightPosition(String const& lightName, Vec2F position);
  void setLightColor(String const& lightName, Color color);
  void setLightPointAngle(String const& lightName, float angle);

  auto hasSound(String const& soundName) const -> bool;
  void setSoundPool(String const& soundName, StringList soundPool);
  // Plays a sound from the given independent sound pool.  Multiple sounds may
  // be played as part of this group, and playing a new one will not interrupt
  // an older one.
  void playSound(String const& soundName, int loops = 0);

  // Setting the sound position, volume, and speed will affect future sounds in
  // this group, as well as any still active sounds from this group.
  void setSoundPosition(String const& soundName, Vec2F const& position);

  void setSoundVolume(String const& soundName, float volume, float rampTime = 0.0f);
  void setSoundPitchMultiplier(String const& soundName, float pitchMultiplier, float rampTime = 0.0f);

  // Stop all sounds played from this sound group
  void stopAllSounds(String const& soundName, float rampTime = 0.0f);

  void setEffectEnabled(String const& effect, bool enabled);

  auto drawables(Vec2F const& translate = Vec2F()) const -> List<Drawable>;
  auto drawablesWithZLevel(Vec2F const& translate = Vec2F()) const -> List<std::pair<Drawable, float>>;

  auto lightSources(Vec2F const& translate = Vec2F()) const -> List<LightSource>;

  // Dynamic target is optional, if not given, generated particles and sounds
  // will be discarded
  void update(float dt, DynamicTarget* dynamicTarget);

  // Run through the current animations until the final frame, including any
  // transition animations.
  void finishAnimations();
  auto version() const -> std::uint8_t;

private:
  struct RotationGroup {
    float angularVelocity;
    Vec2F rotationCenter;

    NetElementFloat targetAngle;
    float currentAngle;

    NetElementEvent netImmediateEvent;
  };

  struct TransformationGroup {
    [[nodiscard]] auto affineTransform() const -> Mat3F;
    void setAffineTransform(Mat3F const& matrix);

    [[nodiscard]] auto localAffineTransform() const -> Mat3F;
    void setLocalAffineTransform(Mat3F const& matrix);

    [[nodiscard]] auto animationAffineTransform() const -> Mat3F;
    void setAnimationAffineTransform(Mat3F const& matrix);
    void setAnimationAffineTransform(Mat3F const& mat1, Mat3F const& mat2, float progress);

    bool interpolated;

    Mat3F localTransform;

    NetElementFloat xTranslation;
    NetElementFloat yTranslation;
    NetElementFloat xScale;
    NetElementFloat yScale;
    NetElementFloat xShear;
    NetElementFloat yShear;

    float xTranslationAnimation;
    float yTranslationAnimation;
    float xScaleAnimation;
    float yScaleAnimation;
    float xShearAnimation;
    float yShearAnimation;
  };

  struct ParticleEmitter {
    struct ParticleConfig {
      ParticleVariantCreator creator;
      unsigned count;
      Vec2F offset;
      bool flip;
    };

    NetElementFloat emissionRate;
    float emissionRateVariance;
    NetElementData<RectF> offsetRegion;
    std::optional<String> anchorPart;
    StringList transformationGroups;
    std::optional<String> rotationGroup;
    std::optional<Vec2F> rotationCenter;

    List<ParticleConfig> particleList;

    NetElementBool active;
    NetElementUInt burstCount;
    NetElementUInt randomSelectCount;
    NetElementEvent burstEvent;

    float timer;
  };

  struct Light {
    NetElementBool active;
    NetElementFloat xPosition;
    NetElementFloat yPosition;
    NetElementData<Color> color;
    NetElementFloat pointAngle;
    std::optional<String> anchorPart;
    StringList transformationGroups;
    std::optional<String> rotationGroup;
    std::optional<Vec2F> rotationCenter;

    std::optional<PeriodicFunction<float>> flicker;
    bool pointLight;
    float pointBeam;
    float beamAmbience;
  };

  enum class SoundSignal {
    Play,
    StopAll
  };

  struct Sound {
    float rangeMultiplier;
    NetElementData<StringList> soundPool;
    NetElementFloat xPosition;
    NetElementFloat yPosition;
    NetElementFloat volumeTarget;
    NetElementFloat volumeRampTime;
    NetElementFloat pitchMultiplierTarget;
    NetElementFloat pitchMultiplierRampTime;
    NetElementInt loops;
    NetElementSignal<SoundSignal> signals;
  };

  struct Effect {
    String type;
    float time;
    Directives directives;

    NetElementBool enabled;
    float timer;
  };

  struct StateInfo {
    NetElementSize stateIndex;
    NetElementEvent startedEvent;
    bool wasUpdated;
    NetElementBool reverse;
  };

  void setupNetStates();

  void netElementsNeedLoad(bool full) override;
  void netElementsNeedStore() override;

  auto mergeIncludes(Json config, Json includes, String relativePath) -> Json;

  String m_relativePath;
  std::uint8_t m_animatorVersion;

  AnimatedPartSet m_animatedParts;
  OrderedHashMap<String, StateInfo> m_stateInfo;
  OrderedHashMap<String, RotationGroup> m_rotationGroups;
  OrderedHashMap<String, TransformationGroup> m_transformationGroups;
  OrderedHashMap<String, ParticleEmitter> m_particleEmitters;
  OrderedHashMap<String, Light> m_lights;
  OrderedHashMap<String, Sound> m_sounds;
  OrderedHashMap<String, Effect> m_effects;

  NetElementData<Directives> m_processingDirectives;
  NetElementFloat m_zoom;

  NetElementBool m_flipped;
  NetElementFloat m_flippedRelativeCenterLine;

  NetElementFloat m_animationRate;

  NetElementHashMap<String, String> m_globalTags;
  StableStringMap<NetElementHashMap<String, String>> m_partTags;
  HashMap<String, String> m_localTags;

  HashMap<String, List<Drawable>> m_partDrawables;

  mutable StringMap<std::pair<std::size_t, Drawable>> m_cachedPartDrawables;
};

}// namespace Star
