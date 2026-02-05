#include "StarClipboardLuaBindings.hpp"
#include "StarLuaConverters.hpp" // IWYU pragma: keep
#include "StarInput.hpp"
#include "StarBuffer.hpp"
#include "StarRootBase.hpp"

namespace Star {

LuaCallbacks LuaBindings::makeClipboardCallbacks(ApplicationControllerPtr appController, bool alwaysAllow) {
  LuaCallbacks callbacks;

  auto available = [alwaysAllow, appController]() { return alwaysAllow || (appController->isFocused() && Input::singleton().clipboardAllowed()); };

  callbacks.registerCallback("available", [available]() -> bool {
    return available();
  });

  callbacks.registerCallback("hasText", [available, appController]() -> bool {
    return available() && appController->hasClipboard();
  });

  callbacks.registerCallback("getText", [available, appController]() -> Maybe<String> {
    if (!available())
      return {};
    else
      return appController->getClipboard();
  });

  callbacks.registerCallback("setText", [appController](String const& text) -> bool {
    if (appController->isFocused()) {
      return appController->setClipboard(text);
    }
    return false;
  });

  callbacks.registerCallback("setData", [appController](LuaTable const& data) -> bool {
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

  callbacks.registerCallback("setImage", [appController](LuaValue const& imgOrPath) -> bool {
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
};

}
