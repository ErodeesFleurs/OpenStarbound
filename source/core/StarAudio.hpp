#pragma once

#include "StarConfig.hpp"
#include "StarException.hpp"
#include "StarIODevice.hpp"

import std;

namespace Star {

extern float const DefaultPerceptualRangeDb;
extern float const DefaultPerceptualBoostRangeDb;

auto perceptualToAmplitude(float perceptual, float normalizedMax = 1.f,
  float range = DefaultPerceptualRangeDb, float boostRange = DefaultPerceptualBoostRangeDb) -> float;

auto amplitudeToPerceptual(float amp, float normalizedMax = 1.f,
  float range = DefaultPerceptualRangeDb, float boostRange = DefaultPerceptualBoostRangeDb) -> float;

class CompressedAudioImpl;
class UncompressedAudioImpl;

using AudioException = ExceptionDerived<"AudioException">;

// Simple class for reading audio files in ogg/vorbis and wav format.
// Reads and allows for decompression of a limited subset of ogg/vorbis.  Does
// not handle multiple bitstreams, sample rate or channel number changes.
// Entire stream is kept in memory, and is implicitly shared so copying Audio
// instances is not expensive.
class Audio {
public:
  explicit Audio(Ptr<IODevice> device, String name = "");
  Audio(Audio const& audio);
  Audio(Audio&& audio);

  auto operator=(Audio const& audio) -> Audio&;
  auto operator=(Audio&& audio) -> Audio&;

  // This function returns the number of channels that this file has.  Channels
  // are static throughout file.
  [[nodiscard]] auto channels() const -> unsigned;

  // This function returns the sample rate that this file has.  Sample rates
  // are static throughout file.
  [[nodiscard]] auto sampleRate() const -> unsigned;

  // This function returns the playtime duration of the file.
  [[nodiscard]] auto totalTime() const -> double;

  // This function returns total number of samples in this file.
  [[nodiscard]] auto totalSamples() const -> std::uint64_t;

  // This function returns true when the datastream or file being read from is
  // a vorbis compressed file.  False otherwise.
  [[nodiscard]] auto compressed() const -> bool;

  // If compressed, permanently uncompresses audio for faster reading.  The
  // uncompressed buffer is shared with all further copies of Audio, and this
  // is irreversible.
  void uncompress();

  // This function seeks the data stream to the given time in seconds.
  void seekTime(double time);

  // This function seeks the data stream to the given sample number
  void seekSample(std::uint64_t sample);

  // This function converts the current offset of the file to the time value of
  // that offset in seconds.
  [[nodiscard]] auto currentTime() const -> double;

  // This function converts the current offset of the file to the current
  // sample number.
  [[nodiscard]] auto currentSample() const -> std::uint64_t;

  // Reads into 16 bit signed buffer with channels interleaved.  Returns total
  // number of samples read (counting each channel individually).  0 indicates
  // end of stream.
  auto readPartial(std::int16_t* buffer, std::size_t bufferSize) -> std::size_t;

  // Same as readPartial, but repeats read attempting to fill buffer as much as
  // possible
  auto read(std::int16_t* buffer, std::size_t bufferSize) -> std::size_t;

  // Read into a given buffer, while also converting into the given number of
  // channels at the given sample rate and playback velocity.  If the number of
  // channels in the file is higher, only populates lower channels, if it is
  // lower, the last channel is copied to the remaining channels.  Attempts to
  // fill the buffer as much as possible up to end of stream.  May fail to fill
  // an entire buffer depending on the destinationSampleRate / velocity /
  // available samples.
  auto resample(unsigned destinationChannels, unsigned destinationSampleRate,
      std::int16_t* destinationBuffer, std::size_t destinationBufferSize,
      double velocity = 1.0) -> std::size_t;

  [[nodiscard]] auto name() const -> String const&;
  void setName(String name);

private:
  // If audio is uncompressed, this will be null.
  Ptr<CompressedAudioImpl> m_compressed;
  Ptr<UncompressedAudioImpl> m_uncompressed;

  ByteArray m_workingBuffer;
  String m_name;
};

}
