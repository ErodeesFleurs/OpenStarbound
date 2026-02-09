#include "StarRebuilder.hpp"

#include "StarLua.hpp"
#include "StarLuaRoot.hpp"
#include "StarRoot.hpp"
#include "StarRootLuaBindings.hpp"
#include "StarUtilityLuaBindings.hpp"

import std;

namespace Star {

Rebuilder::Rebuilder(String const& id) {
  m_luaRoot = std::make_shared<LuaRoot>();
  auto assets = Root::singleton().assets();
  m_contexts = std::make_shared<List<LuaContext>>();

  for (auto& path : assets->assetSources()) {
    auto metadata = assets->assetSourceMetadata(path);
    if (auto scripts = metadata.maybe("errorHandlers")) {
      if (auto scriptPaths = scripts.value().optArray(id)) {
        for (auto& scriptPath : *scriptPaths) {
          auto context = m_luaRoot->createContext(scriptPath.toString());
          context.setCallbacks("root", LuaBindings::makeRootCallbacks());
          context.setCallbacks("sb", LuaBindings::makeUtilityCallbacks());
          m_contexts->push_back(context);
        }
      }
    }
  }
}

auto Rebuilder::rebuild(Json store, String last_error, AttemptCallback attempt) const -> bool {
  RecursiveMutexLocker locker(m_luaMutex);
  for (auto& context : *m_contexts) {
    Json newStore = context.invokePath<Json>("error", store, last_error);
    if (!newStore || newStore == store)
      break;

    auto error = attempt(store = newStore);
    if (!error.empty())
      last_error = error;
    else
      return true;
  }
  return false;
}

}// namespace Star
