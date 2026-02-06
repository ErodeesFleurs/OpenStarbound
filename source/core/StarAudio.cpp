// Fixes unused variable warning
#define OV_EXCLUDE_STATIC_CALLBACKS
#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"


#include "StarBytes.hpp"
#include "StarConfig.hpp"
#include "StarAudio.hpp"
#include "StarBuffer.hpp"
#include "StarFormat.hpp"

import std;

namespace Star {

float const DefaultPerceptualRangeDb = 40.f;
float const DefaultPerceptualBoostRangeDb = 6.f;
// https://github.com/discord/perceptual
auto perceptualToAmplitude(float perceptual, float normalizedMax, float range, float boostRange) -> float {
  if (perceptual == 0.f) return 0.f;
  float dB = perceptual > normalizedMax
    ? ((perceptual - normalizedMax) / normalizedMax) * boostRange
    : (perceptual / normalizedMax) * range - range;
  return normalizedMax * std::pow(10.f, dB / 20.f);
}

auto amplitudeToPerceptual(float amp, float normalizedMax, float range, float boostRange) -> float {
  if (amp == 0.f) return 0.f;
  float const dB = 20.f * std::log10(amp / normalizedMax);
  float perceptual = dB > 0.f
    ? dB / boostRange + 1
    : (range + dB) / range;
  return normalizedMax * perceptual;
}

namespace {
using std::uint32_t;

struct WaveData {
#ifdef STAR_STREAM_AUDIO
    IODevicePtr device;
    unsigned channels;
    unsigned sampleRate;
    size_t dataSize; // get the data size from the header to avoid id3 tag
#else
    WaveData(Ptr<ByteArray> byteArray, unsigned int channels, unsigned int sampleRate) : byteArray(std::move(byteArray)), channels(channels), sampleRate(sampleRate) {}
    Ptr<ByteArray> byteArray;
    unsigned channels;
    unsigned sampleRate;
#endif
  };

  template <typename T>
  auto readLEType(Ptr<IODevice> const& device) -> T {
    T t;
    device->readFull((char*)&t, sizeof(t));
    fromByteOrder(ByteOrder::LittleEndian, (char*)&t, sizeof(t));
    return t;
  }

  auto isUncompressed(Ptr<IODevice> device) -> bool {
    const size_t sigLength = 4;
    std::unique_ptr<char[]> riffSig(new char[sigLength + 1]()); // RIFF\0
    std::unique_ptr<char[]> waveSig(new char[sigLength + 1]()); // WAVE\0

    std::int64_t previousOffset = device->pos();
    device->seek(0);
    device->readFull(riffSig.get(), sigLength);
    device->seek(4, IOSeek::Relative);
    device->readFull(waveSig.get(), sigLength);
    device->seek(previousOffset);
    if (std::strcmp(riffSig.get(), "RIFF") == 0 && std::strcmp(waveSig.get(), "WAVE") == 0) { // bytes are magic
      return true;
    }
    return false;
  }

  auto parseWav(Ptr<IODevice> device) -> WaveData {
    const size_t sigLength = 4;
    std::unique_ptr<char[]> riffSig(new char[sigLength + 1]()); // RIFF\0
    std::unique_ptr<char[]> waveSig(new char[sigLength + 1]()); // WAVE\0
    std::unique_ptr<char[]> fmtSig(new char[sigLength + 1]()); // fmt \0
    std::unique_ptr<char[]> dataSig(new char[sigLength + 1]()); // data\0

    // RIFF Chunk Descriptor
    device->seek(0);
    device->readFull(riffSig.get(), sigLength);

    auto fileSize = readLEType<std::uint32_t>(device);
    fileSize += sigLength + sizeof(fileSize);
    if (fileSize != device->size())
      throw AudioException(strf("Wav file is wrong size, reports {} is actually {}", fileSize, device->size()));

    device->readFull(waveSig.get(), sigLength);

    if ((std::strcmp(riffSig.get(), "RIFF") != 0) || (std::strcmp(waveSig.get(), "WAVE") != 0)) { // bytes are not magic
      auto p = [](char a) -> char { return std::isprint(a) ? a : '?'; };
      throw AudioException(strf("Wav file has wrong magic bytes, got `{:c}{:c}{:c}{:c}' and `{:c}{:c}{:c}{:c}' but expected `RIFF' and `WAVE'",
              p(riffSig[0]), p(riffSig[1]), p(riffSig[2]), p(riffSig[3]), p(waveSig[0]), p(waveSig[1]), p(waveSig[2]), p(waveSig[3])));
    }

    // fmt subchunk

    device->readFull(fmtSig.get(), sigLength);
    if (std::strcmp(fmtSig.get(), "fmt ") != 0) { // friendship is magic
      auto p = [](char a) -> char { return std::isprint(a) ? a : '?'; };
      throw AudioException(strf("Wav file fmt subchunk has wrong magic bytes, got `{:c}{:c}{:c}{:c}' but expected `fmt '",
          p(fmtSig[0]),
          p(fmtSig[1]),
          p(fmtSig[2]),
          p(fmtSig[3])));
    }

    auto fmtSubchunkSize = readLEType<std::uint32_t>(device);
    fmtSubchunkSize += sigLength;
    if (fmtSubchunkSize < 20)
      throw AudioException(strf("fmt subchunk is sized wrong, expected 20 got {}.  Is this wav file not PCM?", fmtSubchunkSize));

    auto audioFormat = readLEType<std::uint16_t>(device);
    if (audioFormat != 1)
      throw AudioException("audioFormat data indicates that wav file is something other than PCM format.  Unsupported.");

    auto wavChannels = readLEType<std::uint16_t>(device);
    auto wavSampleRate = readLEType<std::uint32_t>(device);
    auto wavByteRate = readLEType<std::uint32_t>(device);
    auto wavBlockAlign = readLEType<std::uint16_t>(device);
    auto wavBitsPerSample = readLEType<std::uint16_t>(device);

    if (wavBitsPerSample != 16)
      throw AudioException("Only 16-bit PCM wavs are supported.");
    if (wavByteRate * 8 != wavSampleRate * wavChannels * wavBitsPerSample)
      throw AudioException("Sanity check failed, ByteRate is wrong");
    if (wavBlockAlign * 8 != wavChannels * wavBitsPerSample)
      throw AudioException("Sanity check failed, BlockAlign is wrong");

    device->seek(fmtSubchunkSize - 20, IOSeek::Relative);

    // data subchunk

    device->readFull(dataSig.get(), sigLength);
    if (std::strcmp(dataSig.get(), "data") != 0) { // magic or more magic?
      auto p = [](char a) -> char { return std::isprint(a) ? a : '?'; };
      throw AudioException(strf("Wav file data subchunk has wrong magic bytes, got `{:c}{:c}{:c}{:c}' but expected `data'",
          p(dataSig[0]), p(dataSig[1]), p(dataSig[2]), p(dataSig[3])));
    }

    auto wavDataSize = readLEType<std::uint32_t>(device);
    auto wavDataOffset = (std::size_t)device->pos();
    if (wavDataSize + wavDataOffset > (std::size_t)device->size()) {
      throw AudioException(strf("Wav file data size reported is inconsistent with file size, got {} but expected {}",
          device->size(), wavDataSize + wavDataOffset));
    }

    #ifdef STAR_STREAM_AUDIO
    // Return the original device positioned at the PCM data
    // Note: This means the caller owns handling endianness conversion
    device->seek(wavDataOffset);

    return WaveData{device, wavChannels, wavSampleRate, wavDataSize};
    #else
    Ptr<ByteArray> pcmData = std::make_shared<ByteArray>();
    pcmData->resize(wavDataSize);

    // Copy across data and perform and endianess conversion if needed

    device->readFull(pcmData->ptr(), pcmData->size());
    for (size_t i = 0; i < pcmData->size() / 2; ++i)
      fromByteOrder(ByteOrder::LittleEndian, pcmData->ptr() + i * 2, 2);

    return WaveData{std::move(pcmData), wavChannels, wavSampleRate};
    #endif
  }
  }// namespace

class CompressedAudioImpl {
public:
  #ifdef STAR_STREAM_AUDIO
  CompressedAudioImpl(CompressedAudioImpl const& impl)
      : m_audioData(impl.m_audioData->clone())// Clone instead of sharing
        ,
        m_deviceCallbacks(m_audioData)// Pass reference to cloned data
        ,
        m_vorbisInfo(nullptr) {
    setupCallbacks();

    // Make sure data stream is ready to be read
    m_audioData->open(IOMode::Read);
    m_audioData->seek(0);

    // Add error checking to see what's happening with the clone
    if (!m_audioData->isOpen())
      throw AudioException("Failed to open cloned audio device");

    auto size = m_audioData->size();
    if (size <= 0)
      throw AudioException("Cloned audio device has no data");
  }

  CompressedAudioImpl(IODevicePtr audioData)
      : m_audioData(audioData->clone())// Clone instead of taking ownership
        ,
        m_deviceCallbacks(m_audioData)// Pass reference
        ,
        m_vorbisInfo(nullptr) {
    setupCallbacks();
    m_audioData->open(IOMode::Read);
    m_audioData->seek(0);
  }
  #else
  static auto readFunc(void* ptr, size_t size, size_t nmemb, void* datasource) -> size_t {
    return static_cast<ExternalBuffer*>(datasource)->read((char*)ptr, size * nmemb) / size;
  }

  static auto seekFunc(void* datasource, ogg_int64_t offset, int whence) -> int {
    static_cast<ExternalBuffer*>(datasource)->seek(offset, (IOSeek)whence);
    return 0;
  };

  static auto tellFunc(void* datasource) -> long int {
    return (long int)static_cast<ExternalBuffer*>(datasource)->pos();
  };

    CompressedAudioImpl(CompressedAudioImpl const& impl) {
    m_audioData = impl.m_audioData;
    m_memoryFile.reset(m_audioData->ptr(), m_audioData->size());
    m_vorbisInfo = nullptr;
  }

  CompressedAudioImpl(Ptr<IODevice> audioData) {
    audioData->open(IOMode::Read);
    audioData->seek(0);
    m_audioData = std::make_shared<ByteArray>(audioData->readBytes((size_t)audioData->size()));
    m_memoryFile.reset(m_audioData->ptr(), m_audioData->size());
    m_vorbisInfo = nullptr;
  }
  #endif

  ~CompressedAudioImpl() {
    ov_clear(&m_vorbisFile);
  }

  #ifdef STAR_STREAM_AUDIO
  void setupCallbacks() {
    m_deviceCallbacks.setupOggCallbacks(m_callbacks);
  }
  #endif

  auto open() -> bool {
    #ifdef STAR_STREAM_AUDIO
    int result = ov_open_callbacks(&m_deviceCallbacks, &m_vorbisFile, NULL, 0, m_callbacks);
    if (result < 0) {
      Logger::error("Failed to open ogg stream: error code {}", result);
      return false;
    }
    #else
    m_callbacks.read_func = readFunc;
    m_callbacks.seek_func = seekFunc;
    m_callbacks.tell_func = tellFunc;
    m_callbacks.close_func = nullptr;

    if (ov_open_callbacks(&m_memoryFile, &m_vorbisFile, NULL, 0, m_callbacks) < 0)
      return false;
    #endif

    m_vorbisInfo = ov_info(&m_vorbisFile, -1);
    return true;
  }

  auto channels() -> unsigned {
    return m_vorbisInfo->channels;
  }

  auto sampleRate() -> unsigned {
    return m_vorbisInfo->rate;
  }

  auto totalTime() -> double {
    return ov_time_total(&m_vorbisFile, -1);
  }

  auto totalSamples() -> uint64_t {
    return ov_pcm_total(&m_vorbisFile, -1);
  }

  void seekTime(double time) {
    int ret = ov_time_seek(&m_vorbisFile, time);

    if (ret != 0)
      throw StarException("Cannot seek ogg stream Audio::seekTime");
  }

  void seekSample(uint64_t pos) {
    int ret = ov_pcm_seek(&m_vorbisFile, pos);

    if (ret != 0)
      throw StarException("Cannot seek ogg stream in Audio::seekSample");
  }

  auto currentTime() -> double {
    return ov_time_tell(&m_vorbisFile);
  }

  auto currentSample() -> uint64_t {
    return ov_pcm_tell(&m_vorbisFile);
  }

  auto readPartial(int16_t* buffer, size_t bufferSize) -> size_t {
    int bitstream;
    int read = OV_HOLE;
    // ov_read takes int parameter, so do some magic here to make sure we don't
    // overflow
    bufferSize *= 2;
    do {
#if STAR_LITTLE_ENDIAN
      read = ov_read(&m_vorbisFile, (char*)buffer, bufferSize, 0, 2, 1, &bitstream);
#else
      read = ov_read(&m_vorbisFile, (char*)buffer, bufferSize, 1, 2, 1, &bitstream);
#endif
    } while (read == OV_HOLE);
    if (read < 0)
      throw AudioException::format("Error in Audio::read ({})", read);

    // read in bytes, returning number of int16_t samples.
    return read / 2;
  }

private:
  #ifdef STAR_STREAM_AUDIO
  Ptr<IODevice> m_audioData;
  IODeviceCallbacks m_deviceCallbacks;
  #else
  ConstPtr<ByteArray> m_audioData;
  ExternalBuffer m_memoryFile;
  #endif
  ov_callbacks m_callbacks;
  OggVorbis_File m_vorbisFile;
  vorbis_info* m_vorbisInfo;
};

class UncompressedAudioImpl {
public:
  #ifdef STAR_STREAM_AUDIO
  UncompressedAudioImpl(UncompressedAudioImpl const& impl)
    : m_device(impl.m_device->clone())
    , m_channels(impl.m_channels)
    , m_sampleRate(impl.m_sampleRate)
    , m_dataSize(impl.m_dataSize)
    , m_dataStart(impl.m_dataStart)

  {
    std::int64_t initialPos = m_device->pos(); // Store initial position
    if (!m_device->isOpen())
      m_device->open(IOMode::Read);
    m_device->seek(initialPos); // Restore position after open
  }

  UncompressedAudioImpl(CompressedAudioImpl& impl) {
    m_channels = impl.channels();
    m_sampleRate = impl.sampleRate();

    // Create a memory buffer to store decompressed data
    auto memDevice = make_shared<Buffer>();

    int16_t buffer[1024];
    while (true) {
      size_t ramt = impl.readPartial(buffer, 1024);
      if (ramt == 0)
        break;
      memDevice->writeFull((char*)buffer, ramt * 2);
    }

    m_device = memDevice;
  }

  UncompressedAudioImpl(IODevicePtr device, unsigned channels, unsigned sampleRate, size_t dataSize)
    : m_device(std::move(device))
    , m_channels(channels)
    , m_sampleRate(sampleRate)
    , m_dataSize(dataSize)
    , m_dataStart((size_t)m_device->pos())  // Store current position as data start
  {
    if (!m_device->isOpen())
      m_device->open(IOMode::Read);
  }
  #else
  UncompressedAudioImpl(UncompressedAudioImpl const& impl) {
    m_channels = impl.m_channels;
    m_sampleRate = impl.m_sampleRate;
    m_audioData = impl.m_audioData;
    m_memoryFile.reset(m_audioData->ptr(), m_audioData->size());
  }

  UncompressedAudioImpl(CompressedAudioImpl& impl) {
    m_channels = impl.channels();
    m_sampleRate = impl.sampleRate();

    std::array<std::int16_t, 1024> buffer{};
    Buffer uncompressBuffer;
    while (true) {
      size_t ramt = impl.readPartial(buffer.data(), 1024);

      if (ramt == 0) {
        // End of stream reached
        break;
      } else {
        uncompressBuffer.writeFull((char*)buffer.data(), ramt * 2);
      }
    }

    m_audioData = std::make_shared<ByteArray>(uncompressBuffer.takeData());
    m_memoryFile.reset(m_audioData->ptr(), m_audioData->size());
  }

  UncompressedAudioImpl(ConstPtr<ByteArray> data, unsigned channels, unsigned sampleRate) {
    m_channels = channels;
    m_sampleRate = sampleRate;
    m_audioData = std::move(data);
    m_memoryFile.reset(m_audioData->ptr(), m_audioData->size());
  }
  #endif

  auto open() -> bool {
    return true;
  }

  auto channels() -> unsigned {
    return m_channels;
  }

  auto sampleRate() -> unsigned {
    return m_sampleRate;
  }

  auto totalTime() -> double {
    return (double)totalSamples() / m_sampleRate;
  }

  auto totalSamples() -> uint64_t {
    #ifdef STAR_STREAM_AUDIO
    return m_device->size() / 2 / m_channels;
    #else
    return m_memoryFile.dataSize() / 2 / m_channels;
    #endif
  }

  void seekTime(double time) {
    seekSample((uint64_t)(time * m_sampleRate));
  }

  void seekSample(uint64_t pos) {
    #ifdef STAR_STREAM_AUDIO
    m_device->seek(pos * 2 * m_channels);
    #else
    m_memoryFile.seek(pos * 2 * m_channels);
    #endif
  }

  auto currentTime() -> double {
    return (double)currentSample() / m_sampleRate;
  }

  auto currentSample() -> uint64_t {
    #ifdef STAR_STREAM_AUDIO
    return m_device->pos() / 2 / m_channels;
    #else
    return m_memoryFile.pos() / 2 / m_channels;
    #endif
  }


  auto readPartial(int16_t* buffer, size_t bufferSize) -> size_t {
    if (bufferSize != std::numeric_limits<std::size_t>::max())
      bufferSize = bufferSize * 2;
    #ifndef STAR_STREAM_AUDIO
    return m_memoryFile.read((char*)buffer, bufferSize) / 2;
    #else
    // Calculate remaining valid data
    size_t currentPos = m_device->pos() - m_dataStart;
    size_t remainingBytes = m_dataSize - currentPos;

    // Limit read to remaining valid data
    if (bufferSize > remainingBytes)
      bufferSize = remainingBytes;

    if (bufferSize == 0)
      return 0;

    size_t bytesRead = m_device->read((char*)buffer, bufferSize);

    // Handle endianness conversion
    for (size_t i = 0; i < bytesRead / 2; ++i)
      fromByteOrder(ByteOrder::LittleEndian, ((char*)buffer) + i * 2, 2);

    return bytesRead / 2;
    #endif
  }

private:
  #ifdef STAR_STREAM_AUDIO
  Ptr<IODevice> m_device;
  #endif
  unsigned m_channels;
  unsigned m_sampleRate;
  #ifdef STAR_STREAM_AUDIO
  size_t m_dataSize;
  size_t m_dataStart;
  #else
  ConstPtr<ByteArray> m_audioData;
  ExternalBuffer m_memoryFile;
  #endif
};

Audio::Audio(Ptr<IODevice> device, String name) {
  m_name = name;
  if (!device->isOpen())
    device->open(IOMode::Read);

  if (isUncompressed(device)) {
    WaveData data = parseWav(device);
    #ifdef STAR_STREAM_AUDIO
    m_uncompressed = make_shared<UncompressedAudioImpl>(std::move(data.device), data.channels, data.sampleRate, data.dataSize);
    #else
    m_uncompressed = make_shared<UncompressedAudioImpl>(std::move(data.byteArray), data.channels, data.sampleRate);
    #endif
  } else {
    m_compressed = make_shared<CompressedAudioImpl>(device);
    if (!m_compressed->open())
      throw AudioException("File does not appear to be a valid ogg bitstream");
  }
}

Audio::Audio(Audio const& audio) {
  *this = audio;
}

Audio::Audio(Audio&& audio) {
  operator=(std::move(audio));
}

auto Audio::operator=(Audio const& audio) -> Audio& {
    if (audio.m_uncompressed) {
        m_uncompressed = std::make_shared<UncompressedAudioImpl>(*audio.m_uncompressed);
        if (!m_uncompressed->open())
          throw AudioException("Failed to open uncompressed audio stream during copy");
    } else {
        m_compressed = std::make_shared<CompressedAudioImpl>(*audio.m_compressed);
        if (!m_compressed->open())
            throw AudioException("Failed to open compressed audio stream during copy");
    }

    seekSample(audio.currentSample());
    return *this;
}

auto Audio::operator=(Audio&& audio) -> Audio& {
  m_compressed = std::move(audio.m_compressed);
  m_uncompressed = std::move(audio.m_uncompressed);
  return *this;
}

auto Audio::channels() const -> unsigned {
  if (m_uncompressed)
    return m_uncompressed->channels();
  else
    return m_compressed->channels();
}

auto Audio::sampleRate() const -> unsigned {
  if (m_uncompressed)
    return m_uncompressed->sampleRate();
  else
    return m_compressed->sampleRate();
}

auto Audio::totalTime() const -> double {
  if (m_uncompressed)
    return m_uncompressed->totalTime();
  else
    return m_compressed->totalTime();
}

auto Audio::totalSamples() const -> uint64_t {
  if (m_uncompressed)
    return m_uncompressed->totalSamples();
  else
    return m_compressed->totalSamples();
}

auto Audio::compressed() const -> bool {
  return (bool)m_compressed;
}

void Audio::uncompress() {
  if (m_compressed) {
    m_uncompressed = std::make_shared<UncompressedAudioImpl>(*m_compressed);
    m_compressed.reset();
  }
}

void Audio::seekTime(double time) {
  if (m_uncompressed)
    m_uncompressed->seekTime(time);
  else
    m_compressed->seekTime(time);
}

void Audio::seekSample(uint64_t pos) {
  if (m_uncompressed)
    m_uncompressed->seekSample(pos);
  else
    m_compressed->seekSample(pos);
}

auto Audio::currentTime() const -> double {
  if (m_uncompressed)
    return m_uncompressed->currentTime();
  else
    return m_compressed->currentTime();
}

auto Audio::currentSample() const -> uint64_t {
  if (m_uncompressed)
    return m_uncompressed->currentSample();
  else
    return m_compressed->currentSample();
}

auto Audio::readPartial(int16_t* buffer, size_t bufferSize) -> size_t {
  if (bufferSize == 0)
    return 0;

  if (m_uncompressed)
    return m_uncompressed->readPartial(buffer, bufferSize);
  else
    return m_compressed->readPartial(buffer, bufferSize);
}

auto Audio::read(int16_t* buffer, size_t bufferSize) -> size_t {
  if (bufferSize == 0)
    return 0;

  size_t readTotal = 0;
  while (readTotal < bufferSize) {
    size_t toGo = bufferSize - readTotal;
    size_t ramt = readPartial(buffer + readTotal, toGo);
    readTotal += ramt;
    // End of stream reached
    if (ramt == 0)
      break;
  }
  return readTotal;
}

auto Audio::resample(unsigned destinationChannels, unsigned destinationSampleRate, int16_t* destinationBuffer, size_t destinationBufferSize, double velocity) -> size_t {
  unsigned destinationSamples = destinationBufferSize / destinationChannels;
  if (destinationSamples == 0)
    return 0;

  unsigned sourceChannels = channels();
  unsigned sourceSampleRate = sampleRate();

  if (velocity != 1.0)
    sourceSampleRate = (unsigned)(sourceSampleRate * velocity);

  if (destinationChannels == sourceChannels && destinationSampleRate == sourceSampleRate) {
    // If the destination and source channel count and sample rate are the
    // same, this is the same as a read.

    return read(destinationBuffer, destinationBufferSize);

  } else if (destinationSampleRate == sourceSampleRate) {
    // If the destination and source sample rate are the same, then we can skip
    // the super-sampling math.

    unsigned sourceBufferSize = destinationSamples * sourceChannels;

    m_workingBuffer.resize(sourceBufferSize * sizeof(std::int16_t));
    auto* sourceBuffer = (std::int16_t*)m_workingBuffer.ptr();

    unsigned readSamples = read(sourceBuffer, sourceBufferSize) / sourceChannels;

    for (unsigned sample = 0; sample < readSamples; ++sample) {
      unsigned sourceBufferIndex = sample * sourceChannels;
      unsigned destinationBufferIndex = sample * destinationChannels;

      for (unsigned destinationChannel = 0; destinationChannel < destinationChannels; ++destinationChannel) {
        // If the destination channel count is greater than the source
        // channels, simply copy the last channel
        unsigned sourceChannel = std::min(destinationChannel, sourceChannels - 1);
        destinationBuffer[destinationBufferIndex + destinationChannel] =
            sourceBuffer[sourceBufferIndex + sourceChannel];
      }
    }

    return readSamples * destinationChannels;

  } else {
    // Otherwise, we have to do a full resample.

    unsigned sourceSamples = ((std::uint64_t)sourceSampleRate * destinationSamples + destinationSampleRate - 1) / destinationSampleRate;
    unsigned sourceBufferSize = sourceSamples * sourceChannels;

    m_workingBuffer.resize(sourceBufferSize * sizeof(std::int16_t));
    auto* sourceBuffer = (std::int16_t*)m_workingBuffer.ptr();

    unsigned readSamples = read(sourceBuffer, sourceBufferSize) / sourceChannels;

    if (readSamples == 0)
      return 0;

    unsigned writtenSamples = 0;

    for (unsigned destinationSample = 0; destinationSample < destinationSamples; ++destinationSample) {
      unsigned destinationBufferIndex = destinationSample * destinationChannels;

      for (unsigned destinationChannel = 0; destinationChannel < destinationChannels; ++destinationChannel) {
        static int const SuperSampleFactor = 8;

        // If the destination channel count is greater than the source
        // channels, simply copy the last channel
        unsigned sourceChannel = std::min(destinationChannel, sourceChannels - 1);

        int sample = 0;
        int sampleCount = 0;
        for (int superSample = 0; superSample < SuperSampleFactor; ++superSample) {
          unsigned sourceSample = (unsigned)((destinationSample * SuperSampleFactor + superSample) * sourceSamples / destinationSamples) / SuperSampleFactor;
          if (sourceSample < readSamples) {
            unsigned sourceBufferIndex = sourceSample * sourceChannels;
            sample += sourceBuffer[sourceBufferIndex + sourceChannel];
            ++sampleCount;
          }
        }

        // If sampleCount is zero, then we are past the end of our read data
        // completely, and can stop
        if (sampleCount == 0)
          return writtenSamples * destinationChannels;

        sample /= sampleCount;
        destinationBuffer[destinationBufferIndex + destinationChannel] = (int16_t)sample;
        writtenSamples = destinationSample + 1;
      }
    }

    return writtenSamples * destinationChannels;
  }
}

auto Audio::name() const -> String const& {
  return m_name;
}

void Audio::setName(String name) {
  m_name = std::move(name);
}

}
