module;
import std;
import star.string;
import star.list;
import star.maybe;
import star.json;
import star.lua;
import star.lua_converters;
import star.input_event;
import star.thread;
import star.listener;
import star.hash;


// Match client include order for SIMD intrinsics used by xxhash and fast_float.
import star.input;

module star.input_lua_bindings;

import star.vector;
namespace Star {

LuaCallbacks LuaBindings::makeInputCallbacks() {
  LuaCallbacks callbacks;

  auto input = Input::singletonPtr();

  callbacks.registerCallbackWithSignature<Maybe<unsigned>, String, String>("bindDown", std::bind(std::mem_fn(&Input::bindDown), input, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<bool,            String, String>("bindHeld", std::bind(std::mem_fn(&Input::bindHeld), input, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<bool,            String, String>("bind",     std::bind(std::mem_fn(&Input::bindHeld), input, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<Maybe<unsigned>, String, String>("bindUp",   std::bind(std::mem_fn(&Input::bindUp),   input, std::placeholders::_1, std::placeholders::_2));

  callbacks.registerCallback("keyDown", [input](String const& keyName, Maybe<StringList> const& modNames) -> Maybe<unsigned> {
    Key key = KeyNames.getLeft(keyName);
    Maybe<KeyMod> mod;
    if (modNames) {
      mod = KeyMod::NoMod;
      for (auto& modName : *modNames)
        *mod |= KeyModNames.getLeft(modName);
    }
    return input->keyDown(key, mod);
  });
  auto keyHeld = [input](String const& keyName) -> bool { return input->keyHeld(KeyNames.getLeft(keyName)); };
  callbacks.registerCallback("keyHeld", keyHeld);
  callbacks.registerCallback("key",     keyHeld);
  callbacks.registerCallback("keyUp",   [input](String const& keyName) -> Maybe<unsigned> { return input->keyUp(  KeyNames.getLeft(keyName)); });

  callbacks.registerCallback("mouseDown", [input](String const& buttonName) -> Maybe<List<Vec2F>>
    { return input->mouseDown(MouseButtonNames.getLeft(buttonName)); });

  auto mouseHeld = [input](String const& buttonName) -> bool { return input->mouseHeld(MouseButtonNames.getLeft(buttonName)); };
  callbacks.registerCallback("mouseHeld", mouseHeld);
  callbacks.registerCallback("mouse",     mouseHeld);
  callbacks.registerCallback("mouseUp",   [input](String const& buttonName) -> Maybe<List<Vec2F>>
    { return input->mouseUp(  MouseButtonNames.getLeft(buttonName)); });

  callbacks.registerCallbackWithSignature<void, String, String>("resetBinds",      std::bind(std::mem_fn(&Input::resetBinds),      input, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<void, String, String, Json>("setBinds",  std::bind(std::mem_fn(&Input::setBinds),        input, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  callbacks.registerCallbackWithSignature<Json, String, String>("getDefaultBinds", std::bind(std::mem_fn(&Input::getDefaultBinds), input, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<Json, String, String>("getBinds",        std::bind(std::mem_fn(&Input::getBinds),        input, std::placeholders::_1, std::placeholders::_2));

  callbacks.registerCallback("events", [input]() -> Json {
    JsonArray result;

    for (auto& pair : input->inputEventsThisFrame()) {
      if (auto jEvent = Input::inputEventToJson(pair.first))
        result.emplace_back(jEvent.set("processed", pair.second));
    }

    return result;
  });

  callbacks.registerCallbackWithSignature<Vec2F>("mousePosition", std::bind(std::mem_fn(&Input::mousePosition), input));
  callbacks.registerCallbackWithSignature<unsigned, String>("getTag", std::bind(std::mem_fn(&Input::getTag), input, std::placeholders::_1));

  return callbacks;
}

}
