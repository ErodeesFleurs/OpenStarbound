module;
import std;
import star.type_identity;
import star.json;
import star.json_extra;
import star.id_map;
import star.strong_typedef;
import star.data_stream;
import star.vector;
import star.game_types;
import star.casting;
import star.bi_map;
import star.list;
import star.random;
import star.periodic;
import star.periodic_function;
import star.set;
import star.color;
import star.lua;
import star.audio;
import star.thread;
import star.map;
import star.maybe;
import star.string;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.ordered_map;


import star.uuid;
import star.celestial_coordinate;
import star.warping;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.item_descriptor;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.warp_target_entity;
import star.lua_components;
import star.lua_animation_component;
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
import star.root;

export module star.teleporter_object;

export namespace Star {

class TeleporterObject : public Object, public WarpTargetEntity {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<TeleporterObject, Object, WarpTargetEntity>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<TeleporterObject>();
  }

  TeleporterObject(Root& rootContext, std::shared_ptr<ObjectConfig const> config, Json const& parameters = JsonObject());

  Vec2F footPosition() const override;
};

}

namespace Star {

TeleporterObject::TeleporterObject(Root& rootContext, std::shared_ptr<ObjectConfig const> config, Json const& parameters) : Entity(rootContext), Object(rootContext, config, parameters) {
  setUniqueId(configValue("uniqueId", Uuid(root().random()).hex()).optString());
}

Vec2F TeleporterObject::footPosition() const {
  if (auto footPos = configValue("teleporterFootPosition"))
    return jsonToVec2F(footPos);
  return Vec2F();
}

}
