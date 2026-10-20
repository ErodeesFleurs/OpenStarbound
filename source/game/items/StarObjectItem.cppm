module;
import std;
import star.type_identity;
import star.json;
import star.json_extra;
import star.id_map;
import star.casting;
import star.vector;
import star.data_stream;
import star.game_types;
import star.strong_typedef;
import star.bi_map;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.maybe;
import star.variant;
import star.rpc_promise;
import star.a_star;
import star.spline;
import star.string;
import star.logging;
import star.set;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.listener;
import std;
import star.format;
import star.version;
import star.random;
import star.periodic;
import star.periodic_function;
import star.lua;
import star.audio;
import star.map;
import star.lua_root;
import star.lua_components;
import star.lua_animation_component;


import star.item;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.animation;
import star.particle;
import star.interaction_types;
import star.tile_damage;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
import star.force_regions;
import star.world;
import star.physics_entity;
import star.movement_controller;
import star.platformer_astar_types;
import star.anchorable_entity;
import star.game_timers;
import star.actor_movement_controller;
import star.mobile_entity;
import star.actor_entity;
import star.tool_user_entity;
import star.tool_user_item;
import star.status_effect_item;
import star.fireable_item;
import star.drawable;
import star.non_rotated_drawables_item;
import star.beam_item;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.status_effect_entity;
import star.scripted_entity;
import star.chat_action;
import star.chatty_entity;
import star.wiring;
import star.wire_entity;
import star.inspectable_entity;
import star.animated_part_set;
import star.mixer;
import star.networked_animator;
import star.entity_rendering_types;
import star.entity_rendering;
import star.object;

export module star.object_item;
import star.object_database;


export namespace Star {

class ObjectItem : public Item, public FireableItem, public BeamItem {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<ObjectItem, Item, FireableItem, BeamItem>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<ObjectItem>();
  }

  ObjectItem(Root& root, Json const& config, String const& directory, Json const& objectParameters);
  virtual ~ObjectItem() {}

  std::shared_ptr<Item> clone() const override;

  void init(ToolUserEntity* owner, ToolHand hand) override;
  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;

  List<Drawable> nonRotatedDrawables() const override;

  float cooldownTime() const override;
  void fire(FireMode mode, bool shifting, bool edgeTriggered) override;

  String objectName() const;
  Json objectParameters() const;

  bool placeInWorld(FireMode mode, bool shifting);
  bool canPlace(bool shifting) const;

private:
  bool m_shifting;
};

}

namespace Star {

ObjectItem::ObjectItem(Root& root, Json const& config, String const& directory, Json const& objectParameters)
  : Item(root, config, directory, objectParameters), FireableItem(config), BeamItem(root, config) {
  setTwoHanded(config.getBool("twoHanded", true));

  // Make sure that all script objects that have retainObjectParametersInItem
  // start with a blank scriptStorage entry to help them stack properly.
  if (instanceValue("retainObjectParametersInItem", false).toBool() && instanceValue("scriptStorage").isNull())
    setInstanceValue("scriptStorage", JsonObject());
  m_shifting = false;
}

std::shared_ptr<Item> ObjectItem::clone() const {
  return std::make_shared<ObjectItem>(*this);
}

void ObjectItem::init(ToolUserEntity* owner, ToolHand hand) {
  FireableItem::init(owner, hand);
  BeamItem::init(owner, hand);
}

void ObjectItem::update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) {
  FireableItem::update(dt, fireMode, shifting, moves);
  BeamItem::update(dt, fireMode, shifting, moves);
  setEnd(BeamItem::EndType::Object);
  m_shifting = shifting;
}

List<Drawable> ObjectItem::nonRotatedDrawables() const {
  return beamDrawables(canPlace(m_shifting));
}

float ObjectItem::cooldownTime() const {
  // TODO: Hardcoded
  return 0.25f;
}

void ObjectItem::fire(FireMode mode, bool shifting, bool edgeTriggered) {
  if (!ready())
    return;

  if (placeInWorld(mode, shifting))
    FireableItem::fire(mode, shifting, edgeTriggered);
}

String ObjectItem::objectName() const {
  return instanceValue("objectName", "<objectName missing>").toString();
}

Json ObjectItem::objectParameters() const {
  Json objectParameters = parameters().opt().value(JsonObject{});
  if (!initialized())
    return objectParameters;
  return objectParameters.set("owner", jsonFromMaybe(owner()->uniqueId()));
}

bool ObjectItem::placeInWorld(FireMode, bool shifting) {
  if (!initialized())
    throw ItemException("ObjectItem not init'd properly, or user not recognized as Tool User.");

  if (!ready())
    return false;

  if (!canPlace(shifting))
    return false;

  auto pos = Vec2I(owner()->aimPosition().floor());
  auto objectDatabase = root().objectDatabase();
  try {
    if (auto object = objectDatabase->createForPlacement(world(), objectName(), pos, owner()->walkingDirection(), objectParameters())) {
      if (consume(1)) {
        world()->addEntity(object);
        return true;
      }
    }
  } catch (StarException const& e) {
    root().logger().error("Failed to instantiate object for placement. {} {} : {}",
        objectName(),
        objectParameters().repr(0, true),
        outputException(e, true));
    return true;
  }

  return false;
}

bool ObjectItem::canPlace(bool) const {
  if (initialized()) {
    if (owner()->isAdmin() || owner()->inToolRange()) {
      auto pos = Vec2I(owner()->aimPosition().floor());
      auto objectDatabase = root().objectDatabase();
      return objectDatabase->canPlaceObject(world(), pos, objectName());
    }
  }
  return false;
}

}
