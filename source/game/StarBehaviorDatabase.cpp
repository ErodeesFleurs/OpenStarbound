#include "StarBehaviorDatabase.hpp"
#include "StarAlgorithm.hpp"
#include "StarJsonExtra.hpp"

namespace Star {

EnumMap<NodeParameterType> const NodeParameterTypeNames {
  {NodeParameterType::Json, "json"},
  {NodeParameterType::Entity, "entity"},
  {NodeParameterType::Position, "position"},
  {NodeParameterType::Vec2, "vec2"},
  {NodeParameterType::Number, "number"},
  {NodeParameterType::Bool, "bool"},
  {NodeParameterType::List, "list"},
  {NodeParameterType::Table, "table"},
  {NodeParameterType::String, "string"}
};

[[nodiscard]] NodeParameterValue nodeParameterValueFromJson(Json const& json) {
  if (auto key = json.optString("key"))
    return *key;
  else
    return json.get("value");
}

[[nodiscard]] Json jsonFromNodeParameter(NodeParameter const& parameter) {
  auto const& [parameterType, parameterValue] = parameter;
  JsonObject json {
    {"type", NodeParameterTypeNames.getRight(parameterType)}
  };
  if (auto key = parameterValue.maybe<String>())
    json.set("key", *key);
  else
    json.set("value", parameterValue.get<Json>());
  return json;
}

[[nodiscard]] NodeParameter jsonToNodeParameter(Json const& json) {
  NodeParameterType type = NodeParameterTypeNames.getLeft(json.getString("type"));
  if (auto key = json.optString("key"))
    return {type, *key};
  else
    return {type, json.opt("value").value(Json())};
}

[[nodiscard]] Json jsonFromNodeOutput(NodeOutput const& output) {
  auto const& [parameterType, outputTarget] = output;
  auto const& [outputKey, ephemeral] = outputTarget;
  return JsonObject {
    {"type", NodeParameterTypeNames.getRight(parameterType)},
    {"key", jsonFromMaybe<String>(outputKey, [](String const& s) { return Json(s); })},
    {"ephemeral", ephemeral}
  };
}

[[nodiscard]] NodeOutput jsonToNodeOutput(Json const& json) {
  return {
    NodeParameterTypeNames.getLeft(json.getString("type")),
    {jsonToMaybe<String>(json.get("key"), [](Json const& j) { return j.toString(); }), json.optBool("ephemeral").value(false)}
  };
}

EnumMap<BehaviorNodeType> const BehaviorNodeTypeNames {
  {BehaviorNodeType::Action, "Action"},
  {BehaviorNodeType::Decorator, "Decorator"},
  {BehaviorNodeType::Composite, "Composite"},
  {BehaviorNodeType::Module, "Module"}
};

EnumMap<CompositeType> const CompositeTypeNames {
  {CompositeType::Sequence, "Sequence"},
  {CompositeType::Selector, "Selector"},
  {CompositeType::Parallel, "Parallel"},
  {CompositeType::Dynamic, "Dynamic"},
  {CompositeType::Randomize, "Randomize"}
};

void applyTreeParameters(StringMap<NodeParameter>& nodeParameters, StringMap<NodeParameterValue> const& treeParameters) {
  for (auto& [parameterName, parameter] : nodeParameters) {
    auto& [parameterType, parameterValue] = parameter;
    parameterValue = replaceBehaviorTag(parameterValue, treeParameters);
  }
}

[[nodiscard]] NodeParameterValue replaceBehaviorTag(NodeParameterValue const& parameter, StringMap<NodeParameterValue> const& treeParameters) {
  Maybe<String> strVal = parameter.maybe<String>();
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

[[nodiscard]] Maybe<String> replaceOutputBehaviorTag(Maybe<String> const& output, StringMap<NodeParameterValue> const& treeParameters) {
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
  for (auto& [parameterName, parameterConfig] : parameters)
    parameterConfig = parameterConfig.opt("key").orMaybe(parameterConfig.opt("value")).value(Json());
}

ActionNode::ActionNode(String name, StringMap<NodeParameter> parameters, StringMap<NodeOutput> output)
  : name(std::move(name)), parameters(std::move(parameters)), output(std::move(output)) { }

DecoratorNode::DecoratorNode(String const& name, StringMap<NodeParameter> parameters, BehaviorNodeConstPtr child)
  : name(name), parameters(parameters), child(child) { }

SequenceNode::SequenceNode(List<BehaviorNodeConstPtr> children) : children(children) { }

SelectorNode::SelectorNode(List<BehaviorNodeConstPtr> children) : children(children) { }

ParallelNode::ParallelNode(StringMap<NodeParameter> parameters, List<BehaviorNodeConstPtr> children) : children(children) {
  auto const& [successType, successValue] = parameters.get("success");
  int s = successValue.get<Json>().optInt().value(-1);
  succeed = s == -1 ? children.size() : s;

  auto const& [failType, failValue] = parameters.get("fail");
  int f = failValue.get<Json>().optInt().value(-1);
  fail = f == -1 ? children.size() : f;
}

DynamicNode::DynamicNode(List<BehaviorNodeConstPtr> children) : children(children) { }

RandomizeNode::RandomizeNode(List<BehaviorNodeConstPtr> children) : children(children) { }

BehaviorTree::BehaviorTree(String const& name, StringSet scripts, JsonObject const& parameters)
  : name(name), scripts(scripts), parameters(parameters) { }

BehaviorDatabase::BehaviorDatabase(AssetsConstPtr assets) {
  assets = requireServiceValueAs<StarException>(std::move(assets), "BehaviorDatabase", "assets");

  auto& nodeFiles = assets->scanExtension("nodes");
  assets->queueJsons(nodeFiles);
  for (String const& file : nodeFiles) {
    try {
      Json nodes = assets->json(file);
      for (auto const& [nodeName, nodeConfig] : nodes.iterateObject()) {
        StringMap<NodeParameter> parameters;
        for (auto const& [parameterName, parameterConfig] : nodeConfig.getObject("properties", {}))
          parameters.set(parameterName, jsonToNodeParameter(parameterConfig));

        m_nodeParameters.set(nodeName, parameters);

        StringMap<NodeOutput> output;
        for (auto const& [outputName, outputConfig] : nodeConfig.getObject("output", {}))
          output.set(outputName, jsonToNodeOutput(outputConfig));

        m_nodeOutput.set(nodeName, output);
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

  for (auto const& [behaviorName, behaviorConfig] : m_configs) {
    if (!m_behaviors.contains(behaviorName))
      loadTree(behaviorName);
  }
}

[[nodiscard]] BehaviorTreeConstPtr BehaviorDatabase::behaviorTree(String const& name) const {
  if (!m_behaviors.contains(name))
    throw StarException(strf("No such behavior tree \'{}\'", name));

  return m_behaviors.get(name);
}

[[nodiscard]] BehaviorTreeConstPtr BehaviorDatabase::buildTree(Json const& config, StringMap<NodeParameterValue> const& overrides) const {
  StringSet scripts = jsonToStringSet(config.get("scripts", JsonArray()));
  auto tree = BehaviorTree(config.getString("name"), scripts, config.getObject("parameters", {}));

  StringMap<NodeParameterValue> parameters;
  for (auto const& [parameterName, parameterValue] : config.getObject("parameters", {}))
    parameters.set(parameterName, parameterValue);
  for (auto const& [parameterName, parameterValue] : overrides)
    parameters.set(parameterName, parameterValue);
  BehaviorNodeConstPtr root = behaviorNode(config.get("root"), parameters, tree);
  tree.root = root;
  return std::make_shared<BehaviorTree>(std::move(tree));
}

[[nodiscard]] Json BehaviorDatabase::behaviorConfig(String const& name) const {
  if (!m_configs.contains(name))
    throw StarException(strf("No such behavior tree \'{}\'", name));

  return m_configs.get(name);
}

void BehaviorDatabase::loadTree(String const& name) {
  m_behaviors.set(name, buildTree(m_configs.get(name)));
}

[[nodiscard]] CompositeNode BehaviorDatabase::compositeNode(Json const& config, StringMap<NodeParameter> parameters, StringMap<NodeParameterValue> const& treeParameters, BehaviorTree& tree) const {
  List<BehaviorNodeConstPtr> children = config.getArray("children", {}).transformed([this,treeParameters,&tree](Json const& child) {
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

[[nodiscard]] BehaviorNodeConstPtr BehaviorDatabase::behaviorNode(Json const& json, StringMap<NodeParameterValue> const& treeParameters, BehaviorTree& tree) const {
  BehaviorNodeType type = BehaviorNodeTypeNames.getLeft(json.getString("type"));

  auto name = json.getString("name");
  auto parameterConfig = json.getObject("parameters", {});

  if (type == BehaviorNodeType::Module) {
      // merge in module parameters to a copy of the treeParameters to propagate
      // tree parameters into the sub-tree, but allow modules to override
      auto moduleParameters = treeParameters;
      for (auto const& [parameterName, parameterValue] : parameterConfig)
        moduleParameters.set(parameterName, replaceBehaviorTag(nodeParameterValueFromJson(parameterValue), treeParameters));

      BehaviorTree module = *buildTree(m_configs.get(name), moduleParameters);
      tree.scripts.addAll(module.scripts);
      tree.functions.addAll(module.functions);

      return module.root;
  }

  StringMap<NodeParameter> parameters = m_nodeParameters.get(name);
  for (auto& [parameterName, parameter] : parameters) {
    auto& [parameterType, parameterValue] = parameter;
    parameterValue = parameterConfig.maybe(parameterName).apply(nodeParameterValueFromJson).value(parameterValue);
  }
  applyTreeParameters(parameters, treeParameters);

  if (type == BehaviorNodeType::Action) {
    tree.functions.add(name);

    Json outputConfig = json.getObject("output", {});
    StringMap<NodeOutput> output = m_nodeOutput.get(name);
    for (auto& [outputName, nodeOutput] : output) {
      auto& [parameterType, outputTarget] = nodeOutput;
      auto& [outputKey, ephemeral] = outputTarget;
      outputKey = replaceOutputBehaviorTag(outputConfig.optString(outputName).orMaybe(outputKey), treeParameters);
    }

    return make_shared<BehaviorNode>(ActionNode(name, parameters, output));
  } else if (type == BehaviorNodeType::Decorator) {
    tree.functions.add(name);
    BehaviorNodeConstPtr child = behaviorNode(json.get("child"), treeParameters, tree);
    return make_shared<BehaviorNode>(DecoratorNode(name, parameters, child));
  } else if (type == BehaviorNodeType::Composite) {
    return make_shared<BehaviorNode>(compositeNode(json, parameters, treeParameters, tree));
  }

  // above statement must be exhaustive
  throw StarException(strf("Behavior node type '{}' could not be created from JSON", BehaviorNodeTypeNames.getRight(type)));
}

}
