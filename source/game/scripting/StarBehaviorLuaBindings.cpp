module;
#include "StarJson.hpp"
#include "StarLua.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarBiMap.hpp"
#include "StarThread.hpp"
#include "StarIODevice.hpp"
#include "StarList.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
import star.listener;
#include "StarConfig.hpp"
import star.version;


// Match client include order for SIMD intrinsics used by xxhash and fast_float.
#include "StarLuaGameConverters.hpp"
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

module star.behavior_lua_bindings;

import star.behavior_database;
import star.behavior_state;

namespace Star {

LuaCallbacks LuaBindings::makeBehaviorCallbacks(List<BehaviorStatePtr>* list) {
  LuaCallbacks callbacks;

  callbacks.registerCallback("behavior", [list](Json const& config, JsonObject const& parameters, LuaTable context, Maybe<LuaUserData> blackboard) -> BehaviorStateWeakPtr {
    auto behaviorDatabase = Root::singleton().behaviorDatabase();
    Maybe<BlackboardWeakPtr> board = {};
    if (blackboard && blackboard->is<BlackboardWeakPtr>())
      board = blackboard->get<BlackboardWeakPtr>();

    BehaviorTreeConstPtr tree;
    if (config.isType(Json::Type::String)) {
      if (parameters.empty()) {
        tree = behaviorDatabase->behaviorTree(config.toString());
      } else {
        JsonObject treeConfig = behaviorDatabase->behaviorConfig(config.toString()).toObject();
        treeConfig.set("parameters", jsonMerge(treeConfig.get("parameters"), parameters));
        tree = behaviorDatabase->buildTree(treeConfig);
      }
    } else {
      tree = behaviorDatabase->buildTree(config.set("parameters", jsonMerge(config.getObject("parameters", {}), parameters)));
    }

    BehaviorStatePtr state = make_shared<BehaviorState>(tree, context, board);
    list->append(state);
    return weak_ptr<BehaviorState>(state);
  });

  return callbacks;
}

}
