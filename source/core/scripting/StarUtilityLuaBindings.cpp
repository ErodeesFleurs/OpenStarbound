import std;
import star.lua;
import star.maybe;
import star.format;
import star.json;
import star.xx_hash;
import star.json_extra;
import star.utility_lua_bindings;
import star.data_stream;
import star.uuid;
import star.random;
import star.bi_map;
import star.interpolation;
import star.perlin;
import star.logging;
import star.rpc_promise;
import star.lua_converters;
import star.string;
import star.string_view;
import star.lexical_cast;
import star.vector;
import star.list;
import star.image_processing;
import star.hash;
import star.thread;
import star.directives;
import star.text;
import star.time;

import star.static_random;
namespace Star {

struct LuaRandomSource : RandomSource {
  LuaRandomSource(RandomService& owner, std::uint64_t seed) : RandomSource(seed), owner(owner) {}
  RandomService& owner;
};

template <>
struct LuaConverter<LuaRandomSource> : LuaUserDataConverter<LuaRandomSource> {};

template <>
struct LuaUserDataMethods<LuaRandomSource> {
  static LuaMethods<LuaRandomSource> make() {
    LuaMethods<LuaRandomSource> methods;

    methods.registerMethod("init", [](LuaRandomSource& source, Maybe<std::uint64_t> seed)
      { source.init(seed ? *seed : source.owner.randu64()); });
    methods.registerMethod("addEntropy", [](LuaRandomSource& source, Maybe<std::uint64_t> seed)
      { source.addEntropy(seed ? *seed : source.owner.randu64()); });

    methods.registerMethodWithSignature<std::uint32_t, LuaRandomSource&>("randu32", std::mem_fn(&RandomSource::randu32));
    methods.registerMethodWithSignature<std::uint64_t, LuaRandomSource&>("randu64", std::mem_fn(&RandomSource::randu64));
    methods.registerMethodWithSignature<std::int32_t, LuaRandomSource&>("randi32", std::mem_fn(&RandomSource::randi32));
    methods.registerMethodWithSignature<std::int64_t, LuaRandomSource&>("randi64", std::mem_fn(&RandomSource::randi64));

    methods.registerMethod("randf", [](LuaRandomSource& source, Maybe<float> arg1, Maybe<float> arg2)
      { return (arg1 && arg2) ? source.randf(*arg1, *arg2) : source.randf(); });
    methods.registerMethod("randd", [](LuaRandomSource& source, Maybe<double> arg1, Maybe<double> arg2)
      { return (arg1 && arg2) ? source.randd(*arg1, *arg2) : source.randd(); });

    methods.registerMethodWithSignature<bool, LuaRandomSource&>("randb", std::mem_fn(&RandomSource::randb));

    methods.registerMethod("randInt", [](LuaRandomSource& source, std::int64_t arg1, Maybe<std::int64_t> arg2)
      { return arg2 ? source.randInt(arg1, *arg2) : source.randInt(arg1); });
    methods.registerMethod("randUInt", [](LuaRandomSource& source, std::uint64_t arg1, Maybe<std::uint64_t> arg2)
      { return arg2 ? source.randUInt(arg1, *arg2) : source.randUInt(arg1); });
    return methods;
  }
};

template <>
struct LuaConverter<PerlinF> : LuaUserDataConverter<PerlinF> {};

template <>
struct LuaUserDataMethods<PerlinF> {
  static LuaMethods<PerlinF> make() {
    LuaMethods<PerlinF> methods;

    methods.registerMethod("get",
        [](PerlinF& perlinF, float x, Maybe<float> y, Maybe<float> z) {
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

String LuaBindings::formatLua(String const& string, List<LuaValue> const& args) {
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

LuaCallbacks LuaBindings::makeUtilityCallbacks(RandomService& random, Logger& logger, LogMap& logMap) {
  LuaCallbacks callbacks;

  callbacks.registerCallback("nrand", [&random](Maybe<double> const& stdev, Maybe<double> const& mean) {
    return UtilityCallbacks::nrand(random, stdev, mean);
  });
  callbacks.registerCallback("makeUuid", [&random]() { return UtilityCallbacks::makeUuid(random); });
  callbacks.registerCallback("logInfo", [&logger](String const& str, LuaVariadic<LuaValue> const& args) { UtilityCallbacks::logInfo(logger, str, args); });
  callbacks.registerCallback("logWarn", [&logger](String const& str, LuaVariadic<LuaValue> const& args) { UtilityCallbacks::logWarn(logger, str, args); });
  callbacks.registerCallback("logError", [&logger](String const& str, LuaVariadic<LuaValue> const& args) { UtilityCallbacks::logError(logger, str, args); });
  callbacks.registerCallback("setLogMap", [&logMap](String const& key, String const& value, LuaVariadic<LuaValue> const& args) {
    UtilityCallbacks::setLogMap(logMap, key, value, args);
  });
  callbacks.registerCallback("parseJson", UtilityCallbacks::parseJson);
  callbacks.registerCallback("printJson", UtilityCallbacks::printJson);
  callbacks.registerCallback("print", UtilityCallbacks::print);
  callbacks.registerCallback("interpolateSinEase", UtilityCallbacks::interpolateSinEase);
  callbacks.registerCallback("replaceTags", UtilityCallbacks::replaceTags);
  callbacks.registerCallback("stripEscapeCodes", [](String const& text) { return Text::stripEscapeCodes(text); });
  callbacks.registerCallback("parseJsonSequence", [](String const& json) { return Json::parseSequence(json); });
  callbacks.registerCallback("jsonMerge", [](Json const& a, Json const& b) { return jsonMerge(a, b); });
  callbacks.registerCallback("jsonEqual", [](Json const& a, Json const& b) { return a == b; });
  callbacks.registerCallback("jsonQuery", [](Json const& json, String const& path, Json const& def) { return json.query(path, def); });
  callbacks.registerCallback("makeRandomSource", [&random](Maybe<std::uint64_t> seed) { return LuaRandomSource(random, seed ? *seed : random.randu64()); });
  callbacks.registerCallback("makePerlinSource", [](Json const& config) { return PerlinF(config); });

  callbacks.copyCallback("parseJson", "jsonFromString"); // SE compat

  auto hash64LuaValues = [](LuaVariadic<LuaValue> const& values) -> std::uint64_t {
    XXHash64 hash;

    for (auto const& value : values) {
      if (auto b = value.ptr<LuaBoolean>())
        xxHash64Push(hash, *b);
      else if (auto i = value.ptr<LuaInt>())
        xxHash64Push(hash, *i);
      else if (auto f = value.ptr<LuaFloat>())
        xxHash64Push(hash, *f);
      else if (auto s = value.ptr<LuaString>())
        xxHash64Push(hash, s->ptr());
      else
        throw LuaException("Unhashable lua type passed to staticRandomXX binding");
    }

    return hash.digest();
  };

  callbacks.registerCallback("staticRandomI32",
      [hash64LuaValues](LuaVariadic<LuaValue> const& hashValues) { return (std::int32_t)hash64LuaValues(hashValues); });

  callbacks.registerCallback("staticRandomI32Range",
      [hash64LuaValues](std::int32_t min, std::int32_t max, LuaVariadic<LuaValue> const& hashValues) {
        if (max < min)
          throw LuaException("Maximum bound in staticRandomI32Range must be >= minimum bound!");
        std::uint64_t denom = (std::uint64_t)(-1) / ((std::uint64_t)(max - min) + 1);
        return (std::int32_t)(hash64LuaValues(hashValues) / denom + min);
      });

  callbacks.registerCallback("staticRandomDouble",
      [hash64LuaValues](LuaVariadic<LuaValue> const& hashValues) {
        return (hash64LuaValues(hashValues) & 0x7fffffffffffffff) / 9223372036854775808.0;
      });

  callbacks.registerCallback("staticRandomDoubleRange",
      [hash64LuaValues](double min, double max, LuaVariadic<LuaValue> const& hashValues) {
        if (max < min)
          throw LuaException("Maximum bound in staticRandomDoubleRange must be >= minimum bound!");
        return (hash64LuaValues(hashValues) & 0x7fffffffffffffff) / 9223372036854775808.0 * (max - min) + min;
      });
  
  callbacks.registerCallback("millisecondsSinceEpoch",
      []() -> std::int64_t {
        return Time::millisecondsSinceEpoch();
      });
  
  callbacks.registerCallback("makePromise",
      []() -> std::pair<RpcPromise<Json>,std::shared_ptr<AutoFailRpcPromiseKeeper<Json>>> {
        auto pair = RpcPromise<Json>::createPair();
        return std::make_pair(pair.first,std::make_shared<AutoFailRpcPromiseKeeper<Json>>(pair.second));
      });

  return callbacks;
}

double LuaBindings::UtilityCallbacks::nrand(RandomService& random, Maybe<double> const& stdev, Maybe<double> const& mean) {
  return random.nrandd(stdev.value(1.0), mean.value(0));
}

String LuaBindings::UtilityCallbacks::makeUuid(RandomService& random) {
  return Uuid(random).hex();
}

void LuaBindings::UtilityCallbacks::logInfo(Logger& logger, String const& str, LuaVariadic<LuaValue> const& args) {
  logger.log(LogLevel::Info, formatLua(str, args).utf8Ptr());
}

void LuaBindings::UtilityCallbacks::logWarn(Logger& logger, String const& str, LuaVariadic<LuaValue> const& args) {
  logger.log(LogLevel::Warn, formatLua(str, args).utf8Ptr());
}

void LuaBindings::UtilityCallbacks::logError(Logger& logger, String const& str, LuaVariadic<LuaValue> const& args) {
  logger.log(LogLevel::Error, formatLua(str, args).utf8Ptr());
}

void LuaBindings::UtilityCallbacks::setLogMap(LogMap& logMap, String const& key, String const& value, LuaVariadic<LuaValue> const& args) {
  logMap.set(key, formatLua(value, args));
}

Json LuaBindings::UtilityCallbacks::parseJson(String const& str) {
  return Json::parse(str);
}

String LuaBindings::UtilityCallbacks::printJson(Json const& arg, Maybe<int> pretty) {
  return arg.repr(pretty.value());
}

String LuaBindings::UtilityCallbacks::print(LuaValue const& value) {
  return toString(value);
}

LuaValue LuaBindings::UtilityCallbacks::interpolateSinEase(LuaEngine& engine, double offset, LuaValue const& value1, LuaValue const& value2) {
  if (auto floatValue1 = engine.luaMaybeTo<double>(value1)) {
    auto floatValue2 = engine.luaMaybeTo<double>(value2);
    return sinEase(offset, *floatValue1, *floatValue2);
  } else {
    return engine.luaFrom<Vec2F>(sinEase(offset, engine.luaTo<Vec2F>(value1), engine.luaTo<Vec2F>(value2)));
  }
}

String LuaBindings::UtilityCallbacks::replaceTags(String const& str, StringMap<String> const& tags) {
  return str.replaceTags(tags);
}

}
