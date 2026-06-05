#pragma once
#include "StarJson.hpp"
#include "StarThread.hpp"

namespace Star {

class LuaContext;
class LuaRoot;
using LuaRootPtr = SharedPtr<LuaRoot>;

class Rebuilder {
public:
  Rebuilder(String const& id);
  ~Rebuilder() = default;

  using AttemptCallback = function<String(Json const&)>;
  bool rebuild(Json store, String last_error, AttemptCallback attempt) const;

private:
  LuaRootPtr m_luaRoot;
  mutable RecursiveMutex m_luaMutex;
  SharedPtr<List<LuaContext>> m_contexts; // this is a ptr to avoid having to include Lua.hpp here
};

}
