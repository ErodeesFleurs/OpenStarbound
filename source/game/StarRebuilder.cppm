module;

import std;
import star.string;
import star.list;
import star.json;
import star.thread;

namespace Star {

class LuaContext;
class LuaRoot;
class Root;
class ItemDatabase;

class Rebuilder {
public:
  Rebuilder(Root& root, String const& id, ItemDatabase const* itemCatalog = nullptr);
  ~Rebuilder() = default;

  typedef std::function<String(Json const&)> AttemptCallback;
  bool rebuild(Json store, String last_error, AttemptCallback attempt) const;

private:
  std::shared_ptr<LuaRoot> m_luaRoot;
  mutable RecursiveMutex m_luaMutex;
  std::shared_ptr<List<LuaContext>> m_contexts;
};

}

export module star.rebuilder;

export namespace Star {
  using ::Star::LuaContext;
  using ::Star::LuaRoot;
  using ::Star::Rebuilder;
}
