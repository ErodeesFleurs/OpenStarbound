#include "StarNetworkedAnimatorLuaBindings.hpp"
#include "StarNetworkedAnimator.hpp"
#include "StarJsonExtra.hpp"
#include "StarLuaGameConverters.hpp"

namespace Star {

LuaCallbacks LuaBindings::makeNetworkedAnimatorCallbacks(NetworkedAnimator* networkedAnimator) {
  LuaCallbacks callbacks;

  callbacks.registerCallbackWithSignature<bool, String, String, bool, bool>(
      "setAnimationState", [networkedAnimator](auto&&... args) { return networkedAnimator->setState(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<bool, String, String, bool, bool>(
      "setLocalAnimationState", [networkedAnimator](auto&&... args) { return networkedAnimator->setLocalState(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<String, String>(
      "animationState", [networkedAnimator](auto&&... args) { return networkedAnimator->state(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<Json, String, String, Maybe<String>, Maybe<int>>(
      "animationStateProperty", [networkedAnimator](auto&&... args) { return networkedAnimator->stateProperty(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<Json, String, String>(
      "animationStateNextProperty", [networkedAnimator](auto&&... args) { return networkedAnimator->stateNextProperty(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<int, String>(
      "animationStateFrame", [networkedAnimator](auto&&... args) { return networkedAnimator->stateFrame(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<int, String>(
      "animationStateNextFrame", [networkedAnimator](auto&&... args) { return networkedAnimator->stateNextFrame(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<float, String>(
      "animationStateFrameProgress", [networkedAnimator](auto&&... args) { return networkedAnimator->stateFrameProgress(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<float, String>(
      "animationStateTimer", [networkedAnimator](auto&&... args) { return networkedAnimator->stateTimer(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<bool, String>(
      "animationStateReverse", [networkedAnimator](auto&&... args) { return networkedAnimator->stateReverse(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<bool, String, Maybe<String>>(
      "hasState", [networkedAnimator](auto&&... args) { return networkedAnimator->hasState(std::forward<decltype(args)>(args)...); });

  callbacks.registerCallbackWithSignature<float, String, Maybe<String>>(
      "stateCycle", [networkedAnimator](auto&&... args) { return networkedAnimator->stateCycle(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<int, String, Maybe<String>>(
      "stateFrames", [networkedAnimator](auto&&... args) { return networkedAnimator->stateFrames(std::forward<decltype(args)>(args)...); });

  callbacks.registerCallbackWithSignature<void, String, Maybe<String>>(
      "setGlobalTag", [networkedAnimator](auto&&... args) { return networkedAnimator->setGlobalTag(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String, String, Maybe<String>>(
      "setPartTag", [networkedAnimator](auto&&... args) { return networkedAnimator->setPartTag(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallback("setFlipped",
      [networkedAnimator](bool flipped, Maybe<float> relativeCenterLine) {
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
      "setAnimationRate", [networkedAnimator](auto&&... args) { return networkedAnimator->setAnimationRate(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String, float, bool>(
      "rotateGroup", [networkedAnimator](auto&&... args) { return networkedAnimator->rotateGroup(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<float, String>(
      "currentRotationAngle", [networkedAnimator](auto&&... args) { return networkedAnimator->currentRotationAngle(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<bool, String>(
      "hasTransformationGroup", [networkedAnimator](auto&&... args) { return networkedAnimator->hasTransformationGroup(std::forward<decltype(args)>(args)...); });

  callbacks.registerCallbackWithSignature<void, String, Vec2F>("translateTransformationGroup",
      [networkedAnimator](auto&&... args) { return networkedAnimator->translateTransformationGroup(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallback("rotateTransformationGroup",
      [networkedAnimator](String const& transformationGroup, float rotation, Maybe<Vec2F> const& rotationCenter) {
        networkedAnimator->rotateTransformationGroup(transformationGroup, rotation, rotationCenter.value());
      });
  callbacks.registerCallback("rotateDegreesTransformationGroup",
      [networkedAnimator](String const& transformationGroup, float rotation, Maybe<Vec2F> const& rotationCenter) {
        networkedAnimator->rotateTransformationGroup(transformationGroup, rotation * Star::Constants::pi / 180, rotationCenter.value());
      });
  callbacks.registerCallback("scaleTransformationGroup",
      [networkedAnimator](LuaEngine& engine, String const& transformationGroup, LuaValue scale, Maybe<Vec2F> const& scaleCenter) {
        if (auto cs = engine.luaMaybeTo<Vec2F>(scale))
          networkedAnimator->scaleTransformationGroup(transformationGroup, *cs, scaleCenter.value());
        else
          networkedAnimator->scaleTransformationGroup(transformationGroup, engine.luaTo<float>(scale), scaleCenter.value());
      });
  callbacks.registerCallbackWithSignature<void, String, float, float, float, float, float, float>(
      "transformTransformationGroup",
      [networkedAnimator](auto&&... args) { return networkedAnimator->transformTransformationGroup(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String>(
      "resetTransformationGroup", [networkedAnimator](auto&&... args) { return networkedAnimator->resetTransformationGroup(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String, Mat3F>(
      "setTransformationGroup", [networkedAnimator](auto&&... args) { return networkedAnimator->setTransformationGroup(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<Mat3F, String>(
      "getTransformationGroup", [networkedAnimator](auto&&... args) { return networkedAnimator->getTransformationGroup(std::forward<decltype(args)>(args)...); });

  callbacks.registerCallbackWithSignature<void, String, Vec2F>("translateLocalTransformationGroup",
      [networkedAnimator](auto&&... args) { return networkedAnimator->translateLocalTransformationGroup(std::forward<decltype(args)>(args)...); });
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
      [networkedAnimator](auto&&... args) { return networkedAnimator->transformLocalTransformationGroup(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String>(
      "resetLocalTransformationGroup", [networkedAnimator](auto&&... args) { return networkedAnimator->resetLocalTransformationGroup(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String, Mat3F>(
      "setLocalTransformationGroup", [networkedAnimator](auto&&... args) { return networkedAnimator->setLocalTransformationGroup(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<Mat3F, String>(
      "getLocalTransformationGroup", [networkedAnimator](auto&&... args) { return networkedAnimator->getLocalTransformationGroup(std::forward<decltype(args)>(args)...); });


  callbacks.registerCallbackWithSignature<void, String, bool>(
      "setParticleEmitterActive", [networkedAnimator](auto&&... args) { return networkedAnimator->setParticleEmitterActive(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String, float>("setParticleEmitterEmissionRate",
      [networkedAnimator](auto&&... args) { return networkedAnimator->setParticleEmitterEmissionRate(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String, unsigned>("setParticleEmitterBurstCount",
      [networkedAnimator](auto&&... args) { return networkedAnimator->setParticleEmitterBurstCount(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String, RectF>("setParticleEmitterOffsetRegion",
      [networkedAnimator](auto&&... args) { return networkedAnimator->setParticleEmitterOffsetRegion(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String>(
      "burstParticleEmitter", [networkedAnimator](auto&&... args) { return networkedAnimator->burstParticleEmitter(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String, bool>(
      "setLightActive", [networkedAnimator](auto&&... args) { return networkedAnimator->setLightActive(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String, Vec2F>(
      "setLightPosition", [networkedAnimator](auto&&... args) { return networkedAnimator->setLightPosition(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String, Color>(
      "setLightColor", [networkedAnimator](auto&&... args) { return networkedAnimator->setLightColor(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String, float>(
      "setLightPointAngle", [networkedAnimator](auto&&... args) { return networkedAnimator->setLightPointAngle(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<bool, String>(
      "hasSound", [networkedAnimator](auto&&... args) { return networkedAnimator->hasSound(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String, StringList>(
      "setSoundPool", [networkedAnimator](auto&&... args) { return networkedAnimator->setSoundPool(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String, Vec2F>(
      "setSoundPosition", [networkedAnimator](auto&&... args) { return networkedAnimator->setSoundPosition(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallback("playSound",
      [networkedAnimator](String const& sound, Maybe<int> loops) {
        networkedAnimator->playSound(sound, loops.value());
      });

  callbacks.registerCallback("setSoundVolume",
      [networkedAnimator](String const& sound, float targetVolume, Maybe<float> rampTime) {
        networkedAnimator->setSoundVolume(sound, targetVolume, rampTime.value(0));
      });
  callbacks.registerCallback("setSoundPitch",
      [networkedAnimator](String const& sound, float targetPitch, Maybe<float> rampTime) {
        networkedAnimator->setSoundPitchMultiplier(sound, targetPitch, rampTime.value(0));
      });

  callbacks.registerCallback("stopAllSounds",
      [networkedAnimator](String const& sound, Maybe<float> rampTime) {
        networkedAnimator->stopAllSounds(sound, rampTime.value());
      });

  callbacks.registerCallbackWithSignature<void, String, bool>(
      "setEffectActive", [networkedAnimator](auto&&... args) { return networkedAnimator->setEffectEnabled(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<Maybe<Vec2F>, String, String>("partPoint", [networkedAnimator](auto&&... args) { return networkedAnimator->partPoint(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<Maybe<PolyF>, String, String>("partPoly", [networkedAnimator](auto&&... args) { return networkedAnimator->partPoly(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<Json, String, String, Maybe<String>, Maybe<String>, Maybe<int>>("partProperty", [networkedAnimator](auto&&... args) { return networkedAnimator->partProperty(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<Json, String, String>("partNextProperty", [networkedAnimator](auto&&... args) { return networkedAnimator->partNextProperty(std::forward<decltype(args)>(args)...); });

  callbacks.registerCallback("transformPoint", [networkedAnimator] (Vec2F point, String const& part) -> Vec2F {
      return networkedAnimator->partTransformation(part).transformVec2(point);
    });
  callbacks.registerCallback("transformPoly", [networkedAnimator] (PolyF poly, String const& part) -> PolyF {
      poly.transform(networkedAnimator->partTransformation(part));
      return poly;
    });

  callbacks.registerCallbackWithSignature<void, String, List<Drawable>>(
      "addPartDrawables", [networkedAnimator](auto&&... args) { return networkedAnimator->addPartDrawables(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String, List<Drawable>>(
      "setPartDrawables", [networkedAnimator](auto&&... args) { return networkedAnimator->setPartDrawables(std::forward<decltype(args)>(args)...); });
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
      "applyPartTags", [networkedAnimator](auto&&... args) { return networkedAnimator->applyPartTags(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String, Maybe<String>>(
      "setLocalTag", [networkedAnimator](auto&&... args) { return networkedAnimator->setLocalTag(std::forward<decltype(args)>(args)...); });

  return callbacks;
}

}
