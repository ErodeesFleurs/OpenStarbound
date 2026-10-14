module;
import std;
import star.json;
#include "StarJsonExtra.hpp"
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
import star.maybe;


// Match client include order for SIMD intrinsics used by xxhash and fast_float.
import star.animated_part_set;
import star.drawable;
import star.animation;
import star.particle;
import star.mixer;
import star.light_source;
import star.networked_animator;
#include "StarLuaGameConverters.hpp"

module star.scripted_animator_lua_bindings;

import star.math_common;
namespace Star {

LuaCallbacks LuaBindings::makeScriptedAnimatorCallbacks(NetworkedAnimator* networkedAnimator, std::function<Json(String const&, Json const&)> getParameter) {
  LuaCallbacks callbacks;

  callbacks.registerCallback("animationParameter", getParameter);
  callbacks.registerCallbackWithSignature<Maybe<Vec2F>, String, String>("partPoint", std::bind(&NetworkedAnimator::partPoint, networkedAnimator, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<Maybe<PolyF>, String, String>("partPoly", std::bind(&NetworkedAnimator::partPoly, networkedAnimator, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<Json, String, String, Maybe<String>, Maybe<String>, Maybe<int>>("partProperty", std::bind(&NetworkedAnimator::partProperty, networkedAnimator, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
  callbacks.registerCallbackWithSignature<Json, String, String>("partNextProperty", std::bind(&NetworkedAnimator::partNextProperty, networkedAnimator, std::placeholders::_1, std::placeholders::_2));

  callbacks.registerCallback("transformPoint", [networkedAnimator] (Vec2F point, String const& part) -> Vec2F {
      return networkedAnimator->partTransformation(part).transformVec2(point);
    });
  callbacks.registerCallback("transformPoly", [networkedAnimator] (PolyF poly, String const& part) -> PolyF {
      poly.transform(networkedAnimator->partTransformation(part));
      return poly;
    });

  callbacks.registerCallbackWithSignature<bool, String, String, bool, bool>(
      "setLocalAnimationState", std::bind(&NetworkedAnimator::setLocalState, networkedAnimator, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
  callbacks.registerCallbackWithSignature<Json, String, String, Maybe<String>, Maybe<int>>(
      "animationStateProperty", std::bind(&NetworkedAnimator::stateProperty, networkedAnimator, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
  callbacks.registerCallbackWithSignature<Json, String, String>(
      "animationStateNextProperty", std::bind(&NetworkedAnimator::stateNextProperty, networkedAnimator, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<String, String>(
      "animationState", std::bind(&NetworkedAnimator::state, networkedAnimator, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<int, String>(
      "animationStateFrame", std::bind(&NetworkedAnimator::stateFrame, networkedAnimator, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<int, String>(
      "animationStateNextFrame", std::bind(&NetworkedAnimator::stateNextFrame, networkedAnimator, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<float, String>(
      "animationStateFrameProgress", std::bind(&NetworkedAnimator::stateFrameProgress, networkedAnimator, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<float, String>(
      "animationStateTimer", std::bind(&NetworkedAnimator::stateTimer, networkedAnimator, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<bool, String>(
      "animationStateReverse", std::bind(&NetworkedAnimator::stateReverse, networkedAnimator, std::placeholders::_1));

  callbacks.registerCallbackWithSignature<float, String, Maybe<String>>(
      "stateCycle", std::bind(&NetworkedAnimator::stateCycle, networkedAnimator, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<int, String, Maybe<String>>(
      "stateFrames", std::bind(&NetworkedAnimator::stateFrames, networkedAnimator, std::placeholders::_1, std::placeholders::_2));

  callbacks.registerCallbackWithSignature<bool, String, Maybe<String>>(
      "hasState", std::bind(&NetworkedAnimator::hasState, networkedAnimator, std::placeholders::_1, std::placeholders::_2));

  callbacks.registerCallbackWithSignature<bool, String>(
      "hasTransformationGroup", std::bind(&NetworkedAnimator::hasTransformationGroup, networkedAnimator, std::placeholders::_1));

  callbacks.registerCallbackWithSignature<void, String, Vec2F>("translateLocalTransformationGroup",
      std::bind(&NetworkedAnimator::translateLocalTransformationGroup, networkedAnimator, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallback("rotateLocalTransformationGroup",
      [networkedAnimator](String const& transformationGroup, float rotation, Maybe<Vec2F> const& rotationCenter) {
        networkedAnimator->rotateLocalTransformationGroup(transformationGroup, rotation, rotationCenter.value());
      });
  callbacks.registerCallback("rotateDegreesLocalTransformationGroup",
      [networkedAnimator](String const& transformationGroup, float rotation, Maybe<Vec2F> const& rotationCenter) {
        networkedAnimator->rotateLocalTransformationGroup(transformationGroup, rotation * Star::Constants::pi / 180, rotationCenter.value());
      });
  callbacks.registerCallback("scaleLocalTransformationGroup",
      [networkedAnimator](LuaEngine& engine, String const& transformationGroup, LuaValue scale, Maybe<Vec2F> const& scaleCenter) {
        if (auto cs = engine.luaMaybeTo<Vec2F>(scale))
          networkedAnimator->scaleLocalTransformationGroup(transformationGroup, *cs, scaleCenter.value());
        else
          networkedAnimator->scaleLocalTransformationGroup(transformationGroup, engine.luaTo<float>(scale), scaleCenter.value());
      });
  callbacks.registerCallbackWithSignature<void, String, float, float, float, float, float, float>(
      "transformLocalTransformationGroup",
      std::bind(&NetworkedAnimator::transformLocalTransformationGroup, networkedAnimator, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7));
  callbacks.registerCallbackWithSignature<void, String>(
      "resetLocalTransformationGroup", std::bind(&NetworkedAnimator::resetLocalTransformationGroup, networkedAnimator, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<void, String, Mat3F>(
      "setLocalTransformationGroup", std::bind(&NetworkedAnimator::setLocalTransformationGroup, networkedAnimator, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<Mat3F, String>(
      "getLocalTransformationGroup", std::bind(&NetworkedAnimator::getLocalTransformationGroup, networkedAnimator, std::placeholders::_1));

  callbacks.registerCallbackWithSignature<void, String, List<Drawable>>(
      "addPartDrawables", std::bind(&NetworkedAnimator::addPartDrawables, networkedAnimator, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<void, String, List<Drawable>>(
      "setPartDrawables", std::bind(&NetworkedAnimator::setPartDrawables, networkedAnimator, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallback("addPartJsonDrawables",
    [networkedAnimator](String const& part, JsonArray drawablesConfig) {
      networkedAnimator->addPartDrawables(part, drawablesConfig.transformed([](Json config) -> Drawable {
        return Drawable(config);
      }));
    });
  callbacks.registerCallback("setPartJsonDrawables",
    [networkedAnimator](String const& part, JsonArray drawablesConfig) {
      networkedAnimator->setPartDrawables(part, drawablesConfig.transformed([](Json config) -> Drawable {
        return Drawable(config);
      }));
    });


  callbacks.registerCallbackWithSignature<String, String, String>(
      "applyPartTags", std::bind(&NetworkedAnimator::applyPartTags, networkedAnimator, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<void, String, Maybe<String>>(
      "setLocalTag", std::bind(&NetworkedAnimator::setLocalTag, networkedAnimator, std::placeholders::_1, std::placeholders::_2));

  callbacks.registerCallback("flipped", [networkedAnimator]() -> bool {
        return networkedAnimator->flipped();
    });
  callbacks.registerCallback("flippedRelativeCenterLine", [networkedAnimator]() -> float {
        return networkedAnimator->flippedRelativeCenterLine();
    });
  callbacks.registerCallback("animationRate", [networkedAnimator]() -> float {
        return networkedAnimator->animationRate();
    });


  return callbacks;
}

}
