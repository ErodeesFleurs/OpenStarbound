module;
#include "StarJson.hpp"
#include "StarLua.hpp"
#include "StarLuaConverters.hpp"
#include "StarBiMap.hpp"
#include "StarException.hpp"
#include "StarGameTypes.hpp"
#include "StarMaybe.hpp"
#include "StarThread.hpp"
#include "StarDataStreamDevices.hpp"

// Match client include order for SIMD intrinsics used by xxhash and fast_float.
#include "StarInputEvent.hpp"
import star.application;
#include "StarStatisticsService.hpp"
#include "StarP2PNetworkingService.hpp"
#include "StarUserGeneratedContentService.hpp"
#include "StarDesktopService.hpp"
#include "StarImage.hpp"
#include "StarRect.hpp"
import star.application_controller;
#include <queue>
struct OpusDecoder;
typedef std::unique_ptr<OpusDecoder, void(*)(OpusDecoder*)> OpusDecoderPtr;
struct OpusEncoder;
typedef std::unique_ptr<OpusEncoder, void(*)(OpusEncoder*)> OpusEncoderPtr;
import star.voice;

module star.voice_lua_bindings;

namespace Star {

typedef Voice::SpeakerId SpeakerId;
LuaCallbacks LuaBindings::makeVoiceCallbacks() {
  LuaCallbacks callbacks;

  auto voice = Voice::singletonPtr();

  callbacks.registerCallbackWithSignature<StringList>("devices", bind(&Voice::availableDevices, voice));
  callbacks.registerCallback(  "getSettings", [voice]() -> Json      { return voice->saveJson();         });
  callbacks.registerCallback("mergeSettings", [voice](Json const& settings) { voice->loadJson(settings); });
  // i have an alignment addiction i'm so sorry
  callbacks.registerCallback("setSpeakerMuted",  [voice](SpeakerId speakerId, bool muted)  { voice->speaker(speakerId)->muted = muted; });
  callbacks.registerCallback(   "speakerMuted",  [voice](SpeakerId speakerId) { return (bool)voice->speaker(speakerId)->muted;         });
  // it just looks so neat to me!!
  callbacks.registerCallback("setSpeakerVolume", [voice](SpeakerId speakerId, float volume) { voice->speaker(speakerId)->volume = volume; });
  callbacks.registerCallback(   "speakerVolume", [voice](SpeakerId speakerId) { return (float)voice->speaker(speakerId)->volume;          });

  callbacks.registerCallback("speakerPosition", [voice](SpeakerId speakerId) { return voice->speaker(speakerId)->position; });

  callbacks.registerCallback("speaker",  [voice](Maybe<SpeakerId> speakerId) {
    if (speakerId)
      return voice->speaker(*speakerId)->toJson();
    else
      return voice->localSpeaker()->toJson();
  });

  callbacks.registerCallback("speakers", [voice](Maybe<bool> onlyPlaying) -> List<Json> {
    List<Json> list;

    for (auto& speaker : voice->sortedSpeakers(onlyPlaying.value(true)))
      list.append(speaker->toJson());

    return list;
  });

  return callbacks;
}

}
