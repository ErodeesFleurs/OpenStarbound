module;
import std;
import star.maybe;
import star.json;
import star.lua;
import star.lua_converters;
import star.input_event;
import star.thread;
import star.listener;
import star.hash;
import star.io_device;
import star.string;
import star.buffer;
import star.ordered_map;
import star.bi_map;
import star.list;
import star.image_processing;
import star.data_stream;
import star.string_view;
import star.directives;
import star.asset_path;
import star.ref_ptr;


// Match client include order for SIMD intrinsics used by xxhash and fast_float.
import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
import star.application_controller;
import star.input;
import star.asset_source;
import star.assets;
import star.root_base;
import star.logging;

module star.clipboard_lua_bindings;

import star.byte_array;
namespace Star {

LuaCallbacks LuaBindings::makeClipboardCallbacks(Input& input, RootBase& root, Logger& logger, std::shared_ptr<ApplicationController> appController, bool alwaysAllow) {
  LuaCallbacks callbacks;

  auto available = [appController, alwaysAllow, &input]() {
    return alwaysAllow || (appController->isFocused() && input.clipboardAllowed());
  };

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

  callbacks.registerCallback("setImage", [appController, &root, &logger](LuaValue const& imgOrPath) -> bool {
    if (appController->isFocused()) {
      auto buffer = std::make_shared<Buffer>();
      if (imgOrPath.is<LuaUserData>() && imgOrPath.get<LuaUserData>().is<Image>()) {
        auto& image = imgOrPath.get<LuaUserData>().get<Image>();
        image.writePng(logger, buffer);
        return appController->setClipboardImage(image, &buffer->data());
      } else {
        auto image = root.assets()->image(imgOrPath.get<LuaString>().toString());
        image->writePng(logger, buffer);
        return appController->setClipboardImage(*image, &buffer->data());
      }
    }
    return false;
  });

  return callbacks;
}

}
