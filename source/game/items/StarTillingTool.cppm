module;
#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarJsonExtra.hpp"
#include "StarIdMap.hpp"
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarBiMap.hpp"
#include "StarDataStream.hpp"
#include "StarBiMap.hpp"
#include "StarCasting.hpp"
#include "StarStrongTypedef.hpp"
#include "StarNetElementSystem.hpp"
#include "StarVariant.hpp"
#include "StarRpcPromise.hpp"
#include "StarRect.hpp"
#include "StarAStar.hpp"
#include "StarAudio.hpp"
#include "StarThread.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarSet.hpp"
#include "StarVector.hpp"
#include "StarMaybe.hpp"
#include "StarBiMap.hpp"
#include "StarColor.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
#include "StarGameTypes.hpp"
#include "StarOrderedMap.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
import star.listener;
#include "StarConfig.hpp"
import star.version;


import star.collision_block;
import star.item_descriptor;
import star.item;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.entity;
import star.interaction_types;
import star.tile_damage;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
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
import star.swingable_item;
import star.mixer;
import star.drawable;
import star.entity_rendering_types;
import star.animation;
import star.particle;

import star.light_source;
import star.entity_rendering;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

export module star.tilling_tool;
import star.material_database;


export namespace Star {

STAR_CLASS(TillingTool);

class TillingTool : public Item, public SwingableItem {
public:
  TillingTool(Json const& config, String const& directory, Json const& parameters = JsonObject());

  ItemPtr clone() const override;

  List<Drawable> drawables() const override;
  // In pixels, offset from image center
  Vec2F handPosition() const override;
  void fire(FireMode mode, bool shifting, bool edgeTriggered) override;
  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;
  float getAngle(float aimAngle) override;

private:
  String m_image;
  int m_frames;
  float m_frameCycle;
  float m_frameTiming;
  List<String> m_animationFrame;
  String m_idleFrame;

  Vec2F m_handPosition;

  String m_idleSound;
  StringList m_strikeSounds;
  float m_toolVolume;
};

}

namespace Star {

TillingTool::TillingTool(Json const& config, String const& directory, Json const& parameters)
  : Item(config, directory, parameters), SwingableItem(config) {
  auto assets = Root::singleton().assets();

  m_image = AssetPath::relativeTo(directory, instanceValue("image").toString());
  m_frames = instanceValue("frames", 1).toInt();
  m_frameCycle = instanceValue("animationCycle", 1.0f).toFloat();
  for (size_t i = 0; i < (size_t)m_frames; i++)
    m_animationFrame.append(m_image.replace("{frame}", toString(i)));
  m_idleFrame = m_image.replace("{frame}", "idle");

  m_handPosition = jsonToVec2F(instanceValue("handPosition"));
  m_strikeSounds = jsonToStringList(instanceValue("strikeSounds"));
  m_toolVolume = assets->json("/sfx.config:harvestToolVolume").toFloat();
  m_frameTiming = 0;
}

ItemPtr TillingTool::clone() const {
  return make_shared<TillingTool>(*this);
}

List<Drawable> TillingTool::drawables() const {
  if (m_frameTiming == 0)
    return {Drawable::makeImage(m_idleFrame, 1.0f / TilePixels, true, -handPosition() / TilePixels)};
  else {
    int frame = std::max(0, std::min(m_frames - 1, (int)std::floor((m_frameTiming / m_frameCycle) * m_frames)));
    return {Drawable::makeImage(m_animationFrame[frame], 1.0f / TilePixels, true, -handPosition() / TilePixels)};
  }
}

Vec2F TillingTool::handPosition() const {
  return m_handPosition;
}

void TillingTool::fire(FireMode mode, bool shifting, bool edgeTriggered) {
  if (!ready())
    return;

  auto strikeSound = Random::randValueFrom(m_strikeSounds);

  if (owner() && world()) {
    auto materialDatabase = Root::singleton().materialDatabase();
    Vec2I pos(owner()->aimPosition().floor());

    if (world()->material(pos + Vec2I(0, 1), TileLayer::Foreground) != EmptyMaterialId)
      return;

    bool used = false;
    for (auto layer : {TileLayer::Foreground, TileLayer::Background}) {
      if (world()->material(pos, layer) == EmptyMaterialId)
        pos = pos - Vec2I(0, 1);

      if ((layer == TileLayer::Background)
          && world()->material(pos + Vec2I(0, 1), TileLayer::Background) != EmptyMaterialId)
        continue;

      if (owner()->isAdmin() || owner()->inToolRange()) {
        auto currentMod = world()->mod(pos, layer);
        auto material = world()->material(pos, layer);
        auto tilledMod = materialDatabase->tilledModFor(material);

        if (tilledMod != NoModId && currentMod == NoModId) {
          if (world()->modifyTile(pos, PlaceMod{layer, tilledMod, MaterialHue()}, true))
            used = true;
        } else if (currentMod != tilledMod) {
          auto damageResult = world()->damageTile(pos, layer, owner()->position(), {TileDamageType::Tilling, 1.0f});
          used = damageResult != TileDamageResult::None;
          if (damageResult == TileDamageResult::Protected) {
            strikeSound = Root::singleton().assets()->json("/client.config:defaultDingSound").toString();
          }
        }
      }
    }

    if (used) {
      owner()->addSound(strikeSound, m_toolVolume);
      SwingableItem::fire(mode, shifting, edgeTriggered);
    }
  }
}

void TillingTool::update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) {
  SwingableItem::update(dt, fireMode, shifting, moves);

  if (!ready() && !coolingDown())
    m_frameTiming = std::fmod((m_frameTiming + dt), m_frameCycle);
  else
    m_frameTiming = 0;
}

float TillingTool::getAngle(float aimAngle) {
  if (!ready() && !coolingDown())
    return SwingableItem::getAngle(aimAngle);
  return aimAngle;
}

}
