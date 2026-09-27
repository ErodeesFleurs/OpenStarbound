module;

// Match client include order for SIMD intrinsics used by xxhash and fast_float.
#include "StarJson.hpp"
#include "StarLua.hpp"
#include "StarBehaviorState.hpp"
#include "StarLuaGameConverters.hpp"
#include "StarRoot.hpp"

export module star.behavior_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeBehaviorCallbacks(List<BehaviorStatePtr>* list);
}

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
