#include <cstdio>
#include <lua.hpp>
import std;
import star.lua;
import star.json;
import star.string;
import star.string_view;
import star.list;
import star.map;
import star.set;
import star.optional;
import star.format;
import star.output_proxy;
import star.array;
import star.thread;
import star.time;
#include "imgui_lua_bindings.hpp"

import star.ref_ptr;
import star.memory;
import star.lexical_cast;
import star.byte_array;
import star.math_common;
import star.algorithm;
namespace Star {

namespace {
struct LuaStackRestore {
  lua_State* state;
  int top;
  explicit LuaStackRestore(lua_State* state) : state(state), top(lua_gettop(state)) {}
  ~LuaStackRestore() { lua_settop(state, top); }
};

void resolveStringKey(lua_State* state) {
  if (lua_islightuserdata(state, 2)) {
    auto const& key = *static_cast<std::string_view const*>(lua_touserdata(state, 2));
    lua_pushlstring(state, key.data(), key.size());
    lua_replace(state, 2);
  }
}

int protectedTableGet(lua_State* state) {
  resolveStringKey(state);
  lua_gettable(state, 1);
  return 1;
}

int protectedTableRawGet(lua_State* state) {
  resolveStringKey(state);
  lua_rawget(state, 1);
  return 1;
}

int protectedTableSet(lua_State* state) {
  resolveStringKey(state);
  lua_settable(state, 1);
  return 0;
}

int protectedTableRawSet(lua_State* state) {
  resolveStringKey(state);
  lua_rawset(state, 1);
  return 0;
}

// Only called inside an ownership scope which ends before lua_error.
void pushCallbackError(lua_State* state, StarException const& error, void* instructionKey, void* recursionKey) {
  if (error.is<LuaInstructionLimitReached>()) {
    lua_pushlightuserdata(state, instructionKey);
  } else if (error.is<LuaRecursionLimitReached>()) {
    lua_pushlightuserdata(state, recursionKey);
  } else {
    auto diagnostic = printException(ExceptionDiagnostic(error), true);
    lua_pushcfunction(state, [](lua_State* state) {
      auto diagnostic = static_cast<char const*>(lua_touserdata(state, 1));
      luaL_where(state, 2);
      lua_pushstring(state, diagnostic);
      lua_concat(state, 2);
      return 1;
    });
    lua_pushlightuserdata(state, const_cast<char*>(diagnostic.c_str()));
    // Either the diagnostic or a finalizer failure remains as the Lua error
    // payload. Both must reach lua_error, after diagnostic ownership ends.
    (void)lua_pcall(state, 1, 1, 0);
  }
}

std::expected<void, StarException> validateNativeLuaValue(lua_State* state, int index) {
  int type = lua_type(state, index);
  if (type == LUA_TLIGHTUSERDATA)
    return std::unexpected(LuaException("Unsupported type in popLuaValue"));
  if (type == LUA_TUSERDATA) {
    if (!lua_getmetatable(state, index))
      return std::unexpected(LuaException("Userdata in popLuaValue missing metatable"));
    lua_pop(state, 1);
  }
  return {};
}
}

std::expected<std::reference_wrapper<std::ostream>, StarException> operator<<(std::ostream& os, LuaValue const& value) {
  if (std::holds_alternative<LuaBoolean>(value)) {
    os << (std::get<LuaBoolean>(value) ? "true" : "false");
  } else if (std::holds_alternative<LuaInt>(value)) {
    os << std::get<LuaInt>(value);
  } else if (std::holds_alternative<LuaFloat>(value)) {
    os << std::get<LuaFloat>(value);
  } else if (std::holds_alternative<LuaString>(value)) {
    auto text = std::get<LuaString>(value).view().utf8();
    os << text.substr(0, text.find('\0'));
  } else if (std::holds_alternative<LuaTable>(value)) {
    os << "{";
    bool first = true;
    auto result = std::get<LuaTable>(value).iterate([&os, &first](LuaValue const& key, LuaValue const& value) -> std::expected<void, StarException> {
      if (first)
        first = false;
      else
        os << ", ";
      auto keyResult = operator<<(os, key);
      if (!keyResult)
        return std::unexpected(std::move(keyResult.error()));
      os << ": ";
      auto valueResult = operator<<(os, value);
      if (!valueResult)
        return std::unexpected(std::move(valueResult.error()));
      return {};
    });
    if (!result)
      return std::unexpected(std::move(result.error()));
    os << "}";
  } else if (std::holds_alternative<LuaFunction>(value)) {
    os << "<function reg:" << std::get<LuaFunction>(value).handleIndex() << ">";
  } else if (std::holds_alternative<LuaThread>(value)) {
    os << "<thread reg:" << std::get<LuaThread>(value).handleIndex() << ">";
  } else if (std::holds_alternative<LuaUserData>(value)) {
    os << "<userdata reg:" << std::get<LuaUserData>(value).handleIndex() << ">";
  } else {
    os << "nil";
  }
  return std::ref(os);
}

std::expected<String, StarException> printLuaValue(LuaValue const& value) {
  std::ostringstream stream;
  auto result = operator<<(stream, value);
  if (!result)
    return std::unexpected(std::move(result.error()));
  return String(stream.str());
}

std::expected<bool, StarException> LuaTable::contains(std::string_view key) const {
  auto value = engine().tableGet(false, handleIndex(), key);
  if (!value)
    return std::unexpected(std::move(value.error()));
  return *value != LuaNil;
}

std::expected<void, StarException> LuaTable::remove(std::string_view key) const {
  return engine().tableSet(false, handleIndex(), key, LuaNil);
}

std::expected<LuaInt, StarException> LuaTable::length() const {
  return engine().tableLength(false, handleIndex());
}

std::expected<std::optional<LuaTable>, StarException> LuaTable::getMetatable() const {
  return engine().tableGetMetatable(handleIndex());
}

std::expected<void, StarException> LuaTable::setMetatable(LuaTable const& table) const {
  return engine().tableSetMetatable(handleIndex(), table);
}

std::expected<LuaInt, StarException> LuaTable::rawLength() const {
  return engine().tableLength(true, handleIndex());
}

std::expected<void, StarException> LuaCallbacks::copyCallback(String srcName, String dstName) {
  auto callback = m_callbacks.get(srcName);
  if (!callback)
    return std::unexpected(std::move(callback.error()));
  m_callbacks.set(dstName, callback->get());
  return {};
}

bool LuaCallbacks::removeCallback(String name) {
  return m_callbacks.remove(name);
}

std::expected<void, StarException> LuaCallbacks::merge(LuaCallbacks const& callbacks) {
  for (auto const& pair : callbacks.m_callbacks) {
    auto added = m_callbacks.add(pair.first, pair.second);
    if (!added)
      return std::unexpected(LuaException(strf("Failed to merge LuaCallbacks: {}", outputException(ExceptionDiagnostic(added.error()), true))));
  }
  return {};
}

LuaCallbacks LuaCallbacks::withLifetime(std::stop_token lifetime) const {
  LuaCallbacks guarded;
  for (auto const& entry : m_callbacks) {
    guarded.m_callbacks.set(entry.first,
        [callback = entry.second, lifetime](LuaEngine& engine, std::span<LuaValue> args)
            -> std::expected<LuaDetail::LuaFunctionReturn, StarException> {
      if (lifetime.stop_requested())
        return std::unexpected(LuaException("Lua callback scope is retired"));
      return callback(engine, args);
    });
  }
  return guarded;
}

StringMap<LuaDetail::LuaWrappedFunction> const& LuaCallbacks::callbacks() const {
  return m_callbacks;
}

std::expected<bool, StarException> LuaContext::containsPath(String path) const {
  auto value = engine().contextGetPath(handleIndex(), std::move(path));
  if (!value)
    return std::unexpected(std::move(value.error()));
  return *value != LuaNil;
}

std::expected<void, StarException> LuaContext::load(std::span<char const> contents, String const& name) {
  return engine().contextLoad(handleIndex(), contents, name);
}

std::expected<void, StarException> LuaContext::load(String const& contents, String const& name) {
  return load(std::span(contents.utf8()), name);
}

std::expected<void, StarException> LuaContext::load(ByteArray const& contents, String const& name) {
  return load(contents.span(), name);
}

std::expected<void, StarException> LuaContext::setRequireFunction(RequireFunction requireFunction) {
  return engine().setContextRequire(handleIndex(), std::move(requireFunction));
}

std::expected<void, StarException> LuaContext::setCallbacks(String const& tableName, LuaCallbacks const& callbacks) const {
  auto& eng = engine();
  auto present = LuaContext::contains(tableName);
  if (!present)
    return std::unexpected(std::move(present.error()));
  if (*present)
    return {};
  auto callbackTable = eng.createTable();
  if (!callbackTable)
    return std::unexpected(std::move(callbackTable.error()));
  for (auto const& p : callbacks.callbacks()) {
    auto function = eng.createWrappedFunction(p.second);
    if (!function)
      return std::unexpected(std::move(function.error()));
    auto result = callbackTable->set(p.first, std::move(*function));
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return LuaContext::set(tableName, std::move(*callbackTable));
}

std::expected<LuaString, StarException> LuaContext::createString(String const& str) {
  return engine().createString(str);
}

std::expected<LuaString, StarException> LuaContext::createString(std::string_view str) {
  return engine().createString(str);
}

std::expected<LuaTable, StarException> LuaContext::createTable() {
  return engine().createTable();
}

std::expected<LuaValue, StarException> LuaConverter<Json>::from(LuaEngine& engine, Json const& v) {
  if (v.isType(Json::Type::Null))
    return LuaNil;
  if (v.isType(Json::Type::Float)) {
    auto value = v.toDouble();
    if (!value)
      return std::unexpected(std::move(value.error()));
    return LuaFloat(*value);
  }
  if (v.isType(Json::Type::Bool)) {
    auto value = v.toBool();
    if (!value)
      return std::unexpected(std::move(value.error()));
    return *value;
  }
  if (v.isType(Json::Type::Int)) {
    auto value = v.toInt();
    if (!value)
      return std::unexpected(std::move(value.error()));
    return LuaInt(*value);
  }
  if (v.isType(Json::Type::String)) {
    auto value = v.stringPtr();
    if (!value)
      return std::unexpected(std::move(value.error()));
    return engine.createString(**value);
  }
  auto table = LuaDetail::jsonContainerToTable(engine, v);
  if (!table)
    return std::unexpected(std::move(table.error()));
  return std::move(*table);
}

std::expected<std::optional<Json>, StarException> LuaConverter<Json>::to(LuaEngine&, LuaValue const& v) {
  if (v == LuaNil)
    return Json();
  if (std::holds_alternative<LuaBoolean>(v))
    return Json(std::get<LuaBoolean>(v));
  if (std::holds_alternative<LuaInt>(v))
    return Json(std::get<LuaInt>(v));
  if (std::holds_alternative<LuaFloat>(v))
    return Json(std::get<LuaFloat>(v));
  if (std::holds_alternative<LuaString>(v))
    return Json(std::get<LuaString>(v).toString());
  if (std::holds_alternative<LuaTable>(v))
    return LuaDetail::tableToJsonContainer(std::get<LuaTable>(v));
  return std::optional<Json>();
}

std::expected<LuaValue, StarException> LuaConverter<JsonObject>::from(LuaEngine& engine, JsonObject v) {
  return engine.luaFrom<Json>(Json(std::move(v)));
}

std::expected<std::optional<JsonObject>, StarException> LuaConverter<JsonObject>::to(LuaEngine& engine, LuaValue v) {
  auto j = engine.luaTo<Json>(std::move(v));
  if (!j)
    return std::unexpected(std::move(j.error()));
  if (j->type() == Json::Type::Object) {
    auto object = j->toObject();
    if (!object)
      return std::unexpected(std::move(object.error()));
    return std::move(*object);
  }
  if (j->type() == Json::Type::Array) {
    auto list = j->arrayPtr();
    if (!list)
      return std::unexpected(std::move(list.error()));
    if ((*list)->empty())
      return JsonObject();
  }
  return std::optional<JsonObject>();
}

std::expected<LuaValue, StarException> LuaConverter<JsonArray>::from(LuaEngine& engine, JsonArray v) {
  return engine.luaFrom<Json>(Json(std::move(v)));
}

std::expected<std::optional<JsonArray>, StarException> LuaConverter<JsonArray>::to(LuaEngine& engine, LuaValue v) {
  auto j = engine.luaTo<Json>(std::move(v));
  if (!j)
    return std::unexpected(std::move(j.error()));
  if (j->type() == Json::Type::Array) {
    auto array = j->toArray();
    if (!array)
      return std::unexpected(std::move(array.error()));
    return std::move(*array);
  }
  if (j->type() == Json::Type::Object) {
    auto map = j->objectPtr();
    if (!map)
      return std::unexpected(std::move(map.error()));
    if ((*map)->empty())
      return JsonArray();
  }
  return std::optional<JsonArray>();
}

std::expected<LuaEnginePtr, StarException> LuaEngine::create(bool safe) {
  LuaEnginePtr self(new LuaEngine);

  self->m_state = lua_newstate(allocate, nullptr, luaL_makeseed(nullptr));

  self->m_scriptDefaultEnvRegistryId = LUA_NOREF;
  self->m_wrappedFunctionMetatableRegistryId = LUA_NOREF;
  self->m_requireFunctionMetatableRegistryId = LUA_NOREF;

  self->m_instructionLimit = 0;
  self->m_profilingEnabled = false;
  self->m_instructionMeasureInterval = 1000;
  self->m_instructionCount = 0;
  self->m_recursionLevel = 0;
  self->m_recursionLimit = 0;

  if (!self->m_state)
    return std::unexpected(LuaException("Failed to initialize Lua"));

  lua_checkstack(self->m_state, 5);
  *reinterpret_cast<LuaEngine**>(lua_getextraspace(self->m_state)) = self.get();
  lua_setwarnf(self->m_state, [](void* userdata, char const* message, int continued) {
    auto& self = *static_cast<LuaEngine*>(userdata);
    self.m_warningMessage += message;
    if (!continued) {
      std::fprintf(stderr, "Lua warning: %s\n", self.m_warningMessage.c_str());
      self.m_warningMessage.clear();
    }
  }, self.get());
  auto initialize = [](lua_State* state) {
    auto& self = luaEngine(state);
    bool safe = lua_toboolean(state, 1);
    lua_settop(state, 0);

  // Create the common message handler function for pcall to print a better
  // message with a traceback
  lua_pushcfunction(state, [](lua_State* state) {
      // Don't modify the error if it is one of the special limit errrors
      if (lua_islightuserdata(state, 1)) {
        void* error = lua_touserdata(state, -1);
        if (error == &s_luaInstructionLimitExceptionKey || error == &s_luaRecursionLimitExceptionKey)
          return 1;
      }

      luaL_traceback(state, state, lua_tostring(state, 1), 0);
      lua_remove(state, 1);
      return 1;
    });
  self.m_pcallTracebackMessageHandlerRegistryId = luaL_ref(state, LUA_REGISTRYINDEX);

  // Create the common metatable for wrapped functions
  lua_newtable(state);
  lua_pushcfunction(state, [](lua_State* state) {
      auto& func = *static_cast<LuaDetail::LuaWrappedFunction*>(lua_touserdata(state, 1));
      std::destroy_at(&func);
      return 0;
    });
  LuaDetail::rawSetField(state, -2, "__gc");
  lua_pushboolean(state, 0);
  LuaDetail::rawSetField(state, -2, "__metatable");
  self.m_wrappedFunctionMetatableRegistryId = luaL_ref(state, LUA_REGISTRYINDEX);

  // Create the common metatable for require functions
  lua_newtable(state);
  lua_pushcfunction(state, [](lua_State* state) {
      auto& func = *static_cast<LuaContext::RequireFunction*>(lua_touserdata(state, 1));
      std::destroy_at(&func);
      return 0;
    });
  LuaDetail::rawSetField(state, -2, "__gc");
  lua_pushboolean(state, 0);
  LuaDetail::rawSetField(state, -2, "__metatable");
  self.m_requireFunctionMetatableRegistryId = luaL_ref(state, LUA_REGISTRYINDEX);

  // Load all base libraries and prune them of unsafe functions

  luaL_requiref(state, "_ENV", luaopen_base, true);
  if (safe) {
    StringSet baseWhitelist = {
        "assert",
        "error",
        "getmetatable",
        "ipairs",
        "next",
        "pairs",
        "pcall",
        "print",
        "rawequal",
        "rawget",
        "rawlen",
        "rawset",
        "select",
        "setmetatable",
        "tonumber",
        "tostring",
        "type",
        "unpack",
        "_VERSION",
        "xpcall"};

    lua_pushnil(state);
    while (lua_next(state, -2) != 0) {
      lua_pop(state, 1);
      String key(std::string_view(lua_tostring(state, -1)));

      if (!baseWhitelist.contains(key)) {
        lua_pushvalue(state, -1);
        lua_pushnil(state);
        lua_rawset(state, -4);
      }
    }
  }
  lua_pop(state, 1);

  luaL_requiref(state, "os", luaopen_os, true);
  if (safe) {
    StringSet osWhitelist = {"clock", "difftime", "time", "date"};

    lua_pushnil(state);
    while (lua_next(state, -2) != 0) {
      lua_pop(state, 1);
      String key(std::string_view(lua_tostring(state, -1)));

      if (!osWhitelist.contains(key)) {
        lua_pushvalue(state, -1);
        lua_pushnil(state);
        lua_rawset(state, -4);
      }
    }
  }
  lua_pop(state, 1);

  // loads a lua base library, leaves it at the top of the stack
  // Only terminated literal library names enter this native Lua adapter.
  auto loadBaseLibrary = [](lua_State* state, std::string_view modname, lua_CFunction openf) {
    luaL_requiref(state, modname.data(), openf, true);

    // set __metatable metamethod to false
    // otherwise scripts can access and mutate the metatable, allowing passing values
    // between script contexts, breaking the sandbox
    lua_newtable(state);
    lua_pushliteral(state, "__metatable");
    lua_pushboolean(state, 0);
    lua_rawset(state, -3);
    lua_setmetatable(state, -2);
  };

  loadBaseLibrary(state, "coroutine", luaopen_coroutine);
  // replace coroutine resume with one that appends tracebacks
  lua_pushliteral(state, "resume");
  lua_pushcfunction(state, &LuaEngine::coresumeWithTraceback);
  lua_rawset(state, -3);

  loadBaseLibrary(state, "math", luaopen_math);
  loadBaseLibrary(state, "string", luaopen_string);
  loadBaseLibrary(state, "table", luaopen_table);
  loadBaseLibrary(state, "utf8", luaopen_utf8);
  lua_pop(state, 5);

  if (!safe) {
    loadBaseLibrary(state, "io", luaopen_io);
    loadBaseLibrary(state, "package", luaopen_package);
    loadBaseLibrary(state, "debug", luaopen_debug);
    lua_pop(state, 3);
  }

  // Make a shallow copy of the default script environment and save it for
  // resetting the global state.
  lua_rawgeti(state, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
  lua_newtable(state);
  LuaDetail::shallowCopy(state, -2, -1);
  self.m_scriptDefaultEnvRegistryId = luaL_ref(state, LUA_REGISTRYINDEX);
  lua_pop(state, 1);
  // Use Lua's own freelist layout rather than a version-specific registry slot.
  lua_pushboolean(state, 0);
  int temporary = luaL_ref(state, LUA_REGISTRYINDEX);
  luaL_unref(state, LUA_REGISTRYINDEX, temporary);
    return 0;
  };
  lua_pushcfunction(self->m_state, initialize);
  lua_pushboolean(self->m_state, safe);
  auto initialized = self->handleError(self->m_state, lua_pcall(self->m_state, 1, 0, 0));
  if (!initialized)
    return std::unexpected(std::move(initialized.error()));
  auto install = [&self](std::string_view name, std::expected<LuaFunction, StarException> function) -> std::expected<void, StarException> {
    if (!function)
      return std::unexpected(std::move(function.error()));
    return self->setGlobal(name, std::move(*function));
  };

  auto jarray = install("jarray", self->createFunction(&LuaDetail::jarray));
  if (!jarray)
    return std::unexpected(std::move(jarray.error()));
  auto jobject = install("jobject", self->createFunction(&LuaDetail::jobject));
  if (!jobject)
    return std::unexpected(std::move(jobject.error()));
  auto jremove = install("jremove", self->createFunction(&LuaDetail::jcontRemove));
  if (!jremove)
    return std::unexpected(std::move(jremove.error()));
  auto jsize = install("jsize", self->createFunction(&LuaDetail::jcontSize));
  if (!jsize)
    return std::unexpected(std::move(jsize.error()));
  auto jresize = install("jresize", self->createFunction(&LuaDetail::jcontResize));
  if (!jresize)
    return std::unexpected(std::move(jresize.error()));
  auto sharedTable = self->createTable();
  if (!sharedTable)
    return std::unexpected(std::move(sharedTable.error()));
  auto shared = self->setGlobal("shared", std::move(*sharedTable));
  if (!shared)
    return std::unexpected(std::move(shared.error()));
  return self;
}

LuaEngine::~LuaEngine() {
  if (!m_state)
    return;
  // If we've had a stack space leak, this will not be zero
  if constexpr (Star::DebugEnabled) {
    Star::starAssertFailed((lua_gettop(m_state) == 0));
  }
  lua_close(m_state);
}

void LuaEngine::setInstructionLimit(std::uint64_t instructionLimit) {
  if (instructionLimit != m_instructionLimit) {
    m_instructionLimit = instructionLimit;
    updateCountHook();
  }
}

std::uint64_t LuaEngine::instructionLimit() const {
  return m_instructionLimit;
}

void LuaEngine::setProfilingEnabled(bool profilingEnabled) {
  if (profilingEnabled != m_profilingEnabled) {
    m_profilingEnabled = profilingEnabled;
    m_profileEntries.clear();
    updateCountHook();
  }
}

bool LuaEngine::profilingEnabled() const {
  return m_profilingEnabled;
}

List<LuaProfileEntry> LuaEngine::getProfile() {
  List<LuaProfileEntry> profileEntries;
  for (auto const& p : m_profileEntries) {
    profileEntries.append(*p.second);
  }

  return profileEntries;
}

void LuaEngine::setInstructionMeasureInterval(unsigned measureInterval) {
  if (measureInterval != m_instructionMeasureInterval) {
    m_instructionMeasureInterval = measureInterval;
    updateCountHook();
  }
}

unsigned LuaEngine::instructionMeasureInterval() const {
  return m_instructionMeasureInterval;
}

void LuaEngine::setRecursionLimit(unsigned recursionLimit) {
  m_recursionLimit = recursionLimit;
}

unsigned LuaEngine::recursionLimit() const {
  return m_recursionLimit;
}

std::expected<ByteArray, StarException> LuaEngine::compile(std::span<char const> contents, String const& name) {
  LuaStackRestore stack(m_state);
  if (!lua_checkstack(m_state, 2))
    return std::unexpected(LuaException("Exhausted the Lua stack while compiling a script"));
  auto loaded = handleError(m_state, luaL_loadbuffer(m_state, contents.data(), contents.size(), name.empty() ? nullptr : name.utf8().c_str()));
  if (!loaded)
    return std::unexpected(std::move(loaded.error()));

  ByteArray compiledScript;
  lua_Writer writer = [](lua_State*, void const* data, std::size_t size, void* byteArrayPtr) -> int {
    static_cast<ByteArray*>(byteArrayPtr)->append(std::span(static_cast<char const*>(data), size));
    return 0;
  };
  if (lua_dump(m_state, writer, &compiledScript, false) != 0)
    return std::unexpected(LuaException("Failed to dump compiled Lua script"));
  lua_pop(m_state, 1);

  return compiledScript;
}

std::expected<ByteArray, StarException> LuaEngine::compile(String const& contents, String const& name) {
  return compile(std::span(contents.utf8()), name);
}

std::expected<ByteArray, StarException> LuaEngine::compile(ByteArray const& contents, String const& name) {
  return compile(contents.span(), name);
}

lua_Debug const& LuaEngine::debugInfo(int level, String const& what) {
  lua_Debug& debug = m_debugInfo = lua_Debug();
  lua_getstack(m_state, level, &debug);
  lua_getinfo(m_state, what.utf8().c_str(), &debug);
  return debug;
}

std::expected<LuaString, StarException> LuaEngine::createString(std::string_view str) {
  LuaStackRestore stack(m_state);
  if (!lua_checkstack(m_state, 3))
    return std::unexpected(LuaException("Exhausted the Lua stack while creating a string"));
  lua_pushcfunction(m_state, [](lua_State* state) {
    auto const& input = *static_cast<std::string_view const*>(lua_touserdata(state, 1));
    lua_pushlstring(state, input.data(), input.size());
    return 1;
  });
  lua_pushlightuserdata(m_state, &str);
  auto called = handleError(m_state, pcallWithTraceback(m_state, 1, 1));
  if (!called)
    return std::unexpected(std::move(called.error()));
  auto handle = popHandle(m_state);
  if (!handle)
    return std::unexpected(std::move(handle.error()));
  return LuaString(LuaDetail::LuaHandle(RefPtr<LuaEngine>(this), *handle));
}

std::expected<LuaString, StarException> LuaEngine::createString(String const& str) {
  return createString(str.utf8());
}


std::expected<LuaTable, StarException> LuaEngine::createTable(int narr, int nrec) {
  LuaStackRestore stack(m_state);
  if (!lua_checkstack(m_state, 4))
    return std::unexpected(LuaException("Exhausted the Lua stack while creating a table"));
  lua_pushcfunction(m_state, [](lua_State* state) {
    lua_createtable(state, (int)lua_tointeger(state, 1), (int)lua_tointeger(state, 2));
    return 1;
  });
  lua_pushinteger(m_state, narr);
  lua_pushinteger(m_state, nrec);
  auto called = handleError(m_state, pcallWithTraceback(m_state, 2, 1));
  if (!called)
    return std::unexpected(std::move(called.error()));
  auto handle = popHandle(m_state);
  if (!handle)
    return std::unexpected(std::move(handle.error()));
  return LuaTable(LuaDetail::LuaHandle(RefPtr<LuaEngine>(this), *handle));
}

std::expected<LuaThread, StarException> LuaEngine::createThread() {
  LuaStackRestore stack(m_state);
  if (!lua_checkstack(m_state, 2))
    return std::unexpected(LuaException("Exhausted the Lua stack while creating a thread"));
  lua_pushcfunction(m_state, [](lua_State* state) {
    lua_newthread(state);
    return 1;
  });
  auto called = handleError(m_state, pcallWithTraceback(m_state, 0, 1));
  if (!called)
    return std::unexpected(std::move(called.error()));
  auto handle = popHandle(m_state);
  if (!handle)
    return std::unexpected(std::move(handle.error()));
  return LuaThread(LuaDetail::LuaHandle(RefPtr<LuaEngine>(this), *handle));
}

std::expected<void*, StarException> LuaEngine::allocateUserData(std::size_t size) {
  LuaStackRestore stack(m_state);
  if (!lua_checkstack(m_state, 3))
    return std::unexpected(LuaException("Exhausted the Lua stack while allocating userdata"));
  lua_pushcfunction(m_state, [](lua_State* state) {
    auto size = static_cast<std::size_t*>(lua_touserdata(state, 1));
    lua_newuserdatauv(state, *size, 0);
    return 1;
  });
  lua_pushlightuserdata(m_state, &size);
  auto allocated = handleError(m_state, pcallWithTraceback(m_state, 1, 1));
  if (!allocated)
    return std::unexpected(std::move(allocated.error()));
  auto data = lua_touserdata(m_state, -1);
  ++stack.top;
  return data;
}


std::expected<void, StarException> LuaEngine::threadPushFunction(int threadIndex, int functionIndex) {
  if (!lua_checkstack(m_state, 1))
    return std::unexpected(LuaException("Exhausted the Lua stack while accessing a thread"));
  pushHandle(m_state, threadIndex);
  lua_State* thread = lua_tothread(m_state, -1);
  lua_pop(m_state, 1);

  int status = lua_status(thread);
  lua_Debug ar;
  if (status != LUA_OK || lua_getstack(thread, 0, &ar) > 0 || lua_gettop(thread) > 0)
    return std::unexpected(LuaException(strf("Cannot push function to active or errored thread with status {}", status)));
  if (!lua_checkstack(thread, 1))
    return std::unexpected(LuaException("Exhausted the Lua thread stack"));

  pushHandle(thread, functionIndex);
  return {};
}

LuaThread::Status LuaEngine::threadStatus(int handleIndex) {
  pushHandle(m_state, handleIndex);
  lua_State* thread = lua_tothread(m_state, -1);
  lua_pop(m_state, 1);

  int status = lua_status(thread);
  if (status != LUA_OK && status != LUA_YIELD)
    return LuaThread::Status::Error;

  lua_Debug ar;
  if (status == LUA_YIELD || lua_getstack(thread, 0, &ar) > 0 || lua_gettop(thread) > 0)
    return LuaThread::Status::Active;

  return LuaThread::Status::Dead;
}

std::expected<LuaContext, StarException> LuaEngine::createContext() {
  LuaStackRestore stack(m_state);
  if (!lua_checkstack(m_state, 3))
    return std::unexpected(LuaException("Exhausted the Lua stack while creating a context"));
  lua_pushcfunction(m_state, [](lua_State* state) {
    lua_newtable(state);
    LuaDetail::shallowCopy(state, 1, -1);
    return 1;
  });
  lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_scriptDefaultEnvRegistryId);
  auto created = handleError(m_state, pcallWithTraceback(m_state, 1, 1));
  if (!created)
    return std::unexpected(std::move(created.error()));
  auto handle = popHandle(m_state);
  if (!handle)
    return std::unexpected(std::move(handle.error()));
  auto context = LuaContext(LuaDetail::LuaHandle(RefPtr<LuaEngine>(this), *handle));
  // Add loadstring
  auto handleIndex = context.handleIndex();
  auto loadstring = createFunction([&engine = *this, handleIndex](String const& source, std::optional<String> const& name, std::optional<LuaTable> const& env) -> std::expected<LuaFunction, StarException> {
    String functionName = name ? strf("loadstring: {}", *name) : "loadstring";
    return engine.createFunctionFromSource(env ? env->handleIndex() : handleIndex, std::span(source.utf8()), functionName);
  });
  if (!loadstring)
    return std::unexpected(std::move(loadstring.error()));
  auto installed = context.set("loadstring", std::move(*loadstring));
  if (!installed)
    return std::unexpected(std::move(installed.error()));

  // Then set that environment as the new context environment in the registry.
  return context;
}

void LuaEngine::releaseHandles(lua_State* state) {
  if (m_handleFree.empty() || !lua_checkstack(state, 2))
    return;
  // Existing slots are overwritten without allocating or running finalizers.
  // Retain the queue capacity instead of allocating another batch next time.
  for (int handle : m_handleFree)
    luaL_unref(state, LUA_REGISTRYINDEX, handle);
  m_handleFree.clear();
}

std::expected<void, StarException> LuaEngine::collectGarbage(std::optional<unsigned> steps) {
  LuaStackRestore stack(m_state);
  if (!lua_checkstack(m_state, 4))
    return std::unexpected(LuaException("Exhausted the Lua stack while collecting garbage"));
  struct Input {
    bool incremental;
    std::size_t bytes;
  } input{steps.has_value(), steps ? std::size_t(*steps) * 1024 : 0};
  bool running = lua_gc(m_state, LUA_GCISRUNNING);
  lua_pushcfunction(m_state, [](lua_State* state) {
    auto const& input = *static_cast<Input const*>(lua_touserdata(state, 1));
    if (input.incremental)
      lua_gc(state, LUA_GCSTEP, input.bytes);
    else
      lua_gc(state, LUA_GCCOLLECT);
    return 0;
  });
  lua_pushlightuserdata(m_state, &input);
  int status = pcallWithTraceback(m_state, 1, 0);
  if (bool(lua_gc(m_state, LUA_GCISRUNNING)) != running)
    lua_gc(m_state, running ? LUA_GCRESTART : LUA_GCSTOP);
  return handleError(m_state, status);
}

void LuaEngine::setAutoGarbageCollection(bool autoGarbageColleciton) {
  lua_gc(m_state, autoGarbageColleciton ? LUA_GCRESTART : LUA_GCSTOP);
}

void LuaEngine::tuneAutoGarbageCollection(float pause, float stepMultiplier) {
  lua_gc(m_state, LUA_GCPARAM, LUA_GCPPAUSE, static_cast<int>(std::round(pause * 100)));
  lua_gc(m_state, LUA_GCPARAM, LUA_GCPSTEPMUL, static_cast<int>(std::round(stepMultiplier * 100)));
}

std::size_t LuaEngine::memoryUsage() const {
  return (std::size_t)lua_gc(m_state, LUA_GCCOUNT, 0) * 1024 + lua_gc(m_state, LUA_GCCOUNTB, 0);
}

std::expected<void, StarException> LuaEngine::addImGui() {
  LuaStackRestore stack(m_state);
  if (!lua_checkstack(m_state, 3))
    return std::unexpected(LuaException("Exhausted the Lua stack while adding ImGui bindings"));
  lua_pushcfunction(m_state, [](lua_State* state) {
    pushImguiBindings(state);
    lua_setfield(state, 1, "imgui");
    return 0;
  });
  lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_scriptDefaultEnvRegistryId);
  return handleError(m_state, pcallWithTraceback(m_state, 1, 0));
}

LuaEngine& LuaEngine::luaEngine(lua_State* state) {
  return **reinterpret_cast<LuaEngine**>(lua_getextraspace(state));
}

void LuaEngine::countHook(lua_State* state, lua_Debug* ar) {
  if constexpr (Star::DebugEnabled) {
    Star::starAssertFailed((ar->event == LUA_HOOKCOUNT));
  }
  lua_checkstack(state, 4);

  auto& self = luaEngine(state);

  // If the instruction count is 0, that means in this sequence of calls,
  // we have not hit a debug hook yet.  Since we don't know the state of
  // the internal lua instruction counter at the start, we don't know how
  // many instructions have been executed, only that it is >= 1 and <=
  // m_instructionMeasureInterval, so we pick the low estimate.
  if (self.m_instructionCount == 0)
    self.m_instructionCount = 1;
  else
    self.m_instructionCount += self.m_instructionMeasureInterval;

  if (self.m_instructionLimit != 0 && self.m_instructionCount > self.m_instructionLimit) {
    lua_pushlightuserdata(state, &s_luaInstructionLimitExceptionKey);
    lua_error(state);
  }

  if (self.m_profilingEnabled) {
    // find bottom of the stack
    // ar will contain the stack info from the last call that returns 1
    int stackLevel = -1;
    while (lua_getstack(state, stackLevel + 1, ar) == 1)
      stackLevel++;

    std::shared_ptr<LuaProfileEntry> parentEntry = nullptr;
    while (true) {
      // Get the 'n' name info and 'S' source info
      if (lua_getinfo(state, "nS", ar) == 0)
        break;

      auto key = std::make_tuple(String(ar->short_src), (unsigned)ar->linedefined);
      auto& entryMap = parentEntry ? parentEntry->calls : self.m_profileEntries;
      if (!entryMap.contains(key)) {
        auto e = std::make_shared<LuaProfileEntry>();
        e->source = std::string_view(ar->short_src);
        e->sourceLine = ar->linedefined;
        entryMap.set(key, e);
      }
      auto entry = entryMap.find(key)->second;

      // metadata
      if (!entry->name && ar->name)
        entry->name = String(std::string_view(ar->name));
      if (!entry->nameScope && String() != std::string_view(ar->namewhat))
        entry->nameScope = String(std::string_view(ar->namewhat));

      // Only add timeTaken to the self time for the function we are actually
      // in, not parent functions
      if (stackLevel == 0) {
        entry->totalTime += 1;
        entry->selfTime += 1;
      } else {
        entry->totalTime += 1;
      }

      parentEntry = entry;
      if (lua_getstack(state, --stackLevel, ar) == 0)
        break;
    }
  }
}

void* LuaEngine::allocate(void*, void* ptr, std::size_t oldSize, std::size_t newSize) {
  if (newSize == 0) {
    Star::free(ptr, oldSize);
    return nullptr;
  } else {
    return Star::realloc(ptr, newSize);
  }
}

std::expected<void, StarException> LuaEngine::handleError(lua_State* state, int res) {
  if (res == LUA_OK)
    return {};
  if (lua_islightuserdata(state, -1)) {
    void* error = lua_touserdata(state, -1);
    if (error == &s_luaInstructionLimitExceptionKey) {
      lua_pop(state, 1);
      return std::unexpected(LuaInstructionLimitReached());
    }
    if (error == &s_luaRecursionLimitExceptionKey) {
      lua_pop(state, 1);
      return std::unexpected(LuaRecursionLimitReached());
    }
  }
  if (lua_type(state, -1) == LUA_TNUMBER) {
    // Numeric Lua error objects are string-coerced just as before, but the
    // coercion may collect garbage and run a user finalizer.
    lua_pushcfunction(state, [](lua_State* state) {
      lua_tolstring(state, 1, nullptr);
      lua_pushvalue(state, 1);
      return 1;
    });
    lua_insert(state, -2);
    int converted = lua_pcall(state, 1, 1, 0);
    if (converted != LUA_OK)
      res = converted;
  }
  String error = lua_type(state, -1) == LUA_TSTRING
      ? strf("Error code {}, {}", res, lua_tostring(state, -1))
      : strf("Error code {}, <unknown error>", res);
  lua_pop(state, 1);
  if (error.utf8().ends_with("<eof>"))
    return std::unexpected(LuaIncompleteStatementException(error.takeUtf8()));
  return std::unexpected(LuaException(error.takeUtf8()));
}

int LuaEngine::pcallWithTraceback(lua_State* state, int nargs, int nresults) {
  releaseHandles(state);
  int msghPosition = lua_gettop(state) - nargs;
  lua_rawgeti(state, LUA_REGISTRYINDEX, m_pcallTracebackMessageHandlerRegistryId);
  lua_insert(state, msghPosition);
  int ret = lua_pcall(state, nargs, nresults, msghPosition);
  lua_remove(state, msghPosition);
  releaseHandles(state);
  return ret;
}

int LuaEngine::coresumeWithTraceback(lua_State* state) {
  lua_State* co = lua_tothread(state, 1);
  if (!co) {
    lua_checkstack(state, 2);
    lua_pushboolean(state, 0);
    lua_pushliteral(state, "bad argument #1 to 'resume' (thread expected)");
    return 2;
  }

  int args = lua_gettop(state) - 1;
  lua_checkstack(co, args);
  if (lua_status(co) == LUA_OK && lua_gettop(co) == 0) {
    lua_checkstack(state, 2);
    lua_pushboolean(state, 0);
    lua_pushliteral(state, "cannot resume dead coroutine");
    return 2;
  }

  lua_xmove(state, co, args);
  int res = 0;
  int status = lua_resume(co, state, args, &res);
  if (status == LUA_OK || status == LUA_YIELD) {
    lua_checkstack(state, res + 1);
    lua_pushboolean(state, 1);
    lua_xmove(co, state, res);
    return res + 1;
  } else {
    lua_checkstack(state, 2);
    lua_pushboolean(state, 0);
    propagateErrorWithTraceback(co, state);
    return 2;
  }
}

void LuaEngine::propagateErrorWithTraceback(lua_State* from, lua_State* to) {
  lua_checkstack(to, 4);
  lua_xmove(from, to, 1);
  lua_pushcfunction(to, [](lua_State* state) {
    auto from = static_cast<lua_State*>(lua_touserdata(state, 2));
    if (auto error = lua_tostring(state, 1))
      luaL_traceback(state, from, error, 0);
    else
      lua_pushvalue(state, 1);
    return 1;
  });
  lua_insert(to, -2);
  lua_pushlightuserdata(to, from);
  // A traceback allocation failure is itself the outgoing error object.
  (void)lua_pcall(to, 2, 1, 0);
}


std::size_t LuaEngine::stringLength(int handleIndex) {
  pushHandle(m_state, handleIndex);
  std::size_t len = 0;
  lua_tolstring(m_state, -1, &len);
  lua_pop(m_state, 1);
  return len;
}

String LuaEngine::string(int handleIndex) {
  pushHandle(m_state, handleIndex);
  std::size_t length = 0;
  auto data = lua_tolstring(m_state, -1, &length);
  lua_pop(m_state, 1);
  return String(std::string_view(data, length));
}

StringView LuaEngine::stringView(int handleIndex) {
  pushHandle(m_state, handleIndex);
  std::size_t length = 0;
  auto data = lua_tolstring(m_state, -1, &length);
  lua_pop(m_state, 1);
  return StringView(std::string_view(data, length));
}

std::expected<LuaValue, StarException> LuaEngine::tableGet(bool raw, int handleIndex, LuaValue const& key) {
  LuaStackRestore stack(m_state);
  if (!lua_checkstack(m_state, 4))
    return std::unexpected(LuaException("Exhausted the Lua stack while reading a table"));
  lua_pushcfunction(m_state, raw ? protectedTableRawGet : protectedTableGet);
  pushHandle(m_state, handleIndex);
  auto pushed = pushLuaValue(m_state, key);
  if (!pushed)
    return std::unexpected(std::move(pushed.error()));
  auto called = handleError(m_state, pcallWithTraceback(m_state, 2, 1));
  if (!called)
    return std::unexpected(std::move(called.error()));
  return popLuaValue(m_state);
}

std::expected<LuaValue, StarException> LuaEngine::tableGet(bool raw, int handleIndex, std::string_view key) {
  LuaStackRestore stack(m_state);
  if (!lua_checkstack(m_state, 4))
    return std::unexpected(LuaException("Exhausted the Lua stack while reading a table"));
  lua_pushcfunction(m_state, raw ? protectedTableRawGet : protectedTableGet);
  pushHandle(m_state, handleIndex);
  lua_pushlightuserdata(m_state, &key);
  auto called = handleError(m_state, pcallWithTraceback(m_state, 2, 1));
  if (!called)
    return std::unexpected(std::move(called.error()));
  return popLuaValue(m_state);
}

std::expected<void, StarException> LuaEngine::tableSet(bool raw, int handleIndex, LuaValue const& key, LuaValue const& value) {
  LuaStackRestore stack(m_state);
  if (!lua_checkstack(m_state, 5))
    return std::unexpected(LuaException("Exhausted the Lua stack while writing a table"));
  lua_pushcfunction(m_state, raw ? protectedTableRawSet : protectedTableSet);
  pushHandle(m_state, handleIndex);
  auto pushedKey = pushLuaValue(m_state, key);
  if (!pushedKey)
    return std::unexpected(std::move(pushedKey.error()));
  auto pushedValue = pushLuaValue(m_state, value);
  if (!pushedValue)
    return std::unexpected(std::move(pushedValue.error()));
  return handleError(m_state, pcallWithTraceback(m_state, 3, 0));
}

std::expected<void, StarException> LuaEngine::tableSet(bool raw, int handleIndex, std::string_view key, LuaValue const& value) {
  LuaStackRestore stack(m_state);
  if (!lua_checkstack(m_state, 5))
    return std::unexpected(LuaException("Exhausted the Lua stack while writing a table"));
  lua_pushcfunction(m_state, raw ? protectedTableRawSet : protectedTableSet);
  pushHandle(m_state, handleIndex);
  lua_pushlightuserdata(m_state, &key);
  auto pushedValue = pushLuaValue(m_state, value);
  if (!pushedValue)
    return std::unexpected(std::move(pushedValue.error()));
  return handleError(m_state, pcallWithTraceback(m_state, 3, 0));
}

std::expected<LuaInt, StarException> LuaEngine::tableLength(bool raw, int handleIndex) {
  LuaStackRestore stack(m_state);
  if (!lua_checkstack(m_state, 3))
    return std::unexpected(LuaException("Exhausted the Lua stack while measuring a table"));
  if (raw) {
    pushHandle(m_state, handleIndex);
    return LuaInt(lua_rawlen(m_state, -1));
  }
  lua_pushcfunction(m_state, [](lua_State* state) {
    lua_len(state, 1);
    return 1;
  });
  pushHandle(m_state, handleIndex);
  auto called = handleError(m_state, pcallWithTraceback(m_state, 1, 1));
  if (!called)
    return std::unexpected(std::move(called.error()));
  return LuaInt(lua_tointeger(m_state, -1));
}

std::expected<void, StarException> LuaEngine::tableIterate(int handleIndex, std::function<std::expected<bool, StarException>(LuaValue, LuaValue)> iterator) {
  LuaStackRestore stack(m_state);
  if (!iterator)
    return std::unexpected(LuaException("Cannot invoke an empty Lua table iterator"));
  if (!lua_checkstack(m_state, 6))
    return std::unexpected(LuaException("Exhausted the Lua stack while iterating a table"));
  pushHandle(m_state, handleIndex);
  lua_pushnil(m_state);
  while (true) {
    lua_pushcfunction(m_state, [](lua_State* state) {
      return lua_next(state, 1) ? 2 : 0;
    });
    lua_pushvalue(m_state, stack.top + 1);
    lua_pushvalue(m_state, stack.top + 2);
    auto called = handleError(m_state, pcallWithTraceback(m_state, 2, LUA_MULTRET));
    if (!called)
      return std::unexpected(std::move(called.error()));
    lua_remove(m_state, stack.top + 2);
    if (lua_gettop(m_state) == stack.top + 1)
      return {};
    lua_pushvalue(m_state, -2);
    auto key = popLuaValue(m_state);
    if (!key)
      return std::unexpected(std::move(key.error()));
    auto value = popLuaValue(m_state);
    if (!value)
      return std::unexpected(std::move(value.error()));
    auto cont = iterator(std::move(*key), std::move(*value));
    if (!cont)
      return std::unexpected(std::move(cont.error()));
    if (!*cont)
      return {};
  }
}

std::expected<std::optional<LuaTable>, StarException> LuaEngine::tableGetMetatable(int handleIndex) {
  LuaStackRestore stack(m_state);
  if (!lua_checkstack(m_state, 2))
    return std::unexpected(LuaException("Exhausted the Lua stack while getting a metatable"));
  pushHandle(m_state, handleIndex);
  if (lua_getmetatable(m_state, -1) == 0)
    return std::optional<LuaTable>();
  auto handle = popHandle(m_state);
  if (!handle)
    return std::unexpected(std::move(handle.error()));
  return LuaTable(LuaDetail::LuaHandle(RefPtr<LuaEngine>(this), *handle));
}

std::expected<void, StarException> LuaEngine::tableSetMetatable(int handleIndex, LuaTable const& table) {
  if (&table.engine() != this)
    return std::unexpected(LuaException("lua reference values cannot be shared between engines"));
  if (!lua_checkstack(m_state, 2))
    return std::unexpected(LuaException("Exhausted the Lua stack while setting a metatable"));
  pushHandle(m_state, handleIndex);
  pushHandle(m_state, table.handleIndex());
  lua_setmetatable(m_state, -2);
  lua_pop(m_state, 1);
  return {};
}

std::expected<void, StarException> LuaEngine::setContextRequire(int handleIndex, LuaContext::RequireFunction requireFunction) {
  LuaStackRestore stack(m_state);
  if (!lua_checkstack(m_state, 4))
    return std::unexpected(LuaException("Exhausted the Lua stack while installing require"));

  auto invokeRequire = [](lua_State* state) {
    {
      auto invoke = [state]() -> std::expected<void, StarException> {
        if (!lua_checkstack(state, 2))
          return std::unexpected(LuaException("Exhausted the Lua stack while invoking require"));
        auto& require = *static_cast<LuaContext::RequireFunction*>(lua_touserdata(state, lua_upvalueindex(1)));
        auto& self = luaEngine(state);
        if (lua_gettop(state) == 0)
          return std::unexpected(LuaException("require expects a module name"));
        auto moduleValue = self.popLuaValue(state);
        if (!moduleValue)
          return std::unexpected(std::move(moduleValue.error()));
        auto moduleName = self.luaTo<LuaString>(std::move(*moduleValue));
        if (!moduleName)
          return std::unexpected(std::move(moduleName.error()));
        lua_pushvalue(state, lua_upvalueindex(2));
        auto handle = self.popHandle(state);
        if (!handle)
          return std::unexpected(std::move(handle.error()));
        LuaContext context(LuaDetail::LuaHandle(RefPtr<LuaEngine>(&self), *handle));
        if (!require)
          return std::unexpected(LuaException("Cannot invoke an empty Lua require function"));
        return require(context, *moduleName);
      };
      auto result = invoke();
      if (result)
        return 0;
      lua_settop(state, 0);
      pushCallbackError(state, result.error(), &s_luaInstructionLimitExceptionKey, &s_luaRecursionLimitExceptionKey);
    }
    // Every context, value and diagnostic has been released before longjmp.
    return lua_error(state);
  };

  struct Input {
    std::reference_wrapper<LuaContext::RequireFunction> function;
    int metatable;
    lua_CFunction invoke;
  };
  Input input{std::ref(requireFunction), m_requireFunctionMetatableRegistryId, invokeRequire};
  lua_pushcfunction(m_state, [](lua_State* state) {
    auto& input = *static_cast<Input*>(lua_touserdata(state, 2));
    auto function = (LuaContext::RequireFunction*)lua_newuserdatauv(state, sizeof(LuaContext::RequireFunction), 0);
    new (function) LuaContext::RequireFunction(std::move(input.function.get()));
    lua_rawgeti(state, LUA_REGISTRYINDEX, input.metatable);
    lua_setmetatable(state, -2);
    lua_pushvalue(state, 1);
    lua_pushcclosure(state, input.invoke, 2);
    LuaDetail::rawSetField(state, 1, "require");
    return 0;
  });
  pushHandle(m_state, handleIndex);
  lua_pushlightuserdata(m_state, &input);
  return handleError(m_state, pcallWithTraceback(m_state, 2, 0));
}

std::expected<void, StarException> LuaEngine::contextLoad(int handleIndex, std::span<char const> contents, String const& name) {
  LuaStackRestore stack(m_state);
  if (!lua_checkstack(m_state, 2))
    return std::unexpected(LuaException("Exhausted the Lua stack while loading a script"));
  auto loaded = handleError(m_state, luaL_loadbuffer(m_state, contents.data(), contents.size(), name.empty() ? nullptr : name.utf8().c_str()));
  if (!loaded)
    return std::unexpected(std::move(loaded.error()));
  pushHandle(m_state, handleIndex);
  lua_setupvalue(m_state, -2, 1);
  auto entered = incrementRecursionLevel();
  if (!entered)
    return std::unexpected(std::move(entered.error()));
  int res = pcallWithTraceback(m_state, 0, 0);
  decrementRecursionLevel();
  return handleError(m_state, res);
}

std::expected<LuaDetail::LuaFunctionReturn, StarException> LuaEngine::contextEval(int handleIndex, String const& lua) {
  LuaStackRestore stack(m_state);
  if (!lua_checkstack(m_state, 2))
    return std::unexpected(LuaException("Exhausted the Lua stack while evaluating a script"));
  int loadRes = luaL_loadstring(m_state, ("return " + lua).utf8Ptr());
  if (loadRes == LUA_ERRSYNTAX) {
    lua_pop(m_state, 1);
    loadRes = luaL_loadstring(m_state, lua.utf8Ptr());
  }
  auto loaded = handleError(m_state, loadRes);
  if (!loaded)
    return std::unexpected(std::move(loaded.error()));
  pushHandle(m_state, handleIndex);
  lua_setupvalue(m_state, -2, 1);
  auto entered = incrementRecursionLevel();
  if (!entered)
    return std::unexpected(std::move(entered.error()));
  int callRes = pcallWithTraceback(m_state, 0, LUA_MULTRET);
  decrementRecursionLevel();
  auto called = handleError(m_state, callRes);
  if (!called)
    return std::unexpected(std::move(called.error()));
  int returnValues = lua_gettop(m_state) - stack.top;
  if (returnValues == 0)
    return LuaDetail::LuaFunctionReturn();
  if (returnValues == 1) {
    auto value = popLuaValue(m_state);
    if (!value)
      return std::unexpected(std::move(value.error()));
    return LuaDetail::LuaFunctionReturn(std::move(*value));
  }
  LuaVariadic<LuaValue> ret(returnValues);
  for (int i = returnValues - 1; i >= 0; --i) {
    auto value = popLuaValue(m_state);
    if (!value)
      return std::unexpected(std::move(value.error()));
    ret[i] = std::move(*value);
  }
  return LuaDetail::LuaFunctionReturn(std::move(ret));
}

std::expected<LuaValue, StarException> LuaEngine::contextGetPath(int handleIndex, String path) {
  LuaStackRestore stack(m_state);
  if (!lua_checkstack(m_state, 4))
    return std::unexpected(LuaException("Exhausted the Lua stack while reading a context path"));
  auto utf8Path = path.takeUtf8();
  std::span<char> pathData(utf8Path);
  lua_pushcfunction(m_state, [](lua_State* state) {
    auto& path = *static_cast<std::span<char>*>(lua_touserdata(state, 2));
    lua_pop(state, 1);
    std::size_t start = 0;
    for (std::size_t i = 0; i < path.size(); ++i) {
      if (path[i] != '.')
        continue;
      path[i] = '\0';
      lua_getfield(state, -1, path.data() + start);
      lua_remove(state, -2);
      if (lua_type(state, -1) != LUA_TTABLE) {
        lua_pushnil(state);
        return 1;
      }
      start = i + 1;
    }
    lua_getfield(state, -1, path.data() + start);
    return 1;
  });
  pushHandle(m_state, handleIndex);
  lua_pushlightuserdata(m_state, &pathData);
  auto called = handleError(m_state, pcallWithTraceback(m_state, 2, 1));
  if (!called)
    return std::unexpected(std::move(called.error()));
  return popLuaValue(m_state);
}

std::expected<void, StarException> LuaEngine::contextSetPath(int handleIndex, String path, LuaValue const& value) {
  LuaStackRestore stack(m_state);
  if (!lua_checkstack(m_state, 5))
    return std::unexpected(LuaException("Exhausted the Lua stack while setting a context path"));
  auto utf8Path = path.takeUtf8();
  std::span<char> pathData(utf8Path);
  lua_pushcfunction(m_state, [](lua_State* state) {
    auto& path = *static_cast<std::span<char>*>(lua_touserdata(state, 2));
    lua_pushvalue(state, 1);
    std::size_t start = 0;
    for (std::size_t i = 0; i < path.size(); ++i) {
      if (path[i] != '.')
        continue;
      path[i] = '\0';
      int type = lua_getfield(state, -1, path.data() + start);
      if (type == LUA_TNIL) {
        lua_pop(state, 1);
        lua_newtable(state);
        lua_pushvalue(state, -1);
        lua_setfield(state, -3, path.data() + start);
        lua_remove(state, -2);
      } else if (type == LUA_TTABLE) {
        lua_remove(state, -2);
      } else {
        return luaL_error(state, "Sub-path in setPath is not nil and is not a table");
      }
      start = i + 1;
    }
    lua_pushvalue(state, 3);
    lua_setfield(state, -2, path.data() + start);
    return 0;
  });
  pushHandle(m_state, handleIndex);
  lua_pushlightuserdata(m_state, &pathData);
  auto pushed = pushLuaValue(m_state, value);
  if (!pushed)
    return std::unexpected(std::move(pushed.error()));
  return handleError(m_state, pcallWithTraceback(m_state, 3, 0));
}

std::expected<int, StarException> LuaEngine::popHandle(lua_State* state) {
  LuaStackRestore stack(state);
  --stack.top;
  if (!lua_checkstack(state, 3))
    return std::unexpected(LuaException("Exhausted the Lua stack while acquiring a registry reference"));
  lua_pushcfunction(state, [](lua_State* state) {
    lua_pushvalue(state, 1);
    int reference = luaL_ref(state, LUA_REGISTRYINDEX);
    lua_pushinteger(state, reference);
    return 1;
  });
  lua_insert(state, -2);
  auto acquired = handleError(state, pcallWithTraceback(state, 1, 1));
  if (!acquired)
    return std::unexpected(std::move(acquired.error()));
  return (int)lua_tointeger(state, -1);
}

void LuaEngine::pushHandle(lua_State* state, int handleIndex) {
  lua_rawgeti(state, LUA_REGISTRYINDEX, handleIndex);
}

std::expected<int, StarException> LuaEngine::copyHandle(int handleIndex) {
  LuaStackRestore stack(m_state);
  if (!lua_checkstack(m_state, 1))
    return std::unexpected(LuaException("Exhausted the Lua stack while copying a registry reference"));
  pushHandle(m_state, handleIndex);
  return popHandle(m_state);
}

std::expected<LuaFunction, StarException> LuaEngine::createWrappedFunction(LuaDetail::LuaWrappedFunction function) {
  LuaStackRestore stack(m_state);
  if (!lua_checkstack(m_state, 3))
    return std::unexpected(LuaException("Exhausted the Lua stack while creating a function"));

  auto invokeFunction = [](lua_State* state) {
    auto& func = *static_cast<LuaDetail::LuaWrappedFunction*>(lua_touserdata(state, lua_upvalueindex(1)));
    auto& self = luaEngine(state);
    int argumentCount = lua_gettop(state);
    {
      auto invoke = [&]() -> std::expected<int, StarException> {
        if (!func)
          return std::unexpected(LuaException("Cannot invoke an empty Lua callback"));
        auto invokeArgs = [&]() -> std::expected<LuaDetail::LuaFunctionReturn, StarException> {
          int const MaxArrayArgs = 8;
          if (argumentCount <= MaxArrayArgs) {
            Array<LuaValue, MaxArrayArgs> args;
            for (int i = argumentCount - 1; i >= 0; --i) {
              auto argument = self.popLuaValue(state);
              if (!argument)
                return std::unexpected(std::move(argument.error()));
              args[i] = std::move(*argument);
            }
            return func(self, std::span(args).first(argumentCount));
          }
          List<LuaValue> args(argumentCount);
          for (int i = argumentCount - 1; i >= 0; --i) {
            auto argument = self.popLuaValue(state);
            if (!argument)
              return std::unexpected(std::move(argument.error()));
            args[i] = std::move(*argument);
          }
          return func(self, std::span(args));
        };
        auto res = invokeArgs();
        if (!res)
          return std::unexpected(std::move(res.error()));
        if (std::holds_alternative<LuaValue>(*res)) {
          auto pushed = self.pushLuaValue(state, std::get<LuaValue>(*res));
          if (!pushed)
            return std::unexpected(std::move(pushed.error()));
          return 1;
        }
        if (std::holds_alternative<LuaVariadic<LuaValue>>(*res)) {
          auto const& values = std::get<LuaVariadic<LuaValue>>(*res);
          for (auto const& value : values) {
            auto pushed = self.pushLuaValue(state, value);
            if (!pushed)
              return std::unexpected(std::move(pushed.error()));
          }
          return (int)values.size();
        }
        return 0;
      };
      auto result = invoke();
      if (result)
        return *result;
      lua_settop(state, 0);
      pushCallbackError(state, result.error(), &s_luaInstructionLimitExceptionKey, &s_luaRecursionLimitExceptionKey);
    }
    // Argument storage, returned values and errors must precede the longjmp.
    return lua_error(state);
  };

  struct Input {
    std::reference_wrapper<LuaDetail::LuaWrappedFunction> function;
    int metatable;
    lua_CFunction invoke;
  };
  Input input{std::ref(function), m_wrappedFunctionMetatableRegistryId, invokeFunction};
  lua_pushcfunction(m_state, [](lua_State* state) {
    auto& input = *static_cast<Input*>(lua_touserdata(state, 1));
    auto function = (LuaDetail::LuaWrappedFunction*)lua_newuserdatauv(state, sizeof(LuaDetail::LuaWrappedFunction), 0);
    new (function) LuaDetail::LuaWrappedFunction(std::move(input.function.get()));
    lua_rawgeti(state, LUA_REGISTRYINDEX, input.metatable);
    lua_setmetatable(state, -2);
    lua_pushcclosure(state, input.invoke, 1);
    return 1;
  });
  lua_pushlightuserdata(m_state, &input);
  auto created = handleError(m_state, pcallWithTraceback(m_state, 1, 1));
  if (!created)
    return std::unexpected(std::move(created.error()));

  auto handle = popHandle(m_state);
  if (!handle)
    return std::unexpected(std::move(handle.error()));
  return LuaFunction(LuaDetail::LuaHandle(RefPtr<LuaEngine>(this), *handle));
}

std::expected<LuaFunction, StarException> LuaEngine::createRawFunction(lua_CFunction function) {
  if (!function)
    return std::unexpected(LuaException("Cannot create a Lua function from a null function pointer"));
  if (!lua_checkstack(m_state, 2))
    return std::unexpected(LuaException("Exhausted the Lua stack while creating a C function"));

  lua_pushcfunction(m_state, function);
  auto handle = popHandle(m_state);
  if (!handle)
    return std::unexpected(std::move(handle.error()));
  return LuaFunction(LuaDetail::LuaHandle(RefPtr<LuaEngine>(this), *handle));
}

std::expected<LuaFunction, StarException> LuaEngine::createFunctionFromSource(int handleIndex, std::span<char const> contents, String const& name) {
  LuaStackRestore stack(m_state);
  if (!lua_checkstack(m_state, 2))
    return std::unexpected(LuaException("Exhausted the Lua stack while loading a function"));
  auto loaded = handleError(m_state, luaL_loadbufferx(m_state, contents.data(), contents.size(), name.empty() ? nullptr : name.utf8().c_str(), "t"));
  if (!loaded)
    return std::unexpected(std::move(loaded.error()));

  pushHandle(m_state, handleIndex);
  lua_setupvalue(m_state, -2, 1);

  auto handle = popHandle(m_state);
  if (!handle)
    return std::unexpected(std::move(handle.error()));
  return LuaFunction(LuaDetail::LuaHandle(RefPtr<LuaEngine>(this), *handle));
}

std::expected<void, StarException> LuaEngine::pushLuaValue(lua_State* state, LuaValue const& luaValue) {
  if (!lua_checkstack(state, 1))
    return std::unexpected(LuaException("Exhausted the Lua stack while pushing a value"));
  return std::visit([&engine = *this, state](auto const& value) -> std::expected<void, StarException> {
    using T = std::decay_t<decltype(value)>;
    if constexpr (std::is_same_v<T, LuaNilType>) {
      lua_pushnil(state);
    } else if constexpr (std::is_same_v<T, LuaBoolean>) {
      lua_pushboolean(state, value);
    } else if constexpr (std::is_same_v<T, LuaInt>) {
      lua_pushinteger(state, value);
    } else if constexpr (std::is_same_v<T, LuaFloat>) {
      lua_pushnumber(state, value);
    } else {
      if (&value.engine() != &engine)
        return std::unexpected(LuaException("lua reference values cannot be shared between engines"));
      engine.pushHandle(state, value.handleIndex());
    }
    return {};
  }, luaValue);
}

std::expected<LuaValue, StarException> LuaEngine::popLuaValue(lua_State* state) {
  if (!lua_checkstack(state, 1)) {
    lua_pop(state, 1);
    return std::unexpected(LuaException("Exhausted the Lua stack while reading a value"));
  }

  LuaValue result;
  if constexpr (Star::DebugEnabled) {
    Star::starAssertFailed((!lua_isnone(state, -1)));
  }
  switch (lua_type(state, -1)) {
    case LUA_TNIL: {
      lua_pop(state, 1);
      break;
    }
    case LUA_TBOOLEAN: {
      result = lua_toboolean(state, -1) != 0;
      lua_pop(state, 1);
      break;
    }
    case LUA_TNUMBER: {
      if (lua_isinteger(state, -1)) {
        result = lua_tointeger(state, -1);
        lua_pop(state, 1);
      } else {
        result = lua_tonumber(state, -1);
        lua_pop(state, 1);
      }
      break;
    }
    case LUA_TSTRING: {
      auto handle = popHandle(state);
      if (!handle)
        return std::unexpected(std::move(handle.error()));
      result = LuaString(LuaDetail::LuaHandle(RefPtr<LuaEngine>(this), *handle));
      break;
    }
    case LUA_TTABLE: {
      auto handle = popHandle(state);
      if (!handle)
        return std::unexpected(std::move(handle.error()));
      result = LuaTable(LuaDetail::LuaHandle(RefPtr<LuaEngine>(this), *handle));
      break;
    }
    case LUA_TFUNCTION: {
      auto handle = popHandle(state);
      if (!handle)
        return std::unexpected(std::move(handle.error()));
      result = LuaFunction(LuaDetail::LuaHandle(RefPtr<LuaEngine>(this), *handle));
      break;
    }
    case LUA_TTHREAD: {
      auto handle = popHandle(state);
      if (!handle)
        return std::unexpected(std::move(handle.error()));
      result = LuaThread(LuaDetail::LuaHandle(RefPtr<LuaEngine>(this), *handle));
      break;
    }
    case LUA_TUSERDATA: {
      if (lua_getmetatable(state, -1) == 0) {
        lua_pop(state, 1);
        return std::unexpected(LuaException("Userdata in popLuaValue missing metatable"));
      }
      lua_pop(state, 1);
      auto handle = popHandle(state);
      if (!handle)
        return std::unexpected(std::move(handle.error()));
      result = LuaUserData(LuaDetail::LuaHandle(RefPtr<LuaEngine>(this), *handle));
      break;
    }
    default: {
      lua_pop(state, 1);
      return std::unexpected(LuaException("Unsupported type in popLuaValue"));
    }
  }

  return result;
}

std::expected<void, StarException> LuaEngine::incrementRecursionLevel() {
  // We reset the instruction count and profiling timing only on the *top
  // level* function entrance, not on recursive entrances.
  if (m_recursionLevel == 0) {
    m_instructionCount = 0;
  }

  if (m_recursionLimit != 0 && m_recursionLevel == m_recursionLimit)
    return std::unexpected(LuaRecursionLimitReached());

  ++m_recursionLevel;
  return {};
}

void LuaEngine::decrementRecursionLevel() {
  if constexpr (Star::DebugEnabled) {
    Star::starAssertFailed((m_recursionLevel != 0));
  }
  --m_recursionLevel;
}

void LuaEngine::updateCountHook() {
  if (m_instructionLimit || m_profilingEnabled)
    lua_sethook(m_state, &LuaEngine::countHook, LUA_MASKCOUNT, m_instructionMeasureInterval);
  else
    lua_sethook(m_state, &LuaEngine::countHook, 0, 0);
}

int LuaEngine::s_luaInstructionLimitExceptionKey = 0;
int LuaEngine::s_luaRecursionLimitExceptionKey = 0;

void LuaDetail::rawSetField(lua_State* state, int index, std::string_view key) {
  lua_checkstack(state, 1);

  int absTableIndex = lua_absindex(state, index);
  lua_pushlstring(state, key.data(), key.size());

  // Move the newly pushed key to the secont to top spot, leaving the value in
  // the top spot.
  lua_insert(state, -2);

  // Pops the value and the key
  lua_rawset(state, absTableIndex);
}

void LuaDetail::shallowCopy(lua_State* state, int sourceIndex, int targetIndex) {
  lua_checkstack(state, 3);

  int absSourceIndex = lua_absindex(state, sourceIndex);
  int absTargetIndex = lua_absindex(state, targetIndex);

  lua_pushnil(state);
  while (lua_next(state, absSourceIndex) != 0) {
    lua_pushvalue(state, -2);
    lua_insert(state, -2);
    lua_rawset(state, absTargetIndex);
  }
}

std::expected<LuaTable, StarException> LuaDetail::insertJsonMetatable(LuaEngine& engine, LuaTable const& table, Json::Type type) {
  auto newIndexMetaMethod = [](lua_State* state) {
    if (!lua_getmetatable(state, 1))
      return luaL_error(state, "JSON table missing metatable");
    lua_getfield(state, -1, "__nils");
    if (!lua_istable(state, -1))
      return luaL_error(state, "JSON table missing null tracking table");
    lua_pushvalue(state, 2);
    if (lua_isnil(state, 3))
      lua_pushinteger(state, 0);
    else
      lua_pushnil(state);
    lua_rawset(state, -3);
    lua_settop(state, 3);
    lua_rawset(state, 1);
    return 0;
  };
  auto mt = engine.createTable(0, 3);
  if (!mt)
    return std::unexpected(std::move(mt.error()));
  auto nils = engine.createTable(0, 0);
  if (!nils)
    return std::unexpected(std::move(nils.error()));
  auto nilTable = mt->rawSet("__nils", *nils);
  if (!nilTable)
    return std::unexpected(std::move(nilTable.error()));
  auto function = engine.createRawFunction(newIndexMetaMethod);
  if (!function)
    return std::unexpected(std::move(function.error()));
  auto newIndex = mt->rawSet("__newindex", std::move(*function));
  if (!newIndex)
    return std::unexpected(std::move(newIndex.error()));
  auto typeHint = mt->rawSet("__typehint", type == Json::Type::Array ? 1 : 2);
  if (!typeHint)
    return std::unexpected(std::move(typeHint.error()));
  auto metatable = table.setMetatable(*mt);
  if (!metatable)
    return std::unexpected(std::move(metatable.error()));
  return nils;
}

std::expected<LuaTable, StarException> LuaDetail::jsonContainerToTable(LuaEngine& engine, Json const& container) {
  if (!container.isType(Json::Type::Array) && !container.isType(Json::Type::Object))
    return std::unexpected(LuaException("jsonContainerToTable called on improper json type"));
  auto size = container.size();
  if (!size)
    return std::unexpected(std::move(size.error()));
  if (*size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    return std::unexpected(LuaException("Too many entries for a Lua JSON container"));
  bool array = container.isType(Json::Type::Array);
  auto table = engine.createTable(array ? static_cast<int>(*size) : 0, array ? 0 : static_cast<int>(*size));
  if (!table)
    return std::unexpected(std::move(table.error()));
  auto nils = insertJsonMetatable(engine, *table, container.type());
  if (!nils)
    return std::unexpected(std::move(nils.error()));
  if (container.isType(Json::Type::Array)) {
    auto vlist = container.arrayPtr();
    if (!vlist)
      return std::unexpected(std::move(vlist.error()));
    for (std::size_t i = 0; i < (*vlist)->size(); ++i) {
      auto const& val = (**vlist)[i];
      auto inserted = val ? table->rawSet(i + 1, val) : nils->rawSet(i + 1, 0);
      if (!inserted)
        return std::unexpected(std::move(inserted.error()));
    }
  } else {
    auto object = container.objectPtr();
    if (!object)
      return std::unexpected(std::move(object.error()));
    for (auto const& pair : **object) {
      auto inserted = pair.second ? table->rawSet(pair.first, pair.second) : nils->rawSet(pair.first, 0);
      if (!inserted)
        return std::unexpected(std::move(inserted.error()));
    }
  }
  return table;
}

std::expected<std::optional<Json>, StarException> LuaDetail::tableToJsonContainer(LuaTable const& table) {
  auto& engine = table.engine();
  LuaStackRestore restore(engine.m_state);
  if (!lua_checkstack(engine.m_state, 1))
    return std::unexpected(LuaException("Exhausted the Lua stack while reading JSON"));
  engine.pushHandle(engine.m_state, table.handleIndex());
  return engine.stackToJson(-1);
}

std::expected<std::optional<Json>, StarException> LuaEngine::stackToJson(int index) {
  LuaStackRestore restore(m_state);
  index = lua_absindex(m_state, index);
  switch (lua_type(m_state, index)) {
    case LUA_TNIL:
      return Json();
    case LUA_TBOOLEAN:
      return Json(lua_toboolean(m_state, index) != 0);
    case LUA_TNUMBER:
      if (lua_isinteger(m_state, index))
        return Json(lua_tointeger(m_state, index));
      return Json(lua_tonumber(m_state, index));
    case LUA_TSTRING: {
      std::size_t size;
      char const* data = lua_tolstring(m_state, index, &size);
      return Json(String(std::string_view(data, size)));
    }
    case LUA_TTABLE:
      break;
    default:
      return std::optional<Json>();
  }
  if (!lua_checkstack(m_state, 8))
    return std::unexpected(LuaException("Exhausted the Lua stack while reading JSON"));

  // Only the metadata lookup can invoke user metamethods. Keep it protected;
  // all C++ JSON ownership remains outside Lua's longjmp boundary.
  lua_pushcfunction(m_state, [](lua_State* state) {
    if (!lua_getmetatable(state, 1)) {
      lua_pushnil(state);
      lua_pushnil(state);
    } else {
      lua_getfield(state, -1, "__typehint");
      lua_getfield(state, -2, "__nils");
    }
    return 2;
  });
  lua_pushvalue(m_state, index);
  auto metadata = handleError(m_state, pcallWithTraceback(m_state, 1, 2));
  if (!metadata)
    return std::unexpected(std::move(metadata.error()));
  lua_pushvalue(m_state, -2);
  auto hintValue = popLuaValue(m_state);
  if (!hintValue)
    return std::unexpected(std::move(hintValue.error()));
  auto hint = luaTo<std::optional<int>>(std::move(*hintValue));
  if (!hint)
    return std::unexpected(std::move(hint.error()));
  int typeHint = hint->value_or(0);
  int nilsIndex = lua_gettop(m_state);
  if (!lua_isnil(m_state, nilsIndex) && !lua_istable(m_state, nilsIndex))
    return std::unexpected(LuaConversionException("JSON null tracking value is not a table"));

  JsonObject stringEntries;
  struct Entry {
    unsigned index;
    std::size_t order;
    Json value;
  };
  std::vector<Entry> intEntries;
  intEntries.reserve(lua_rawlen(m_state, index));
  bool ordered = true;
  auto visit = [&](int tableIndex, bool nulls) -> std::expected<bool, StarException> {
    LuaStackRestore cursorRestore(m_state);
    lua_pushnil(m_state);
    int cursor = lua_gettop(m_state);
    while (true) {
      lua_pushcfunction(m_state, [](lua_State* state) {
        return lua_next(state, 1) ? 2 : 0;
      });
      lua_pushvalue(m_state, tableIndex);
      lua_pushvalue(m_state, cursor);
      auto next = handleError(m_state, pcallWithTraceback(m_state, 2, LUA_MULTRET));
      if (!next)
        return std::unexpected(std::move(next.error()));
      lua_remove(m_state, cursor);
      if (lua_gettop(m_state) < cursor)
        return true;
      // Preserve the native boundary's error precedence even when the value
      // is ignored by null tracking or a different value cannot become JSON.
      for (int position : {cursor, cursor + 1}) {
        auto representable = validateNativeLuaValue(m_state, position);
        if (!representable)
          return std::unexpected(std::move(representable.error()));
      }
      Json value;
      if (!nulls) {
        auto converted = stackToJson(cursor + 1);
        if (!converted)
          return std::unexpected(std::move(converted.error()));
        if (!*converted)
          return false;
        value = std::move(**converted);
      }
      int keyType = lua_type(m_state, cursor);
      if (keyType == LUA_TNUMBER) {
        LuaValue key = lua_isinteger(m_state, cursor)
            ? LuaValue(lua_tointeger(m_state, cursor)) : LuaValue(lua_tonumber(m_state, cursor));
        if (auto integer = LuaDetail::asInteger(key)) {
          unsigned keyIndex = static_cast<unsigned>(*integer);
          if (!intEntries.empty() && intEntries.back().index >= keyIndex)
            ordered = false;
          intEntries.push_back({keyIndex, intEntries.size(), std::move(value)});
        } else {
          stringEntries[String(toString(lua_tonumber(m_state, cursor)))] = std::move(value);
        }
      } else if (keyType == LUA_TSTRING) {
        std::size_t size;
        char const* data = lua_tolstring(m_state, cursor, &size);
        stringEntries[String(std::string_view(data, size))] = std::move(value);
      } else {
        return false;
      }
      lua_pop(m_state, 1); // The next iteration retains only the original key.
    }
  };
  if (lua_istable(m_state, nilsIndex)) {
    auto visited = visit(nilsIndex, true);
    if (!visited)
      return std::unexpected(std::move(visited.error()));
    if (!*visited)
      return std::optional<Json>();
  }
  auto visited = visit(index, false);
  if (!visited)
    return std::unexpected(std::move(visited.error()));
  if (!*visited)
    return std::optional<Json>();
  if (!ordered) {
    std::sort(intEntries.begin(), intEntries.end(), [](Entry const& a, Entry const& b) {
      return a.index < b.index || (a.index == b.index && a.order < b.order);
    });
    std::size_t count = 0;
    for (std::size_t i = 0; i < intEntries.size(); ++i) {
      if (count && intEntries[count - 1].index == intEntries[i].index)
        intEntries[count - 1].value = std::move(intEntries[i].value);
      else {
        if (count != i)
          intEntries[count] = std::move(intEntries[i]);
        ++count;
      }
    }
    intEntries.resize(count);
  }
  bool interpretAsList = stringEntries.empty()
      && (typeHint == 1 || (typeHint != 2 && !intEntries.empty() && intEntries.back().index == intEntries.size()));
  if (interpretAsList) {
    if (!intEntries.empty() && intEntries.front().index == 0)
      return std::unexpected(LuaException("JSON array index must be positive"));
    JsonArray list(intEntries.empty() ? 0 : intEntries.back().index);
    for (auto& entry : intEntries)
      list[entry.index - 1] = std::move(entry.value);
    return Json(std::move(list));
  }
  for (auto& entry : intEntries)
    stringEntries[toString(entry.index)] = std::move(entry.value);
  return Json(std::move(stringEntries));
}

Json LuaDetail::jarrayCreate() {
  return JsonArray();
}

Json LuaDetail::jobjectCreate() {
  return JsonObject();
}

std::expected<LuaTable, StarException> LuaDetail::jarray(LuaEngine& engine, std::optional<LuaTable> table) {
  if (table) {
    auto inserted = insertJsonMetatable(engine, *table, Json::Type::Array);
    if (!inserted)
      return std::unexpected(std::move(inserted.error()));
    return std::move(*table);
  }
  return jsonContainerToTable(engine, JsonArray());
}

std::expected<LuaTable, StarException> LuaDetail::jobject(LuaEngine& engine, std::optional<LuaTable> table) {
  if (table) {
    auto inserted = insertJsonMetatable(engine, *table, Json::Type::Object);
    if (!inserted)
      return std::unexpected(std::move(inserted.error()));
    return std::move(*table);
  }
  return jsonContainerToTable(engine, JsonObject());
}

std::expected<void, StarException> LuaDetail::jcontRemove(LuaTable const& table, LuaValue const& key) {
  auto mt = table.getMetatable();
  if (!mt)
    return std::unexpected(std::move(mt.error()));
  if (*mt) {
    auto nils = (*mt)->rawGet<std::optional<LuaTable>>("__nils");
    if (!nils)
      return std::unexpected(std::move(nils.error()));
    if (*nils) {
      auto removed = (**nils).rawSet(key, LuaNil);
      if (!removed)
        return std::unexpected(std::move(removed.error()));
    }
  }
  return table.rawSet(key, LuaNil);
}

std::expected<std::size_t, StarException> LuaDetail::jcontSize(LuaTable const& table) {
  std::size_t elemCount = 0;
  std::size_t highestIndex = 0;
  bool hintList = false;
  auto count = [&](LuaValue const& key, LuaValue const&) -> std::expected<void, StarException> {
    auto i = asInteger(key);
    if (i && *i >= 0)
      highestIndex = std::max<std::size_t>(*i, highestIndex);
    else
      hintList = false;
    ++elemCount;
    return {};
  };
  auto mt = table.getMetatable();
  if (!mt)
    return std::unexpected(std::move(mt.error()));
  if (*mt) {
    auto hint = (*mt)->rawGet<std::optional<int>>("__typehint");
    if (!hint)
      return std::unexpected(std::move(hint.error()));
    hintList = *hint == 1;
    auto nils = (*mt)->rawGet<std::optional<LuaTable>>("__nils");
    if (!nils)
      return std::unexpected(std::move(nils.error()));
    if (*nils) {
      auto counted = (**nils).iterate(count);
      if (!counted)
        return std::unexpected(std::move(counted.error()));
    }
  }
  auto counted = table.iterate(count);
  if (!counted)
    return std::unexpected(std::move(counted.error()));
  return hintList ? highestIndex : elemCount;
}

std::expected<void, StarException> LuaDetail::jcontResize(LuaTable const& table, std::size_t targetSize) {
  auto mt = table.getMetatable();
  if (!mt)
    return std::unexpected(std::move(mt.error()));
  if (*mt) {
    auto nils = (*mt)->rawGet<std::optional<LuaTable>>("__nils");
    if (!nils)
      return std::unexpected(std::move(nils.error()));
    if (*nils) {
      auto resized = (**nils).iterate([&](LuaValue const& key, LuaValue const&) -> std::expected<void, StarException> {
        auto i = asInteger(key);
        if (i && *i > 0 && (std::size_t)*i > targetSize)
          return (**nils).rawSet(key, LuaNil);
        return {};
      });
      if (!resized)
        return std::unexpected(std::move(resized.error()));
    }
  }
  auto resized = table.iterate([&](LuaValue const& key, LuaValue const&) -> std::expected<void, StarException> {
    auto i = asInteger(key);
    if (i && *i > 0 && (std::size_t)*i > targetSize)
      return table.rawSet(key, LuaNil);
    return {};
  });
  if (!resized)
    return std::unexpected(std::move(resized.error()));
  auto last = table.get(targetSize);
  if (!last)
    return std::unexpected(std::move(last.error()));
  return table.set(targetSize, std::move(*last));
}

std::optional<LuaInt> LuaDetail::asInteger(LuaValue const& v) {
  if (std::holds_alternative<LuaInt>(v))
    return std::get<LuaInt>(v);
  if (std::holds_alternative<LuaFloat>(v)) {
    auto f = std::get<LuaFloat>(v);
    if (!std::isfinite(f) || f < LuaFloat(std::numeric_limits<LuaInt>::min())
        || f >= -LuaFloat(std::numeric_limits<LuaInt>::min()))
      return {};
    if ((LuaFloat)(LuaInt)f == f)
      return (LuaInt)f;
    return {};
  }
  return {};
}

}
