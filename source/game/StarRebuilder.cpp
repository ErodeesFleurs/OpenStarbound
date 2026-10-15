import star.json;
import star.ordered_map;
import star.bi_map;
import star.thread;
import star.io_device;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.directives;
import star.asset_path;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.string;
import star.version;
import star.lua;

import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.lua_root;
import star.utility_lua_bindings;

import star.root_lua_bindings;
import star.rebuilder;

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