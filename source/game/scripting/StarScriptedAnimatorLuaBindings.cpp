module;
import star.lua_root;
import star.root;
import star.image_metadata_database;
import std;
import star.format;
import star.json;
import star.json_extra;
import star.lua;
import star.random;
import star.periodic_function;
import star.ordered_map;
import star.matrix3;
import star.string;
import star.data_stream;
import star.poly;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.bi_map;
import star.vector;
import star.audio;
import star.map;
import star.set;


// Match client include order for SIMD intrinsics used by xxhash and fast_float.
import star.animated_part_set;
import star.drawable;
import star.animation;
import star.particle;
import star.mixer;
import star.light_source;
import star.networked_animator;
import star.lua_game_converters;

module star.scripted_animator_lua_bindings;

import star.math_common;
namespace Star {

std::expected<LuaCallbacks, StarException> LuaBindings::makeScriptedAnimatorCallbacks(NetworkedAnimator* networkedAnimator, std::function<std::expected<Json, StarException>(String const&, Json const&)> getParameter) {
  LuaCallbacks callbacks;

  if (auto result = callbacks.registerCallback("animationParameter", getParameter); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<std::optional<Vec2F>, String, String>("partPoint", std::bind(&NetworkedAnimator::partPoint, networkedAnimator, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<std::optional<PolyF>, String, String>("partPoly", std::bind(&NetworkedAnimator::partPoly, networkedAnimator, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<Json, String, String, std::optional<String>, std::optional<String>, std::optional<int>>("partProperty", std::bind(&NetworkedAnimator::partProperty, networkedAnimator, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<Json, String, String>("partNextProperty", std::bind(&NetworkedAnimator::partNextProperty, networkedAnimator, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));

  if (auto result = callbacks.registerCallback("transformPoint", [networkedAnimator] (Vec2F point, String const& part) -> std::expected<Vec2F, StarException> {
      auto transformation = networkedAnimator->partTransformation(part);
      if (!transformation) return std::unexpected(std::move(transformation.error()));
      return transformation->transformVec2(point);
    }); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("transformPoly", [networkedAnimator] (PolyF poly, String const& part) -> std::expected<PolyF, StarException> {
      auto transformation = networkedAnimator->partTransformation(part);
      if (!transformation) return std::unexpected(std::move(transformation.error()));
      poly.transform(*transformation);
      return poly;
    }); !result) return std::unexpected(std::move(result.error()));

  if (auto result = callbacks.registerCallbackWithSignature<bool, String, String, bool, bool>(
      "setLocalAnimationState", std::bind(&NetworkedAnimator::setLocalState, networkedAnimator, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<Json, String, String, std::optional<String>, std::optional<int>>(
      "animationStateProperty", std::bind(&NetworkedAnimator::stateProperty, networkedAnimator, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<Json, String, String>(
      "animationStateNextProperty", std::bind(&NetworkedAnimator::stateNextProperty, networkedAnimator, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<String, String>(
      "animationState", std::bind(&NetworkedAnimator::state, networkedAnimator, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<int, String>(
      "animationStateFrame", std::bind(&NetworkedAnimator::stateFrame, networkedAnimator, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<int, String>(
      "animationStateNextFrame", std::bind(&NetworkedAnimator::stateNextFrame, networkedAnimator, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<float, String>(
      "animationStateFrameProgress", std::bind(&NetworkedAnimator::stateFrameProgress, networkedAnimator, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<float, String>(
      "animationStateTimer", std::bind(&NetworkedAnimator::stateTimer, networkedAnimator, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<bool, String>(
      "animationStateReverse", std::bind(&NetworkedAnimator::stateReverse, networkedAnimator, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));

  if (auto result = callbacks.registerCallbackWithSignature<float, String, std::optional<String>>(
      "stateCycle", std::bind(&NetworkedAnimator::stateCycle, networkedAnimator, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<int, String, std::optional<String>>(
      "stateFrames", std::bind(&NetworkedAnimator::stateFrames, networkedAnimator, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));

  if (auto result = callbacks.registerCallbackWithSignature<bool, String, std::optional<String>>(
      "hasState", std::bind(&NetworkedAnimator::hasState, networkedAnimator, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));

  if (auto result = callbacks.registerCallbackWithSignature<bool, String>(
      "hasTransformationGroup", std::bind(&NetworkedAnimator::hasTransformationGroup, networkedAnimator, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));

  if (auto result = callbacks.registerCallbackWithSignature<void, String, Vec2F>("translateLocalTransformationGroup",
      std::bind(&NetworkedAnimator::translateLocalTransformationGroup, networkedAnimator, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("rotateLocalTransformationGroup",
      [networkedAnimator](String const& transformationGroup, float rotation, std::optional<Vec2F> const& rotationCenter) {
        return networkedAnimator->rotateLocalTransformationGroup(transformationGroup, rotation, rotationCenter.value_or(Vec2F{}));
      }); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("rotateDegreesLocalTransformationGroup",
      [networkedAnimator](String const& transformationGroup, float rotation, std::optional<Vec2F> const& rotationCenter) {
        return networkedAnimator->rotateLocalTransformationGroup(transformationGroup, rotation * std::numbers::pi / 180, rotationCenter.value_or(Vec2F{}));
      }); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("scaleLocalTransformationGroup",
      [networkedAnimator](LuaEngine& engine, String const& transformationGroup, LuaValue scale, std::optional<Vec2F> const& scaleCenter) -> std::expected<void, StarException> {
        auto cs = engine.luaMaybeTo<Vec2F>(scale);
        if (!cs) return std::unexpected(std::move(cs.error()));
        if (*cs) return networkedAnimator->scaleLocalTransformationGroup(transformationGroup, **cs, scaleCenter.value_or(Vec2F{}));
        auto scalar = engine.luaTo<float>(scale);
        if (!scalar) return std::unexpected(std::move(scalar.error()));
        return networkedAnimator->scaleLocalTransformationGroup(transformationGroup, *scalar, scaleCenter.value_or(Vec2F{}));
      }); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String, float, float, float, float, float, float>(
      "transformLocalTransformationGroup",
      std::bind(&NetworkedAnimator::transformLocalTransformationGroup, networkedAnimator, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String>(
      "resetLocalTransformationGroup", std::bind(&NetworkedAnimator::resetLocalTransformationGroup, networkedAnimator, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String, Mat3F>(
      "setLocalTransformationGroup", std::bind(&NetworkedAnimator::setLocalTransformationGroup, networkedAnimator, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<Mat3F, String>(
      "getLocalTransformationGroup", std::bind(&NetworkedAnimator::getLocalTransformationGroup, networkedAnimator, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));

  if (auto result = callbacks.registerCallbackWithSignature<void, String, List<Drawable>>(
      "addPartDrawables", std::bind(&NetworkedAnimator::addPartDrawables, networkedAnimator, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String, List<Drawable>>(
      "setPartDrawables", std::bind(&NetworkedAnimator::setPartDrawables, networkedAnimator, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("addPartJsonDrawables",
    [networkedAnimator](LuaEngine& engine, String const& part, JsonArray drawablesConfig) -> std::expected<void, StarException> {
      auto root = luaExecutionRoot(engine);
      if (!root) return std::unexpected(std::move(root.error()));
      auto metadata = (*root)->imageMetadataDatabase();
      if (!metadata) return std::unexpected(std::move(metadata.error()));
      List<Drawable> drawables;
      for (auto const& config : drawablesConfig) {
        auto drawable = Drawable::create(**metadata, config);
        if (!drawable) return std::unexpected(std::move(drawable.error()));
        drawables.append(std::move(*drawable));
      }
      networkedAnimator->addPartDrawables(part, std::move(drawables));
      return {};
    }); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("setPartJsonDrawables",
    [networkedAnimator](LuaEngine& engine, String const& part, JsonArray drawablesConfig) -> std::expected<void, StarException> {
      auto root = luaExecutionRoot(engine);
      if (!root) return std::unexpected(std::move(root.error()));
      auto metadata = (*root)->imageMetadataDatabase();
      if (!metadata) return std::unexpected(std::move(metadata.error()));
      List<Drawable> drawables;
      for (auto const& config : drawablesConfig) {
        auto drawable = Drawable::create(**metadata, config);
        if (!drawable) return std::unexpected(std::move(drawable.error()));
        drawables.append(std::move(*drawable));
      }
      networkedAnimator->setPartDrawables(part, std::move(drawables));
      return {};
    }); !result) return std::unexpected(std::move(result.error()));


  if (auto result = callbacks.registerCallbackWithSignature<String, String, String>(
      "applyPartTags", std::bind(&NetworkedAnimator::applyPartTags, networkedAnimator, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String, std::optional<String>>(
      "setLocalTag", std::bind(&NetworkedAnimator::setLocalTag, networkedAnimator, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));

  if (auto result = callbacks.registerCallback("flipped", [networkedAnimator]() -> bool {
        return networkedAnimator->flipped();
    }); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("flippedRelativeCenterLine", [networkedAnimator]() -> float {
        return networkedAnimator->flippedRelativeCenterLine();
    }); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("animationRate", [networkedAnimator]() -> float {
        return networkedAnimator->animationRate();
    }); !result) return std::unexpected(std::move(result.error()));


  return callbacks;
}

}
