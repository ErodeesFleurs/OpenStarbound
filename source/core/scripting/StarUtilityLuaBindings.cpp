#include "StarUtilityLuaBindings.hpp"
#include "StarInterpolation.hpp"
#include "StarLogging.hpp"
#include "StarLuaConverters.hpp"// IWYU pragma: export
#include "StarPerlin.hpp"
#include "StarRandom.hpp"
#include "StarText.hpp"
#include "StarUuid.hpp"

import std;

namespace Star {

template <>
struct LuaConverter<RandomSource> : LuaUserDataConverter<RandomSource> {};

template <>
struct LuaUserDataMethods<RandomSource> {
  static auto make() -> LuaMethods<RandomSource> {
    LuaMethods<RandomSource> methods;

    methods.registerMethod("init", [](RandomSource& randomSource, std::optional<std::uint64_t> seed) -> void { seed ? randomSource.init(*seed) : randomSource.init(); });
    methods.registerMethod("addEntropy", [](RandomSource& randomSource, std::optional<std::uint64_t> seed) -> void { seed ? randomSource.addEntropy(*seed) : randomSource.addEntropy(); });

    methods.registerMethodWithSignature<std::uint32_t, RandomSource&>("randu32", std::mem_fn(&RandomSource::randu32));
    methods.registerMethodWithSignature<std::uint64_t, RandomSource&>("randu64", std::mem_fn(&RandomSource::randu64));
    methods.registerMethodWithSignature<std::int32_t, RandomSource&>("randi32", std::mem_fn(&RandomSource::randi32));
    methods.registerMethodWithSignature<std::int64_t, RandomSource&>("randi64", std::mem_fn(&RandomSource::randi64));

    methods.registerMethod("randf", [](RandomSource& randomSource, std::optional<float> arg1, std::optional<float> arg2) -> float { return (arg1 && arg2) ? randomSource.randf(*arg1, *arg2) : randomSource.randf(); });
    methods.registerMethod("randd", [](RandomSource& randomSource, std::optional<double> arg1, std::optional<double> arg2) -> double { return (arg1 && arg2) ? randomSource.randd(*arg1, *arg2) : randomSource.randd(); });

    methods.registerMethodWithSignature<bool, RandomSource&>("randb", std::mem_fn(&RandomSource::randb));

    methods.registerMethod("randInt", [](RandomSource& randomSource, std::int64_t arg1, std::optional<std::int64_t> arg2) -> std::int64_t { return arg2 ? randomSource.randInt(arg1, *arg2) : randomSource.randInt(arg1); });

    methods.registerMethod("randUInt", [](RandomSource& randomSource, std::uint64_t arg1, std::optional<std::uint64_t> arg2) -> std::uint64_t { return arg2 ? randomSource.randUInt(arg1, *arg2) : randomSource.randUInt(arg1); });

    return methods;
  }
};

template <>
struct LuaConverter<PerlinF> : LuaUserDataConverter<PerlinF> {};

template <>
struct LuaUserDataMethods<PerlinF> {
  static auto make() -> LuaMethods<PerlinF> {
    LuaMethods<PerlinF> methods;

    methods.registerMethod("get",
                           [](PerlinF& perlinF, float x, std::optional<float> y, std::optional<float> z) -> float {
                             if (y && z)
                               return perlinF.get(x, *y, *z);
                             else if (y)
                               return perlinF.get(x, *y);
                             else
                               return perlinF.get(x);
                           });

    return methods;
  }
};

auto LuaBindings::formatLua(String const& string, List<LuaValue> const& args) -> String {
  auto argsIt = args.begin();
  auto argsEnd = args.end();
  auto popArg = [&argsIt, &argsEnd]() -> LuaValue {
    if (argsIt == argsEnd)
      return LuaNil;
    return *argsIt++;
  };

  auto stringIt = string.begin();
  auto stringEnd = string.end();

  String result;

  while (stringIt != stringEnd) {
    if (*stringIt == '%') {
      auto next = stringIt;
      ++next;

      if (next == stringEnd)
        throw StarException("No specifier following '%'");
      else if (*next == '%')
        result += '%';
      else if (*next == 's')
        result += toString(popArg());
      else
        throw StarException::format("Improper lua log format specifier {}", (char)*next);
      ++next;
      stringIt = next;
    } else {
      result += *stringIt++;
    }
  }

  return result;
}

auto LuaBindings::makeUtilityCallbacks() -> LuaCallbacks {
  LuaCallbacks callbacks;

  callbacks.registerCallback("nrand", UtilityCallbacks::nrand);
  callbacks.registerCallback("makeUuid", UtilityCallbacks::makeUuid);
  callbacks.registerCallback("logInfo", UtilityCallbacks::logInfo);
  callbacks.registerCallback("logWarn", UtilityCallbacks::logWarn);
  callbacks.registerCallback("logError", UtilityCallbacks::logError);
  callbacks.registerCallback("setLogMap", UtilityCallbacks::setLogMap);
  callbacks.registerCallback("parseJson", UtilityCallbacks::parseJson);
  callbacks.registerCallback("printJson", UtilityCallbacks::printJson);
  callbacks.registerCallback("print", UtilityCallbacks::print);
  callbacks.registerCallback("interpolateSinEase", UtilityCallbacks::interpolateSinEase);
  callbacks.registerCallback("replaceTags", UtilityCallbacks::replaceTags);
  callbacks.registerCallback("stripEscapeCodes", [](String const& text) -> String { return Text::stripEscapeCodes(text); });
  callbacks.registerCallback("parseJsonSequence", [](String const& json) -> Json { return Json::parseSequence(json); });
  callbacks.registerCallback("jsonMerge", [](Json const& a, Json const& b) -> Json { return jsonMerge(a, b); });
  callbacks.registerCallback("jsonEqual", [](Json const& a, Json const& b) -> bool { return a == b; });
  callbacks.registerCallback("jsonQuery", [](Json const& json, String const& path, Json const& def) -> Json { return json.query(path, def); });
  callbacks.registerCallback("makeRandomSource", [](std::optional<std::uint64_t> seed) -> RandomSource { return seed ? RandomSource(*seed) : RandomSource(); });
  callbacks.registerCallback("makePerlinSource", [](Json const& config) -> PerlinF { return PerlinF(config); });

  callbacks.copyCallback("parseJson", "jsonFromString");// SE compat

  auto hash64LuaValues = [](LuaVariadic<LuaValue> const& values) -> std::uint64_t {
    std::size_t seed = 233;

    for (auto const& value : values) {
      if (auto b = value.ptr<LuaBoolean>()) {
        hashCombine(seed, std::hash<int>{}(1));
        hashCombine(seed, std::hash<bool>{}(*b));
      } else if (auto i = value.ptr<LuaInt>()) {
        hashCombine(seed, std::hash<int>{}(2));
        hashCombine(seed, std::hash<long long>{}(*i));
      } else if (auto f = value.ptr<LuaFloat>()) {
        hashCombine(seed, std::hash<int>{}(3));
        hashCombine(seed, std::hash<double>{}(*f));
      } else if (auto s = value.ptr<LuaString>()) {
        hashCombine(seed, std::hash<int>{}(4));
        std::string_view sv(s->ptr(), s->length());
        hashCombine(seed, std::hash<std::string_view>{}(sv));
      } else {
        throw LuaException("Unhashable lua type passed to staticRandomXX binding");
      }
    }

    return static_cast<std::uint64_t>(seed);
  };

  callbacks.registerCallback("staticRandomI32",
                             [hash64LuaValues](LuaVariadic<LuaValue> const& hashValues) -> std::int32_t { return (std::int32_t)hash64LuaValues(hashValues); });

  callbacks.registerCallback("staticRandomI32Range",
                             [hash64LuaValues](std::int32_t min, std::int32_t max, LuaVariadic<LuaValue> const& hashValues) -> std::int32_t {
                               if (max < min)
                                 throw LuaException("Maximum bound in staticRandomI32Range must be >= minimum bound!");
                               std::uint64_t denom = (std::uint64_t)(-1) / ((std::uint64_t)(max - min) + 1);
                               return (std::int32_t)(hash64LuaValues(hashValues) / denom + min);
                             });

  callbacks.registerCallback("staticRandomDouble",
                             [hash64LuaValues](LuaVariadic<LuaValue> const& hashValues) -> double {
                               return (hash64LuaValues(hashValues) & 0x7fffffffffffffff) / 9223372036854775808.0;
                             });

  callbacks.registerCallback("staticRandomDoubleRange",
                             [hash64LuaValues](double min, double max, LuaVariadic<LuaValue> const& hashValues) -> double {
                               if (max < min)
                                 throw LuaException("Maximum bound in staticRandomDoubleRange must be >= minimum bound!");
                               return (hash64LuaValues(hashValues) & 0x7fffffffffffffff) / 9223372036854775808.0 * (max - min) + min;
                             });

  return callbacks;
}

auto LuaBindings::UtilityCallbacks::nrand(std::optional<double> const& stdev, std::optional<double> const& mean) -> double {
  return Random::nrandd(stdev.value_or(1.0), mean.value_or(0));
}

auto LuaBindings::UtilityCallbacks::makeUuid() -> String {
  return Uuid().hex();
}

void LuaBindings::UtilityCallbacks::logInfo(String const& str, LuaVariadic<LuaValue> const& args) {
  Logger::log(LogLevel::Info, formatLua(str, args).utf8Ptr());
}

void LuaBindings::UtilityCallbacks::logWarn(String const& str, LuaVariadic<LuaValue> const& args) {
  Logger::log(LogLevel::Warn, formatLua(str, args).utf8Ptr());
}

void LuaBindings::UtilityCallbacks::logError(String const& str, LuaVariadic<LuaValue> const& args) {
  Logger::log(LogLevel::Error, formatLua(str, args).utf8Ptr());
}

void LuaBindings::UtilityCallbacks::setLogMap(String const& key, String const& value, LuaVariadic<LuaValue> const& args) {
  LogMap::set(key, formatLua(value, args));
}

auto LuaBindings::UtilityCallbacks::parseJson(String const& str) -> Json {
  return Json::parse(str);
}

auto LuaBindings::UtilityCallbacks::printJson(Json const& arg, std::optional<int> pretty) -> String {
  return arg.repr(pretty.value_or(0));
}

auto LuaBindings::UtilityCallbacks::print(LuaValue const& value) -> String {
  return toString(value);
}

auto LuaBindings::UtilityCallbacks::interpolateSinEase(LuaEngine& engine, double offset, LuaValue const& value1, LuaValue const& value2) -> LuaValue {
  if (auto floatValue1 = engine.luaMaybeTo<double>(value1)) {
    auto floatValue2 = engine.luaMaybeTo<double>(value2);
    return sinEase(offset, *floatValue1, *floatValue2);
  } else {
    return engine.luaFrom<Vec2F>(sinEase(offset, engine.luaTo<Vec2F>(value1), engine.luaTo<Vec2F>(value2)));
  }
}

auto LuaBindings::UtilityCallbacks::replaceTags(String const& str, StringMap<String> const& tags) -> String {
  return str.replaceTags(tags);
}

}// namespace Star
