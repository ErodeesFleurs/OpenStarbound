module;
import star.string;
import star.list;
import star.json;
import star.maybe;
import star.lua;

namespace Star {

namespace LuaBindings {
  LuaCallbacks makeUtilityCallbacks();

  String formatLua(String const& string, List<LuaValue> const& args);

  namespace UtilityCallbacks {
    double nrand(Maybe<double> const& stdev, Maybe<double> const& mean);
    String makeUuid();
    void logInfo(String const& str, LuaVariadic<LuaValue> const& args);
    void logWarn(String const& str, LuaVariadic<LuaValue> const& args);
    void logError(String const& str, LuaVariadic<LuaValue> const& args);
    void setLogMap(String const& key, String const& value, LuaVariadic<LuaValue> const& args);
    Json parseJson(String const& str);
    String printJson(Json const& arg, Maybe<int> pretty);
    String print(LuaValue const& arg);
    LuaValue interpolateSinEase(LuaEngine& engine, double offset, LuaValue const& value1, LuaValue const& value2);
    String replaceTags(String const& str, StringMap<String> const& tags);
  }
}
}

export module star.utility_lua_bindings;

export namespace Star::LuaBindings {
  using ::Star::LuaBindings::makeUtilityCallbacks;
  using ::Star::LuaBindings::formatLua;
}

export namespace Star::LuaBindings::UtilityCallbacks {
  using ::Star::LuaBindings::UtilityCallbacks::nrand;
  using ::Star::LuaBindings::UtilityCallbacks::makeUuid;
  using ::Star::LuaBindings::UtilityCallbacks::logInfo;
  using ::Star::LuaBindings::UtilityCallbacks::logWarn;
  using ::Star::LuaBindings::UtilityCallbacks::logError;
  using ::Star::LuaBindings::UtilityCallbacks::setLogMap;
  using ::Star::LuaBindings::UtilityCallbacks::parseJson;
  using ::Star::LuaBindings::UtilityCallbacks::printJson;
  using ::Star::LuaBindings::UtilityCallbacks::print;
  using ::Star::LuaBindings::UtilityCallbacks::interpolateSinEase;
  using ::Star::LuaBindings::UtilityCallbacks::replaceTags;
}
