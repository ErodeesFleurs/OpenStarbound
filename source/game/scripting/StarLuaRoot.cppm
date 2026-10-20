module;
import std;
import star.string;
import star.thread;
import star.lua;
import star.listener;
import star.byte_array;
import star.type_identity;

namespace Star {

class Root;

class LuaExecutionScope final : public LuaExecutionHost {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<LuaExecutionScope, LuaExecutionHost>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return typeName<LuaExecutionScope>();
  }
  explicit LuaExecutionScope(Root& root) : m_root(root) {}
  Root& root() const { return m_root; }
private:
  Root& m_root;
};

Root& luaExecutionRoot(LuaEngine& engine);

// Loads and caches Lua scripts from assets, clearing the cache on root reload.
// The internal engine and all contexts require single-threaded access.
// Root is borrowed and must outlive the engine and every returned context.
class LuaRoot {
public:
  explicit LuaRoot(Root& root);
  ~LuaRoot();
  Root& root() const;

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

  void collectGarbage(std::optional<unsigned> steps = {});
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
    explicit ScriptCache(Root& root) : m_root(root) {}
    void loadScript(LuaEngine& engine, String const& assetPath);
    bool scriptLoaded(String const& assetPath) const;
    void unloadScript(String const& assetPath);
    void clear();
    void loadContextScript(LuaContext& context, String const& assetPath);
    std::size_t memoryUsage() const;

  private:
    Root& m_root;
    mutable RecursiveMutex mutex;
    StringMap<ByteArray> scripts;
  };

  Root& m_root;
  LuaEnginePtr m_luaEngine;
  StringMap<LuaCallbacks> m_luaCallbacks;
  std::shared_ptr<ScriptCache> m_scriptCache;

  std::shared_ptr<Listener> m_rootReloadListener;
  ListenerSubscription m_rootReloadSubscription;

  String m_storageDirectory;
};

}

export module star.lua_root;

export namespace Star {
  using ::Star::LuaRoot;
  using ::Star::LuaExecutionScope;
  using ::Star::luaExecutionRoot;
}
