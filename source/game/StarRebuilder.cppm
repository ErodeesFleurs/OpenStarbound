module;

#include "StarJson.hpp"
#include "StarThread.hpp"

namespace Star {

STAR_CLASS(LuaContext);
STAR_CLASS(LuaRoot);

class Rebuilder {
public:
  Rebuilder(String const& id);
  ~Rebuilder() = default;

  typedef function<String(Json const&)> AttemptCallback;
  bool rebuild(Json store, String last_error, AttemptCallback attempt) const;

private:
  LuaRootPtr m_luaRoot;
  mutable RecursiveMutex m_luaMutex;
  shared_ptr<List<LuaContext>> m_contexts; // this is a ptr to avoid having to include Lua.hpp here
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
