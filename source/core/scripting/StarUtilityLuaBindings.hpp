#pragma once

#include "StarLua.hpp"

namespace Star {

namespace LuaBindings {
  [[nodiscard]] LuaCallbacks makeUtilityCallbacks();

  [[nodiscard]] String formatLua(String const& string, List<LuaValue> const& args);

  namespace UtilityCallbacks {
    [[nodiscard]] double nrand(Maybe<double> const& stdev, Maybe<double> const& mean);
    [[nodiscard]] String makeUuid();
    void logInfo(String const& str, LuaVariadic<LuaValue> const& args);
    void logWarn(String const& str, LuaVariadic<LuaValue> const& args);
    void logError(String const& str, LuaVariadic<LuaValue> const& args);
    void setLogMap(String const& key, String const& value, LuaVariadic<LuaValue> const& args);
    [[nodiscard]] Json parseJson(String const& str);
    [[nodiscard]] String printJson(Json const& arg, Maybe<int> pretty);
    [[nodiscard]] String print(LuaValue const& arg);
    [[nodiscard]] LuaValue interpolateSinEase(LuaEngine& engine, double offset, LuaValue const& value1, LuaValue const& value2);
    [[nodiscard]] String replaceTags(String const& str, StringMap<String> const& tags);
  }
}
}
