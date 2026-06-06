#include "StarMovementControllerLuaBindings.hpp"
#include "StarMovementController.hpp"
#include "StarLuaGameConverters.hpp"

namespace Star {

LuaCallbacks LuaBindings::makeMovementControllerCallbacks(MovementController* movementController) {
  LuaCallbacks callbacks;

  callbacks.registerCallback(
      "parameters", [movementController]() { return movementController->parameters().toJson(); });
  callbacks.registerCallbackWithSignature<void, Json>(
      "applyParameters", [movementController](auto&&... args) { return movementController->applyParameters(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, Json>(
      "resetParameters", [movementController](auto&&... args) { return movementController->resetParameters(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<float>("mass", [movementController]() { return movementController->mass(); });
  callbacks.registerCallbackWithSignature<PolyF>(
      "collisionPoly", [movementController]() { return movementController->collisionPoly(); });
  callbacks.registerCallback("boundBox", [movementController]() -> RectF {
      return movementController->collisionPoly().boundBox();
    });
  callbacks.registerCallback("collisionArea", [movementController]() -> float {
      return movementController->collisionPoly().convexArea();
    });

  callbacks.registerCallbackWithSignature<Vec2F>("position", [movementController]() { return movementController->position(); });
  callbacks.registerCallbackWithSignature<float>("xPosition", [movementController]() { return movementController->xPosition(); });
  callbacks.registerCallbackWithSignature<float>("yPosition", [movementController]() { return movementController->yPosition(); });
  callbacks.registerCallbackWithSignature<Vec2F>("velocity", [movementController]() { return movementController->velocity(); });
  callbacks.registerCallbackWithSignature<float>("xVelocity", [movementController]() { return movementController->xVelocity(); });
  callbacks.registerCallbackWithSignature<float>("yVelocity", [movementController]() { return movementController->yVelocity(); });
  callbacks.registerCallbackWithSignature<float>("rotation", [movementController]() { return movementController->rotation(); });
  callbacks.registerCallbackWithSignature<PolyF>(
      "collisionBody", [movementController]() { return movementController->collisionBody(); });
  callbacks.registerCallbackWithSignature<RectF>(
      "collisionBoundBox", [movementController]() { return movementController->collisionBoundBox(); });
  callbacks.registerCallbackWithSignature<RectF>(
      "localBoundBox", [movementController]() { return movementController->localBoundBox(); });
  callbacks.registerCallbackWithSignature<bool>(
      "isColliding", [movementController]() { return movementController->isColliding(); });
  callbacks.registerCallbackWithSignature<bool>(
      "isNullColliding", [movementController]() { return movementController->isNullColliding(); });
  callbacks.registerCallbackWithSignature<bool>(
      "isCollisionStuck", [movementController]() { return movementController->isCollisionStuck(); });
  callbacks.registerCallbackWithSignature<Maybe<float>>(
      "stickingDirection", [movementController]() { return movementController->stickingDirection(); });
  callbacks.registerCallbackWithSignature<float>(
      "liquidPercentage", [movementController]() { return movementController->liquidPercentage(); });
  callbacks.registerCallbackWithSignature<LiquidId>(
      "liquidId", [movementController]() { return movementController->liquidId(); });
  callbacks.registerCallbackWithSignature<bool>("onGround", [movementController]() { return movementController->onGround(); });
  callbacks.registerCallbackWithSignature<bool>("zeroG", [movementController]() { return movementController->zeroG(); });
  callbacks.registerCallbackWithSignature<bool, bool>("atWorldLimit", [movementController](auto&&... args) { return movementController->atWorldLimit(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, Vec2F>(
      "setPosition", [movementController](auto&&... args) { return movementController->setPosition(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, float>(
      "setXPosition", [movementController](auto&&... args) { return movementController->setXPosition(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, float>(
      "setYPosition", [movementController](auto&&... args) { return movementController->setYPosition(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, Vec2F>(
      "translate", [movementController](auto&&... args) { return movementController->translate(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, Vec2F>(
      "setVelocity", [movementController](auto&&... args) { return movementController->setVelocity(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, float>(
      "setXVelocity", [movementController](auto&&... args) { return movementController->setXVelocity(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, float>(
      "setYVelocity", [movementController](auto&&... args) { return movementController->setYVelocity(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, Vec2F>(
      "addMomentum", [movementController](auto&&... args) { return movementController->addMomentum(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, float>(
      "setRotation", [movementController](auto&&... args) { return movementController->setRotation(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, float>(
      "rotate", [movementController](auto&&... args) { return movementController->rotate(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, Vec2F>(
      "accelerate", [movementController](auto&&... args) { return movementController->accelerate(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, Vec2F>(
      "force", [movementController](auto&&... args) { return movementController->force(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, Vec2F, float>(
      "approachVelocity", [movementController](auto&&... args) { return movementController->approachVelocity(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, float, float, float, bool>("approachVelocityAlongAngle",
      [movementController](auto&&... args) { return movementController->approachVelocityAlongAngle(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, float, float>(
      "approachXVelocity", [movementController](auto&&... args) { return movementController->approachXVelocity(std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, float, float>(
      "approachYVelocity", [movementController](auto&&... args) { return movementController->approachYVelocity(std::forward<decltype(args)>(args)...); });

  return callbacks;
}

}
