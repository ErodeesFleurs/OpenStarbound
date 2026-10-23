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
import star.rect;
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

module star.networked_animator_lua_bindings;

import star.math_common;
namespace Star {

namespace {
std::expected<void, StarException> applyAnimatorJsonDrawables(NetworkedAnimator* networkedAnimator,
    void (NetworkedAnimator::*apply)(String const&, List<Drawable>), LuaEngine& engine,
    String const& part, JsonArray const& drawablesConfig) {
  // Keep this execution scope alive through the mutation, including empty lists.
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
  (networkedAnimator->*apply)(part, std::move(drawables));
  return {};
}
}

std::expected<void, StarException> LuaBindings::Detail::registerAnimatorStateCallbacks(LuaCallbacks& callbacks, NetworkedAnimator* networkedAnimator, AnimatorStateCallbackOrder order) {
  // Preserve each table's historical order for these shared callbacks.
  if (auto result = callbacks.registerCallbackWithSignature<bool, String, String, bool, bool>(
      "setLocalAnimationState", std::bind_front(&NetworkedAnimator::setLocalState, networkedAnimator)); !result) return std::unexpected(std::move(result.error()));
  if (order == AnimatorStateCallbackOrder::Networked) {
    if (auto result = callbacks.registerCallbackWithSignature<String, String>(
        "animationState", std::bind_front(&NetworkedAnimator::state, networkedAnimator)); !result) return std::unexpected(std::move(result.error()));
  }
  if (auto result = callbacks.registerCallbackWithSignature<Json, String, String, std::optional<String>, std::optional<int>>(
      "animationStateProperty", std::bind_front(&NetworkedAnimator::stateProperty, networkedAnimator)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<Json, String, String>(
      "animationStateNextProperty", std::bind_front(&NetworkedAnimator::stateNextProperty, networkedAnimator)); !result) return std::unexpected(std::move(result.error()));
  if (order == AnimatorStateCallbackOrder::Scripted) {
    if (auto result = callbacks.registerCallbackWithSignature<String, String>(
        "animationState", std::bind_front(&NetworkedAnimator::state, networkedAnimator)); !result) return std::unexpected(std::move(result.error()));
  }
  if (auto result = callbacks.registerCallbackWithSignature<int, String>(
      "animationStateFrame", std::bind_front(&NetworkedAnimator::stateFrame, networkedAnimator)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<int, String>(
      "animationStateNextFrame", std::bind_front(&NetworkedAnimator::stateNextFrame, networkedAnimator)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<float, String>(
      "animationStateFrameProgress", std::bind_front(&NetworkedAnimator::stateFrameProgress, networkedAnimator)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<float, String>(
      "animationStateTimer", std::bind_front(&NetworkedAnimator::stateTimer, networkedAnimator)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<bool, String>(
      "animationStateReverse", std::bind_front(&NetworkedAnimator::stateReverse, networkedAnimator)); !result) return std::unexpected(std::move(result.error()));
  if (order == AnimatorStateCallbackOrder::Networked) {
    if (auto result = callbacks.registerCallbackWithSignature<bool, String, std::optional<String>>(
        "hasState", std::bind_front(&NetworkedAnimator::hasState, networkedAnimator)); !result) return std::unexpected(std::move(result.error()));
  }
  if (auto result = callbacks.registerCallbackWithSignature<float, String, std::optional<String>>(
      "stateCycle", std::bind_front(&NetworkedAnimator::stateCycle, networkedAnimator)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<int, String, std::optional<String>>(
      "stateFrames", std::bind_front(&NetworkedAnimator::stateFrames, networkedAnimator)); !result) return std::unexpected(std::move(result.error()));
  if (order == AnimatorStateCallbackOrder::Scripted) {
    if (auto result = callbacks.registerCallbackWithSignature<bool, String, std::optional<String>>(
        "hasState", std::bind_front(&NetworkedAnimator::hasState, networkedAnimator)); !result) return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> LuaBindings::Detail::registerAnimatorLocalTransformationCallbacks(LuaCallbacks& callbacks, NetworkedAnimator* networkedAnimator) {
  if (auto result = callbacks.registerCallbackWithSignature<void, String, Vec2F>("translateLocalTransformationGroup",
      std::bind_front(&NetworkedAnimator::translateLocalTransformationGroup, networkedAnimator)); !result) return std::unexpected(std::move(result.error()));
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
      std::bind_front(&NetworkedAnimator::transformLocalTransformationGroup, networkedAnimator)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String>(
      "resetLocalTransformationGroup", std::bind_front(&NetworkedAnimator::resetLocalTransformationGroup, networkedAnimator)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String, Mat3F>(
      "setLocalTransformationGroup", std::bind_front(&NetworkedAnimator::setLocalTransformationGroup, networkedAnimator)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<Mat3F, String>(
      "getLocalTransformationGroup", std::bind_front(&NetworkedAnimator::getLocalTransformationGroup, networkedAnimator)); !result) return std::unexpected(std::move(result.error()));
  return {};
}

std::expected<void, StarException> LuaBindings::Detail::registerAnimatorPartQueryCallbacks(LuaCallbacks& callbacks, NetworkedAnimator* networkedAnimator) {
  if (auto result = callbacks.registerCallbackWithSignature<std::optional<Vec2F>, String, String>("partPoint", std::bind_front(&NetworkedAnimator::partPoint, networkedAnimator)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<std::optional<PolyF>, String, String>("partPoly", std::bind_front(&NetworkedAnimator::partPoly, networkedAnimator)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<Json, String, String, std::optional<String>, std::optional<String>, std::optional<int>>("partProperty", std::bind_front(&NetworkedAnimator::partProperty, networkedAnimator)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<Json, String, String>("partNextProperty", std::bind_front(&NetworkedAnimator::partNextProperty, networkedAnimator)); !result) return std::unexpected(std::move(result.error()));

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
  return {};
}

std::expected<void, StarException> LuaBindings::Detail::registerAnimatorPlaybackQueryCallbacks(LuaCallbacks& callbacks, NetworkedAnimator* networkedAnimator) {
  if (auto result = callbacks.registerCallback("flipped", [networkedAnimator]() -> bool {
        return networkedAnimator->flipped();
    }); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("flippedRelativeCenterLine", [networkedAnimator]() -> float {
        return networkedAnimator->flippedRelativeCenterLine();
    }); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("animationRate", [networkedAnimator]() -> float {
        return networkedAnimator->animationRate();
    }); !result) return std::unexpected(std::move(result.error()));
  return {};
}

std::expected<void, StarException> LuaBindings::Detail::registerAnimatorDrawableCallbacks(LuaCallbacks& callbacks, NetworkedAnimator* networkedAnimator) {
  if (auto result = callbacks.registerCallbackWithSignature<void, String, List<Drawable>>(
      "addPartDrawables", std::bind_front(&NetworkedAnimator::addPartDrawables, networkedAnimator)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String, List<Drawable>>(
      "setPartDrawables", std::bind_front(&NetworkedAnimator::setPartDrawables, networkedAnimator)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("addPartJsonDrawables",
    [networkedAnimator](LuaEngine& engine, String const& part, JsonArray drawablesConfig) -> std::expected<void, StarException> {
      return applyAnimatorJsonDrawables(networkedAnimator, &NetworkedAnimator::addPartDrawables, engine, part, drawablesConfig);
    }); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("setPartJsonDrawables",
    [networkedAnimator](LuaEngine& engine, String const& part, JsonArray drawablesConfig) -> std::expected<void, StarException> {
      return applyAnimatorJsonDrawables(networkedAnimator, &NetworkedAnimator::setPartDrawables, engine, part, drawablesConfig);
    }); !result) return std::unexpected(std::move(result.error()));

  if (auto result = callbacks.registerCallbackWithSignature<String, String, String>(
      "applyPartTags", std::bind_front(&NetworkedAnimator::applyPartTags, networkedAnimator)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String, std::optional<String>>(
      "setLocalTag", std::bind_front(&NetworkedAnimator::setLocalTag, networkedAnimator)); !result) return std::unexpected(std::move(result.error()));
  return {};
}


std::expected<LuaCallbacks, StarException> LuaBindings::makeNetworkedAnimatorCallbacks(NetworkedAnimator* networkedAnimator) {
  LuaCallbacks callbacks;

  if (auto result = callbacks.registerCallbackWithSignature<bool, String, String, bool, bool>(
      "setAnimationState", std::bind(&NetworkedAnimator::setState, networkedAnimator, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = Detail::registerAnimatorStateCallbacks(callbacks, networkedAnimator, Detail::AnimatorStateCallbackOrder::Networked); !result) return std::unexpected(std::move(result.error()));

  if (auto result = callbacks.registerCallbackWithSignature<void, String, std::optional<String>>(
      "setGlobalTag", std::bind(&NetworkedAnimator::setGlobalTag, networkedAnimator, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String, String, std::optional<String>>(
      "setPartTag", std::bind(&NetworkedAnimator::setPartTag, networkedAnimator, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("setFlipped",
      [networkedAnimator](bool flipped, std::optional<float> relativeCenterLine) {
        networkedAnimator->setFlipped(flipped, relativeCenterLine.value_or(0.0f));
      }); !result) return std::unexpected(std::move(result.error()));
  if (auto result = Detail::registerAnimatorPlaybackQueryCallbacks(callbacks, networkedAnimator); !result) return std::unexpected(std::move(result.error()));

  if (auto result = callbacks.registerCallbackWithSignature<void, float>(
      "setAnimationRate", std::bind(&NetworkedAnimator::setAnimationRate, networkedAnimator, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String, float, bool>(
      "rotateGroup", std::bind(&NetworkedAnimator::rotateGroup, networkedAnimator, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<float, String>(
      "currentRotationAngle", std::bind(&NetworkedAnimator::currentRotationAngle, networkedAnimator, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<bool, String>(
      "hasTransformationGroup", std::bind(&NetworkedAnimator::hasTransformationGroup, networkedAnimator, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));

  if (auto result = callbacks.registerCallbackWithSignature<void, String, Vec2F>("translateTransformationGroup",
      std::bind(&NetworkedAnimator::translateTransformationGroup, networkedAnimator, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("rotateTransformationGroup",
      [networkedAnimator](String const& transformationGroup, float rotation, std::optional<Vec2F> const& rotationCenter) {
        return networkedAnimator->rotateTransformationGroup(transformationGroup, rotation, rotationCenter.value_or(Vec2F{}));
      }); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("rotateDegreesTransformationGroup",
      [networkedAnimator](String const& transformationGroup, float rotation, std::optional<Vec2F> const& rotationCenter) {
        return networkedAnimator->rotateTransformationGroup(transformationGroup, rotation * std::numbers::pi / 180, rotationCenter.value_or(Vec2F{}));
      }); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("scaleTransformationGroup",
      [networkedAnimator](LuaEngine& engine, String const& transformationGroup, LuaValue scale, std::optional<Vec2F> const& scaleCenter) -> std::expected<void, StarException> {
        auto cs = engine.luaMaybeTo<Vec2F>(scale);
        if (!cs) return std::unexpected(std::move(cs.error()));
        if (*cs) return networkedAnimator->scaleTransformationGroup(transformationGroup, **cs, scaleCenter.value_or(Vec2F{}));
        auto scalar = engine.luaTo<float>(scale);
        if (!scalar) return std::unexpected(std::move(scalar.error()));
        return networkedAnimator->scaleTransformationGroup(transformationGroup, *scalar, scaleCenter.value_or(Vec2F{}));
      }); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String, float, float, float, float, float, float>(
      "transformTransformationGroup",
      std::bind(&NetworkedAnimator::transformTransformationGroup, networkedAnimator, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String>(
      "resetTransformationGroup", std::bind(&NetworkedAnimator::resetTransformationGroup, networkedAnimator, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String, Mat3F>(
      "setTransformationGroup", std::bind(&NetworkedAnimator::setTransformationGroup, networkedAnimator, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<Mat3F, String>(
      "getTransformationGroup", std::bind(&NetworkedAnimator::getTransformationGroup, networkedAnimator, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));

  if (auto result = Detail::registerAnimatorLocalTransformationCallbacks(callbacks, networkedAnimator); !result) return std::unexpected(std::move(result.error()));


  if (auto result = callbacks.registerCallbackWithSignature<void, String, bool>(
      "setParticleEmitterActive", std::bind(&NetworkedAnimator::setParticleEmitterActive, networkedAnimator, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String, float>("setParticleEmitterEmissionRate",
      std::bind(&NetworkedAnimator::setParticleEmitterEmissionRate, networkedAnimator, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String, unsigned>("setParticleEmitterBurstCount",
      std::bind(&NetworkedAnimator::setParticleEmitterBurstCount, networkedAnimator, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String, RectF>("setParticleEmitterOffsetRegion",
      std::bind(&NetworkedAnimator::setParticleEmitterOffsetRegion, networkedAnimator, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String>(
      "burstParticleEmitter", std::bind(&NetworkedAnimator::burstParticleEmitter, networkedAnimator, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String, bool>(
      "setLightActive", std::bind(&NetworkedAnimator::setLightActive, networkedAnimator, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String, Vec2F>(
      "setLightPosition", std::bind(&NetworkedAnimator::setLightPosition, networkedAnimator, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String, Color>(
      "setLightColor", std::bind(&NetworkedAnimator::setLightColor, networkedAnimator, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String, float>(
      "setLightPointAngle", std::bind(&NetworkedAnimator::setLightPointAngle, networkedAnimator, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<bool, String>(
      "hasSound", std::bind(&NetworkedAnimator::hasSound, networkedAnimator, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String, StringList>(
      "setSoundPool", std::bind(&NetworkedAnimator::setSoundPool, networkedAnimator, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String, Vec2F>(
      "setSoundPosition", std::bind(&NetworkedAnimator::setSoundPosition, networkedAnimator, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("playSound",
      [networkedAnimator](String const& sound, std::optional<int> loops) {
        return networkedAnimator->playSound(sound, loops.value_or(0));
      }); !result) return std::unexpected(std::move(result.error()));

  if (auto result = callbacks.registerCallback("setSoundVolume",
      [networkedAnimator](String const& sound, float targetVolume, std::optional<float> rampTime) {
        return networkedAnimator->setSoundVolume(sound, targetVolume, rampTime.value_or(0));
      }); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("setSoundPitch",
      [networkedAnimator](String const& sound, float targetPitch, std::optional<float> rampTime) {
        return networkedAnimator->setSoundPitchMultiplier(sound, targetPitch, rampTime.value_or(0));
      }); !result) return std::unexpected(std::move(result.error()));

  if (auto result = callbacks.registerCallback("stopAllSounds",
      [networkedAnimator](String const& sound, std::optional<float> rampTime) {
        return networkedAnimator->stopAllSounds(sound, rampTime.value_or(0.0f));
      }); !result) return std::unexpected(std::move(result.error()));

  if (auto result = callbacks.registerCallbackWithSignature<void, String, bool>(
      "setEffectActive", std::bind(&NetworkedAnimator::setEffectEnabled, networkedAnimator, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = Detail::registerAnimatorPartQueryCallbacks(callbacks, networkedAnimator); !result) return std::unexpected(std::move(result.error()));

  if (auto result = Detail::registerAnimatorDrawableCallbacks(callbacks, networkedAnimator); !result) return std::unexpected(std::move(result.error()));

  return callbacks;
}

}
