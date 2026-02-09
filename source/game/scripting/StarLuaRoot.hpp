#pragma once

#include "StarConfig.hpp"
#include "StarListener.hpp"
#include "StarLua.hpp"
#include "StarLuaConverters.hpp"// IWYU pragma: export
#include "StarThread.hpp"

import std;

namespace Star {

// Loads and caches lua scripts from assets.  Automatically clears cache on
// root reload.  Uses an internal LuaEngine, so this and all contexts are meant
// for single threaded access and have no locking.
class LuaRoot {
public:
  LuaRoot();
  ~LuaRoot();

  void loadScript(String const& assetPath);
  [[nodiscard]] auto scriptLoaded(String const& assetPath) const -> bool;
  void unloadScript(String const& assetPath);

  void restart();
  void shutdown();

  // A script context can be created from the combination of several scripts,
  // the functions / data in each script will be loaded in order, so that later
  // specified scripts will overwrite previous ones.
  //
  // The LuaContext that is returned will have its 'require' function
  // overloaded to take absolute asset paths and load that asset path as a lua
  // module, with protection from duplicate loading.
  auto createContext(String const& script) -> LuaContext;
  auto createContext(StringList const& scriptPaths = {}) -> LuaContext;

  void collectGarbage(std::optional<unsigned> steps = {});
  void setAutoGarbageCollection(bool autoGarbageColleciton);
  void tuneAutoGarbageCollection(float pause, float stepMultiplier);
  [[nodiscard]] auto luaMemoryUsage() const -> size_t;

  [[nodiscard]] auto scriptCacheMemoryUsage() const -> size_t;
  void clearScriptCache() const;

  void addCallbacks(String const& groupName, LuaCallbacks const& callbacks);

  [[nodiscard]] auto luaEngine() const -> LuaEngine&;

private:
  class ScriptCache {
  public:
    void loadScript(LuaEngine& engine, String const& assetPath);
    auto scriptLoaded(String const& assetPath) const -> bool;
    void unloadScript(String const& assetPath);
    void clear();
    void loadContextScript(LuaContext& context, String const& assetPath);
    auto memoryUsage() const -> size_t;

  private:
    mutable RecursiveMutex mutex;
    StringMap<ByteArray> scripts;
  };

  LuaEnginePtr m_luaEngine;
  StringMap<LuaCallbacks> m_luaCallbacks;
  std::shared_ptr<ScriptCache> m_scriptCache;

  Ptr<Listener> m_rootReloadListener;

  String m_storageDirectory;
};

}// namespace Star
