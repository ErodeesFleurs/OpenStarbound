module;
import std;
import star.format;
import star.string;
import star.list;
import star.json;
import star.lua;
import star.lua_converters;
import star.input_event;
import star.thread;
import star.listener;
import star.hash;


// Match client include order for SIMD intrinsics used by xxhash.
import star.input;

module star.input_lua_bindings;

import star.vector;
namespace Star {

std::expected<LuaCallbacks, StarException> LuaBindings::makeInputCallbacks(Input& owner) {
  LuaCallbacks callbacks;

  auto input = &owner;

  if (auto registration = callbacks.registerCallbackWithSignature<std::optional<unsigned>, String, String>("bindDown", std::bind(std::mem_fn(&Input::bindDown), input, std::placeholders::_1, std::placeholders::_2)); !registration)
    return std::unexpected(std::move(registration.error()));
  if (auto registration = callbacks.registerCallbackWithSignature<bool,            String, String>("bindHeld", std::bind(std::mem_fn(&Input::bindHeld), input, std::placeholders::_1, std::placeholders::_2)); !registration)
    return std::unexpected(std::move(registration.error()));
  if (auto registration = callbacks.registerCallbackWithSignature<bool,            String, String>("bind",     std::bind(std::mem_fn(&Input::bindHeld), input, std::placeholders::_1, std::placeholders::_2)); !registration)
    return std::unexpected(std::move(registration.error()));
  if (auto registration = callbacks.registerCallbackWithSignature<std::optional<unsigned>, String, String>("bindUp",   std::bind(std::mem_fn(&Input::bindUp),   input, std::placeholders::_1, std::placeholders::_2)); !registration)
    return std::unexpected(std::move(registration.error()));

  if (auto registration = callbacks.registerCallback("keyDown", [input](String const& keyName, std::optional<StringList> const& modNames) -> std::expected<std::optional<unsigned>, StarException> {
    if (!KeyNames)
      return std::unexpected(KeyNames.error());
    auto key = KeyNames->getLeft(keyName);
    if (!key)
      return std::unexpected(std::move(key.error()));
    std::optional<KeyMod> mod;
    if (modNames) {
      if (!KeyModNames)
        return std::unexpected(KeyModNames.error());
      mod = KeyMod::NoMod;
      for (auto& modName : *modNames) {
        auto modifier = KeyModNames->getLeft(modName);
        if (!modifier)
          return std::unexpected(std::move(modifier.error()));
        *mod |= **modifier;
      }
    }
    return input->keyDown(**key, mod);
  }); !registration)
    return std::unexpected(std::move(registration.error()));
  auto keyHeld = [input](String const& keyName) -> std::expected<bool, StarException> {
    if (!KeyNames)
      return std::unexpected(KeyNames.error());
    auto key = KeyNames->getLeft(keyName);
    if (!key)
      return std::unexpected(std::move(key.error()));
    return input->keyHeld(**key);
  };
  if (auto registration = callbacks.registerCallback("keyHeld", keyHeld); !registration)
    return std::unexpected(std::move(registration.error()));
  if (auto registration = callbacks.registerCallback("key",     keyHeld); !registration)
    return std::unexpected(std::move(registration.error()));
  if (auto registration = callbacks.registerCallback("keyUp", [input](String const& keyName) -> std::expected<std::optional<unsigned>, StarException> {
    if (!KeyNames)
      return std::unexpected(KeyNames.error());
    auto key = KeyNames->getLeft(keyName);
    if (!key)
      return std::unexpected(std::move(key.error()));
    return input->keyUp(**key);
  }); !registration)
    return std::unexpected(std::move(registration.error()));

  if (auto registration = callbacks.registerCallback("mouseDown", [input](String const& buttonName) -> std::expected<std::optional<List<Vec2F>>, StarException> {
    if (!MouseButtonNames)
      return std::unexpected(MouseButtonNames.error());
    auto button = MouseButtonNames->getLeft(buttonName);
    if (!button)
      return std::unexpected(std::move(button.error()));
    return input->mouseDown(**button);
  }); !registration)
    return std::unexpected(std::move(registration.error()));

  auto mouseHeld = [input](String const& buttonName) -> std::expected<bool, StarException> {
    if (!MouseButtonNames)
      return std::unexpected(MouseButtonNames.error());
    auto button = MouseButtonNames->getLeft(buttonName);
    if (!button)
      return std::unexpected(std::move(button.error()));
    return input->mouseHeld(**button);
  };
  if (auto registration = callbacks.registerCallback("mouseHeld", mouseHeld); !registration)
    return std::unexpected(std::move(registration.error()));
  if (auto registration = callbacks.registerCallback("mouse",     mouseHeld); !registration)
    return std::unexpected(std::move(registration.error()));
  if (auto registration = callbacks.registerCallback("mouseUp", [input](String const& buttonName) -> std::expected<std::optional<List<Vec2F>>, StarException> {
    if (!MouseButtonNames)
      return std::unexpected(MouseButtonNames.error());
    auto button = MouseButtonNames->getLeft(buttonName);
    if (!button)
      return std::unexpected(std::move(button.error()));
    return input->mouseUp(**button);
  }); !registration)
    return std::unexpected(std::move(registration.error()));

  if (auto registration = callbacks.registerCallbackWithSignature<void, String, String>("resetBinds",      std::bind(std::mem_fn(&Input::resetBinds),      input, std::placeholders::_1, std::placeholders::_2)); !registration)
    return std::unexpected(std::move(registration.error()));
  if (auto registration = callbacks.registerCallbackWithSignature<void, String, String, Json>("setBinds",  std::bind(std::mem_fn(&Input::setBinds),        input, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)); !registration)
    return std::unexpected(std::move(registration.error()));
  if (auto registration = callbacks.registerCallbackWithSignature<Json, String, String>("getDefaultBinds", std::bind(std::mem_fn(&Input::getDefaultBinds), input, std::placeholders::_1, std::placeholders::_2)); !registration)
    return std::unexpected(std::move(registration.error()));
  if (auto registration = callbacks.registerCallbackWithSignature<Json, String, String>("getBinds",        std::bind(std::mem_fn(&Input::getBinds),        input, std::placeholders::_1, std::placeholders::_2)); !registration)
    return std::unexpected(std::move(registration.error()));

  if (auto registration = callbacks.registerCallback("events", [input]() -> std::expected<Json, StarException> {
    JsonArray result;
    for (auto& pair : input->inputEventsThisFrame()) {
      auto event = Input::inputEventToJson(pair.first);
      if (!event)
        return std::unexpected(std::move(event.error()));
      if (*event) {
        auto processed = event->set("processed", pair.second);
        if (!processed)
          return std::unexpected(std::move(processed.error()));
        result.emplace_back(std::move(*processed));
      }
    }
    return result;
  }); !registration)
    return std::unexpected(std::move(registration.error()));

  if (auto registration = callbacks.registerCallbackWithSignature<Vec2F>("mousePosition", std::bind(std::mem_fn(&Input::mousePosition), input)); !registration)
    return std::unexpected(std::move(registration.error()));
  if (auto registration = callbacks.registerCallbackWithSignature<unsigned, String>("getTag", std::bind(std::mem_fn(&Input::getTag), input, std::placeholders::_1)); !registration)
    return std::unexpected(std::move(registration.error()));

  return callbacks;
}

}
