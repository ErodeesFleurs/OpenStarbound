#include "StarMixer.hpp"

#include "StarConfig.hpp"
#include "StarInterpolation.hpp"
#include "StarLogging.hpp"
#include "StarTime.hpp"

import std;

namespace Star {

namespace {
auto rateOfChangeFromRampTime(float rampTime) -> float {
  static const float MaxRate = 10000.0f;

  if (rampTime < 1.0f / MaxRate)
    return MaxRate;
  else
    return 1.0f / rampTime;
}
}// namespace

AudioInstance::AudioInstance(Audio const& audio)
    : m_audio(std::move(audio)) {
  m_mixerGroup = MixerGroup::Effects;

  m_volume = {.value = 1.0f, .target = 1.0f, .velocity = 0};

  m_pitchMultiplier = 1.0f;
  m_pitchMultiplierTarget = 1.0f;
  m_pitchMultiplierVelocity = 0;

  m_loops = 0;
  m_stopping = false;
  m_finished = false;

  m_rangeMultiplier = 1.0f;

  m_clockStopFadeOut = 0.0f;
}

auto AudioInstance::position() const -> std::optional<Vec2F> {
  MutexLocker locker(m_mutex);
  return m_position;
}

void AudioInstance::setPosition(std::optional<Vec2F> position) {
  MutexLocker locker(m_mutex);
  m_position = position;
}

void AudioInstance::translate(Vec2F const& distance) {
  MutexLocker locker(m_mutex);
  if (m_position)
    *m_position += distance;
  else
    m_position = distance;
}

auto AudioInstance::rangeMultiplier() const -> float {
  MutexLocker locker(m_mutex);
  return m_rangeMultiplier;
}

void AudioInstance::setRangeMultiplier(float rangeMultiplier) {
  MutexLocker locker(m_mutex);
  m_rangeMultiplier = rangeMultiplier;
}

void AudioInstance::setVolume(float targetValue, float rampTime) {
  MutexLocker locker(m_mutex);

  if (m_stopping)
    return;

  if (rampTime <= 0.0f) {
    m_volume.value = targetValue;
    m_volume.target = targetValue;
    m_volume.velocity = 0.0f;
  } else {
    m_volume.target = targetValue;
    m_volume.velocity = rateOfChangeFromRampTime(rampTime);
  }
}

void AudioInstance::setPitchMultiplier(float targetValue, float rampTime) {
  MutexLocker locker(m_mutex);

  if (m_stopping)
    return;

  if (rampTime <= 0.0f) {
    m_pitchMultiplier = targetValue;
    m_pitchMultiplierTarget = targetValue;
    m_pitchMultiplierVelocity = 0.0f;
  } else {
    m_pitchMultiplierTarget = targetValue;
    m_pitchMultiplierVelocity = rateOfChangeFromRampTime(rampTime);
  }
}

auto AudioInstance::loops() const -> int {
  MutexLocker locker(m_mutex);
  return m_loops;
}

void AudioInstance::setLoops(int loops) {
  MutexLocker locker(m_mutex);
  m_loops = loops;
}

auto AudioInstance::currentTime() const -> double {
  return m_audio.currentTime();
}

auto AudioInstance::totalTime() const -> double {
  return m_audio.totalTime();
}

void AudioInstance::seekTime(double time) {
  m_audio.seekTime(time);
}

auto AudioInstance::mixerGroup() const -> MixerGroup {
  MutexLocker locker(m_mutex);
  return m_mixerGroup;
}

void AudioInstance::setMixerGroup(MixerGroup mixerGroup) {
  MutexLocker locker(m_mutex);
  m_mixerGroup = mixerGroup;
}

void AudioInstance::setClockStart(std::optional<std::int64_t> clockStartTime) {
  MutexLocker locker(m_mutex);
  m_clockStart = clockStartTime;
}

void AudioInstance::setClockStop(std::optional<std::int64_t> clockStopTime, std::int64_t fadeOutTime) {
  MutexLocker locker(m_mutex);
  m_clockStop = clockStopTime;
  m_clockStopFadeOut = fadeOutTime;
}

void AudioInstance::stop(float rampTime) {
  MutexLocker locker(m_mutex);

  if (rampTime <= 0.0f) {
    m_volume.value = 0.0f;
    m_volume.target = 0.0f;
    m_volume.velocity = 0.0f;

    m_pitchMultiplierTarget = 0.0f;
    m_pitchMultiplierVelocity = 0.0f;
  } else {
    m_volume.target = 0.0f;
    m_volume.velocity = rateOfChangeFromRampTime(rampTime);
  }

  m_stopping = true;
}

auto AudioInstance::finished() const -> bool {
  return m_finished;
}

Mixer::Mixer(unsigned sampleRate, unsigned channels) {
  m_sampleRate = sampleRate;
  m_channels = channels;

  m_volume = {.value = 1.0f, .target = 1.0f, .velocity = 0};

  m_groupVolumes[MixerGroup::Effects] = {.value = 1.0f, .target = 1.0f, .velocity = 0};
  m_groupVolumes[MixerGroup::Music] = {.value = 1.0f, .target = 1.0f, .velocity = 0};
  m_groupVolumes[MixerGroup::Cinematic] = {.value = 1.0f, .target = 1.0f, .velocity = 0};
  m_groupVolumes[MixerGroup::Instruments] = {.value = 1.0f, .target = 1.0f, .velocity = 0};

  m_speed = 1.0f;
}

auto Mixer::sampleRate() const -> unsigned {
  return m_sampleRate;
}

auto Mixer::channels() const -> unsigned {
  return m_channels;
}

void Mixer::addEffect(String const& effectName, EffectFunction effectFunction, float rampTime) {
  MutexLocker locker(m_effectsMutex);
  m_effects[effectName] = std::make_shared<EffectInfo>(EffectInfo{.effectFunction = effectFunction, .amount = 0.0f, .velocity = rateOfChangeFromRampTime(rampTime), .finished = false});
}

void Mixer::removeEffect(String const& effectName, float rampTime) {
  MutexLocker locker(m_effectsMutex);
  if (m_effects.contains(effectName))
    m_effects[effectName]->velocity = -rateOfChangeFromRampTime(rampTime);
}

auto Mixer::currentEffects() -> StringList {
  MutexLocker locker(m_effectsMutex);
  return m_effects.keys();
}

auto Mixer::hasEffect(String const& effectName) -> bool {
  MutexLocker locker(m_effectsMutex);
  return m_effects.contains(effectName);
}

void Mixer::setSpeed(float speed) {
  m_speed = speed;
}

void Mixer::setVolume(float volume, float rampTime) {
  MutexLocker locker(m_mutex);
  m_volume.target = volume;
  m_volume.velocity = rateOfChangeFromRampTime(rampTime);
}

void Mixer::play(Ptr<AudioInstance> sample) {
  MutexLocker locker(m_queueMutex);
  m_audios.add(std::move(sample), AudioState{List<float>(m_channels, 1.0f)});
}

void Mixer::stopAll(float rampTime) {
  MutexLocker locker(m_queueMutex);
  float vel = rateOfChangeFromRampTime(rampTime);
  for (auto const& p : m_audios)
    p.first->stop(vel);
}

void Mixer::read(std::int16_t* outBuffer, std::size_t frameCount, ExtraMixFunction extraMixFunction) {
  // Make this method as least locky as possible by copying all the needed
  // member data before the expensive audio / effect stuff.
  float speed;
  unsigned sampleRate;
  unsigned channels;
  float volume;
  float volumeVelocity;
  float targetVolume;
  Map<MixerGroup, RampedValue> groupVolumes;

  {
    MutexLocker locker(m_mutex);
    speed = m_speed;
    sampleRate = m_sampleRate;
    channels = m_channels;
    volume = m_volume.value;
    volumeVelocity = m_volume.velocity;
    targetVolume = m_volume.target;
    groupVolumes = m_groupVolumes;
  }

  std::size_t bufferSize = frameCount * m_channels;
  m_mixBuffer.resize(bufferSize, 0);

  float time = (float)frameCount / sampleRate;
  float beginVolume = volume;
  float endVolume = approach(targetVolume, volume, volumeVelocity * time);

  Map<MixerGroup, float> groupEndVolumes;
  for (auto p : groupVolumes)
    groupEndVolumes[p.first] = approach(p.second.target, p.second.value, p.second.velocity * time);

  auto sampleStartTime = Time::millisecondsSinceEpoch();
  unsigned millisecondsInBuffer = (bufferSize * 1000) / (channels * sampleRate);
  auto sampleEndTime = sampleStartTime + millisecondsInBuffer;

  for (std::size_t i = 0; i < bufferSize; ++i)
    outBuffer[i] = 0;

  {
    MutexLocker locker(m_queueMutex);
    // Mix all active sounds
    for (auto& p : m_audios) {
      auto& audioInstance = p.first;
      auto& audioState = p.second;

      MutexLocker audioLocker(audioInstance->m_mutex);

      if (audioInstance->m_finished)
        continue;

      if (audioInstance->m_clockStart && *audioInstance->m_clockStart > sampleEndTime)
        continue;

      float groupVolume = groupVolumes[audioInstance->m_mixerGroup].value;
      float groupEndVolume = groupEndVolumes[audioInstance->m_mixerGroup];

      bool finished = false;

      float audioStopVolBegin = audioInstance->m_volume.value;
      float audioStopVolEnd = (audioInstance->m_volume.velocity > 0)
        ? approach(audioInstance->m_volume.target, audioStopVolBegin, audioInstance->m_volume.velocity * time)
        : audioInstance->m_volume.value;

      float pitchMultiplier = (audioInstance->m_pitchMultiplierVelocity > 0)
        ? approach(audioInstance->m_pitchMultiplierTarget, audioInstance->m_pitchMultiplier, audioInstance->m_pitchMultiplierVelocity * time)
        : audioInstance->m_pitchMultiplier;

      if (audioInstance->m_mixerGroup == MixerGroup::Effects || audioInstance->m_mixerGroup == MixerGroup::Instruments)
        pitchMultiplier *= speed;

      if (audioStopVolEnd == 0.0f && audioInstance->m_stopping)
        finished = true;

      std::size_t ramt = 0;
      if (audioInstance->m_clockStart && *audioInstance->m_clockStart > sampleStartTime) {
        int silentSamples = (*audioInstance->m_clockStart - sampleStartTime) * sampleRate / 1000;
        for (unsigned i = 0; i < silentSamples * channels; ++i)
          m_mixBuffer[i] = 0;
        ramt += silentSamples * channels;
      }
      try {
        ramt += audioInstance->m_audio.resample(channels, sampleRate, m_mixBuffer.ptr() + ramt, bufferSize - ramt, pitchMultiplier);
        while (ramt != bufferSize && !finished) {
          // Only seek back to the beginning and read more data if loops is < 0
          // (loop forever), or we have more loops to go, otherwise, the sample is
          // finished.
          if (audioInstance->m_loops != 0) {
            audioInstance->m_audio.seekSample(0);
            ramt += audioInstance->m_audio.resample(channels, sampleRate, m_mixBuffer.ptr() + ramt, bufferSize - ramt, pitchMultiplier);
            if (audioInstance->m_loops > 0)
              --audioInstance->m_loops;
          } else {
            finished = true;
          }
        }
        if (audioInstance->m_clockStop && *audioInstance->m_clockStop < sampleEndTime) {
          for (std::size_t s = 0; s < ramt / channels; ++s) {
            unsigned millisecondsInBuffer = (s * 1000) / sampleRate;
            auto sampleTime = sampleStartTime + millisecondsInBuffer;
            if (sampleTime > *audioInstance->m_clockStop) {
              float volume = 0.0f;
              if (audioInstance->m_clockStopFadeOut > 0)
                volume = 1.0f - (float)(sampleTime - *audioInstance->m_clockStop) / (float)audioInstance->m_clockStopFadeOut;

              if (volume <= 0) {
                for (std::size_t c = 0; c < channels; ++c)
                  m_mixBuffer[s * channels + c] = 0;
              } else {
                for (std::size_t c = 0; c < channels; ++c)
                  m_mixBuffer[s * channels + c] *= volume;
              }
            }
          }
          if (sampleEndTime > *audioInstance->m_clockStop + audioInstance->m_clockStopFadeOut)
            finished = true;
        }

        for (std::size_t s = 0; s < ramt / channels; ++s) {
          float vol = lerp((float)s / frameCount, beginVolume * groupVolume * audioStopVolBegin, endVolume * groupEndVolume * audioStopVolEnd);
          for (std::size_t c = 0; c < channels; ++c) {
            float sample = m_mixBuffer[s * channels + c] * vol * audioState.positionalChannelVolumes[c] * audioInstance->m_volume.value;
            std::int16_t& outSample = outBuffer[s * channels + c];
            outSample = clamp(sample + outSample, -32767.0f, 32767.0f);
          }
        }
      } catch (Star::AudioException const& e) {
        Logger::error("Error reading audio '{}': {}", audioInstance->m_audio.name(), e.what());
        finished = true;
      }

      audioInstance->m_volume.value = audioStopVolEnd;
      audioInstance->m_finished = finished;
    }
  }

  if (extraMixFunction)
    extraMixFunction(outBuffer, frameCount, channels);

  {
    MutexLocker locker(m_effectsMutex);
    // Apply all active effects
    for (auto const& pair : m_effects) {
      auto const& effectInfo = pair.second;
      if (effectInfo->finished)
        continue;

      float effectBegin = effectInfo->amount;
      float effectEnd;
      if (effectInfo->velocity < 0)
        effectEnd = approach(0.0f, effectBegin, -effectInfo->velocity * time);
      else
        effectEnd = approach(1.0f, effectBegin, effectInfo->velocity * time);

      std::copy(outBuffer, outBuffer + bufferSize, m_mixBuffer.begin());
      effectInfo->effectFunction(m_mixBuffer.ptr(), frameCount, channels);

      for (std::size_t s = 0; s < frameCount; ++s) {
        float amt = lerp((float)s / frameCount, effectBegin, effectEnd);
        for (std::size_t c = 0; c < channels; ++c) {
          std::int16_t prev = outBuffer[s * channels + c];
          outBuffer[s * channels + c] = lerp(amt, prev, m_mixBuffer[s * channels + c]);
        }
      }

      effectInfo->amount = effectEnd;
      effectInfo->finished = effectInfo->amount <= 0.0f;
    }
  }

  {
    MutexLocker locker(m_mutex);

    m_volume.value = endVolume;

    for (auto p : groupEndVolumes)
      m_groupVolumes[p.first].value = p.second;
  }
}

auto Mixer::lowpass(std::size_t avgSize) const -> Mixer::EffectFunction {
  struct LowPass {
    LowPass(std::size_t avgSize) : avgSize(avgSize) {}

    std::size_t avgSize;
    List<Deque<float>> filter;

    void operator()(std::int16_t* buffer, std::size_t frames, unsigned channels) {
      filter.resize(channels);
      for (std::size_t f = 0; f < frames; ++f) {
        for (std::size_t c = 0; c < channels; ++c) {
          auto& filterChannel = filter[c];
          filterChannel.append(buffer[f * channels + c] / 32767.0f);
          while (filterChannel.size() > avgSize)
            filterChannel.takeFirst();
          buffer[f * channels + c] = sum(filterChannel) / (float)avgSize * 32767.0f;
        }
      }
    }
  };

  return LowPass(avgSize);
}

auto Mixer::echo(float time, float dry, float wet) const -> Mixer::EffectFunction {
  struct Echo {
    unsigned echoLength;
    float dry;
    float wet;
    List<Deque<float>> filter;

    void operator()(std::int16_t* buffer, std::size_t frames, unsigned channels) {
      if (echoLength == 0)
        return;

      filter.resize(channels);
      for (std::size_t c = 0; c < channels; ++c) {
        auto& filterChannel = filter[c];
        if (filterChannel.empty())
          filterChannel.resize(echoLength, 0);
      }

      for (std::size_t f = 0; f < frames; ++f) {
        for (std::size_t c = 0; c < channels; ++c) {
          auto& filterChannel = filter[c];
          buffer[f * channels + c] = buffer[f * channels + c] * dry + filter[c][0] * wet;
          filterChannel.append(buffer[f * channels + c]);
          while (filterChannel.size() > echoLength)
            filterChannel.takeFirst();
        }
      }
    }
  };

  return Echo{.echoLength = (unsigned)(time * m_sampleRate), .dry = dry, .wet = wet, .filter = {}};
}

void Mixer::setGroupVolume(MixerGroup group, float targetValue, float rampTime) {
  MutexLocker locker(m_mutex);
  if (rampTime <= 0.0f) {
    m_groupVolumes[group].value = targetValue;
    m_groupVolumes[group].target = targetValue;
    m_groupVolumes[group].velocity = 0.0f;
  } else {
    m_groupVolumes[group].target = targetValue;
    m_groupVolumes[group].velocity = rateOfChangeFromRampTime(rampTime);
  }
}

void Mixer::update(float, PositionalAttenuationFunction positionalAttenuationFunction) {
  {
    MutexLocker locker(m_queueMutex);
    eraseWhere(m_audios, [&](auto& p) -> auto {
      if (p.first->m_finished)
        return true;

      if (positionalAttenuationFunction && p.first->m_position) {
        for (unsigned c = 0; c < m_channels; ++c)
          p.second.positionalChannelVolumes[c] = 1.0f - positionalAttenuationFunction(c, *p.first->m_position, p.first->m_rangeMultiplier);
      } else {
        for (unsigned c = 0; c < m_channels; ++c)
          p.second.positionalChannelVolumes[c] = 1.0f;
      }
      return false;
    });
  }

  {
    MutexLocker locker(m_effectsMutex);
    eraseWhere(m_effects, [](auto const& p) -> auto {
      return p.second->finished;
    });
  }
}

}// namespace Star
