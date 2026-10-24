module;
import std;
import star.format;
import star.string;
import star.json;
import star.thread;
import star.vector;
// Match existing consumers' include order for SIMD intrinsics used by xxhash.
import star.game_types;

import star.lua;
import star.lua_converters;

import star.songbook;

export module star.songbook_lua_bindings;

export namespace Star::LuaBindings {
  [[nodiscard]] std::expected<LuaCallbacks, StarException> makeSongbookCallbacks(Songbook* songbook);
}

namespace Star {

std::expected<LuaCallbacks, StarException> LuaBindings::makeSongbookCallbacks(Songbook* songbook) {
  LuaCallbacks callbacks;

  if (auto registration = callbacks.registerCallbackWithSignature<void, Json, String>("play", std::bind(std::mem_fn(&Songbook::play), songbook, std::placeholders::_1, std::placeholders::_2)); !registration)
    return std::unexpected(std::move(registration.error()));
  if (auto registration = callbacks.registerCallbackWithSignature<void, String, Vec2F>("keepAlive", std::bind(std::mem_fn(&Songbook::keepAlive), songbook, std::placeholders::_1, std::placeholders::_2)); !registration)
    return std::unexpected(std::move(registration.error()));
  if (auto registration = callbacks.registerCallbackWithSignature<void>("stop", std::bind(std::mem_fn(&Songbook::stop), songbook)); !registration)
    return std::unexpected(std::move(registration.error()));
  if (auto registration = callbacks.registerCallbackWithSignature<bool>("active", std::bind(std::mem_fn(&Songbook::active), songbook)); !registration)
    return std::unexpected(std::move(registration.error()));
  if (auto registration = callbacks.registerCallbackWithSignature<std::optional<String>>("band", std::bind(std::mem_fn(&Songbook::timeSource), songbook)); !registration)
    return std::unexpected(std::move(registration.error()));
  if (auto registration = callbacks.registerCallbackWithSignature<std::optional<String>>("instrument", std::bind(std::mem_fn(&Songbook::instrument), songbook)); !registration)
    return std::unexpected(std::move(registration.error()));
  if (auto registration = callbacks.registerCallbackWithSignature<bool>("instrumentPlaying", std::bind(std::mem_fn(&Songbook::instrumentPlaying), songbook)); !registration)
    return std::unexpected(std::move(registration.error()));
  if (auto registration = callbacks.registerCallbackWithSignature<Json>("song", std::bind(std::mem_fn(&Songbook::song), songbook)); !registration)
    return std::unexpected(std::move(registration.error()));

  return callbacks;
}

}
