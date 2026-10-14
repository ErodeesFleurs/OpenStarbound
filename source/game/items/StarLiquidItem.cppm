module;
import star.json;
import star.id_map;
#include "StarCasting.hpp"
#include "StarStrongTypedef.hpp"
import star.variant;
import star.rpc_promise;
import star.a_star;
import star.vector;
import star.spline;
import star.audio;
import star.thread;
import star.list;
import star.map;
import star.set;
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
import star.liquid_types;
import star.material_types;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.version;


import star.item;
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
#include "StarLuaRoot.hpp"
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
#include "StarLuaComponents.hpp"
import star.fireable_item;
import star.non_rotated_drawables_item;
import star.beam_item;
import star.mixer;
import star.drawable;
import star.entity_rendering_types;
import star.animation;
import star.particle;

import star.light_source;
import star.entity_rendering;
struct PreviewTile;
using PreviewTilePtr = std::shared_ptr<PreviewTile>;
using PreviewTileConstPtr = std::shared_ptr<const PreviewTile>;
using PreviewTileWeakPtr = std::weak_ptr<PreviewTile>;
using PreviewTileConstWeakPtr = std::weak_ptr<const PreviewTile>;
using PreviewTileUPtr = std::unique_ptr<PreviewTile>;
using PreviewTileConstUPtr = std::unique_ptr<const PreviewTile>;
class PreviewTileTool;
using PreviewTileToolPtr = std::shared_ptr<PreviewTileTool>;
using PreviewTileToolConstPtr = std::shared_ptr<const PreviewTileTool>;
using PreviewTileToolWeakPtr = std::weak_ptr<PreviewTileTool>;
using PreviewTileToolConstWeakPtr = std::weak_ptr<const PreviewTileTool>;
using PreviewTileToolUPtr = std::unique_ptr<PreviewTileTool>;
using PreviewTileToolConstUPtr = std::unique_ptr<const PreviewTileTool>;
import star.preview_tile_tool;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

export module star.liquid_item;
import star.liquids_database;

export namespace Star {

class LiquidItem;
using LiquidItemPtr = std::shared_ptr<LiquidItem>;
using LiquidItemConstPtr = std::shared_ptr<const LiquidItem>;
using LiquidItemWeakPtr = std::weak_ptr<LiquidItem>;
using LiquidItemConstWeakPtr = std::weak_ptr<const LiquidItem>;
using LiquidItemUPtr = std::unique_ptr<LiquidItem>;
using LiquidItemConstUPtr = std::unique_ptr<const LiquidItem>;

class LiquidItem : public Item, public FireableItem, public PreviewTileTool, public BeamItem {
public:
  LiquidItem(Json const& config, String const& directory, Json const& settings);
  virtual ~LiquidItem() {}

  ItemPtr clone() const override;

  void init(ToolUserEntity* owner, ToolHand hand) override;
  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;

  List<Drawable> nonRotatedDrawables() const override;

  void fire(FireMode mode, bool shifting, bool edgeTriggered) override;

  LiquidId liquidId() const;
  float liquidQuantity() const;

  List<PreviewTile> previewTiles(bool shifting) const override;

  bool canPlace(bool shifting) const;
  bool canPlaceAtTile(Vec2I pos) const;
  bool multiplaceEnabled() const;

private:
  LiquidId m_liquidId;
  float m_quantity;

  float m_blockRadius;
  float m_altBlockRadius;
  bool m_shifting;
};

}

namespace Star {

LiquidItem::LiquidItem(Json const& config, String const& directory, Json const& settings)
  : Item(config, directory, settings), FireableItem(config), BeamItem(config) {
  m_liquidId = Root::singleton().liquidsDatabase()->liquidId(config.getString("liquid"));

  setTwoHanded(config.getBool("twoHanded", true));

  auto assets = Root::singleton().assets();
  m_quantity = assets->json("/items/defaultParameters.config:liquidItems.bucketSize").toUInt();
  setCooldownTime(assets->json("/items/defaultParameters.config:liquidItems.cooldown").toFloat());
  m_blockRadius = assets->json("/items/defaultParameters.config:blockRadius").toFloat();
  m_altBlockRadius = assets->json("/items/defaultParameters.config:altBlockRadius").toFloat();
  m_shifting = false;
}

ItemPtr LiquidItem::clone() const {
  return std::make_shared<LiquidItem>(*this);
}

void LiquidItem::init(ToolUserEntity* owner, ToolHand hand) {
  FireableItem::init(owner, hand);
  BeamItem::init(owner, hand);
}

void LiquidItem::update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) {
  FireableItem::update(dt, fireMode, shifting, moves);
  BeamItem::update(dt, fireMode, shifting, moves);
  if (shifting || !multiplaceEnabled())
    setEnd(BeamItem::EndType::Tile);
  else
    setEnd(BeamItem::EndType::TileGroup);

  m_shifting = shifting;
}

List<Drawable> LiquidItem::nonRotatedDrawables() const {
  return beamDrawables(canPlace(m_shifting));
}

void LiquidItem::fire(FireMode mode, bool shifting, bool edgeTriggered) {
  if (!initialized() || !ready() || !owner()->inToolRange())
    return;

  PlaceLiquid placeLiquid{liquidId(), liquidQuantity()};
  TileModificationList modifications;

  float radius;

  if (!shifting)
    radius = m_blockRadius;
  else
    radius = m_altBlockRadius;

  if (!multiplaceEnabled())
    radius = 1;

  for (auto pos : tileAreaBrush(radius, owner()->aimPosition(), true)) {
    if (canPlaceAtTile(pos))
      modifications.append({pos, placeLiquid});
  }

  // Make sure not to make any more modifications than we have consumables.
  if (modifications.size() > count())
    modifications.resize(count());
  std::size_t failed = world()->applyTileModifications(modifications, false).size();
  if (failed < modifications.size()) {
    FireableItem::fire(mode, shifting, edgeTriggered);
    consume(modifications.size() - failed);
  }
}

LiquidId LiquidItem::liquidId() const {
  return m_liquidId;
}

float LiquidItem::liquidQuantity() const {
  return m_quantity;
}

List<PreviewTile> LiquidItem::previewTiles(bool shifting) const {
  List<PreviewTile> result;
  if (initialized()) {
    auto liquid = liquidId();

    float radius;
    if (!shifting)
      radius = m_blockRadius;
    else
      radius = m_altBlockRadius;

    if (!multiplaceEnabled())
      radius = 1;

    std::size_t c = 0;

    for (auto pos : tileAreaBrush(radius, owner()->aimPosition(), true)) {
      if (c >= count())
        break;
      if (canPlaceAtTile(pos))
        c++;
      result.append({pos, liquid});
    }
  }
  return result;
}

bool LiquidItem::canPlace(bool shifting) const {
  if (initialized()) {
    float radius;
    if (!shifting)
      radius = m_blockRadius;
    else
      radius = m_altBlockRadius;

    if (!multiplaceEnabled())
      radius = 1;

    for (auto pos : tileAreaBrush(radius, owner()->aimPosition(), true)) {
      if (canPlaceAtTile(pos))
        return true;
    }
  }
  return false;
}

bool LiquidItem::canPlaceAtTile(Vec2I pos) const {
  auto bgTileMaterial = world()->material(pos, TileLayer::Background);
  if (bgTileMaterial != EmptyMaterialId) {
    auto fgTileMaterial = world()->material(pos, TileLayer::Foreground);
    if (fgTileMaterial == EmptyMaterialId) {
      auto tileLiquid = world()->liquidLevel(pos).liquid;
      if (tileLiquid == EmptyLiquidId || tileLiquid == liquidId())
        return true;
    }
  }
  return false;
}

bool LiquidItem::multiplaceEnabled() const {
  return (count() > 1);
}

}
