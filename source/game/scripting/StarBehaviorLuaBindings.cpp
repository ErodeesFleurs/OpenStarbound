module;
import star.json;
import star.lua;
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
import star.maybe;
import star.version;


// Match client include order for SIMD intrinsics used by xxhash and fast_float.
import star.lua_game_converters;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

module star.behavior_lua_bindings;

import star.behavior_database;
import star.behavior_state;

namespace Star {

LuaCallbacks LuaBindings::makeBehaviorCallbacks(List<std::shared_ptr<BehaviorState>>* list) {
  LuaCallbacks callbacks;

  callbacks.registerCallback("behavior", [list](Json const& config, JsonObject const& parameters, LuaTable context, Maybe<LuaUserData> blackboard) -> std::weak_ptr<BehaviorState> {
    auto behaviorDatabase = Root::singleton().behaviorDatabase();
    Maybe<std::weak_ptr<Blackboard>> board = {};
    if (blackboard && blackboard->is<std::weak_ptr<Blackboard>>())
      board = blackboard->get<std::weak_ptr<Blackboard>>();

    std::shared_ptr<BehaviorTree const> tree;
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

    std::shared_ptr<BehaviorState> state = std::make_shared<BehaviorState>(tree, context, board);
    list->append(state);
    return std::weak_ptr<BehaviorState>(state);
  });

  return callbacks;
}

}
