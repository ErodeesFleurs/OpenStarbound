#include "StarBehaviorLuaBindings.hpp"
#include "StarConfig.hpp"
#include "StarRoot.hpp"

import std;

namespace Star {

auto LuaBindings::makeBehaviorCallbacks(List<Ptr<BehaviorState>>* list) -> LuaCallbacks {
  LuaCallbacks callbacks;

  callbacks.registerCallback("behavior", [list](Json const& config, JsonObject const& parameters, LuaTable context, std::optional<LuaUserData> blackboard) -> WeakPtr<BehaviorState> {
    auto behaviorDatabase = Root::singleton().behaviorDatabase();
    std::optional<WeakPtr<Blackboard>> board = {};
    if (blackboard && blackboard->is<WeakPtr<Blackboard>>())
      board = blackboard->get<WeakPtr<Blackboard>>();

    ConstPtr<BehaviorTree> tree;
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

    Ptr<BehaviorState> state = make_shared<BehaviorState>(tree, context, board);
    list->append(state);
    return {state};
  });

  return callbacks;
}

}// namespace Star
