module;
import std;
import star.json;
import star.string;
import star.list;
import star.map;
import star.set;
import star.byte_array;
import star.array;
import star.bi_map;
import star.format;
import star.vector;
import star.game_types;
import star.maybe;
import star.thread;
import star.data_stream_devices;




struct OpusDecoder;
struct OpusEncoder;

namespace Star {
class Root;


inline String const VoiceBroadcastPrefix = std::string("Voice\0", sizeof("Voice\0") - 1);

struct VoiceExceptionTag {
  static constexpr char const* name() { return "VoiceException"; }
};
using VoiceException = StarError<VoiceExceptionTag, StarException>;

enum class VoiceInputMode : std::uint8_t { VoiceActivity, PushToTalk };
extern EnumMap<VoiceInputMode> const VoiceInputModeNames;

enum class VoiceChannelMode: std::uint8_t { Mono = 1, Stereo = 2 };
extern EnumMap<VoiceChannelMode> const VoiceChannelModeNames;

class VoiceAudioStream;
class ApplicationController;

struct VoiceAudioChunk {
  std::unique_ptr<std::int16_t[]> data;
  std::size_t remaining;
  std::size_t offset = 0;

  VoiceAudioChunk(std::unique_ptr<std::int16_t[]> ptr, std::size_t size)
    : data(std::move(ptr)), remaining(size) {}

  inline std::size_t takeSamples(std::vector<std::int16_t>& out, std::size_t count) {
    std::size_t toRead = std::min<std::size_t>(count, remaining);
    std::int16_t* start = data.get() + offset;
    out.insert(out.end(), start, start + toRead);
    offset += toRead;
    remaining -= toRead;
    return toRead;
  }

  //this one's unsafe
  inline std::int16_t takeSample() {
    --remaining;
    return *(data.get() + offset++);
  }

  inline bool exhausted() { return remaining == 0; }
};


class Voice {
public:
  // Individual speakers are represented by their connection ID.
  typedef ConnectionId SpeakerId;

  class Speaker {
  public:
    SpeakerId speakerId = 0;
    EntityId entityId = 0;

    Vec2F position = Vec2F();
    String name = "Unnamed";

    std::unique_ptr<::OpusDecoder, void(*)(::OpusDecoder*)> decoderMono;
    std::unique_ptr<::OpusDecoder, void(*)(::OpusDecoder*)> decoderStereo;
    std::shared_ptr<VoiceAudioStream> audioStream;
    Mutex mutex;

    std::int64_t lastReceiveTime = 0;
    std::int64_t lastPlayTime = 0;
    float smoothDb = -96.0f;
    Array<float, 10> dbHistory = Array<float, 10>::filled(0);

    std::atomic<bool> muted = false;
    std::atomic<bool> playing = 0;
    std::atomic<float> decibelLevel = -96.0f;
    std::atomic<float> volume = 1.0f;
    Vec2F channelVolumes = Vec2F::filled(1.f);

    unsigned int minimumPlaySamples = 4096;

    Speaker(SpeakerId speakerId);
    Json toJson() const;
  };

  Voice(Root& root, std::shared_ptr<ApplicationController> appController);
  ~Voice();

  Voice(Voice const&) = delete;
  Voice& operator=(Voice const&) = delete;

  void init();

  void loadJson(Json const& config, bool skipSave = false);
  Json saveJson() const;

  void save() const;
  void scheduleSave();

  // Sets the local speaker ID and returns the local speaker. Must be called upon loading into a world.
  std::shared_ptr<Speaker> setLocalSpeaker(SpeakerId speakerId);
  std::shared_ptr<Speaker> localSpeaker();
  std::shared_ptr<Speaker> speaker(SpeakerId speakerId);
  HashMap<SpeakerId, std::shared_ptr<Speaker>>& speakers();
  List<std::shared_ptr<Voice::Speaker>> sortedSpeakers(bool onlyPlaying);
  void clearSpeakers();

  // Called when receiving input audio data from SDL, on its own thread.
  void readAudioData(std::uint8_t* stream, int len);

  // Called to mix voice audio with the game.
  void mix(std::int16_t* buffer, std::size_t frames, unsigned channels);

  typedef std::function<float(unsigned, Vec2F, float)> PositionalAttenuationFunction;
  void update(float dt, PositionalAttenuationFunction positionalAttenuationFunction = {});

  void setDeviceName(Maybe<String> device);
  StringList availableDevices();

  int send(DataStreamBuffer& out, std::size_t budget = 0);
  bool receive(std::shared_ptr<Speaker> speaker, std::string_view view);

  // Must be called every frame with input state, expires after 1s.
  void setInput(bool input = true);

  inline int encoderChannels() const { return (int)m_channelMode; }

  static OpusDecoder* createDecoder(int channels);
  static OpusEncoder* createEncoder(int channels);
private:
  Root& m_root;
  void resetEncoder();
  void resetDevice();
  void openDevice();
  void closeDevice();
  inline bool shouldEnableInput() const { return m_enabled && m_inputEnabled; }

  bool playSpeaker(std::shared_ptr<Speaker> const& speaker, int channels);

  void thread();

  SpeakerId m_speakerId = 0;
  std::shared_ptr<Speaker> m_clientSpeaker;
  HashMap<SpeakerId, std::shared_ptr<Speaker>> m_speakers;

  Mutex m_activeSpeakersMutex;
  HashSet<std::shared_ptr<Speaker>> m_activeSpeakers;



  std::unique_ptr<::OpusEncoder, void(*)(::OpusEncoder*)> m_encoder;
  
  float m_outputVolume = 1.0f;
  float m_inputVolume = 1.0f;

  float m_outputAmplitude = 1.0f;
  float m_inputAmplitude = 1.0f;

  float m_threshold = -50.0f;
  
  std::int64_t m_lastSentTime = 0;
  std::int64_t m_lastInputTime = 0;
  std::int64_t m_lastThresholdTime = 0;
  std::int64_t m_nextSaveTime = 0;
  bool m_enabled = true;
  bool m_inputEnabled = false;
  bool m_loopback = false;

  int m_deviceChannels = 1;
  bool m_deviceOpen = false;
  Maybe<String> m_deviceName;
  VoiceInputMode m_inputMode;
  VoiceChannelMode m_channelMode;
  unsigned m_bitrate = 0;

  ThreadFunction<void> m_thread;
  Mutex m_threadMutex;
  ConditionVariable m_threadCond;
  std::atomic<bool> m_stopThread;

  std::vector<std::int16_t> m_decodeBuffer;
  std::vector<std::int16_t> m_resampleBuffer;
  std::vector<std::int16_t> m_mixFinalBuffer;
  std::vector<std::int16_t> m_mixSpeakerBuffer;
  std::vector<std::int32_t> m_mixSharedBuffer;

  std::shared_ptr<ApplicationController> m_applicationController;


  Mutex m_encodeMutex;
  std::vector<ByteArray> m_encodedChunks;
  std::size_t m_encodedChunksLength = 0;

  Mutex m_captureMutex;
  std::queue<VoiceAudioChunk> m_capturedChunks;
  std::size_t m_capturedChunksFrames = 0;
};
  
}

export module star.voice;

export namespace Star {
  using ::Star::VoiceExceptionTag;
  using ::Star::VoiceException;
  using ::Star::VoiceInputMode;
  using ::Star::VoiceInputModeNames;
  using ::Star::VoiceChannelMode;
  using ::Star::VoiceChannelModeNames;
  using ::Star::Voice;
  using ::Star::VoiceAudioStream;
  using ::Star::ApplicationController;
  using ::Star::VoiceAudioChunk;
  using ::Star::VoiceBroadcastPrefix;
}

export using ::OpusDecoder;
export using ::OpusEncoder;
