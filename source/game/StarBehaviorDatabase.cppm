module;

import std;
import star.string;
import star.list;
import star.variant;
import star.maybe;
import star.bi_map;
import star.json;
import star.game_types;

namespace Star {

class BehaviorDatabase;
using BehaviorDatabasePtr = std::shared_ptr<BehaviorDatabase>;
using BehaviorDatabaseConstPtr = std::shared_ptr<const BehaviorDatabase>;
using BehaviorDatabaseWeakPtr = std::weak_ptr<BehaviorDatabase>;
using BehaviorDatabaseConstWeakPtr = std::weak_ptr<const BehaviorDatabase>;
using BehaviorDatabaseUPtr = std::unique_ptr<BehaviorDatabase>;
using BehaviorDatabaseConstUPtr = std::unique_ptr<const BehaviorDatabase>;
struct ActionNode;
using ActionNodePtr = std::shared_ptr<ActionNode>;
using ActionNodeConstPtr = std::shared_ptr<const ActionNode>;
using ActionNodeWeakPtr = std::weak_ptr<ActionNode>;
using ActionNodeConstWeakPtr = std::weak_ptr<const ActionNode>;
using ActionNodeUPtr = std::unique_ptr<ActionNode>;
using ActionNodeConstUPtr = std::unique_ptr<const ActionNode>;
struct DecoratorNode;
using DecoratorNodePtr = std::shared_ptr<DecoratorNode>;
using DecoratorNodeConstPtr = std::shared_ptr<const DecoratorNode>;
using DecoratorNodeWeakPtr = std::weak_ptr<DecoratorNode>;
using DecoratorNodeConstWeakPtr = std::weak_ptr<const DecoratorNode>;
using DecoratorNodeUPtr = std::unique_ptr<DecoratorNode>;
using DecoratorNodeConstUPtr = std::unique_ptr<const DecoratorNode>;
struct SequenceNode;
using SequenceNodePtr = std::shared_ptr<SequenceNode>;
using SequenceNodeConstPtr = std::shared_ptr<const SequenceNode>;
using SequenceNodeWeakPtr = std::weak_ptr<SequenceNode>;
using SequenceNodeConstWeakPtr = std::weak_ptr<const SequenceNode>;
using SequenceNodeUPtr = std::unique_ptr<SequenceNode>;
using SequenceNodeConstUPtr = std::unique_ptr<const SequenceNode>;
struct SelectorNode;
using SelectorNodePtr = std::shared_ptr<SelectorNode>;
using SelectorNodeConstPtr = std::shared_ptr<const SelectorNode>;
using SelectorNodeWeakPtr = std::weak_ptr<SelectorNode>;
using SelectorNodeConstWeakPtr = std::weak_ptr<const SelectorNode>;
using SelectorNodeUPtr = std::unique_ptr<SelectorNode>;
using SelectorNodeConstUPtr = std::unique_ptr<const SelectorNode>;
struct ParallelNode;
using ParallelNodePtr = std::shared_ptr<ParallelNode>;
using ParallelNodeConstPtr = std::shared_ptr<const ParallelNode>;
using ParallelNodeWeakPtr = std::weak_ptr<ParallelNode>;
using ParallelNodeConstWeakPtr = std::weak_ptr<const ParallelNode>;
using ParallelNodeUPtr = std::unique_ptr<ParallelNode>;
using ParallelNodeConstUPtr = std::unique_ptr<const ParallelNode>;
struct DynamicNode;
using DynamicNodePtr = std::shared_ptr<DynamicNode>;
using DynamicNodeConstPtr = std::shared_ptr<const DynamicNode>;
using DynamicNodeWeakPtr = std::weak_ptr<DynamicNode>;
using DynamicNodeConstWeakPtr = std::weak_ptr<const DynamicNode>;
using DynamicNodeUPtr = std::unique_ptr<DynamicNode>;
using DynamicNodeConstUPtr = std::unique_ptr<const DynamicNode>;
struct RandomizeNode;
using RandomizeNodePtr = std::shared_ptr<RandomizeNode>;
using RandomizeNodeConstPtr = std::shared_ptr<const RandomizeNode>;
using RandomizeNodeWeakPtr = std::weak_ptr<RandomizeNode>;
using RandomizeNodeConstWeakPtr = std::weak_ptr<const RandomizeNode>;
using RandomizeNodeUPtr = std::unique_ptr<RandomizeNode>;
using RandomizeNodeConstUPtr = std::unique_ptr<const RandomizeNode>;
struct BehaviorTree;
using BehaviorTreePtr = std::shared_ptr<BehaviorTree>;
using BehaviorTreeConstPtr = std::shared_ptr<const BehaviorTree>;
using BehaviorTreeWeakPtr = std::weak_ptr<BehaviorTree>;
using BehaviorTreeConstWeakPtr = std::weak_ptr<const BehaviorTree>;
using BehaviorTreeUPtr = std::unique_ptr<BehaviorTree>;
using BehaviorTreeConstUPtr = std::unique_ptr<const BehaviorTree>;

typedef Variant<SequenceNode, SelectorNode, ParallelNode, DynamicNode, RandomizeNode> CompositeNode;

typedef Variant<ActionNode, DecoratorNode, CompositeNode, BehaviorTreeConstPtr> BehaviorNode;
typedef std::shared_ptr<const BehaviorNode> BehaviorNodeConstPtr;

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

typedef Variant<String, Json> NodeParameterValue;
typedef std::pair<NodeParameterType, NodeParameterValue> NodeParameter;
typedef std::pair<NodeParameterType, std::pair<Maybe<String>, bool>> NodeOutput;

NodeParameterValue nodeParameterValueFromJson(Json const& json);

Json jsonFromNodeParameter(NodeParameter const& parameter);
NodeParameter jsonToNodeParameter(Json const& json);

Json jsonFromNodeOutput(NodeOutput const& output);
NodeOutput jsonToNodeOutput(Json const& json);

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
NodeParameterValue replaceBehaviorTag(NodeParameterValue const& parameter, StringMap<NodeParameterValue> const& treeParameters);
Maybe<String> replaceOutputBehaviorTag(Maybe<String> const& output, StringMap<NodeParameterValue> const& treeParameters);
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

typedef std::shared_ptr<const BehaviorNode> BehaviorNodeConstPtr;

class BehaviorDatabase {
public:
  BehaviorDatabase();

  BehaviorTreeConstPtr behaviorTree(String const& name) const;
  BehaviorTreeConstPtr buildTree(Json const& config, StringMap<NodeParameterValue> const& overrides = {}) const;
  Json behaviorConfig(String const& name) const;

private:
  StringMap<Json> m_configs;
  StringMap<BehaviorTreeConstPtr> m_behaviors;
  StringMap<StringMap<NodeParameter>> m_nodeParameters;
  StringMap<StringMap<NodeOutput>> m_nodeOutput;

  void loadTree(String const& name);

  // constructs node variants
  CompositeNode compositeNode(Json const& config, StringMap<NodeParameter> parameters, StringMap<NodeParameterValue> const& treeParameters, BehaviorTree& tree) const;
  BehaviorNodeConstPtr behaviorNode(Json const& json, StringMap<NodeParameterValue> const& treeParameters, BehaviorTree& tree) const;
};

}

export module star.behavior_database;

export namespace Star {
  using ::Star::CompositeNode;
  using ::Star::BehaviorNode;
  using ::Star::BehaviorNodeConstPtr;
  using ::Star::NodeParameterType;
  using ::Star::NodeParameterTypeNames;
  using ::Star::NodeParameterValue;
  using ::Star::NodeParameter;
  using ::Star::NodeOutput;
  using ::Star::nodeParameterValueFromJson;
  using ::Star::jsonFromNodeParameter;
  using ::Star::jsonToNodeParameter;
  using ::Star::jsonFromNodeOutput;
  using ::Star::jsonToNodeOutput;
  using ::Star::BehaviorNodeType;
  using ::Star::BehaviorNodeTypeNames;
  using ::Star::CompositeType;
  using ::Star::CompositeTypeNames;
  using ::Star::replaceBehaviorTag;
  using ::Star::replaceOutputBehaviorTag;
  using ::Star::applyTreeParameters;
  using ::Star::BehaviorDatabase;
  using ::Star::BehaviorDatabasePtr;
  using ::Star::BehaviorDatabaseConstPtr;
  using ::Star::BehaviorDatabaseWeakPtr;
  using ::Star::BehaviorDatabaseConstWeakPtr;
  using ::Star::BehaviorDatabaseUPtr;
  using ::Star::BehaviorDatabaseConstUPtr;
  using ::Star::ActionNode;
  using ::Star::ActionNodePtr;
  using ::Star::ActionNodeConstPtr;
  using ::Star::ActionNodeWeakPtr;
  using ::Star::ActionNodeConstWeakPtr;
  using ::Star::ActionNodeUPtr;
  using ::Star::ActionNodeConstUPtr;
  using ::Star::DecoratorNode;
  using ::Star::DecoratorNodePtr;
  using ::Star::DecoratorNodeConstPtr;
  using ::Star::DecoratorNodeWeakPtr;
  using ::Star::DecoratorNodeConstWeakPtr;
  using ::Star::DecoratorNodeUPtr;
  using ::Star::DecoratorNodeConstUPtr;
  using ::Star::SequenceNode;
  using ::Star::SequenceNodePtr;
  using ::Star::SequenceNodeConstPtr;
  using ::Star::SequenceNodeWeakPtr;
  using ::Star::SequenceNodeConstWeakPtr;
  using ::Star::SequenceNodeUPtr;
  using ::Star::SequenceNodeConstUPtr;
  using ::Star::SelectorNode;
  using ::Star::SelectorNodePtr;
  using ::Star::SelectorNodeConstPtr;
  using ::Star::SelectorNodeWeakPtr;
  using ::Star::SelectorNodeConstWeakPtr;
  using ::Star::SelectorNodeUPtr;
  using ::Star::SelectorNodeConstUPtr;
  using ::Star::ParallelNode;
  using ::Star::ParallelNodePtr;
  using ::Star::ParallelNodeConstPtr;
  using ::Star::ParallelNodeWeakPtr;
  using ::Star::ParallelNodeConstWeakPtr;
  using ::Star::ParallelNodeUPtr;
  using ::Star::ParallelNodeConstUPtr;
  using ::Star::DynamicNode;
  using ::Star::DynamicNodePtr;
  using ::Star::DynamicNodeConstPtr;
  using ::Star::DynamicNodeWeakPtr;
  using ::Star::DynamicNodeConstWeakPtr;
  using ::Star::DynamicNodeUPtr;
  using ::Star::DynamicNodeConstUPtr;
  using ::Star::RandomizeNode;
  using ::Star::RandomizeNodePtr;
  using ::Star::RandomizeNodeConstPtr;
  using ::Star::RandomizeNodeWeakPtr;
  using ::Star::RandomizeNodeConstWeakPtr;
  using ::Star::RandomizeNodeUPtr;
  using ::Star::RandomizeNodeConstUPtr;
  using ::Star::BehaviorTree;
  using ::Star::BehaviorTreePtr;
  using ::Star::BehaviorTreeConstPtr;
  using ::Star::BehaviorTreeWeakPtr;
  using ::Star::BehaviorTreeConstWeakPtr;
  using ::Star::BehaviorTreeUPtr;
  using ::Star::BehaviorTreeConstUPtr;
}
