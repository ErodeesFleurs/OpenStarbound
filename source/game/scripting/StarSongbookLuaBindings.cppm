module;
#include "StarThread.hpp"
#include "StarNetElementSystem.hpp"
#include "StarGameTypes.hpp"

// Match existing consumers' include order for SIMD intrinsics used by xxhash and fast_float.
#include "StarJson.hpp"
#include "StarLua.hpp"
#include "StarLuaConverters.hpp"

import star.songbook;

export module star.songbook_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeSongbookCallbacks(Songbook* songbook);
}

namespace Star {

LuaCallbacks LuaBindings::makeSongbookCallbacks(Songbook* songbook) {
  LuaCallbacks callbacks;

  callbacks.registerCallbackWithSignature<void, Json, String>("play", bind(mem_fn(&Songbook::play), songbook, _1, _2));
  callbacks.registerCallbackWithSignature<void, String, Vec2F>("keepAlive", bind(mem_fn(&Songbook::keepAlive), songbook, _1, _2));
  callbacks.registerCallbackWithSignature<void>("stop", bind(mem_fn(&Songbook::stop), songbook));
  callbacks.registerCallbackWithSignature<bool>("active", bind(mem_fn(&Songbook::active), songbook));
  callbacks.registerCallbackWithSignature<Maybe<String>>("band", bind(mem_fn(&Songbook::timeSource), songbook));
  callbacks.registerCallbackWithSignature<Maybe<String>>("instrument", bind(mem_fn(&Songbook::instrument), songbook));
  callbacks.registerCallbackWithSignature<bool>("instrumentPlaying", bind(mem_fn(&Songbook::instrumentPlaying), songbook));
  callbacks.registerCallbackWithSignature<Json>("song", bind(mem_fn(&Songbook::song), songbook));

  return callbacks;
}

}
