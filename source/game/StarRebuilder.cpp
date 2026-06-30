#include "StarRebuilder.hpp"
#include "StarAlgorithm.hpp"
#include "StarLuaRoot.hpp"
#include "StarLua.hpp"
#include "StarRootLuaBindings.hpp"
#include "StarUtilityLuaBindings.hpp"

namespace Star {

Rebuilder::Rebuilder(AssetsConstPtr assets, String const& id, LuaRootServices luaRootServices) {
  requireNotNull(assets, "Rebuilder", "assets");

  m_luaRoot = make_shared<LuaRoot>(std::move(luaRootServices));
  m_contexts = make_shared<List<LuaContext>>();

  for (auto& path : assets->assetSources()) {
    auto metadata = assets->assetSourceMetadata(path);
    if (auto scripts = metadata.maybe("errorHandlers")) {
      if (auto scriptPaths = scripts.value().optArray(id)) {
        for (auto& scriptPath : *scriptPaths) {
          auto context = m_luaRoot->createContext(scriptPath.toString());
          context.setCallbacks("sb", LuaBindings::makeUtilityCallbacks());
          m_contexts->push_back(context);
        }
      }
    }
  }
}

bool Rebuilder::rebuild(Json store, String last_error, AttemptCallback attempt) const {
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

}
