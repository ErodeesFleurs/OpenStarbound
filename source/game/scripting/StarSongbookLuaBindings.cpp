#include "StarSongbookLuaBindings.hpp"
#include "StarLuaConverters.hpp"

namespace Star {

LuaCallbacks LuaBindings::makeSongbookCallbacks(Songbook* songbook) {
  LuaCallbacks callbacks;

  callbacks.registerCallbackWithSignature<void, Json, String>("play", [songbook](Json const& music, String const& name) { songbook->play(music, name); });
  callbacks.registerCallbackWithSignature<void, String, Vec2F>("keepAlive", [songbook](String const& name, Vec2F const& pos) { songbook->keepAlive(name, pos); });
  callbacks.registerCallbackWithSignature<void>("stop", [songbook]() { songbook->stop(); });
  callbacks.registerCallbackWithSignature<bool>("active", [songbook]() { return songbook->active(); });
  callbacks.registerCallbackWithSignature<Maybe<String>>("band", [songbook]() { return songbook->timeSource(); });
  callbacks.registerCallbackWithSignature<Maybe<String>>("instrument", [songbook]() { return songbook->instrument(); });
  callbacks.registerCallbackWithSignature<bool>("instrumentPlaying", [songbook]() { return songbook->instrumentPlaying(); });
  callbacks.registerCallbackWithSignature<Json>("song", [songbook]() { return songbook->song(); });

  return callbacks;
}

}
