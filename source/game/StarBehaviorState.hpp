#pragma once

#include "StarBehaviorDatabase.hpp"
#include "StarConfig.hpp"
#include "StarException.hpp"
#include "StarLua.hpp"

import std;

namespace Star {

struct ActionState;
struct DecoratorState;
struct CompositeState;

using BehaviorException = ExceptionDerived<"BehaviorException">;

extern List<NodeParameterType> BlackboardTypes;

class Blackboard {
public:
  Blackboard(LuaTable luaContext);

  [[nodiscard]] auto get(NodeParameterType type, String const& key) const -> LuaValue;
  void set(NodeParameterType type, String const& key, LuaValue value);

  auto parameters(StringMap<NodeParameter> const& nodeParameters, std::uint64_t nodeId) -> LuaTable;
  void setOutput(ActionNode const& node, LuaTable const& output);

  // takes the set of currently held ephemeral values
  auto takeEphemerals() -> Set<std::pair<NodeParameterType, String>>;

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

using NodeState = std::optional<Variant<ActionState, DecoratorState, CompositeState>>;
using NodeStatePtr = std::shared_ptr<NodeState>;

using Coroutine = std::pair<LuaFunction, LuaThread>;

enum class NodeStatus {
  Invalid,
  Success,
  Failure,
  Running
};

using ActionReturn = LuaTupleReturn<NodeStatus, LuaValue>;
struct ActionState {
  LuaThread thread;
};

struct DecoratorState {
  DecoratorState(LuaThread thread);
  LuaThread thread;
  NodeStatePtr child;
};

struct CompositeState {
  CompositeState(size_t children);
  CompositeState(size_t children, size_t index);

  size_t index;
  List<NodeStatePtr> children;
};

class BehaviorState {
public:
  BehaviorState(ConstPtr<BehaviorTree> tree, LuaTable context, std::optional<WeakPtr<Blackboard>> blackboard = {});

  auto run(float dt) -> NodeStatus;
  void clear();

  auto blackboardPtr() -> WeakPtr<Blackboard>;

private:
  auto board() -> Ptr<Blackboard>;

  auto nodeLuaThread(String const& funcName) -> LuaThread;

  auto runNode(BehaviorNode const& node, NodeState& state) -> NodeStatus;

  auto runAction(ActionNode const& node, NodeState& state) -> NodeStatus;
  auto runDecorator(DecoratorNode const& node, NodeState& state) -> NodeStatus;

  auto runComposite(CompositeNode const& node, NodeState& state) -> NodeStatus;
  auto runSequence(SequenceNode const& node, NodeState& state) -> NodeStatus;
  auto runSelector(SelectorNode const& node, NodeState& state) -> NodeStatus;
  auto runParallel(ParallelNode const& node, NodeState& state) -> NodeStatus;
  auto runDynamic(DynamicNode const& node, NodeState& state) -> NodeStatus;
  auto runRandomize(RandomizeNode const& node, NodeState& state) -> NodeStatus;

  ConstPtr<BehaviorTree> m_tree;
  NodeState m_rootState;

  LuaTable m_luaContext;

  // The blackboard can either be created and owned by this behavior,
  // or a blackboard from another behavior can be used
  Variant<Ptr<Blackboard>, WeakPtr<Blackboard>> m_board;

  // Keep threads here for recycling
  List<LuaThread> m_threads;
  StringMap<LuaFunction> m_functions;

  float m_lastDt;
};

}// namespace Star
