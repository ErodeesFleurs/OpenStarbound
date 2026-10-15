module;
import std;
import star.json;
import star.json_extra;
import star.id_map;
import star.casting;
import star.strong_typedef;
import star.variant;
import star.rpc_promise;
import star.a_star;
import star.audio;
import star.thread;
import star.list;
import star.map;
import star.set;
import star.vector;
import star.maybe;
import star.bi_map;
import star.color;
import star.string;
import star.data_stream;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.game_types;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;


import star.item;
import star.pointable_item;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.entity;
import star.interaction_types;
import star.tile_damage;
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
import star.platformer_astar_types;
import star.anchorable_entity;
import star.game_timers;
import star.actor_movement_controller;
import star.mobile_entity;
import star.actor_entity;
import star.tool_user_entity;
import star.tool_user_item;
import star.mixer;
import star.drawable;
import star.entity_rendering_types;
import star.animation;
import star.particle;

import star.light_source;
import star.entity_rendering;
import star.asset_source;
import star.assets;

export module star.flashlight;

export namespace Star {

class Flashlight : public Item, public PointableItem, public ToolUserItem {
public:
  Flashlight(Json const& config, String const& directory, Json const& parameters = JsonObject());

  std::shared_ptr<Item> clone() const override;

  List<Drawable> drawables() const override;

  List<LightSource> lightSources() const;

private:
  String m_image;
  Vec2F m_handPosition;
  Vec2F m_lightPosition;
  Color m_lightColor;
  float m_beamWidth;
  float m_ambientFactor;
};

}

namespace Star {

Flashlight::Flashlight(Json const& config, String const& directory, Json const& parameters)
  : Item(config, directory, parameters) {
  m_image = AssetPath::relativeTo(directory, instanceValue("image").toString());
  m_handPosition = jsonToVec2F(instanceValue("handPosition"));
  m_lightPosition = jsonToVec2F(instanceValue("lightPosition"));
  m_lightColor = jsonToColor(instanceValue("lightColor"));
  m_beamWidth = instanceValue("beamLevel").toFloat();
  m_ambientFactor = instanceValue("beamAmbience").toFloat();
}

std::shared_ptr<Item> Flashlight::clone() const {
  return std::make_shared<Flashlight>(*this);
}

List<Drawable> Flashlight::drawables() const {
  return {Drawable::makeImage(m_image, 1.0f / TilePixels, true, -m_handPosition / TilePixels)};
}

List<LightSource> Flashlight::lightSources() const {
  if (!initialized())
    return {};

  float angle = world()->geometry().diff(owner()->aimPosition(), owner()->position()).angle();
  LightSource lightSource;
  lightSource.type = LightType::Point;
  lightSource.position = owner()->position() + owner()->handPosition(hand(), (m_lightPosition - m_handPosition) / TilePixels);
  lightSource.color = m_lightColor.toRgbF();
  lightSource.pointBeam = m_beamWidth;
  lightSource.beamAngle = angle;
  lightSource.beamAmbience = m_ambientFactor;
  return {std::move(lightSource)};
}

}
