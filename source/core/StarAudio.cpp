// Fixes unused variable warning
#define OV_EXCLUDE_STATIC_CALLBACKS

#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"

#include "StarAudio.hpp"
#include "StarBuffer.hpp"
#include "StarFile.hpp"
#include "StarFormat.hpp"
#include "StarLogging.hpp"
#include "StarDataStreamDevices.hpp"
#include "StarSha256.hpp"
#include "StarEncode.hpp"

constexpr size_t AudioDecodeBufferFrames = 1024;

namespace Star {

float const DefaultPerceptualRangeDb = 40.f;
float const DefaultPerceptualBoostRangeDb = 6.f;
// https://github.com/discord/perceptual
float perceptualToAmplitude(float perceptual, float normalizedMax, float range, float boostRange) {
  if (perceptual == 0.f) return 0.f;
  float dB = perceptual > normalizedMax
    ? ((perceptual - normalizedMax) / normalizedMax) * boostRange
    : (perceptual / normalizedMax) * range - range;
  return normalizedMax * pow(10.f, dB / 20.f);
}

float amplitudeToPerceptual(float amp, float normalizedMax, float range, float boostRange) {
  if (amp == 0.f) return 0.f;
  float const dB = 20.f * log10(amp / normalizedMax);
  float perceptual = dB > 0.f
    ? dB / boostRange + 1
    : (range + dB) / range;
  return normalizedMax * perceptual;
}

namespace {
  struct WaveData {
    ByteArrayPtr byteArray;
    unsigned channels;
    unsigned sampleRate;
  };

  template <typename T>
  T readLEType(IODevicePtr const& device) {
    T t;
    device->readFull(reinterpret_cast<char*>(&t), sizeof(t));
    fromByteOrder(ByteOrder::LittleEndian, reinterpret_cast<char*>(&t), sizeof(t));
    return t;
  }

  bool isUncompressed(IODevicePtr device) {
    const size_t sigLength = 4;
    auto riffSig = make_unique<char[]>(sigLength + 1); // RIFF\0
    auto waveSig = make_unique<char[]>(sigLength + 1); // WAVE\0

    StreamOffset previousOffset = device->pos();
    device->seek(0);
    device->readFull(riffSig.get(), sigLength);
    device->seek(4, IOSeek::Relative);
    device->readFull(waveSig.get(), sigLength);
    device->seek(previousOffset);
    if (strcmp(riffSig.get(), "RIFF") == 0 && strcmp(waveSig.get(), "WAVE") == 0) { // bytes are magic
      return true;
    }
    return false;
  }

  WaveData parseWav(IODevicePtr device) {
    const size_t sigLength = 4;
    auto riffSig = make_unique<char[]>(sigLength + 1); // RIFF\0
    auto waveSig = make_unique<char[]>(sigLength + 1); // WAVE\0
    auto fmtSig = make_unique<char[]>(sigLength + 1); // fmt \0
    auto dataSig = make_unique<char[]>(sigLength + 1); // data\0

    // RIFF Chunk Descriptor
    device->seek(0);
    device->readFull(riffSig.get(), sigLength);

    uint32_t fileSize = readLEType<uint32_t>(device);
    fileSize += sigLength + sizeof(fileSize);
    if (fileSize != device->size())
      throw AudioException(strf("Wav file is wrong size, reports {} is actually {}", fileSize, device->size()));

    device->readFull(waveSig.get(), sigLength);

    if ((strcmp(riffSig.get(), "RIFF") != 0) || (strcmp(waveSig.get(), "WAVE") != 0)) { // bytes are not magic
      auto p = [](char a) { return isprint(a) ? a : '?'; };
      throw AudioException(strf("Wav file has wrong magic bytes, got `{:c}{:c}{:c}{:c}' and `{:c}{:c}{:c}{:c}' but expected `RIFF' and `WAVE'",
              p(riffSig[0]), p(riffSig[1]), p(riffSig[2]), p(riffSig[3]), p(waveSig[0]), p(waveSig[1]), p(waveSig[2]), p(waveSig[3])));
    }

    // fmt subchunk

    device->readFull(fmtSig.get(), sigLength);
    if (strcmp(fmtSig.get(), "fmt ") != 0) { // friendship is magic
      auto p = [](char a) { return isprint(a) ? a : '?'; };
      throw AudioException(strf("Wav file fmt subchunk has wrong magic bytes, got `{:c}{:c}{:c}{:c}' but expected `fmt '",
          p(fmtSig[0]),
          p(fmtSig[1]),
          p(fmtSig[2]),
          p(fmtSig[3])));
    }

    uint32_t fmtSubchunkSize = readLEType<uint32_t>(device);
    fmtSubchunkSize += sigLength;
    if (fmtSubchunkSize < 20)
      throw AudioException(strf("fmt subchunk is sized wrong, expected 20 got {}.  Is this wav file not PCM?", fmtSubchunkSize));

    uint16_t audioFormat = readLEType<uint16_t>(device);
    if (audioFormat != 1)
      throw AudioException("audioFormat data indicates that wav file is something other than PCM format.  Unsupported.");

    uint16_t wavChannels = readLEType<uint16_t>(device);
    uint32_t wavSampleRate = readLEType<uint32_t>(device);
    uint32_t wavByteRate = readLEType<uint32_t>(device);
    uint16_t wavBlockAlign = readLEType<uint16_t>(device);
    uint16_t wavBitsPerSample = readLEType<uint16_t>(device);

    if (wavBitsPerSample != 16)
      throw AudioException("Only 16-bit PCM wavs are supported.");
    if (wavByteRate * 8 != wavSampleRate * wavChannels * wavBitsPerSample)
      throw AudioException("Sanity check failed, ByteRate is wrong");
    if (wavBlockAlign * 8 != wavChannels * wavBitsPerSample)
      throw AudioException("Sanity check failed, BlockAlign is wrong");

    device->seek(fmtSubchunkSize - 20, IOSeek::Relative);

    // data subchunk

    device->readFull(dataSig.get(), sigLength);
    if (strcmp(dataSig.get(), "data") != 0) { // magic or more magic?
      auto p = [](char a) { return isprint(a) ? a : '?'; };
      throw AudioException(strf("Wav file data subchunk has wrong magic bytes, got `{:c}{:c}{:c}{:c}' but expected `data'",
          p(dataSig[0]), p(dataSig[1]), p(dataSig[2]), p(dataSig[3])));
    }

    uint32_t wavDataSize = readLEType<uint32_t>(device);
    size_t wavDataOffset = static_cast<size_t>(device->pos());
    if (wavDataSize + wavDataOffset > static_cast<size_t>(device->size())) {
      throw AudioException(strf("Wav file data size reported is inconsistent with file size, got {} but expected {}",
          device->size(), wavDataSize + wavDataOffset));
    }

    ByteArrayPtr pcmData = make_shared<ByteArray>();
    pcmData->resize(wavDataSize);

    // Copy across data and perform and endianess conversion if needed

    device->readFull(pcmData->ptr(), pcmData->size());
    for (size_t i = 0; i < pcmData->size() / 2; ++i)
      fromByteOrder(ByteOrder::LittleEndian, pcmData->ptr() + i * 2, 2);

    return WaveData{std::move(pcmData), wavChannels, wavSampleRate};
  }
}

class CompressedAudioImpl {
public:
  static size_t readFunc(void* ptr, size_t size, size_t nmemb, void* datasource) {
    return static_cast<ExternalBuffer*>(datasource)->read(static_cast<char*>(ptr), size * nmemb) / size;
  }

  static int seekFunc(void* datasource, ogg_int64_t offset, int whence) {
    static_cast<ExternalBuffer*>(datasource)->seek(offset, static_cast<IOSeek>(whence));
    return 0;
  };

  static long int tellFunc(void* datasource) {
    return static_cast<long int>(static_cast<ExternalBuffer*>(datasource)->pos());
  };

  CompressedAudioImpl(CompressedAudioImpl const& impl) {
    m_audioData = impl.m_audioData;
    m_memoryFile.reset(m_audioData->ptr(), m_audioData->size());
    m_vorbisInfo = nullptr;
  }

  CompressedAudioImpl(IODevicePtr audioData) {
    audioData->open(IOMode::Read);
    audioData->seek(0);
    m_audioData = make_shared<ByteArray>(audioData->readBytes(static_cast<size_t>(audioData->size())));
    m_memoryFile.reset(m_audioData->ptr(), m_audioData->size());
    m_vorbisInfo = nullptr;
  }

  ~CompressedAudioImpl() {
    ov_clear(&m_vorbisFile);
  }

  bool open() {
    m_callbacks.read_func = readFunc;
    m_callbacks.seek_func = seekFunc;
    m_callbacks.tell_func = tellFunc;
    m_callbacks.close_func = nullptr;

    if (ov_open_callbacks(&m_memoryFile, &m_vorbisFile, nullptr, 0, m_callbacks) < 0)
      return false;

    m_vorbisInfo = ov_info(&m_vorbisFile, -1);
    return true;
  }

  unsigned channels() {
    return m_vorbisInfo->channels;
  }

  unsigned sampleRate() {
    return m_vorbisInfo->rate;
  }

  double totalTime() {
    return ov_time_total(&m_vorbisFile, -1);
  }

  uint64_t totalSamples() {
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

  double currentTime() {
    return ov_time_tell(&m_vorbisFile);
  }

  uint64_t currentSample() {
    return ov_pcm_tell(&m_vorbisFile);
  }

  size_t readPartial(int16_t* buffer, size_t bufferSize) {
    int bitstream;
    int read = OV_HOLE;
    // ov_read takes int parameter, so do some magic here to make sure we don't
    // overflow
    bufferSize *= 2;
    do {
#if STAR_LITTLE_ENDIAN
      read = ov_read(&m_vorbisFile, reinterpret_cast<char*>(buffer), bufferSize, 0, 2, 1, &bitstream);
#else
      read = ov_read(&m_vorbisFile, reinterpret_cast<char*>(buffer), bufferSize, 1, 2, 1, &bitstream);
#endif
    } while (read == OV_HOLE);
    if (read < 0)
      throw AudioException::format("Error in Audio::read ({})", read);
    
    // read in bytes, returning number of int16_t samples.
    return read / 2;
  }
  
private:
  ByteArrayConstPtr m_audioData;
  ExternalBuffer m_memoryFile;
  ov_callbacks m_callbacks;
  OggVorbis_File m_vorbisFile;
  vorbis_info* m_vorbisInfo;
};

class UncompressedAudioImpl {
public:
  UncompressedAudioImpl(UncompressedAudioImpl const& impl) {
    m_channels = impl.m_channels;
    m_sampleRate = impl.m_sampleRate;
    m_audioData = impl.m_audioData;
    m_memoryFile.reset(m_audioData->ptr(), m_audioData->size());
  }

  UncompressedAudioImpl(CompressedAudioImpl& impl) {
    m_channels = impl.channels();
    m_sampleRate = impl.sampleRate();

    int16_t buffer[AudioDecodeBufferFrames];
    Buffer uncompressBuffer;
    while (true) {
      size_t ramt = impl.readPartial(buffer, AudioDecodeBufferFrames);

      if (ramt == 0) {
        // End of stream reached
        break;
      } else {
        uncompressBuffer.writeFull(reinterpret_cast<char*>(buffer), ramt * 2);
      }
    }

    m_audioData = make_shared<ByteArray>(uncompressBuffer.takeData());
    m_memoryFile.reset(m_audioData->ptr(), m_audioData->size());
  }

  UncompressedAudioImpl(ByteArrayConstPtr data, unsigned channels, unsigned sampleRate) {
    m_channels = channels;
    m_sampleRate = sampleRate;
    m_audioData = std::move(data);
    m_memoryFile.reset(m_audioData->ptr(), m_audioData->size());
  }

  bool open() {
    return true;
  }

  unsigned channels() {
    return m_channels;
  }

  unsigned sampleRate() {
    return m_sampleRate;
  }

  double totalTime() {
    return static_cast<double>(totalSamples()) / m_sampleRate;
  }

  uint64_t totalSamples() {
    return m_memoryFile.dataSize() / 2 / m_channels;
  }

  void seekTime(double time) {
    seekSample(static_cast<uint64_t>(time * m_sampleRate));
  }

  void seekSample(uint64_t pos) {
    m_memoryFile.seek(pos * 2 * m_channels);
  }

  double currentTime() {
    return static_cast<double>(currentSample()) / m_sampleRate;
  }

  uint64_t currentSample() {
    return m_memoryFile.pos() / 2 / m_channels;
  }


  size_t readPartial(int16_t* buffer, size_t bufferSize) {
    if (bufferSize != NPos)
      bufferSize = bufferSize * 2;
    return m_memoryFile.read(reinterpret_cast<char*>(buffer), bufferSize) / 2;
  }

private:
  unsigned m_channels;
  unsigned m_sampleRate;
  ByteArrayConstPtr m_audioData;
  ExternalBuffer m_memoryFile;
};

Audio::Audio(IODevicePtr device, String name) {
  m_name = name;
  if (!device->isOpen())
    device->open(IOMode::Read);

  if (isUncompressed(device)) {
    WaveData data = parseWav(device);
    m_uncompressed = make_shared<UncompressedAudioImpl>(std::move(data.byteArray), data.channels, data.sampleRate);
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

Audio& Audio::operator=(Audio const& audio) {
    if (audio.m_uncompressed) {
        m_uncompressed = make_shared<UncompressedAudioImpl>(*audio.m_uncompressed);
        if (!m_uncompressed->open())
          throw AudioException("Failed to open uncompressed audio stream during copy");
    } else {
        m_compressed = make_shared<CompressedAudioImpl>(*audio.m_compressed);
        if (!m_compressed->open()) 
            throw AudioException("Failed to open compressed audio stream during copy");
    }

    seekSample(audio.currentSample());
    return *this;
}

Audio& Audio::operator=(Audio&& audio) {
  m_compressed = std::move(audio.m_compressed);
  m_uncompressed = std::move(audio.m_uncompressed);
  return *this;
}

unsigned Audio::channels() const {
  if (m_uncompressed)
    return m_uncompressed->channels();
  else
    return m_compressed->channels();
}

unsigned Audio::sampleRate() const {
  if (m_uncompressed)
    return m_uncompressed->sampleRate();
  else
    return m_compressed->sampleRate();
}

double Audio::totalTime() const {
  if (m_uncompressed)
    return m_uncompressed->totalTime();
  else
    return m_compressed->totalTime();
}

uint64_t Audio::totalSamples() const {
  if (m_uncompressed)
    return m_uncompressed->totalSamples();
  else
    return m_compressed->totalSamples();
}

bool Audio::compressed() const {
  return static_cast<bool>(m_compressed);
}

void Audio::uncompress() {
  if (m_compressed) {
    m_uncompressed = make_shared<UncompressedAudioImpl>(*m_compressed);
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

double Audio::currentTime() const {
  if (m_uncompressed)
    return m_uncompressed->currentTime();
  else
    return m_compressed->currentTime();
}

uint64_t Audio::currentSample() const {
  if (m_uncompressed)
    return m_uncompressed->currentSample();
  else
    return m_compressed->currentSample();
}

size_t Audio::readPartial(int16_t* buffer, size_t bufferSize) {
  if (bufferSize == 0)
    return 0;

  if (m_uncompressed)
    return m_uncompressed->readPartial(buffer, bufferSize);
  else
    return m_compressed->readPartial(buffer, bufferSize);
}

size_t Audio::read(int16_t* buffer, size_t bufferSize) {
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

size_t Audio::resample(unsigned destinationChannels, unsigned destinationSampleRate, int16_t* destinationBuffer, size_t destinationBufferSize, double velocity) {
  unsigned destinationSamples = destinationBufferSize / destinationChannels;
  if (destinationSamples == 0)
    return 0;

  unsigned sourceChannels = channels();
  unsigned sourceSampleRate = sampleRate();

  if (velocity != 1.0)
    sourceSampleRate = static_cast<unsigned>(sourceSampleRate * velocity);

  if (destinationChannels == sourceChannels && destinationSampleRate == sourceSampleRate) {
    // If the destination and source channel count and sample rate are the
    // same, this is the same as a read.

    return read(destinationBuffer, destinationBufferSize);

  } else if (destinationSampleRate == sourceSampleRate) {
    // If the destination and source sample rate are the same, then we can skip
    // the super-sampling math.

    unsigned sourceBufferSize = destinationSamples * sourceChannels;

    m_workingBuffer.resize(sourceBufferSize * sizeof(int16_t));
    auto* sourceBuffer = reinterpret_cast<int16_t*>(m_workingBuffer.ptr());

    unsigned readSamples = read(sourceBuffer, sourceBufferSize) / sourceChannels;

    for (unsigned sample = 0; sample < readSamples; ++sample) {
      unsigned sourceBufferIndex = sample * sourceChannels;
      unsigned destinationBufferIndex = sample * destinationChannels;

      for (unsigned destinationChannel = 0; destinationChannel < destinationChannels; ++destinationChannel) {
        // If the destination channel count is greater than the source
        // channels, simply copy the last channel
        unsigned sourceChannel = min(destinationChannel, sourceChannels - 1);
        destinationBuffer[destinationBufferIndex + destinationChannel] =
            sourceBuffer[sourceBufferIndex + sourceChannel];
      }
    }

    return readSamples * destinationChannels;

  } else {
    // Otherwise, we have to do a full resample.

    unsigned sourceSamples = (static_cast<uint64_t>(sourceSampleRate) * destinationSamples + destinationSampleRate - 1) / destinationSampleRate;
    unsigned sourceBufferSize = sourceSamples * sourceChannels;

    m_workingBuffer.resize(sourceBufferSize * sizeof(int16_t));
    auto* sourceBuffer = reinterpret_cast<int16_t*>(m_workingBuffer.ptr());

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
        unsigned sourceChannel = min(destinationChannel, sourceChannels - 1);

        int sample = 0;
        int sampleCount = 0;
        for (int superSample = 0; superSample < SuperSampleFactor; ++superSample) {
          unsigned sourceSample = static_cast<unsigned>((destinationSample * SuperSampleFactor + superSample) * sourceSamples / destinationSamples) / SuperSampleFactor;
          if (sourceSample < readSamples) {
            unsigned sourceBufferIndex = sourceSample * sourceChannels;
            starAssert(sourceBufferIndex + sourceChannel < sourceBufferSize);
            sample += sourceBuffer[sourceBufferIndex + sourceChannel];
            ++sampleCount;
          }
        }

        // If sampleCount is zero, then we are past the end of our read data
        // completely, and can stop
        if (sampleCount == 0)
          return writtenSamples * destinationChannels;

        sample /= sampleCount;
        destinationBuffer[destinationBufferIndex + destinationChannel] = static_cast<int16_t>(sample);
        writtenSamples = destinationSample + 1;
      }
    }

    return writtenSamples * destinationChannels;
  }
}

String const& Audio::name() const {
  return m_name;
}

void Audio::setName(String name) {
  m_name = std::move(name);
}

}
