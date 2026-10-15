module;

import std;
import star.string;
import star.list;
import star.json;
import star.thread;

namespace Star {

class LuaContext;
class LuaRoot;

class Rebuilder {
public:
  Rebuilder(String const& id);
  ~Rebuilder() = default;

  typedef std::function<String(Json const&)> AttemptCallback;
  bool rebuild(Json store, String last_error, AttemptCallback attempt) const;

private:
  std::shared_ptr<LuaRoot> m_luaRoot;
  mutable RecursiveMutex m_luaMutex;
  std::shared_ptr<List<LuaContext>> m_contexts; // this is a ptr to avoid having to include Lua.hpp here
};

}

export module star.rebuilder;

export namespace Star {
  using ::Star::LuaContext;
  using ::Star::LuaRoot;
  using ::Star::Rebuilder;
}
