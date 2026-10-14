module;
import std;
import star.string;
import star.list;
import star.map;
import star.maybe;
import star.json;

import star.thread;
import star.vector;
import star.game_types;

import star.net_element_basic_fields;
import star.net_element_sync_group;

namespace Star {

class Audio;
using AudioPtr = std::shared_ptr<Audio>;
using AudioConstPtr = std::shared_ptr<const Audio>;
using AudioWeakPtr = std::weak_ptr<Audio>;
using AudioConstWeakPtr = std::weak_ptr<const Audio>;
using AudioUPtr = std::unique_ptr<Audio>;
using AudioConstUPtr = std::unique_ptr<const Audio>;
class AudioInstance;
using AudioInstancePtr = std::shared_ptr<AudioInstance>;
using AudioInstanceConstPtr = std::shared_ptr<const AudioInstance>;
using AudioInstanceWeakPtr = std::weak_ptr<AudioInstance>;
using AudioInstanceConstWeakPtr = std::weak_ptr<const AudioInstance>;
using AudioInstanceUPtr = std::unique_ptr<AudioInstance>;
using AudioInstanceConstUPtr = std::unique_ptr<const AudioInstance>;
class World;
using WorldPtr = std::shared_ptr<World>;
using WorldConstPtr = std::shared_ptr<const World>;
using WorldWeakPtr = std::weak_ptr<World>;
using WorldConstWeakPtr = std::weak_ptr<const World>;
using WorldUPtr = std::unique_ptr<World>;
using WorldConstUPtr = std::unique_ptr<const World>;
class Songbook;
using SongbookPtr = std::shared_ptr<Songbook>;
using SongbookConstPtr = std::shared_ptr<const Songbook>;
using SongbookWeakPtr = std::weak_ptr<Songbook>;
using SongbookConstWeakPtr = std::weak_ptr<const Songbook>;
using SongbookUPtr = std::unique_ptr<Songbook>;
using SongbookConstUPtr = std::unique_ptr<const Songbook>;
class RenderCallback;
using RenderCallbackPtr = std::shared_ptr<RenderCallback>;
using RenderCallbackConstPtr = std::shared_ptr<const RenderCallback>;
using RenderCallbackWeakPtr = std::weak_ptr<RenderCallback>;
using RenderCallbackConstWeakPtr = std::weak_ptr<const RenderCallback>;
using RenderCallbackUPtr = std::unique_ptr<RenderCallback>;
using RenderCallbackConstUPtr = std::unique_ptr<const RenderCallback>;

class Songbook : public NetElementSyncGroup {
public:
  Songbook(String const& species);
  ~Songbook();

  void update(EntityMode mode, World* world);
  void render(RenderCallback* renderCallback);

  // instrument needs to tell the songbook what type it is, and needs to keep
  // calling it to signal
  // the instrument is still equiped
  void keepAlive(String const& instrument, Vec2F const& position);

  void stop();
  void play(Json const& song, String const& timeSource);
  bool active() const;
  bool instrumentPlaying() const;

  Maybe<String> timeSource() const;
  Maybe<String> instrument() const;
  Json song() const;

private:
  struct Note {
    String instrument;
    String file;
    double timecode;
    double duration;
    double fadeout;
    double velocity;
  };

  struct HeldNote {
    AudioInstancePtr audio;
    double start;
    double end;
  };

  struct NoteMapping {
    List<String> files;
    double frequency;
    double velocity;
    double fadeout;
  };

  struct TimeSource {
    std::int64_t keepalive;
    std::int64_t epoch;
  };

  static double fundamentalFrequency(double p);
  static double fundamentalPitch(double p);

  void netElementsNeedLoad(bool full) override;
  void netElementsNeedStore() override;

  static StringMap<std::shared_ptr<TimeSource>> s_timeSources;
  static Mutex s_timeSourcesMutex;

  String m_species;
  Vec2F m_position;

  bool m_serverMode;

  std::int64_t m_globalNowDelta;
  int m_activeCooldown;
  bool m_dataUpdated;
  bool m_dataChanged;

  String m_timeSource;
  std::int64_t m_timeSourceEpoch;
  bool m_epochUpdated;
  String m_instrument;
  Json m_song;
  bool m_stopped;

  Deque<Note> m_track;

  List<HeldNote> m_heldNotes;
  std::shared_ptr<TimeSource> m_timeSourceInstance;

  List<AudioInstancePtr> m_pendingAudio;

  List<Note> parseABC(String const& abc);

  StringMap<Map<int, NoteMapping>> m_noteMapping;

  NoteMapping& noteMapping(String const& instrument, String const& species, int note);

  StringMap<AudioConstPtr> m_uncompressedSamples;

  void playback();

  NetElementData<Json> m_songNetState;
  NetElementInt m_timeSourceEpochNetState;
  NetElementString m_timeSourceNetState;
  NetElementBool m_activeNetState;
  NetElementString m_instrumentNetState;
};

}

export module star.songbook;

export namespace Star {
  using ::Star::Audio;
  using ::Star::AudioPtr;
  using ::Star::AudioConstPtr;
  using ::Star::AudioWeakPtr;
  using ::Star::AudioConstWeakPtr;
  using ::Star::AudioUPtr;
  using ::Star::AudioConstUPtr;
  using ::Star::AudioInstance;
  using ::Star::AudioInstancePtr;
  using ::Star::AudioInstanceConstPtr;
  using ::Star::AudioInstanceWeakPtr;
  using ::Star::AudioInstanceConstWeakPtr;
  using ::Star::AudioInstanceUPtr;
  using ::Star::AudioInstanceConstUPtr;
  using ::Star::World;
  using ::Star::WorldPtr;
  using ::Star::WorldConstPtr;
  using ::Star::WorldWeakPtr;
  using ::Star::WorldConstWeakPtr;
  using ::Star::WorldUPtr;
  using ::Star::WorldConstUPtr;
  using ::Star::Songbook;
  using ::Star::SongbookPtr;
  using ::Star::SongbookConstPtr;
  using ::Star::SongbookWeakPtr;
  using ::Star::SongbookConstWeakPtr;
  using ::Star::SongbookUPtr;
  using ::Star::SongbookConstUPtr;
  using ::Star::RenderCallback;
  using ::Star::RenderCallbackPtr;
  using ::Star::RenderCallbackConstPtr;
  using ::Star::RenderCallbackWeakPtr;
  using ::Star::RenderCallbackConstWeakPtr;
  using ::Star::RenderCallbackUPtr;
  using ::Star::RenderCallbackConstUPtr;
}
