import star.json;
import star.json_extra;
import star.id_map;
import star.variant;
import star.casting;
import star.vector;
import star.data_stream;
import star.game_types;
import star.strong_typedef;
import star.bi_map;
import star.list;
import star.interpolation;
import star.lua_converters;
import star.random;
import star.periodic;
import star.periodic_function;
import star.set;
import star.color;
import star.lua;
import star.audio;
import star.thread;
import star.map;
import star.string;
import star.rect;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.version;


import star.lua_root;
import star.lua_components;
import star.lua_animation_component;
import star.tile_damage;
import star.interaction_types;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.tile_entity;
import star.status_effect_entity;
import star.scripted_entity;
import star.chat_action;
import star.chatty_entity;
import star.wiring;
import star.wire_entity;
import star.inspectable_entity;
import star.animated_part_set;
import star.drawable;
import star.animation;
import star.particle;
import star.mixer;
import star.networked_animator;
import star.entity_rendering_types;
import star.entity_rendering;
import star.object;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.collision_block;
import star.force_regions;
import star.physics_entity;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

import star.physics_object;
import star.object_database;


import star.algorithm;
namespace Star {

PhysicsObject::PhysicsObject(Root& rootContext, std::shared_ptr<ObjectConfig const> config, Json const& parameters) : Entity(rootContext), Object(rootContext, std::move(config), parameters) {
  for (auto const& p : configValue("physicsForces", JsonObject()).iterateObject()) {
    auto& forceConfig = m_physicsForces[p.first];

    forceConfig.forceRegion = jsonToPhysicsForceRegion(p.second);
    forceConfig.enabled.set(p.second.getBool("enabled", true));
  }

  for (auto const& p : configValue("physicsCollisions", JsonObject()).iterateObject()) {
    auto& collisionConfig = m_physicsCollisions[p.first];
    collisionConfig.movingCollision = PhysicsMovingCollision::fromJson(p.second);
    collisionConfig.xPosition.set(take(collisionConfig.movingCollision.position[0]));
    collisionConfig.yPosition.set(take(collisionConfig.movingCollision.position[1]));
    collisionConfig.enabled.set(p.second.getBool("enabled", true));
  }

  m_physicsForces.sortByKey();
  for (auto& p : m_physicsForces)
    m_netGroup.addNetElement(&p.second.enabled);

  m_physicsCollisions.sortByKey();
  for (auto& p : m_physicsCollisions) {
    m_netGroup.addNetElement(&p.second.xPosition);
    m_netGroup.addNetElement(&p.second.yPosition);
    p.second.xPosition.setInterpolator(lerp<float, float>);
    p.second.yPosition.setInterpolator(lerp<float, float>);
    m_netGroup.addNetElement(&p.second.enabled);
  }
}

void PhysicsObject::enableInterpolation(float extrapolationHint) {
  m_netGroup.enableNetInterpolation(extrapolationHint);
}

void PhysicsObject::disableInterpolation() {
  m_netGroup.disableNetInterpolation();
}

void PhysicsObject::init(World* world, EntityId entityId, EntityMode mode) {
  if (mode == EntityMode::Master) {
    LuaCallbacks physicsCallbacks;
    physicsCallbacks.registerCallback("setForceEnabled", [this](String const& force, bool enabled) {
        m_physicsForces.get(force).enabled.set(enabled);
      });
    physicsCallbacks.registerCallback("setCollisionPosition", [this](String const& collision, Vec2F const& pos) {
        auto& collisionConfig = m_physicsCollisions.get(collision);
        collisionConfig.xPosition.set(pos[0]);
        collisionConfig.yPosition.set(pos[1]);
      });
    physicsCallbacks.registerCallback("setCollisionEnabled", [this](String const& collision, bool const& enabled) {
        auto& collisionConfig = m_physicsCollisions.get(collision);
        collisionConfig.enabled.set(enabled);
      });
    m_scriptComponent.addCallbacks("physics", std::move(physicsCallbacks));
  }
  Object::init(world, entityId, mode);
  m_metaBoundBox = Object::metaBoundBox();
  for (auto const& p : m_physicsForces) {
    PhysicsForceRegion forceRegion = p.second.forceRegion;
    forceRegion.call([pos = position()](auto& fr) { fr.translate(pos); });
    m_metaBoundBox.combine(forceRegion.call([](auto& fr) { return fr.boundBox(); }));
  }
}

void PhysicsObject::uninit() {
  m_scriptComponent.removeCallbacks("physics");
  Object::uninit();
}

void PhysicsObject::update(float dt, std::uint64_t currentStep) {
  Object::update(dt, currentStep);
  if (isSlave())
    m_netGroup.tickNetInterpolation(dt);
}

RectF PhysicsObject::metaBoundBox() const {
  return m_metaBoundBox;
}

List<PhysicsForceRegion> PhysicsObject::forceRegions() const {
  List<PhysicsForceRegion> forces;
  for (auto const& p : m_physicsForces) {
    if (p.second.enabled.get()) {
      PhysicsForceRegion forceRegion = p.second.forceRegion;
      forceRegion.call([pos = position()](auto& fr) { fr.translate(pos); });
      forces.append(std::move(forceRegion));
    }
  }
  return forces;
}

std::size_t PhysicsObject::movingCollisionCount() const {
  return m_physicsCollisions.size();
}

std::optional<PhysicsMovingCollision> PhysicsObject::movingCollision(std::size_t positionIndex) const {
  auto const& collisionConfig = m_physicsCollisions.valueAt(positionIndex);
  if (!collisionConfig.enabled.get())
    return {};
  PhysicsMovingCollision collision = collisionConfig.movingCollision;
  collision.translate(position() + Vec2F(collisionConfig.xPosition.get(), collisionConfig.yPosition.get()));
  return collision;
}

}
