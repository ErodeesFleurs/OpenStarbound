module;
import std;
import star.string;
import star.maybe;
import star.json;
import star.thread;
import star.vector;
// Match existing consumers' include order for SIMD intrinsics used by xxhash and fast_float.
import star.game_types;

import star.lua;
import star.lua_converters;

import star.songbook;

export module star.songbook_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeSongbookCallbacks(Songbook* songbook);
}

namespace Star {

LuaCallbacks LuaBindings::makeSongbookCallbacks(Songbook* songbook) {
  LuaCallbacks callbacks;

  callbacks.registerCallbackWithSignature<void, Json, String>("play", std::bind(std::mem_fn(&Songbook::play), songbook, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<void, String, Vec2F>("keepAlive", std::bind(std::mem_fn(&Songbook::keepAlive), songbook, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<void>("stop", std::bind(std::mem_fn(&Songbook::stop), songbook));
  callbacks.registerCallbackWithSignature<bool>("active", std::bind(std::mem_fn(&Songbook::active), songbook));
  callbacks.registerCallbackWithSignature<Maybe<String>>("band", std::bind(std::mem_fn(&Songbook::timeSource), songbook));
  callbacks.registerCallbackWithSignature<Maybe<String>>("instrument", std::bind(std::mem_fn(&Songbook::instrument), songbook));
  callbacks.registerCallbackWithSignature<bool>("instrumentPlaying", std::bind(std::mem_fn(&Songbook::instrumentPlaying), songbook));
  callbacks.registerCallbackWithSignature<Json>("song", std::bind(std::mem_fn(&Songbook::song), songbook));

  return callbacks;
}

}
