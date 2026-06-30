#pragma once

#include "StarObserverPtr.hpp"
#include "StarThread.hpp"
#include "StarLua.hpp"
#include "StarAssets.hpp"
#include "StarConfiguration.hpp"
#include "StarListener.hpp"

namespace Star {

class LuaRoot;
using LuaRootPtr = SharedPtr<LuaRoot>;
class Root;

struct LuaRootServices {
  observer_ptr<Root> root;
  AssetsConstPtr assets;
  ConfigurationPtr configuration;
  function<void(ListenerWeakPtr)> registerReloadListener;
  String storageDirectory;
};

[[nodiscard]] LuaRootServices requireLuaRootServices(LuaRootServices services, char const* context);

// Loads and caches lua scripts from assets.  Automatically clears cache on
// root reload.  Uses an internal LuaEngine, so this and all contexts are meant
// for single threaded access and have no locking.
class LuaRoot {
public:
  explicit LuaRoot(LuaRootServices services);
  ~LuaRoot();

  void loadScript(String const& assetPath);
  [[nodiscard]] bool scriptLoaded(String const& assetPath) const;
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
  [[nodiscard]] LuaContext createContext(String const& script);
  [[nodiscard]] LuaContext createContext(StringList const& scriptPaths = {});

  void collectGarbage(Maybe<unsigned> steps = {});
  void setAutoGarbageCollection(bool autoGarbageColleciton);
  void tuneAutoGarbageCollection(float pause, float stepMultiplier);
  [[nodiscard]] size_t luaMemoryUsage() const;

  [[nodiscard]] size_t scriptCacheMemoryUsage() const;
  void clearScriptCache() const;

  void addCallbacks(String const& groupName, LuaCallbacks const& callbacks);
  void registerReloadListener(ListenerWeakPtr reloadListener);
  [[nodiscard]] LuaRootServices const& services() const;

  [[nodiscard]] LuaEngine& luaEngine() const;
private:
  class ScriptCache {
  public:
    explicit ScriptCache(AssetsConstPtr assets);

    void loadScript(LuaEngine& engine, String const& assetPath);
    [[nodiscard]] bool scriptLoaded(String const& assetPath) const;
    void unloadScript(String const& assetPath);
    void clear();
    void loadContextScript(LuaContext& context, String const& assetPath);
    [[nodiscard]] size_t memoryUsage() const;

  private:
    AssetsConstPtr m_assets;
    mutable RecursiveMutex mutex;
    StringMap<ByteArray> scripts;
  };

  LuaRootServices m_services;
  AssetsConstPtr m_assets;
  LuaEnginePtr m_luaEngine;
  StringMap<LuaCallbacks> m_luaCallbacks;
  SharedPtr<ScriptCache> m_scriptCache;

  ListenerPtr m_rootReloadListener;
  ListenerGroup m_reloadListeners;

  String m_storageDirectory;
};

}
