module;
import std;
import star.string;
import star.maybe;
import star.thread;
import star.lua;
import star.listener;
import star.byte_array;

namespace Star {

// Loads and caches lua scripts from assets.  Automatically clears cache on
// root reload.  Uses an internal LuaEngine, so this and all contexts are meant
// for single threaded access and have no locking.
class LuaRoot {
public:
  LuaRoot();
  ~LuaRoot();

  void loadScript(String const& assetPath);
  bool scriptLoaded(String const& assetPath) const;
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
  LuaContext createContext(String const& script);
  LuaContext createContext(StringList const& scriptPaths = {});

  void collectGarbage(Maybe<unsigned> steps = {});
  void setAutoGarbageCollection(bool autoGarbageColleciton);
  void tuneAutoGarbageCollection(float pause, float stepMultiplier);
  std::size_t luaMemoryUsage() const;

  std::size_t scriptCacheMemoryUsage() const;
  void clearScriptCache() const;

  void addCallbacks(String const& groupName, LuaCallbacks const& callbacks);

  LuaEngine& luaEngine() const;
private:
  class ScriptCache {
  public:
    void loadScript(LuaEngine& engine, String const& assetPath);
    bool scriptLoaded(String const& assetPath) const;
    void unloadScript(String const& assetPath);
    void clear();
    void loadContextScript(LuaContext& context, String const& assetPath);
    std::size_t memoryUsage() const;

  private:
    mutable RecursiveMutex mutex;
    StringMap<ByteArray> scripts;
  };

  LuaEnginePtr m_luaEngine;
  StringMap<LuaCallbacks> m_luaCallbacks;
  std::shared_ptr<ScriptCache> m_scriptCache;

  std::shared_ptr<Listener> m_rootReloadListener;

  String m_storageDirectory;
};

}

export module star.lua_root;

export namespace Star {
  using ::Star::LuaRoot;
}
