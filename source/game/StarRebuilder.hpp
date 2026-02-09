#pragma once

#include "StarConfig.hpp"
#include "StarJson.hpp"
#include "StarThread.hpp"

import std;

namespace Star {

class LuaRoot;
class LuaContext;

class Rebuilder {
public:
  Rebuilder(String const& id);
  ~Rebuilder() = default;

  using AttemptCallback = std::function<String(Json const&)>;
  auto rebuild(Json store, String last_error, AttemptCallback attempt) const -> bool;

private:
  Ptr<LuaRoot> m_luaRoot;
  mutable RecursiveMutex m_luaMutex;
  std::shared_ptr<List<LuaContext>> m_contexts;// this is a ptr to avoid having to include Lua.hpp here
};

}// namespace Star
