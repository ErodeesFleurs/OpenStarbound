module;
import std;
import star.json;
import star.lua;
import star.string;
import star.list;
import star.lua_converters;
import star.bi_map;
import star.format;
import star.game_types;
import star.thread;
import star.data_stream_devices;

// Match client include order for SIMD intrinsics used by xxhash and fast_float.
import star.input_event;
import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
import star.application_controller;
struct OpusDecoder;
struct OpusEncoder;
import star.voice;

module star.voice_lua_bindings;

namespace Star {

typedef Voice::SpeakerId SpeakerId;
LuaCallbacks LuaBindings::makeVoiceCallbacks(Voice& owner) {
  LuaCallbacks callbacks;

  auto voice = &owner;

  callbacks.registerCallbackWithSignature<StringList>("devices", std::bind(&Voice::availableDevices, voice));
  callbacks.registerCallback(  "getSettings", [voice]() -> Json      { return voice->saveJson();         });
  callbacks.registerCallback("mergeSettings", [voice](Json const& settings) { voice->loadJson(settings); });
  // i have an alignment addiction i'm so sorry
  callbacks.registerCallback("setSpeakerMuted",  [voice](SpeakerId speakerId, bool muted)  { voice->speaker(speakerId)->muted = muted; });
  callbacks.registerCallback(   "speakerMuted",  [voice](SpeakerId speakerId) { return (bool)voice->speaker(speakerId)->muted;         });
  // it just looks so neat to me!!
  callbacks.registerCallback("setSpeakerVolume", [voice](SpeakerId speakerId, float volume) { voice->speaker(speakerId)->volume = volume; });
  callbacks.registerCallback(   "speakerVolume", [voice](SpeakerId speakerId) { return (float)voice->speaker(speakerId)->volume;          });

  callbacks.registerCallback("speakerPosition", [voice](SpeakerId speakerId) { return voice->speaker(speakerId)->position; });

  callbacks.registerCallback("speaker",  [voice](std::optional<SpeakerId> speakerId) {
    if (speakerId)
      return voice->speaker(*speakerId)->toJson();
    else
      return voice->localSpeaker()->toJson();
  });

  callbacks.registerCallback("speakers", [voice](std::optional<bool> onlyPlaying) -> List<Json> {
    List<Json> list;

    for (auto& speaker : voice->sortedSpeakers(onlyPlaying.value_or(true)))
      list.append(speaker->toJson());

    return list;
  });

  return callbacks;
}

}
