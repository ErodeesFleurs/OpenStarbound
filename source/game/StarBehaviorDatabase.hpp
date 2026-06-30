#pragma once

#include "StarAssets.hpp"
#include "StarGameTypes.hpp"
#include "StarJson.hpp"

namespace Star {

class BehaviorDatabase;
using BehaviorDatabasePtr = SharedPtr<BehaviorDatabase>;
using BehaviorDatabaseConstPtr = SharedPtr<BehaviorDatabase const>;
struct ActionNode;
struct DecoratorNode;
struct SequenceNode;
struct SelectorNode;
struct ParallelNode;
struct DynamicNode;
struct RandomizeNode;
struct BehaviorTree;
using BehaviorTreeConstPtr = SharedPtr<BehaviorTree const>;

using CompositeNode = Variant<SequenceNode, SelectorNode, ParallelNode, DynamicNode, RandomizeNode>;

using BehaviorNode = Variant<ActionNode, DecoratorNode, CompositeNode, BehaviorTreeConstPtr>;
using BehaviorNodeConstPtr = SharedPtr<BehaviorNode const>;

enum class NodeParameterType : uint8_t {
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
using NodeParameter = pair<NodeParameterType, NodeParameterValue>;
using NodeOutput = pair<NodeParameterType, pair<Maybe<String>, bool>>;

[[nodiscard]] NodeParameterValue nodeParameterValueFromJson(Json const& json);

[[nodiscard]] Json jsonFromNodeParameter(NodeParameter const& parameter);
[[nodiscard]] NodeParameter jsonToNodeParameter(Json const& json);

[[nodiscard]] Json jsonFromNodeOutput(NodeOutput const& output);
[[nodiscard]] NodeOutput jsonToNodeOutput(Json const& json);

enum class BehaviorNodeType : uint16_t {
  Action,
  Decorator,
  Composite,
  Module
};
extern EnumMap<BehaviorNodeType> const BehaviorNodeTypeNames;

enum class CompositeType : uint16_t {
  Sequence,
  Selector,
  Parallel,
  Dynamic,
  Randomize
};
extern EnumMap<CompositeType> const CompositeTypeNames;

// replaces global tags in nodeParameters in place
[[nodiscard]] NodeParameterValue replaceBehaviorTag(NodeParameterValue const& parameter, StringMap<NodeParameterValue> const& treeParameters);
[[nodiscard]] Maybe<String> replaceOutputBehaviorTag(Maybe<String> const& output, StringMap<NodeParameterValue> const& treeParameters);
void applyTreeParameters(StringMap<NodeParameter>& nodeParameters, StringMap<NodeParameterValue> const& treeParameters);

struct ActionNode {
  ActionNode(String name, StringMap<NodeParameter> parameters, StringMap<NodeOutput> output);

  String name;
  StringMap<NodeParameter> parameters;
  StringMap<NodeOutput> output;
};

struct DecoratorNode {
  DecoratorNode(String const& name, StringMap<NodeParameter> parameters, BehaviorNodeConstPtr child);

  String name;
  StringMap<NodeParameter> parameters;
  BehaviorNodeConstPtr child;
};

struct SequenceNode {
  SequenceNode(List<BehaviorNodeConstPtr> children);

  List<BehaviorNodeConstPtr> children;
};

struct SelectorNode {
  SelectorNode(List<BehaviorNodeConstPtr> children);

  List<BehaviorNodeConstPtr> children;
};

struct ParallelNode {
  ParallelNode(StringMap<NodeParameter>, List<BehaviorNodeConstPtr> children);

  int succeed;
  int fail;
  List<BehaviorNodeConstPtr> children;
};

struct DynamicNode {
  DynamicNode(List<BehaviorNodeConstPtr> children);

  List<BehaviorNodeConstPtr> children;
};

struct RandomizeNode {
  RandomizeNode(List<BehaviorNodeConstPtr> children);

  List<BehaviorNodeConstPtr> children;
};

struct BehaviorTree {
  BehaviorTree(String const& name, StringSet scripts, JsonObject const& parameters);

  String name;
  StringSet scripts;
  StringSet functions;
  JsonObject parameters;

  BehaviorNodeConstPtr root;
};

class BehaviorDatabase {
public:
  BehaviorDatabase(AssetsConstPtr assets);

  [[nodiscard]] BehaviorTreeConstPtr behaviorTree(String const& name) const;
  [[nodiscard]] BehaviorTreeConstPtr buildTree(Json const& config, StringMap<NodeParameterValue> const& overrides = {}) const;
  [[nodiscard]] Json behaviorConfig(String const& name) const;

private:
  StringMap<Json> m_configs;
  StringMap<BehaviorTreeConstPtr> m_behaviors;
  StringMap<StringMap<NodeParameter>> m_nodeParameters;
  StringMap<StringMap<NodeOutput>> m_nodeOutput;

  void loadTree(String const& name);

  // constructs node variants
  [[nodiscard]] CompositeNode compositeNode(Json const& config, StringMap<NodeParameter> parameters, StringMap<NodeParameterValue> const& treeParameters, BehaviorTree& tree) const;
  [[nodiscard]] BehaviorNodeConstPtr behaviorNode(Json const& json, StringMap<NodeParameterValue> const& treeParameters, BehaviorTree& tree) const;
};

}
