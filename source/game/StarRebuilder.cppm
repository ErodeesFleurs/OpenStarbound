module;

import std;
import star.string;
import star.list;
import star.json;
import star.thread;

namespace Star {

class LuaContext;
using LuaContextPtr = std::shared_ptr<LuaContext>;
using LuaContextConstPtr = std::shared_ptr<const LuaContext>;
using LuaContextWeakPtr = std::weak_ptr<LuaContext>;
using LuaContextConstWeakPtr = std::weak_ptr<const LuaContext>;
using LuaContextUPtr = std::unique_ptr<LuaContext>;
using LuaContextConstUPtr = std::unique_ptr<const LuaContext>;
class LuaRoot;
using LuaRootPtr = std::shared_ptr<LuaRoot>;
using LuaRootConstPtr = std::shared_ptr<const LuaRoot>;
using LuaRootWeakPtr = std::weak_ptr<LuaRoot>;
using LuaRootConstWeakPtr = std::weak_ptr<const LuaRoot>;
using LuaRootUPtr = std::unique_ptr<LuaRoot>;
using LuaRootConstUPtr = std::unique_ptr<const LuaRoot>;

class Rebuilder {
public:
  Rebuilder(String const& id);
  ~Rebuilder() = default;

  typedef std::function<String(Json const&)> AttemptCallback;
  bool rebuild(Json store, String last_error, AttemptCallback attempt) const;

private:
  LuaRootPtr m_luaRoot;
  mutable RecursiveMutex m_luaMutex;
  std::shared_ptr<List<LuaContext>> m_contexts; // this is a ptr to avoid having to include Lua.hpp here
};

}

export module star.rebuilder;

export namespace Star {
  using ::Star::LuaContext;
  using ::Star::LuaContextPtr;
  using ::Star::LuaContextConstPtr;
  using ::Star::LuaContextWeakPtr;
  using ::Star::LuaContextConstWeakPtr;
  using ::Star::LuaContextUPtr;
  using ::Star::LuaContextConstUPtr;
  using ::Star::LuaRoot;
  using ::Star::LuaRootPtr;
  using ::Star::LuaRootConstPtr;
  using ::Star::LuaRootWeakPtr;
  using ::Star::LuaRootConstWeakPtr;
  using ::Star::LuaRootUPtr;
  using ::Star::LuaRootConstUPtr;
  using ::Star::Rebuilder;
}
