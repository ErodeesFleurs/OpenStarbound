module;
import std;
import star.json;
import star.id_map;
import star.lua;
import star.poly;
import star.casting;
import star.vector;
import star.data_stream;
import star.game_types;
import star.liquid_types;
import star.strong_typedef;
import star.bi_map;
import star.list;
import star.rpc_promise;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
import star.lua_root;
import star.force_regions;
import star.world;
import star.physics_entity;
import star.movement_controller;
import star.lua_game_converters;

module star.movement_controller_lua_bindings;

import star.rect;
namespace Star {

LuaCallbacks LuaBindings::makeMovementControllerCallbacks(MovementController* movementController) {
  LuaCallbacks callbacks;

  callbacks.registerCallback(
      "parameters", [movementController]() { return movementController->parameters().toJson(); });
  callbacks.registerCallbackWithSignature<void, Json>(
      "applyParameters", std::bind(&MovementController::applyParameters, movementController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<void, Json>(
      "resetParameters", std::bind(&MovementController::resetParameters, movementController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<float>("mass", std::bind(&MovementController::mass, movementController));
  callbacks.registerCallbackWithSignature<PolyF>(
      "collisionPoly", std::bind(&MovementController::collisionPoly, movementController));
  callbacks.registerCallback("boundBox", [movementController]() -> RectF {
      return movementController->collisionPoly().boundBox();
    });
  callbacks.registerCallback("collisionArea", [movementController]() -> float {
      return movementController->collisionPoly().convexArea();
    });

  callbacks.registerCallbackWithSignature<Vec2F>("position", std::bind(&MovementController::position, movementController));
  callbacks.registerCallbackWithSignature<float>("xPosition", std::bind(&MovementController::xPosition, movementController));
  callbacks.registerCallbackWithSignature<float>("yPosition", std::bind(&MovementController::yPosition, movementController));
  callbacks.registerCallbackWithSignature<Vec2F>("velocity", std::bind(&MovementController::velocity, movementController));
  callbacks.registerCallbackWithSignature<float>("xVelocity", std::bind(&MovementController::xVelocity, movementController));
  callbacks.registerCallbackWithSignature<float>("yVelocity", std::bind(&MovementController::yVelocity, movementController));
  callbacks.registerCallbackWithSignature<float>("rotation", std::bind(&MovementController::rotation, movementController));
  callbacks.registerCallbackWithSignature<PolyF>(
      "collisionBody", std::bind(&MovementController::collisionBody, movementController));
  callbacks.registerCallbackWithSignature<RectF>(
      "collisionBoundBox", std::bind(&MovementController::collisionBoundBox, movementController));
  callbacks.registerCallbackWithSignature<RectF>(
      "localBoundBox", std::bind(&MovementController::localBoundBox, movementController));
  callbacks.registerCallbackWithSignature<bool>(
      "isColliding", std::bind(&MovementController::isColliding, movementController));
  callbacks.registerCallbackWithSignature<bool>(
      "isNullColliding", std::bind(&MovementController::isNullColliding, movementController));
  callbacks.registerCallbackWithSignature<bool>(
      "isCollisionStuck", std::bind(&MovementController::isCollisionStuck, movementController));
  callbacks.registerCallbackWithSignature<std::optional<float>>(
      "stickingDirection", std::bind(&MovementController::stickingDirection, movementController));
  callbacks.registerCallbackWithSignature<float>(
      "liquidPercentage", std::bind(&MovementController::liquidPercentage, movementController));
  callbacks.registerCallbackWithSignature<LiquidId>(
      "liquidId", std::bind(&MovementController::liquidId, movementController));
  callbacks.registerCallbackWithSignature<bool>("onGround", std::bind(&MovementController::onGround, movementController));
  callbacks.registerCallbackWithSignature<bool>("zeroG", std::bind(&MovementController::zeroG, movementController));
  callbacks.registerCallbackWithSignature<bool, bool>("atWorldLimit", std::bind(&MovementController::atWorldLimit, movementController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<void, Vec2F>(
      "setPosition", std::bind(&MovementController::setPosition, movementController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<void, float>(
      "setXPosition", std::bind(&MovementController::setXPosition, movementController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<void, float>(
      "setYPosition", std::bind(&MovementController::setYPosition, movementController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<void, Vec2F>(
      "translate", std::bind(&MovementController::translate, movementController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<void, Vec2F>(
      "setVelocity", std::bind(&MovementController::setVelocity, movementController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<void, float>(
      "setXVelocity", std::bind(&MovementController::setXVelocity, movementController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<void, float>(
      "setYVelocity", std::bind(&MovementController::setYVelocity, movementController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<void, Vec2F>(
      "addMomentum", std::bind(&MovementController::addMomentum, movementController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<void, float>(
      "setRotation", std::bind(&MovementController::setRotation, movementController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<void, float>(
      "rotate", std::bind(&MovementController::rotate, movementController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<void, Vec2F>(
      "accelerate", std::bind(&MovementController::accelerate, movementController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<void, Vec2F>(
      "force", std::bind(&MovementController::force, movementController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<void, Vec2F, float>(
      "approachVelocity", std::bind(&MovementController::approachVelocity, movementController, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<void, float, float, float, bool>("approachVelocityAlongAngle",
      std::bind(&MovementController::approachVelocityAlongAngle, movementController, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
  callbacks.registerCallbackWithSignature<void, float, float>(
      "approachXVelocity", std::bind(&MovementController::approachXVelocity, movementController, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<void, float, float>(
      "approachYVelocity", std::bind(&MovementController::approachYVelocity, movementController, std::placeholders::_1, std::placeholders::_2));

  return callbacks;
}

}
