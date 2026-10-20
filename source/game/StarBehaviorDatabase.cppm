module;

import std;
import star.string;
import star.list;
import star.variant;
import star.bi_map;
import star.json;
import star.game_types;

namespace Star {

class Assets;

struct ActionNode;
struct DecoratorNode;
struct SequenceNode;
struct SelectorNode;
struct ParallelNode;
struct DynamicNode;
struct RandomizeNode;
struct BehaviorTree;

typedef Variant<SequenceNode, SelectorNode, ParallelNode, DynamicNode, RandomizeNode> CompositeNode;

typedef Variant<ActionNode, DecoratorNode, CompositeNode, std::shared_ptr<BehaviorTree const>> BehaviorNode;

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
typedef std::pair<NodeParameterType, std::pair<std::optional<String>, bool>> NodeOutput;

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
std::optional<String> replaceOutputBehaviorTag(std::optional<String> const& output, StringMap<NodeParameterValue> const& treeParameters);
void applyTreeParameters(StringMap<NodeParameter>& nodeParameters, StringMap<NodeParameterValue> const& treeParameters);

struct ActionNode {
  ActionNode(String name, StringMap<NodeParameter> parameters, StringMap<NodeOutput> output);

  String name;
  StringMap<NodeParameter> parameters;
  StringMap<NodeOutput> output;
};

struct DecoratorNode {
  DecoratorNode(String const& name, StringMap<NodeParameter> parameters, std::shared_ptr<BehaviorNode const> child);

  String name;
  StringMap<NodeParameter> parameters;
  std::shared_ptr<BehaviorNode const> child;
};

struct SequenceNode {
  SequenceNode(List<std::shared_ptr<BehaviorNode const>> children);

  List<std::shared_ptr<BehaviorNode const>> children;
};

struct SelectorNode {
  SelectorNode(List<std::shared_ptr<BehaviorNode const>> children);

  List<std::shared_ptr<BehaviorNode const>> children;
};

struct ParallelNode {
  ParallelNode(StringMap<NodeParameter>, List<std::shared_ptr<BehaviorNode const>> children);

  int succeed;
  int fail;
  List<std::shared_ptr<BehaviorNode const>> children;
};

struct DynamicNode {
  DynamicNode(List<std::shared_ptr<BehaviorNode const>> children);

  List<std::shared_ptr<BehaviorNode const>> children;
};

struct RandomizeNode {
  RandomizeNode(List<std::shared_ptr<BehaviorNode const>> children);

  List<std::shared_ptr<BehaviorNode const>> children;
};

struct BehaviorTree {
  BehaviorTree(String const& name, StringSet scripts, JsonObject const& parameters);

  String name;
  StringSet scripts;
  StringSet functions;
  JsonObject parameters;

  std::shared_ptr<BehaviorNode const> root;
};

class BehaviorDatabase {
public:
  explicit BehaviorDatabase(std::shared_ptr<Assets const> assets);

  std::shared_ptr<BehaviorTree const> behaviorTree(String const& name) const;
  std::shared_ptr<BehaviorTree const> buildTree(Json const& config, StringMap<NodeParameterValue> const& overrides = {}) const;
  Json behaviorConfig(String const& name) const;

private:
  StringMap<Json> m_configs;
  StringMap<std::shared_ptr<BehaviorTree const>> m_behaviors;
  StringMap<StringMap<NodeParameter>> m_nodeParameters;
  StringMap<StringMap<NodeOutput>> m_nodeOutput;

  void loadTree(String const& name);

  // constructs node variants
  CompositeNode compositeNode(Json const& config, StringMap<NodeParameter> parameters, StringMap<NodeParameterValue> const& treeParameters, BehaviorTree& tree) const;
  std::shared_ptr<BehaviorNode const> behaviorNode(Json const& json, StringMap<NodeParameterValue> const& treeParameters, BehaviorTree& tree) const;
};

}

export module star.behavior_database;

export namespace Star {
  using ::Star::CompositeNode;
  using ::Star::BehaviorNode;
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
  using ::Star::ActionNode;
  using ::Star::DecoratorNode;
  using ::Star::SequenceNode;
  using ::Star::SelectorNode;
  using ::Star::ParallelNode;
  using ::Star::DynamicNode;
  using ::Star::RandomizeNode;
  using ::Star::BehaviorTree;
}
