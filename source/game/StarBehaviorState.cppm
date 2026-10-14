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

class Blackboard;
using BlackboardPtr = std::shared_ptr<Blackboard>;
using BlackboardConstPtr = std::shared_ptr<const Blackboard>;
using BlackboardWeakPtr = std::weak_ptr<Blackboard>;
using BlackboardConstWeakPtr = std::weak_ptr<const Blackboard>;
using BlackboardUPtr = std::unique_ptr<Blackboard>;
using BlackboardConstUPtr = std::unique_ptr<const Blackboard>;
class BehaviorState;
using BehaviorStatePtr = std::shared_ptr<BehaviorState>;
using BehaviorStateConstPtr = std::shared_ptr<const BehaviorState>;
using BehaviorStateWeakPtr = std::weak_ptr<BehaviorState>;
using BehaviorStateConstWeakPtr = std::weak_ptr<const BehaviorState>;
using BehaviorStateUPtr = std::unique_ptr<BehaviorState>;
using BehaviorStateConstUPtr = std::unique_ptr<const BehaviorState>;
struct ActionState;
using ActionStatePtr = std::shared_ptr<ActionState>;
using ActionStateConstPtr = std::shared_ptr<const ActionState>;
using ActionStateWeakPtr = std::weak_ptr<ActionState>;
using ActionStateConstWeakPtr = std::weak_ptr<const ActionState>;
using ActionStateUPtr = std::unique_ptr<ActionState>;
using ActionStateConstUPtr = std::unique_ptr<const ActionState>;
struct DecoratorState;
using DecoratorStatePtr = std::shared_ptr<DecoratorState>;
using DecoratorStateConstPtr = std::shared_ptr<const DecoratorState>;
using DecoratorStateWeakPtr = std::weak_ptr<DecoratorState>;
using DecoratorStateConstWeakPtr = std::weak_ptr<const DecoratorState>;
using DecoratorStateUPtr = std::unique_ptr<DecoratorState>;
using DecoratorStateConstUPtr = std::unique_ptr<const DecoratorState>;
struct CompositeState;
using CompositeStatePtr = std::shared_ptr<CompositeState>;
using CompositeStateConstPtr = std::shared_ptr<const CompositeState>;
using CompositeStateWeakPtr = std::weak_ptr<CompositeState>;
using CompositeStateConstWeakPtr = std::weak_ptr<const CompositeState>;
using CompositeStateUPtr = std::unique_ptr<CompositeState>;
using CompositeStateConstUPtr = std::unique_ptr<const CompositeState>;

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
typedef std::shared_ptr<NodeState> NodeStatePtr;

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
  NodeStatePtr child;
};

struct CompositeState {
  CompositeState(std::size_t children);
  CompositeState(std::size_t children, std::size_t index);

  std::size_t index;
  List<NodeStatePtr> children;
};

class BehaviorState {
public:
  BehaviorState(BehaviorTreeConstPtr tree, LuaTable context, Maybe<BlackboardWeakPtr> blackboard = {});

  NodeStatus run(float dt);
  void clear();

  BlackboardWeakPtr blackboardPtr();
private:
  BlackboardPtr board();

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

  BehaviorTreeConstPtr m_tree;
  NodeState m_rootState;

  LuaTable m_luaContext;

  // The blackboard can either be created and owned by this behavior,
  // or a blackboard from another behavior can be used
  Variant<BlackboardPtr, BlackboardWeakPtr> m_board;

  // Keep threads here for recycling
  List<LuaThread> m_threads;
  StringMap<LuaFunction> m_functions;

  float m_lastDt;
};

}

export module star.behavior_state;

export namespace Star {
using ::Star::Blackboard;
using ::Star::BlackboardPtr;
using ::Star::BlackboardConstPtr;
using ::Star::BlackboardWeakPtr;
using ::Star::BlackboardConstWeakPtr;
using ::Star::BlackboardUPtr;
using ::Star::BlackboardConstUPtr;
using ::Star::BehaviorState;
using ::Star::BehaviorStatePtr;
using ::Star::BehaviorStateConstPtr;
using ::Star::BehaviorStateWeakPtr;
using ::Star::BehaviorStateConstWeakPtr;
using ::Star::BehaviorStateUPtr;
using ::Star::BehaviorStateConstUPtr;
using ::Star::ActionState;
using ::Star::ActionStatePtr;
using ::Star::ActionStateConstPtr;
using ::Star::ActionStateWeakPtr;
using ::Star::ActionStateConstWeakPtr;
using ::Star::ActionStateUPtr;
using ::Star::ActionStateConstUPtr;
using ::Star::DecoratorState;
using ::Star::DecoratorStatePtr;
using ::Star::DecoratorStateConstPtr;
using ::Star::DecoratorStateWeakPtr;
using ::Star::DecoratorStateConstWeakPtr;
using ::Star::DecoratorStateUPtr;
using ::Star::DecoratorStateConstUPtr;
using ::Star::CompositeState;
using ::Star::CompositeStatePtr;
using ::Star::CompositeStateConstPtr;
using ::Star::CompositeStateWeakPtr;
using ::Star::CompositeStateConstWeakPtr;
using ::Star::CompositeStateUPtr;
using ::Star::CompositeStateConstUPtr;
using ::Star::BehaviorExceptionTag;
using ::Star::BehaviorException;
using ::Star::BlackboardTypes;
using ::Star::NodeState;
using ::Star::NodeStatePtr;
using ::Star::Coroutine;
using ::Star::NodeStatus;
using ::Star::ActionReturn;
}
