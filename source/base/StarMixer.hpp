#pragma once

#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarMaybe.hpp"
#include "StarSet.hpp"
#include "StarThread.hpp"
#include "StarVector.hpp"

namespace Star {

class AudioInstance;
using AudioInstancePtr = SharedPtr<AudioInstance>;
class Mixer;
using MixerPtr = SharedPtr<Mixer>;

struct RampedValue {
  float value;
  float target;
  float velocity;
};

enum class MixerGroup : uint8_t {
  Effects,
  Music,
  Cinematic,
  Instruments
};

class AudioInstance {
public:
  AudioInstance(Audio const& audio);

  [[nodiscard]] Maybe<Vec2F> position() const;
  void setPosition(Maybe<Vec2F> position);
  // If the audio has no position, sets the position to zero before translating
  void translate(Vec2F const& distance);

  [[nodiscard]] float rangeMultiplier() const;
  void setRangeMultiplier(float rangeMultiplier);

  void setVolume(float targetValue, float rampTime = 0.0f);
  void setPitchMultiplier(float targetValue, float rampTime = 0.0f);

  // Returns the currently remaining loops
  [[nodiscard]] int loops() const;
  // Sets the remaining loops, set to 0 to stop looping
  void setLoops(int loops);

  // Returns the current audio playing time position
  [[nodiscard]] double currentTime() const;
  // Total length of time of the audio in seconds
  [[nodiscard]] double totalTime() const;
  // Seeks the audio to the current time in seconds
  void seekTime(double time);

  // The MixerGroup defaults to Effects
  [[nodiscard]] MixerGroup mixerGroup() const;
  void setMixerGroup(MixerGroup mixerGroup);

  // If set, uses wall clock time in milliseconds to set precise start and stop
  // times for the AudioInstance
  void setClockStart(Maybe<int64_t> clockStartTime);
  void setClockStop(Maybe<int64_t> clockStopTime, int64_t fadeOutTime = 0);

  void stop(float rampTime = 0.0f);
  [[nodiscard]] bool finished() const;

private:
  friend class Mixer;

  mutable ReadersWriterMutex m_mutex;

  Audio m_audio;

  MixerGroup m_mixerGroup = MixerGroup::Effects;

  RampedValue m_volume = {1.0f, 1.0f, 0};

  float m_pitchMultiplier = 1.0f;
  float m_pitchMultiplierTarget = 1.0f;
  float m_pitchMultiplierVelocity = 0.0f;

  int m_loops = 0;
  bool m_stopping = false;
  bool m_finished = false;

  Maybe<Vec2F> m_position;
  float m_rangeMultiplier = 1.0f;

  Maybe<int64_t> m_clockStart;
  Maybe<int64_t> m_clockStop;
  int64_t m_clockStopFadeOut = 0;
};

// Thread safe mixer class with basic effects support.
class Mixer {
public:
  using ExtraMixFunction = function<void(int16_t* buffer, size_t frames, unsigned channels)>;
  using EffectFunction = function<void(int16_t* buffer, size_t frames, unsigned channels)>;
  using PositionalAttenuationFunction = function<float(unsigned, Vec2F, float)>;

  Mixer(unsigned sampleRate, unsigned channels);

  [[nodiscard]] unsigned sampleRate() const;
  [[nodiscard]] unsigned channels() const;

  // Construct a really crappy low-pass filter based on averaging
  [[nodiscard]] EffectFunction lowpass(size_t avgSize) const;
  // Construct a very simple echo filter.
  [[nodiscard]] EffectFunction echo(float time, float dry, float wet) const;

  // Adds / removes effects that affect all playback.
  void addEffect(String const& effectName, EffectFunction effectFunction, float rampTime);
  void removeEffect(String const& effectName, float rampTime);
  [[nodiscard]] StringList currentEffects();
  [[nodiscard]] bool hasEffect(String const& effectName);

  // Global speed
  void setSpeed(float speed);

  // Global volume
  void setVolume(float volume, float rampTime);

  // per mixer group volume
  void setGroupVolume(MixerGroup group, float targetValue, float rampTime = 0.0f);

  void play(AudioInstancePtr sample);

  void stopAll(float rampTime);

  // Reads pending audio data.  This is thread safe with the other Mixer
  // methods, but only one call to read may be active at a time.
  void read(int16_t* samples, size_t frameCount, ExtraMixFunction extraMixFunction = {});

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

  HashMap<AudioInstancePtr, AudioState> m_audios;

  Mutex m_effectsMutex;
  StringMap<shared_ptr<EffectInfo>> m_effects;

  List<int16_t> m_mixBuffer;

  Map<MixerGroup, RampedValue> m_groupVolumes;
  atomic<float> m_speed;
};

}// namespace Star
