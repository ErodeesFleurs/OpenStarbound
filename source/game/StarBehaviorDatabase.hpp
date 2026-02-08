#pragma once

#include "StarBiMap.hpp"
#include "StarConfig.hpp"
#include "StarJson.hpp"

import std;

namespace Star {

struct SequenceNode;
struct SelectorNode;
struct ParallelNode;
struct DynamicNode;
struct RandomizeNode;
struct ActionNode;
struct DecoratorNode;
struct BehaviorTree;

using CompositeNode = Variant<SequenceNode, SelectorNode, ParallelNode, DynamicNode, RandomizeNode>;

using BehaviorNode = Variant<ActionNode, DecoratorNode, CompositeNode, ConstPtr<BehaviorTree>>;

enum class NodeParameterType : std::uint8_t {
  Json,
  Entity,
  Position,
  Vec2,
  Number,
  Bool,
  List,
  Table,
  String
};
extern EnumMap<NodeParameterType> const NodeParameterTypeNames;

using NodeParameterValue = Variant<String, Json>;
using NodeParameter = std::pair<NodeParameterType, NodeParameterValue>;
using NodeOutput = std::pair<NodeParameterType, std::pair<std::optional<String>, bool>>;

auto nodeParameterValueFromJson(Json const& json) -> NodeParameterValue;

auto jsonFromNodeParameter(NodeParameter const& parameter) -> Json;
auto jsonToNodeParameter(Json const& json) -> NodeParameter;

auto jsonFromNodeOutput(NodeOutput const& output) -> Json;
auto jsonToNodeOutput(Json const& json) -> NodeOutput;

enum class BehaviorNodeType : std::uint16_t {
  Action,
  Decorator,
  Composite,
  Module
};
extern EnumMap<BehaviorNodeType> const BehaviorNodeTypeNames;

enum class CompositeType : std::uint16_t {
  Sequence,
  Selector,
  Parallel,
  Dynamic,
  Randomize
};
extern EnumMap<CompositeType> const CompositeTypeNames;

// replaces global tags in nodeParameters in place
auto replaceBehaviorTag(NodeParameterValue const& parameter, StringMap<NodeParameterValue> const& treeParameters) -> NodeParameterValue;
auto replaceOutputBehaviorTag(std::optional<String> const& output, StringMap<NodeParameterValue> const& treeParameters) -> std::optional<String>;
void applyTreeParameters(StringMap<NodeParameter>& nodeParameters, StringMap<NodeParameterValue> const& treeParameters);

struct ActionNode {
  ActionNode(String name, StringMap<NodeParameter> parameters, StringMap<NodeOutput> output);

  String name;
  StringMap<NodeParameter> parameters;
  StringMap<NodeOutput> output;
};

struct DecoratorNode {
  DecoratorNode(String const& name, StringMap<NodeParameter> parameters, ConstPtr<BehaviorNode> child);

  String name;
  StringMap<NodeParameter> parameters;
  ConstPtr<BehaviorNode> child;
};

struct SequenceNode {
  SequenceNode(List<ConstPtr<BehaviorNode>> children);

  List<ConstPtr<BehaviorNode>> children;
};

struct SelectorNode {
  SelectorNode(List<ConstPtr<BehaviorNode>> children);

  List<ConstPtr<BehaviorNode>> children;
};

struct ParallelNode {
  ParallelNode(StringMap<NodeParameter>, List<ConstPtr<BehaviorNode>> children);

  int succeed;
  int fail;
  List<ConstPtr<BehaviorNode>> children;
};

struct DynamicNode {
  DynamicNode(List<ConstPtr<BehaviorNode>> children);

  List<ConstPtr<BehaviorNode>> children;
};

struct RandomizeNode {
  RandomizeNode(List<ConstPtr<BehaviorNode>> children);

  List<ConstPtr<BehaviorNode>> children;
};

struct BehaviorTree {
  BehaviorTree(String const& name, StringSet scripts, JsonObject const& parameters);

  String name;
  StringSet scripts;
  StringSet functions;
  JsonObject parameters;

  ConstPtr<BehaviorNode> root;
};

class BehaviorDatabase {
public:
  BehaviorDatabase();

  [[nodiscard]] auto behaviorTree(String const& name) const -> ConstPtr<BehaviorTree>;
  [[nodiscard]] auto buildTree(Json const& config, StringMap<NodeParameterValue> const& overrides = {}) const -> ConstPtr<BehaviorTree>;
  [[nodiscard]] auto behaviorConfig(String const& name) const -> Json;

private:
  StringMap<Json> m_configs;
  StringMap<ConstPtr<BehaviorTree>> m_behaviors;
  StringMap<StringMap<NodeParameter>> m_nodeParameters;
  StringMap<StringMap<NodeOutput>> m_nodeOutput;

  void loadTree(String const& name);

  // constructs node variants
  auto compositeNode(Json const& config, StringMap<NodeParameter> parameters, StringMap<NodeParameterValue> const& treeParameters, BehaviorTree& tree) const -> CompositeNode;
  auto behaviorNode(Json const& json, StringMap<NodeParameterValue> const& treeParameters, BehaviorTree& tree) const -> ConstPtr<BehaviorNode>;
};

}// namespace Star
