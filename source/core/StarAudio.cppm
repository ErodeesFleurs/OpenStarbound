module;
import std;
import star.byte_array;
import star.string;
import star.format;
import star.io_device;
//#define STAR_STREAM_AUDIO

namespace Star {

extern float const DefaultPerceptualRangeDb;
extern float const DefaultPerceptualBoostRangeDb;

float perceptualToAmplitude(float perceptual, float normalizedMax = 1.f,
  float range = DefaultPerceptualRangeDb, float boostRange = DefaultPerceptualBoostRangeDb);

float amplitudeToPerceptual(float amp, float normalizedMax = 1.f,
  float range = DefaultPerceptualRangeDb, float boostRange = DefaultPerceptualBoostRangeDb);

class CompressedAudioImpl;
class UncompressedAudioImpl;

struct AudioExceptionTag {
  static constexpr char const* name() { return "AudioException"; }
};
using AudioException = StarError<AudioExceptionTag, StarException>;

// Simple class for reading audio files in ogg/vorbis and wav format.
// Reads and allows for decompression of a limited subset of ogg/vorbis.  Does
// not handle multiple bitstreams, sample rate or channel number changes.
// Entire stream is kept in memory by default. clone() explicitly creates an
// independent cursor while sharing immutable audio bytes.
class Audio {
public:
  [[nodiscard]] static std::expected<Audio, StarException> create(std::shared_ptr<IODevice> device, String name = "");
  Audio(Audio const&) = delete;
  Audio& operator=(Audio const&) = delete;
  Audio(Audio&& audio) noexcept;
  Audio& operator=(Audio&& audio) noexcept;
  [[nodiscard]] std::expected<Audio, StarException> clone() const;

  // This function returns the number of channels that this file has.  Channels
  // are static throughout file.
  unsigned channels() const;

  // This function returns the sample rate that this file has.  Sample rates
  // are static throughout file.
  unsigned sampleRate() const;

  // This function returns the playtime duration of the file.
  [[nodiscard]] std::expected<double, StarException> totalTime() const;

  // This function returns total number of samples in this file.
  [[nodiscard]] std::expected<std::uint64_t, StarException> totalSamples() const;

  // This function returns true when the datastream or file being read from is
  // a vorbis compressed file.  False otherwise.
  bool compressed() const;

  // If compressed, permanently uncompresses audio for faster reading.
  // On failure the original decoder and its cursor are retained. Decoded bytes
  // are shared with all further clones.
  [[nodiscard]] std::expected<void, StarException> uncompress();

  // This function seeks the data stream to the given time in seconds.
  [[nodiscard]] std::expected<void, StarException> seekTime(double time);

  // This function seeks the data stream to the given sample number
  [[nodiscard]] std::expected<void, StarException> seekSample(std::uint64_t sample);

  // This function converts the current offset of the file to the time value of
  // that offset in seconds.
  [[nodiscard]] std::expected<double, StarException> currentTime() const;

  // This function converts the current offset of the file to the current
  // sample number.
  [[nodiscard]] std::expected<std::uint64_t, StarException> currentSample() const;

  // Reads into 16 bit signed buffer with channels interleaved.  Returns total
  // number of samples read (counting each channel individually).  0 indicates
  // end of stream.
  [[nodiscard]] std::expected<std::size_t, StarException> readPartial(std::int16_t* buffer, std::size_t bufferSize);

  // Same as readPartial, but repeats read attempting to fill buffer as much as
  // possible
  [[nodiscard]] std::expected<std::size_t, StarException> read(std::int16_t* buffer, std::size_t bufferSize);

  // Read into a given buffer, while also converting into the given number of
  // channels at the given sample rate and playback velocity.  If the number of
  // channels in the file is higher, only populates lower channels, if it is
  // lower, the last channel is copied to the remaining channels.  Attempts to
  // fill the buffer as much as possible up to end of stream.  May fail to fill
  // an entire buffer depending on the destinationSampleRate / velocity /
  // available samples.
  [[nodiscard]] std::expected<std::size_t, StarException> resample(unsigned destinationChannels, unsigned destinationSampleRate,
      std::int16_t* destinationBuffer, std::size_t destinationBufferSize,
      double velocity = 1.0);

  String const& name() const;
  void setName(String name);

private:
  Audio() = default;
  // If audio is uncompressed, this will be null.
  std::shared_ptr<CompressedAudioImpl> m_compressed;
  std::shared_ptr<UncompressedAudioImpl> m_uncompressed;

  ByteArray m_workingBuffer;
  String m_name;
};

}

export module star.audio;

export namespace Star {
  using ::Star::DefaultPerceptualRangeDb;
  using ::Star::DefaultPerceptualBoostRangeDb;
  using ::Star::perceptualToAmplitude;
  using ::Star::amplitudeToPerceptual;
  using ::Star::AudioExceptionTag;
  using ::Star::AudioException;
  using ::Star::CompressedAudioImpl;
  using ::Star::UncompressedAudioImpl;
  using ::Star::Audio;
}
