module;

import std;
import star.string;
import star.list;
import star.map;
import star.set;
import star.variant;
import star.maybe;
import star.format;
import star.json;
import star.game_types;
import star.lua;

import star.behavior_database;

namespace Star {

struct ActionState;
struct DecoratorState;
struct CompositeState;

struct BehaviorExceptionTag {
  static constexpr char const* name() { return "BehaviorException"; }
};
using BehaviorException = StarError<BehaviorExceptionTag, StarException>;

extern List<NodeParameterType> BlackboardTypes;

class Blackboard {
public:
  Blackboard(LuaTable luaContext);

  LuaValue get(NodeParameterType type, String const& key) const;
  void set(NodeParameterType type, String const& key, LuaValue value);

  LuaTable parameters(StringMap<NodeParameter> const& nodeParameters, std::uint64_t nodeId);
  void setOutput(ActionNode const& node, LuaTable const& output);

  // takes the set of currently held ephemeral values
  Set<std::pair<NodeParameterType, String>> takeEphemerals();

  // clears any provided ephemerals that are not currently held
  void clearEphemerals(Set<std::pair<NodeParameterType, String>> ephemerals);
private:
  LuaTable m_luaContext;

  HashMap<std::uint64_t, LuaTable> m_parameters;
  HashMap<NodeParameterType, StringMap<LuaValue>> m_board;

  HashMap<NodeParameterType, StringMap<List<std::pair<std::uint64_t, String>>>> m_input;
  StringMap<List<std::pair<std::uint64_t, LuaTable>>> m_vectorNumberInput;

  Set<std::pair<NodeParameterType, String>> m_ephemeral;
};

typedef Maybe<Variant<ActionState,DecoratorState,CompositeState>> NodeState;

typedef std::pair<LuaFunction, LuaThread> Coroutine;

enum class NodeStatus {
  Invalid,
  Success,
  Failure,
  Running
};

typedef LuaTupleReturn<NodeStatus, LuaValue> ActionReturn;
struct ActionState {
  LuaThread thread;
};

struct DecoratorState {
  DecoratorState(LuaThread thread);
  LuaThread thread;
  std::shared_ptr<NodeState> child;
};

struct CompositeState {
  CompositeState(std::size_t children);
  CompositeState(std::size_t children, std::size_t index);

  std::size_t index;
  List<std::shared_ptr<NodeState>> children;
};

class BehaviorState {
public:
  BehaviorState(std::shared_ptr<BehaviorTree const> tree, LuaTable context, Maybe<std::weak_ptr<Blackboard>> blackboard = {});

  NodeStatus run(float dt);
  void clear();

  std::weak_ptr<Blackboard> blackboardPtr();
private:
  std::shared_ptr<Blackboard> board();

  LuaThread nodeLuaThread(String const& funcName);

  NodeStatus runNode(BehaviorNode const& node, NodeState& state);

  NodeStatus runAction(ActionNode const& node, NodeState& state);
  NodeStatus runDecorator(DecoratorNode const& node, NodeState& state);

  NodeStatus runComposite(CompositeNode const& node, NodeState& state);
  NodeStatus runSequence(SequenceNode const& node, NodeState& state);
  NodeStatus runSelector(SelectorNode const& node, NodeState& state);
  NodeStatus runParallel(ParallelNode const& node, NodeState& state);
  NodeStatus runDynamic(DynamicNode const& node, NodeState& state);
  NodeStatus runRandomize(RandomizeNode const& node, NodeState& state);

  std::shared_ptr<BehaviorTree const> m_tree;
  NodeState m_rootState;

  LuaTable m_luaContext;

  // The blackboard can either be created and owned by this behavior,
  // or a blackboard from another behavior can be used
  Variant<std::shared_ptr<Blackboard>, std::weak_ptr<Blackboard>> m_board;

  // Keep threads here for recycling
  List<LuaThread> m_threads;
  StringMap<LuaFunction> m_functions;

  float m_lastDt;
};

}

export module star.behavior_state;

export namespace Star {
using ::Star::Blackboard;
using ::Star::BehaviorState;
using ::Star::ActionState;
using ::Star::DecoratorState;
using ::Star::CompositeState;
using ::Star::BehaviorExceptionTag;
using ::Star::BehaviorException;
using ::Star::BlackboardTypes;
using ::Star::NodeState;
using ::Star::Coroutine;
using ::Star::NodeStatus;
using ::Star::ActionReturn;
}
