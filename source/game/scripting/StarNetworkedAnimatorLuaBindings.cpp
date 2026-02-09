#include "StarNetworkedAnimatorLuaBindings.hpp"

#include "StarNetworkedAnimator.hpp"

import std;

namespace Star {

auto LuaBindings::makeNetworkedAnimatorCallbacks(NetworkedAnimator* networkedAnimator) -> LuaCallbacks {
  LuaCallbacks callbacks;

  callbacks.registerCallbackWithSignature<bool, String, String, bool, bool>(
    "setAnimationState", [networkedAnimator](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4) -> auto { return networkedAnimator->setState(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4)); });
  callbacks.registerCallbackWithSignature<bool, String, String, bool, bool>(
    "setLocalAnimationState", [networkedAnimator](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4) -> auto { return networkedAnimator->setLocalState(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4)); });
  callbacks.registerCallbackWithSignature<String, String>(
    "animationState", [networkedAnimator](auto&& PH1) -> auto { return networkedAnimator->state(std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<Json, String, String, std::optional<String>, std::optional<int>>(
    "animationStateProperty", [networkedAnimator](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4) -> auto { return networkedAnimator->stateProperty(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4)); });
  callbacks.registerCallbackWithSignature<Json, String, String>(
    "animationStateNextProperty", [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { return networkedAnimator->stateNextProperty(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
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
  callbacks.registerCallbackWithSignature<bool, String, std::optional<String>>(
    "hasState", [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { return networkedAnimator->hasState(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });

  callbacks.registerCallbackWithSignature<float, String, std::optional<String>>(
    "stateCycle", [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { return networkedAnimator->stateCycle(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<int, String, std::optional<String>>(
    "stateFrames", [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { return networkedAnimator->stateFrames(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });

  callbacks.registerCallbackWithSignature<void, String, std::optional<String>>(
    "setGlobalTag", [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { networkedAnimator->setGlobalTag(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<void, String, String, std::optional<String>>(
    "setPartTag", [networkedAnimator](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { networkedAnimator->setPartTag(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallback("setFlipped",
                             [networkedAnimator](bool flipped, std::optional<float> relativeCenterLine) -> void {
                               networkedAnimator->setFlipped(flipped, relativeCenterLine.value());
                             });
  callbacks.registerCallback("flipped", [networkedAnimator]() -> bool {
    return networkedAnimator->flipped();
  });
  callbacks.registerCallback("flippedRelativeCenterLine", [networkedAnimator]() -> float {
    return networkedAnimator->flippedRelativeCenterLine();
  });
  callbacks.registerCallback("animationRate", [networkedAnimator]() -> float {
    return networkedAnimator->animationRate();
  });

  callbacks.registerCallbackWithSignature<void, float>(
    "setAnimationRate", [networkedAnimator](auto&& PH1) -> auto { networkedAnimator->setAnimationRate(std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<void, String, float, bool>(
    "rotateGroup", [networkedAnimator](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { networkedAnimator->rotateGroup(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallbackWithSignature<float, String>(
    "currentRotationAngle", [networkedAnimator](auto&& PH1) -> auto { return networkedAnimator->currentRotationAngle(std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<bool, String>(
    "hasTransformationGroup", [networkedAnimator](auto&& PH1) -> auto { return networkedAnimator->hasTransformationGroup(std::forward<decltype(PH1)>(PH1)); });

  callbacks.registerCallbackWithSignature<void, String, Vec2F>("translateTransformationGroup",
                                                               [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { networkedAnimator->translateTransformationGroup(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallback("rotateTransformationGroup",
                             [networkedAnimator](String const& transformationGroup, float rotation, std::optional<Vec2F> const& rotationCenter) -> void {
                               networkedAnimator->rotateTransformationGroup(transformationGroup, rotation, rotationCenter.value());
                             });
  callbacks.registerCallback("rotateDegreesTransformationGroup",
                             [networkedAnimator](String const& transformationGroup, float rotation, std::optional<Vec2F> const& rotationCenter) -> void {
                               networkedAnimator->rotateTransformationGroup(transformationGroup, rotation * Star::Constants::pi / 180, rotationCenter.value());
                             });
  callbacks.registerCallback("scaleTransformationGroup",
                             [networkedAnimator](LuaEngine& engine, String const& transformationGroup, LuaValue scale, std::optional<Vec2F> const& scaleCenter) -> void {
                               if (auto cs = engine.luaMaybeTo<Vec2F>(scale))
                                 networkedAnimator->scaleTransformationGroup(transformationGroup, *cs, scaleCenter.value());
                               else
                                 networkedAnimator->scaleTransformationGroup(transformationGroup, engine.luaTo<float>(scale), scaleCenter.value());
                             });
  callbacks.registerCallbackWithSignature<void, String, float, float, float, float, float, float>(
    "transformTransformationGroup",
    [networkedAnimator](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4, auto&& PH5, auto&& PH6, auto&& PH7) -> auto { networkedAnimator->transformTransformationGroup(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4), std::forward<decltype(PH5)>(PH5), std::forward<decltype(PH6)>(PH6), std::forward<decltype(PH7)>(PH7)); });
  callbacks.registerCallbackWithSignature<void, String>(
    "resetTransformationGroup", [networkedAnimator](auto&& PH1) -> auto { networkedAnimator->resetTransformationGroup(std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<void, String, Mat3F>(
    "setTransformationGroup", [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { networkedAnimator->setTransformationGroup(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<Mat3F, String>(
    "getTransformationGroup", [networkedAnimator](auto&& PH1) -> auto { return networkedAnimator->getTransformationGroup(std::forward<decltype(PH1)>(PH1)); });

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

  callbacks.registerCallbackWithSignature<void, String, bool>(
    "setParticleEmitterActive", [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { networkedAnimator->setParticleEmitterActive(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<void, String, float>("setParticleEmitterEmissionRate",
                                                               [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { networkedAnimator->setParticleEmitterEmissionRate(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<void, String, unsigned>("setParticleEmitterBurstCount",
                                                                  [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { networkedAnimator->setParticleEmitterBurstCount(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<void, String, RectF>("setParticleEmitterOffsetRegion",
                                                               [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { networkedAnimator->setParticleEmitterOffsetRegion(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<void, String>(
    "burstParticleEmitter", [networkedAnimator](auto&& PH1) -> auto { networkedAnimator->burstParticleEmitter(std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<void, String, bool>(
    "setLightActive", [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { networkedAnimator->setLightActive(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<void, String, Vec2F>(
    "setLightPosition", [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { networkedAnimator->setLightPosition(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<void, String, Color>(
    "setLightColor", [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { networkedAnimator->setLightColor(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<void, String, float>(
    "setLightPointAngle", [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { networkedAnimator->setLightPointAngle(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<bool, String>(
    "hasSound", [networkedAnimator](auto&& PH1) -> auto { return networkedAnimator->hasSound(std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<void, String, StringList>(
    "setSoundPool", [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { networkedAnimator->setSoundPool(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<void, String, Vec2F>(
    "setSoundPosition", [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { networkedAnimator->setSoundPosition(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallback("playSound",
                             [networkedAnimator](String const& sound, std::optional<int> loops) -> void {
                               networkedAnimator->playSound(sound, loops.value());
                             });

  callbacks.registerCallback("setSoundVolume",
                             [networkedAnimator](String const& sound, float targetVolume, std::optional<float> rampTime) -> void {
                               networkedAnimator->setSoundVolume(sound, targetVolume, rampTime.value_or(0));
                             });
  callbacks.registerCallback("setSoundPitch",
                             [networkedAnimator](String const& sound, float targetPitch, std::optional<float> rampTime) -> void {
                               networkedAnimator->setSoundPitchMultiplier(sound, targetPitch, rampTime.value_or(0));
                             });

  callbacks.registerCallback("stopAllSounds",
                             [networkedAnimator](String const& sound, std::optional<float> rampTime) -> void {
                               networkedAnimator->stopAllSounds(sound, rampTime.value());
                             });

  callbacks.registerCallbackWithSignature<void, String, bool>(
    "setEffectActive", [networkedAnimator](auto&& PH1, auto&& PH2) -> auto { networkedAnimator->setEffectEnabled(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
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

  return callbacks;
}

}// namespace Star
