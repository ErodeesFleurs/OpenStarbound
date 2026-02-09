#include "StarScriptedAnimatorLuaBindings.hpp"

import std;

namespace Star {

auto LuaBindings::makeScriptedAnimatorCallbacks(NetworkedAnimator* networkedAnimator, std::function<Json(String const&, Json const&)> getParameter) -> LuaCallbacks {
  LuaCallbacks callbacks;

  callbacks.registerCallback("animationParameter", getParameter);
  callbacks.registerCallbackWithSignature<std::optional<Vec2F>, String, String>("partPoint", [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { return networkedAnimator->partPoint(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<std::optional<PolyF>, String, String>("partPoly", [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { return networkedAnimator->partPoly(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<Json, String, String, std::optional<String>, std::optional<String>, std::optional<int>>("partProperty", [networkedAnimator](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4, auto&& PH5) -> auto { return networkedAnimator->partProperty(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4), std::forward<decltype(PH5)>(PH5)); });
  callbacks.registerCallbackWithSignature<Json, String, String>("partNextProperty", [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { return networkedAnimator->partNextProperty(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });

  callbacks.registerCallback("transformPoint", [networkedAnimator](Vec2F point, String const& part) -> Vec2F {
    return networkedAnimator->partTransformation(part).transformVec2(point);
  });
  callbacks.registerCallback("transformPoly", [networkedAnimator](PolyF poly, String const& part) -> PolyF {
    poly.transform(networkedAnimator->partTransformation(part));
    return poly;
  });

  callbacks.registerCallbackWithSignature<bool, String, String, bool, bool>(
    "setLocalAnimationState", [networkedAnimator](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4) -> auto { return networkedAnimator->setLocalState(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4)); });
  callbacks.registerCallbackWithSignature<Json, String, String, std::optional<String>, std::optional<int>>(
    "animationStateProperty", [networkedAnimator](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4) -> auto { return networkedAnimator->stateProperty(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4)); });
  callbacks.registerCallbackWithSignature<Json, String, String>(
    "animationStateNextProperty", [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { return networkedAnimator->stateNextProperty(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<String, String>(
    "animationState", [networkedAnimator](auto&& PH1) -> auto { return networkedAnimator->state(std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<int, String>(
    "animationStateFrame", [networkedAnimator](auto&& PH1) -> auto { return networkedAnimator->stateFrame(std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<int, String>(
    "animationStateNextFrame", [networkedAnimator](auto&& PH1) -> auto { return networkedAnimator->stateNextFrame(std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<float, String>(
    "animationStateFrameProgress", [networkedAnimator](auto&& PH1) -> auto { return networkedAnimator->stateFrameProgress(std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<float, String>(
    "animationStateTimer", [networkedAnimator](auto&& PH1) -> auto { return networkedAnimator->stateTimer(std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<bool, String>(
    "animationStateReverse", [networkedAnimator](auto&& PH1) -> auto { return networkedAnimator->stateReverse(std::forward<decltype(PH1)>(PH1)); });

  callbacks.registerCallbackWithSignature<float, String, std::optional<String>>(
    "stateCycle", [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { return networkedAnimator->stateCycle(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<int, String, std::optional<String>>(
    "stateFrames", [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { return networkedAnimator->stateFrames(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });

  callbacks.registerCallbackWithSignature<bool, String, std::optional<String>>(
    "hasState", [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { return networkedAnimator->hasState(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });

  callbacks.registerCallbackWithSignature<bool, String>(
    "hasTransformationGroup", [networkedAnimator](auto&& PH1) -> auto { return networkedAnimator->hasTransformationGroup(std::forward<decltype(PH1)>(PH1)); });

  callbacks.registerCallbackWithSignature<void, String, Vec2F>("translateLocalTransformationGroup",
                                                               [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { networkedAnimator->translateLocalTransformationGroup(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallback("rotateLocalTransformationGroup",
                             [networkedAnimator](String const& transformationGroup, float rotation, std::optional<Vec2F> const& rotationCenter) -> void {
                               networkedAnimator->rotateLocalTransformationGroup(transformationGroup, rotation, rotationCenter.value());
                             });
  callbacks.registerCallback("rotateDegreesLocalTransformationGroup",
                             [networkedAnimator](String const& transformationGroup, float rotation, std::optional<Vec2F> const& rotationCenter) -> void {
                               networkedAnimator->rotateLocalTransformationGroup(transformationGroup, rotation * Star::Constants::pi / 180, rotationCenter.value());
                             });
  callbacks.registerCallback("scaleLocalTransformationGroup",
                             [networkedAnimator](LuaEngine& engine, String const& transformationGroup, LuaValue scale, std::optional<Vec2F> const& scaleCenter) -> void {
                               if (auto cs = engine.luaMaybeTo<Vec2F>(scale))
                                 networkedAnimator->scaleLocalTransformationGroup(transformationGroup, *cs, scaleCenter.value());
                               else
                                 networkedAnimator->scaleLocalTransformationGroup(transformationGroup, engine.luaTo<float>(scale), scaleCenter.value());
                             });
  callbacks.registerCallbackWithSignature<void, String, float, float, float, float, float, float>(
    "transformLocalTransformationGroup",
    [networkedAnimator](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4, auto&& PH5, auto&& PH6, auto&& PH7) -> auto { networkedAnimator->transformLocalTransformationGroup(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4), std::forward<decltype(PH5)>(PH5), std::forward<decltype(PH6)>(PH6), std::forward<decltype(PH7)>(PH7)); });
  callbacks.registerCallbackWithSignature<void, String>(
    "resetLocalTransformationGroup", [networkedAnimator](auto&& PH1) -> auto { networkedAnimator->resetLocalTransformationGroup(std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<void, String, Mat3F>(
    "setLocalTransformationGroup", [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { networkedAnimator->setLocalTransformationGroup(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<Mat3F, String>(
    "getLocalTransformationGroup", [networkedAnimator](auto&& PH1) -> auto { return networkedAnimator->getLocalTransformationGroup(std::forward<decltype(PH1)>(PH1)); });

  callbacks.registerCallbackWithSignature<void, String, List<Drawable>>(
    "addPartDrawables", [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { networkedAnimator->addPartDrawables(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<void, String, List<Drawable>>(
    "setPartDrawables", [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { networkedAnimator->setPartDrawables(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallback("addPartJsonDrawables",
                             [networkedAnimator](String const& part, JsonArray drawablesConfig) -> void {
                               networkedAnimator->addPartDrawables(part, drawablesConfig.transformed([](Json config) -> Drawable {
                                 return Drawable(config);
                               }));
                             });
  callbacks.registerCallback("setPartJsonDrawables",
                             [networkedAnimator](String const& part, JsonArray drawablesConfig) -> void {
                               networkedAnimator->setPartDrawables(part, drawablesConfig.transformed([](Json config) -> Drawable {
                                 return Drawable(config);
                               }));
                             });

  callbacks.registerCallbackWithSignature<String, String, String>(
    "applyPartTags", [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { return networkedAnimator->applyPartTags(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<void, String, std::optional<String>>(
    "setLocalTag", [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { networkedAnimator->setLocalTag(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });

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

}// namespace Star
