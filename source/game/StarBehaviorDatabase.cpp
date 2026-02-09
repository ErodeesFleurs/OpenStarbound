#include "StarBehaviorDatabase.hpp"

#include "StarConfig.hpp"
#include "StarJsonExtra.hpp"
#include "StarRoot.hpp"

import std;

namespace Star {

EnumMap<NodeParameterType> const NodeParameterTypeNames{
  {NodeParameterType::Json, "json"},
  {NodeParameterType::Entity, "entity"},
  {NodeParameterType::Position, "position"},
  {NodeParameterType::Vec2, "vec2"},
  {NodeParameterType::Number, "number"},
  {NodeParameterType::Bool, "bool"},
  {NodeParameterType::List, "list"},
  {NodeParameterType::Table, "table"},
  {NodeParameterType::String, "string"}};

auto nodeParameterValueFromJson(Json const& json) -> NodeParameterValue {
  if (auto key = json.optString("key"))
    return *key;
  else
    return json.get("value");
}

auto jsonFromNodeParameter(NodeParameter const& parameter) -> Json {
  JsonObject json{
    {"type", NodeParameterTypeNames.getRight(parameter.first)}};
  if (auto key = parameter.second.maybe<String>())
    json.set("key", *key);
  else
    json.set("value", parameter.second.get<Json>());
  return json;
}

auto jsonToNodeParameter(Json const& json) -> NodeParameter {
  NodeParameterType type = NodeParameterTypeNames.getLeft(json.getString("type"));
  if (auto key = json.optString("key"))
    return {type, *key};
  else
    return {type, json.opt("value").value_or(Json())};
}

auto nodeOutputToJson(NodeOutput const& output) -> Json {
  return JsonObject{
    {"type", NodeParameterTypeNames.getRight(output.first)},
    {"key", jsonFromMaybe<String>(output.second.first, [](String const& s) -> Json { return {s}; })},
    {"ephemeral", output.second.second}};
}

auto jsonToNodeOutput(Json const& json) -> NodeOutput {
  return {
    NodeParameterTypeNames.getLeft(json.getString("type")),
    {jsonToMaybe<String>(json.get("key"), [](Json const& j) -> String { return j.toString(); }), json.optBool("ephemeral").value_or(false)}};
}

EnumMap<BehaviorNodeType> const BehaviorNodeTypeNames{
  {BehaviorNodeType::Action, "Action"},
  {BehaviorNodeType::Decorator, "Decorator"},
  {BehaviorNodeType::Composite, "Composite"},
  {BehaviorNodeType::Module, "Module"}};

EnumMap<CompositeType> const CompositeTypeNames{
  {CompositeType::Sequence, "Sequence"},
  {CompositeType::Selector, "Selector"},
  {CompositeType::Parallel, "Parallel"},
  {CompositeType::Dynamic, "Dynamic"},
  {CompositeType::Randomize, "Randomize"}};

void applyTreeParameters(StringMap<NodeParameter>& nodeParameters, StringMap<NodeParameterValue> const& treeParameters) {
  for (auto& p : nodeParameters) {
    NodeParameter& parameter = p.second;
    parameter.second = replaceBehaviorTag(parameter.second, treeParameters);
  }
}

auto replaceBehaviorTag(NodeParameterValue const& parameter, StringMap<NodeParameterValue> const& treeParameters) -> NodeParameterValue {
  std::optional<String> strVal = parameter.maybe<String>();
  if (!strVal && parameter.get<Json>().isType(Json::Type::String))
    strVal = parameter.get<Json>().toString();
  if (strVal) {
    String key = *strVal;
    if (key.beginsWith('<') && key.endsWith('>')) {
      String treeKey = key.substr(1, key.size() - 2);

      if (auto replace = treeParameters.maybe(treeKey)) {
        return *replace;
      } else {
        throw StarException(strf("No parameter specified for tag '{}'", key));
      }
    }
  }
  return parameter;
}

auto replaceOutputBehaviorTag(std::optional<String> const& output, StringMap<NodeParameterValue> const& treeParameters) -> std::optional<String> {
  if (auto out = output) {
    if (out->beginsWith('<') && out->endsWith('>')) {
      if (auto replace = treeParameters.maybe(out->substr(1, out->size() - 2))) {
        if (auto key = replace->maybe<String>())
          return *key;
        else if (replace->get<Json>().isType(Json::Type::String))
          return replace->get<Json>().toString();
        else
          return {};
      } else {
        throw StarException(strf("No parameter specified for tag '{}'", *out));
      }
    }
  }
  return output;
}

// TODO: This is temporary until BehaviorState can handle valueType:value pairs
void parseNodeParameters(JsonObject& parameters) {
  for (auto& p : parameters)
    p.second = p.second.opt("key").or_else([&]() -> std::optional<Json> { return p.second.opt("value"); }).value_or(Json());
}

ActionNode::ActionNode(String name, StringMap<NodeParameter> parameters, StringMap<NodeOutput> output)
    : name(std::move(name)), parameters(std::move(parameters)), output(std::move(output)) {}

DecoratorNode::DecoratorNode(String const& name, StringMap<NodeParameter> parameters, ConstPtr<BehaviorNode> child)
    : name(std::move(name)), parameters(std::move(parameters)), child(std::move(child)) {}

SequenceNode::SequenceNode(List<ConstPtr<BehaviorNode>> children) : children(std::move(children)) {}

SelectorNode::SelectorNode(List<ConstPtr<BehaviorNode>> children) : children(std::move(children)) {}

ParallelNode::ParallelNode(StringMap<NodeParameter> parameters, List<ConstPtr<BehaviorNode>> children) : children(children) {
  int s = parameters.get("success").second.get<Json>().optInt().value_or(-1);
  succeed = s == -1 ? children.size() : s;

  int f = parameters.get("fail").second.get<Json>().optInt().value_or(-1);
  fail = f == -1 ? children.size() : f;
}

DynamicNode::DynamicNode(List<ConstPtr<BehaviorNode>> children) : children(std::move(children)) {}

RandomizeNode::RandomizeNode(List<ConstPtr<BehaviorNode>> children) : children(std::move(children)) {}

BehaviorTree::BehaviorTree(String const& name, StringSet scripts, JsonObject const& parameters)
    : name(std::move(name)), scripts(std::move(scripts)), parameters(std::move(parameters)) {}

BehaviorDatabase::BehaviorDatabase() {
  auto assets = Root::singleton().assets();

  auto& nodeFiles = assets->scanExtension("nodes");
  assets->queueJsons(nodeFiles);
  for (String const& file : nodeFiles) {
    try {
      Json nodes = assets->json(file);
      for (auto& node : nodes.toObject()) {
        StringMap<NodeParameter> parameters;
        for (auto p : node.second.getObject("properties", {}))
          parameters.set(p.first, jsonToNodeParameter(p.second));

        m_nodeParameters.set(node.first, parameters);

        StringMap<NodeOutput> output;
        for (auto p : node.second.getObject("output", {}))
          output.set(p.first, jsonToNodeOutput(p.second));

        m_nodeOutput.set(node.first, output);
      }
    } catch (StarException const& e) {
      throw StarException(strf("Could not load nodes file \'{}\'", file), e);
    }
  }

  auto& behaviorFiles = assets->scanExtension("behavior");
  assets->queueJsons(behaviorFiles);
  for (auto const& file : behaviorFiles) {
    try {
      auto config = assets->json(file);
      auto name = config.getString("name");

      if (m_configs.contains(name))
        throw StarException(strf("Duplicate behavior tree \'{}\'", name));

      m_configs[name] = config;
    } catch (StarException const& e) {
      throw StarException(strf("Could not load behavior file \'{}\'", file), e);
    }
  }

  for (auto& pair : m_configs) {
    if (!m_behaviors.contains(pair.first))
      loadTree(pair.first);
  }
}

auto BehaviorDatabase::behaviorTree(String const& name) const -> ConstPtr<BehaviorTree> {
  if (!m_behaviors.contains(name))
    throw StarException(strf("No such behavior tree \'{}\'", name));

  return m_behaviors.get(name);
}

auto BehaviorDatabase::buildTree(Json const& config, StringMap<NodeParameterValue> const& overrides) const -> ConstPtr<BehaviorTree> {
  StringSet scripts = jsonToStringSet(config.get("scripts", JsonArray()));
  auto tree = BehaviorTree(config.getString("name"), scripts, config.getObject("parameters", {}));

  StringMap<NodeParameterValue> parameters;
  for (auto p : config.getObject("parameters", {}))
    parameters.set(p.first, p.second);
  for (auto p : overrides)
    parameters.set(p.first, p.second);
  ConstPtr<BehaviorNode> root = behaviorNode(config.get("root"), parameters, tree);
  tree.root = root;
  return std::make_shared<BehaviorTree>(std::move(tree));
}

auto BehaviorDatabase::behaviorConfig(String const& name) const -> Json {
  if (!m_configs.contains(name))
    throw StarException(strf("No such behavior tree \'{}\'", name));

  return m_configs.get(name);
}

void BehaviorDatabase::loadTree(String const& name) {
  m_behaviors.set(name, buildTree(m_configs.get(name)));
}

auto BehaviorDatabase::compositeNode(Json const& config, StringMap<NodeParameter> parameters, StringMap<NodeParameterValue> const& treeParameters, BehaviorTree& tree) const -> CompositeNode {
  List<ConstPtr<BehaviorNode>> children = config.getArray("children", {}).transformed([this, treeParameters, &tree](Json const& child) -> ConstPtr<BehaviorNode> {
    return behaviorNode(child, treeParameters, tree);
  });

  CompositeType type = CompositeTypeNames.getLeft(config.getString("name"));
  if (type == CompositeType::Sequence)
    return SequenceNode(children);
  else if (type == CompositeType::Selector)
    return SelectorNode(children);
  else if (type == CompositeType::Parallel)
    return ParallelNode(parameters, children);
  else if (type == CompositeType::Dynamic)
    return DynamicNode(children);
  else if (type == CompositeType::Randomize)
    return RandomizeNode(children);

  // above statement needs to be exhaustive
  throw StarException(strf("Composite node type '{}' could not be created from JSON", CompositeTypeNames.getRight(type)));
}

auto BehaviorDatabase::behaviorNode(Json const& json, StringMap<NodeParameterValue> const& treeParameters, BehaviorTree& tree) const -> ConstPtr<BehaviorNode> {
  BehaviorNodeType type = BehaviorNodeTypeNames.getLeft(json.getString("type"));

  auto name = json.getString("name");
  auto parameterConfig = json.getObject("parameters", {});

  if (type == BehaviorNodeType::Module) {
    // merge in module parameters to a copy of the treeParameters to propagate
    // tree parameters into the sub-tree, but allow modules to override
    auto moduleParameters = treeParameters;
    for (auto p : parameterConfig)
      moduleParameters.set(p.first, replaceBehaviorTag(nodeParameterValueFromJson(p.second), treeParameters));

    BehaviorTree module = *buildTree(m_configs.get(name), moduleParameters);
    tree.scripts.addAll(module.scripts);
    tree.functions.addAll(module.functions);

    return module.root;
  }

  StringMap<NodeParameter> parameters = m_nodeParameters.get(name);
  for (auto& p : parameters)
    p.second.second = parameterConfig.maybe(p.first).transform(nodeParameterValueFromJson).value_or(p.second.second);
  applyTreeParameters(parameters, treeParameters);

  if (type == BehaviorNodeType::Action) {
    tree.functions.add(name);

    Json outputConfig = json.getObject("output", {});
    StringMap<NodeOutput> output = m_nodeOutput.get(name);
    for (auto& p : output)
      p.second.second.first = replaceOutputBehaviorTag(outputConfig.optString(p.first).or_else([&]() -> std::optional<Star::String> { return p.second.second.first; }), treeParameters);

    return std::make_shared<BehaviorNode>(ActionNode(name, parameters, output));
  } else if (type == BehaviorNodeType::Decorator) {
    tree.functions.add(name);
    ConstPtr<BehaviorNode> child = behaviorNode(json.get("child"), treeParameters, tree);
    return std::make_shared<BehaviorNode>(DecoratorNode(name, parameters, child));
  } else if (type == BehaviorNodeType::Composite) {
    return std::make_shared<BehaviorNode>(compositeNode(json, parameters, treeParameters, tree));
  }

  // above statement must be exhaustive
  throw StarException(strf("Behavior node type '{}' could not be created from JSON", BehaviorNodeTypeNames.getRight(type)));
}

}// namespace Star
