#pragma once

#include "StarAudio.hpp"
#include "StarConfig.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarThread.hpp"
#include "StarVector.hpp"

import std;

namespace Star {

struct RampedValue {
  float value;
  float target;
  float velocity;
};

enum class MixerGroup : std::uint8_t {
  Effects,
  Music,
  Cinematic,
  Instruments
};

class AudioInstance {
public:
  AudioInstance(Audio const& audio);

  auto position() const -> std::optional<Vec2F>;
  void setPosition(std::optional<Vec2F> position);
  // If the audio has no position, sets the position to zero before translating
  void translate(Vec2F const& distance);

  auto rangeMultiplier() const -> float;
  void setRangeMultiplier(float rangeMultiplier);

  void setVolume(float targetValue, float rampTime = 0.0f);
  void setPitchMultiplier(float targetValue, float rampTime = 0.0f);

  // Returns the currently remaining loops
  auto loops() const -> int;
  // Sets the remaining loops, set to 0 to stop looping
  void setLoops(int loops);

  // Returns the current audio playing time position
  auto currentTime() const -> double;
  // Total length of time of the audio in seconds
  auto totalTime() const -> double;
  // Seeks the audio to the current time in seconds
  void seekTime(double time);

  // The MixerGroup defaults to Effects
  auto mixerGroup() const -> MixerGroup;
  void setMixerGroup(MixerGroup mixerGroup);

  // If set, uses wall clock time in milliseconds to set precise start and stop
  // times for the AudioInstance
  void setClockStart(std::optional<std::int64_t> clockStartTime);
  void setClockStop(std::optional<std::int64_t> clockStopTime, std::int64_t fadeOutTime = 0);

  void stop(float rampTime = 0.0f);
  auto finished() const -> bool;

private:
  friend class Mixer;

  mutable Mutex m_mutex;

  Audio m_audio;

  MixerGroup m_mixerGroup;

  RampedValue m_volume;

  float m_pitchMultiplier;
  float m_pitchMultiplierTarget;
  float m_pitchMultiplierVelocity;

  int m_loops;
  bool m_stopping;
  bool m_finished;

  std::optional<Vec2F> m_position;
  float m_rangeMultiplier;

  std::optional<std::int64_t> m_clockStart;
  std::optional<std::int64_t> m_clockStop;
  std::int64_t m_clockStopFadeOut;
};

// Thread safe mixer class with basic effects support.
class Mixer {
public:
  using ExtraMixFunction = std::function<void(std::int16_t* buffer, std::size_t frames, unsigned channels)>;
  using EffectFunction = std::function<void(std::int16_t* buffer, std::size_t frames, unsigned channels)>;
  using PositionalAttenuationFunction = std::function<float(unsigned, Vec2F, float)>;

  Mixer(unsigned sampleRate, unsigned channels);

  auto sampleRate() const -> unsigned;
  auto channels() const -> unsigned;

  // Construct a really crappy low-pass filter based on averaging
  auto lowpass(std::size_t avgSize) const -> EffectFunction;
  // Construct a very simple echo filter.
  auto echo(float time, float dry, float wet) const -> EffectFunction;

  // Adds / removes effects that affect all playback.
  void addEffect(String const& effectName, EffectFunction effectFunction, float rampTime);
  void removeEffect(String const& effectName, float rampTime);
  auto currentEffects() -> StringList;
  auto hasEffect(String const& effectName) -> bool;

  // Global speed
  void setSpeed(float speed);

  // Global volume
  void setVolume(float volume, float rampTime);

  // per mixer group volume
  void setGroupVolume(MixerGroup group, float targetValue, float rampTime = 0.0f);

  void play(Ptr<AudioInstance> sample);

  void stopAll(float rampTime);

  // Reads pending audio data.  This is thread safe with the other Mixer
  // methods, but only one call to read may be active at a time.
  void read(std::int16_t* samples, std::size_t frameCount, ExtraMixFunction extraMixFunction = {});

  // Call within the main loop of the program using Mixer, calculates
  // positional attenuation of audio and does cleanup.
  void update(float dt, PositionalAttenuationFunction positionalAttenuationFunction = {});

private:
  struct EffectInfo {
    EffectFunction effectFunction;
    float amount;
    float velocity;
    bool finished;
  };

  struct AudioState {
    List<float> positionalChannelVolumes;
  };

  Mutex m_mutex;
  unsigned m_sampleRate;
  unsigned m_channels;

  RampedValue m_volume;

  Mutex m_queueMutex;

  HashMap<Ptr<AudioInstance>, AudioState> m_audios;

  Mutex m_effectsMutex;
  StringMap<std::shared_ptr<EffectInfo>> m_effects;

  List<std::int16_t> m_mixBuffer;

  Map<MixerGroup, RampedValue> m_groupVolumes;
  std::atomic<float> m_speed;
};

}// namespace Star
