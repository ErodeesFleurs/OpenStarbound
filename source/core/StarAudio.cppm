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
using CompressedAudioImplPtr = std::shared_ptr<CompressedAudioImpl>;
using CompressedAudioImplConstPtr = std::shared_ptr<const CompressedAudioImpl>;
using CompressedAudioImplWeakPtr = std::weak_ptr<CompressedAudioImpl>;
using CompressedAudioImplConstWeakPtr = std::weak_ptr<const CompressedAudioImpl>;
using CompressedAudioImplUPtr = std::unique_ptr<CompressedAudioImpl>;
using CompressedAudioImplConstUPtr = std::unique_ptr<const CompressedAudioImpl>;
class UncompressedAudioImpl;
using UncompressedAudioImplPtr = std::shared_ptr<UncompressedAudioImpl>;
using UncompressedAudioImplConstPtr = std::shared_ptr<const UncompressedAudioImpl>;
using UncompressedAudioImplWeakPtr = std::weak_ptr<UncompressedAudioImpl>;
using UncompressedAudioImplConstWeakPtr = std::weak_ptr<const UncompressedAudioImpl>;
using UncompressedAudioImplUPtr = std::unique_ptr<UncompressedAudioImpl>;
using UncompressedAudioImplConstUPtr = std::unique_ptr<const UncompressedAudioImpl>;
class Audio;
using AudioPtr = std::shared_ptr<Audio>;
using AudioConstPtr = std::shared_ptr<const Audio>;
using AudioWeakPtr = std::weak_ptr<Audio>;
using AudioConstWeakPtr = std::weak_ptr<const Audio>;
using AudioUPtr = std::unique_ptr<Audio>;
using AudioConstUPtr = std::unique_ptr<const Audio>;

struct AudioExceptionTag {
  static constexpr char const* name() { return "AudioException"; }
};
using AudioException = StarError<AudioExceptionTag, StarException>;

// Simple class for reading audio files in ogg/vorbis and wav format.
// Reads and allows for decompression of a limited subset of ogg/vorbis.  Does
// not handle multiple bitstreams, sample rate or channel number changes.
// Entire stream is kept in memory, and is implicitly shared so copying Audio
// instances is not expensive.
class Audio {
public:
  explicit Audio(IODevicePtr device, String name = "");
  Audio(Audio const& audio);
  Audio(Audio&& audio);

  Audio& operator=(Audio const& audio);
  Audio& operator=(Audio&& audio);

  // This function returns the number of channels that this file has.  Channels
  // are static throughout file.
  unsigned channels() const;

  // This function returns the sample rate that this file has.  Sample rates
  // are static throughout file.
  unsigned sampleRate() const;

  // This function returns the playtime duration of the file.
  double totalTime() const;

  // This function returns total number of samples in this file.
  std::uint64_t totalSamples() const;

  // This function returns true when the datastream or file being read from is
  // a vorbis compressed file.  False otherwise.
  bool compressed() const;

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
  double currentTime() const;

  // This function converts the current offset of the file to the current
  // sample number.
  std::uint64_t currentSample() const;

  // Reads into 16 bit signed buffer with channels interleaved.  Returns total
  // number of samples read (counting each channel individually).  0 indicates
  // end of stream.
  std::size_t readPartial(std::int16_t* buffer, std::size_t bufferSize);

  // Same as readPartial, but repeats read attempting to fill buffer as much as
  // possible
  std::size_t read(std::int16_t* buffer, std::size_t bufferSize);

  // Read into a given buffer, while also converting into the given number of
  // channels at the given sample rate and playback velocity.  If the number of
  // channels in the file is higher, only populates lower channels, if it is
  // lower, the last channel is copied to the remaining channels.  Attempts to
  // fill the buffer as much as possible up to end of stream.  May fail to fill
  // an entire buffer depending on the destinationSampleRate / velocity /
  // available samples.
  std::size_t resample(unsigned destinationChannels, unsigned destinationSampleRate,
      std::int16_t* destinationBuffer, std::size_t destinationBufferSize,
      double velocity = 1.0);

  String const& name() const;
  void setName(String name);

private:
  // If audio is uncompressed, this will be null.
  CompressedAudioImplPtr m_compressed;
  UncompressedAudioImplPtr m_uncompressed;

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
  using ::Star::CompressedAudioImplPtr;
  using ::Star::CompressedAudioImplConstPtr;
  using ::Star::CompressedAudioImplWeakPtr;
  using ::Star::CompressedAudioImplConstWeakPtr;
  using ::Star::CompressedAudioImplUPtr;
  using ::Star::CompressedAudioImplConstUPtr;
  using ::Star::UncompressedAudioImpl;
  using ::Star::UncompressedAudioImplPtr;
  using ::Star::UncompressedAudioImplConstPtr;
  using ::Star::UncompressedAudioImplWeakPtr;
  using ::Star::UncompressedAudioImplConstWeakPtr;
  using ::Star::UncompressedAudioImplUPtr;
  using ::Star::UncompressedAudioImplConstUPtr;
  using ::Star::Audio;
  using ::Star::AudioPtr;
  using ::Star::AudioConstPtr;
  using ::Star::AudioWeakPtr;
  using ::Star::AudioConstWeakPtr;
  using ::Star::AudioUPtr;
  using ::Star::AudioConstUPtr;
}
