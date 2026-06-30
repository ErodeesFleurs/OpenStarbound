#include "StarInputLuaBindings.hpp"
#include "StarLuaConverters.hpp"
#include "StarInput.hpp"

namespace Star {

LuaCallbacks LuaBindings::makeInputCallbacks() {
  LuaCallbacks callbacks;

  auto input = Input::singletonPtr();

  callbacks.registerCallbackWithSignature<Maybe<unsigned>, String, String>("bindDown", [input](String const& category, String const& bindName) { return input->bindDown(category, bindName); });
  callbacks.registerCallbackWithSignature<bool,            String, String>("bindHeld", [input](String const& category, String const& bindName) { return input->bindHeld(category, bindName); });
  callbacks.registerCallbackWithSignature<bool,            String, String>("bind",     [input](String const& category, String const& bindName) { return input->bindHeld(category, bindName); });
  callbacks.registerCallbackWithSignature<Maybe<unsigned>, String, String>("bindUp",   [input](String const& category, String const& bindName) { return input->bindUp(category, bindName); });

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

  callbacks.registerCallbackWithSignature<void, String, String>("resetBinds",      [input](String const& category, String const& bindName) { input->resetBinds(category, bindName); });
  callbacks.registerCallbackWithSignature<void, String, String, Json>("setBinds",  [input](String const& category, String const& bindName, Json const& binding) { input->setBinds(category, bindName, binding); });
  callbacks.registerCallbackWithSignature<Json, String, String>("getDefaultBinds", [input](String const& category, String const& bindName) { return input->getDefaultBinds(category, bindName); });
  callbacks.registerCallbackWithSignature<Json, String, String>("getBinds",        [input](String const& category, String const& bindName) { return input->getBinds(category, bindName); });

  callbacks.registerCallback("events", [input]() -> Json {
    JsonArray result;

    for (auto& pair : input->inputEventsThisFrame()) {
      if (auto jEvent = Input::inputEventToJson(pair.first))
        result.emplace_back(jEvent.set("processed", pair.second));
    }

    return result;
  });

  callbacks.registerCallbackWithSignature<Vec2F>("mousePosition", [input]() { return input->mousePosition(); });
  callbacks.registerCallbackWithSignature<unsigned, String>("getTag", [input](String const& tagName) { return input->getTag(tagName); });

  return callbacks;
}


}
