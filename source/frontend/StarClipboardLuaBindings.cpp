module;
#include "StarJson.hpp"
#include "StarLua.hpp"
#include "StarLuaConverters.hpp"
#include "StarInputEvent.hpp"
#include "StarListener.hpp"
#include "StarHash.hpp"
#include "StarBuffer.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarBiMap.hpp"
#include "StarThread.hpp"
#include "StarIODevice.hpp"
#include "StarAssetPath.hpp"
#include "StarRefPtr.hpp"


// Match client include order for SIMD intrinsics used by xxhash and fast_float.
#include "StarApplicationController.hpp"
import star.input;
import star.asset_source;
import star.assets;
import star.root_base;

module star.clipboard_lua_bindings;

namespace Star {

LuaCallbacks LuaBindings::makeClipboardCallbacks(ApplicationControllerPtr appController, bool alwaysAllow) {
  LuaCallbacks callbacks;

  auto available = [=]() { return alwaysAllow || (appController->isFocused() && Input::singleton().clipboardAllowed()); };

  callbacks.registerCallback("available", [=]() -> bool {
    return available();
  });

  callbacks.registerCallback("hasText", [=]() -> bool {
    return available() && appController->hasClipboard();
  });

  callbacks.registerCallback("getText", [=]() -> Maybe<String> {
    if (!available())
      return {};
    else
      return appController->getClipboard();
  });

  callbacks.registerCallback("setText", [=](String const& text) -> bool {
    if (appController->isFocused()) {
      return appController->setClipboard(text);
    }
    return false;
  });

  callbacks.registerCallback("setData", [=](LuaTable const& data) -> bool {
    if (appController->isFocused()) {
      StringMap<ByteArray> clipboardData;
      data.iterate([&](LuaValue const& key, LuaValue const& value) {
        ByteArray bytes(value.get<LuaString>().ptr(), value.get<LuaString>().length());
        clipboardData[key.get<LuaString>().toString()] = std::move(bytes);
      });
      return appController->setClipboardData(std::move(clipboardData));
    }
    return false;
  });

  callbacks.registerCallback("setImage", [=](LuaValue const& imgOrPath) -> bool {
    if (appController->isFocused()) {
      auto buffer = make_shared<Buffer>();
      if (imgOrPath.is<LuaUserData>() && imgOrPath.get<LuaUserData>().is<Image>()) {
        auto& image = imgOrPath.get<LuaUserData>().get<Image>();
        image.writePng(buffer);
        return appController->setClipboardImage(image, &buffer->data());
      } else {
        auto image = RootBase::singleton().assets()->image(imgOrPath.get<LuaString>().toString());
        image->writePng(buffer);
        return appController->setClipboardImage(*image, &buffer->data());
      }
    }
    return false;
  });

  return callbacks;
}

}
