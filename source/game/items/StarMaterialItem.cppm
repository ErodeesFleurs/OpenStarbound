module;
#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarJsonExtra.hpp"
#include "StarXXHash.hpp"
#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarIdMap.hpp"
#include "StarDataStream.hpp"
#include "StarBiMap.hpp"
#include "StarCasting.hpp"
#include "StarStrongTypedef.hpp"
#include "StarVariant.hpp"
#include "StarRpcPromise.hpp"
#include "StarAStar.hpp"
#include "StarSpline.hpp"
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
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
#include "StarGameTypes.hpp"
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarBiMap.hpp"
#include "StarOrderedMap.hpp"
#include "StarBlockAllocator.hpp"
import star.lru_cache;
import star.time;
#include "StarRandom.hpp"
import star.ttl_cache;
#include "StarImage.hpp"
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarBiMap.hpp"
#include "StarMultiArray.hpp"
#include "StarGameTypes.hpp"
#include "StarMathCommon.hpp"
#include "StarVector.hpp"
#include "StarNetElementSystem.hpp"
#include "StarConfig.hpp"
import star.version;
#include "StarColor.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarGameTypes.hpp"
#include "StarBiMap.hpp"
#include "StarEither.hpp"
#include "StarVector.hpp"
#include "StarMaybe.hpp"
#include "StarNetElementSystem.hpp"
import star.weighted_pool;
#include "StarThread.hpp"
#include "StarEither.hpp"
#include "StarRect.hpp"
#include "StarImage.hpp"
#include "StarColor.hpp"
#include "StarInterpolation.hpp"
#include "StarList.hpp"
#include "StarVector.hpp"
#include "StarThread.hpp"
#include "StarRect.hpp"
#include "StarBiMap.hpp"
#include "StarMultiArray.hpp"
#include "StarGameTypes.hpp"
#include "StarVector.hpp"
#include "StarNetElementSystem.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarOrderedMap.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
import star.listener;
#include "StarArray.hpp"
#include "StarNetCompatibility.hpp"
import star.worker_pool;

#include "thread"
import star.sector_array_2d;
import star.perlin;
#include "StarBTree.hpp"
#include "StarDataStreamDevices.hpp"
import star.btree_database;
#include "StarOrderedSet.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarInputEvent.hpp"
#include "StarHash.hpp"
#include <functional>
import star.periodic_function;
#include "StarMatrix3.hpp"
#include "StarNetElement.hpp"


#include "StarLuaRoot.hpp"
#include "StarLuaComponents.hpp"
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.item_descriptor;
import star.item;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.entity;
import star.interaction_types;
import star.quest_descriptor;
import star.interactive_entity;
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
import star.non_rotated_drawables_item;
import star.beam_item;
import star.mixer;
import star.drawable;
import star.entity_rendering_types;
import star.animation;
import star.particle;

import star.light_source;
import star.entity_rendering;
STAR_STRUCT(PreviewTile);
STAR_CLASS(PreviewTileTool);
import star.preview_tile_tool;
import star.renderable_item;
import star.previewable_item;
import star.collision_block;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.worker_pool;
import star.tile_sector_array;
import star.world_parameters;
import star.celestial_parameters;
import star.world_layout;
import star.celestial_types;
import star.chat_types;
import star.uuid;
import star.warping;
import star.wiring;
import star.inspectable_entity;
import star.plant;
import star.biome_placement;
import star.versioning_database;
import star.world_storage;
import star.player_types;
import star.system_world;
import star.damage_manager;
import star.net_packets;
import star.world_structure;
import star.chat_action;
import star.world_client_state;
import star.interpolation_tracker;
import star.ambient;
import star.weather;
import star.world_client;
import star.world_template;
import star.input;
import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.collision_generator;
import star.world_tiles;
import star.drawable;
import star.entity_rendering_types;
import star.sky_types;
import star.celestial_coordinate;
import star.sky_parameters;
import star.sky_render_data;
import star.tile_damage;
import star.plant_database;
import star.parallax;
import star.animation;
import star.particle;
import star.weather_types;
import star.cellular_light_array;
import star.cellular_lighting;
import star.world_render_data;
import star.tile_damage;
import star.material_render_profile;
import star.drawable;
import star.animated_part_set;
import star.networked_animator;
import star.humanoid;
import star.lounging_entities;
import star.chatty_entity;
import star.emote_entity;
import star.portrait_entity;
import star.damage_bar_entity;
import star.nametag_entity;
import star.inventory_types;
import star.ai_types;
import star.radio_message_database;
import star.player;

export module star.material_item;
import star.material_database;
import star.tile_drawer;


export namespace Star {

STAR_CLASS(MaterialItem);

class MaterialItem : public Item, public FireableItem, public PreviewTileTool, public RenderableItem, public PreviewableItem, public BeamItem {
public:
  MaterialItem(Json const& config, String const& directory, Json const& settings);
  virtual ~MaterialItem() {}

  ItemPtr clone() const override;

  void init(ToolUserEntity* owner, ToolHand hand) override;
  void uninit() override;
  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;
  void render(RenderCallback* renderCallback, EntityRenderLayer renderLayer) override;

  virtual List<Drawable> preview(PlayerPtr const& viewer = {}) const override;
  virtual List<Drawable> dropDrawables() const override;
  List<Drawable> nonRotatedDrawables() const override;

  void fire(FireMode mode, bool shifting, bool edgeTriggered) override;
  void endFire(FireMode mode, bool shifting) override;

  MaterialId materialId() const;
  MaterialHue materialHueShift() const;

  bool canPlace(bool shifting) const;
  bool multiplaceEnabled() const;

  float& blockRadius();
  float& altBlockRadius();
  TileCollisionOverride& collisionOverride();

  List<PreviewTile> previewTiles(bool shifting) const override;
  List<Drawable> const& generatedPreview(Vec2I position = {}) const;
private:
  size_t blockSwap(float radius, TileLayer layer);
  void updatePropertiesFromPlayer(Player* player);
  float calcRadius(bool shifting) const;
  List<Vec2I>& tileArea(float radius, Vec2F const& position) const;
  MaterialHue placementHueShift(Vec2I const& position) const;

  MaterialId m_material;
  MaterialHue m_materialHueShift;

  float m_blockRadius;
  float m_altBlockRadius;
  bool m_blockSwap;
  bool m_shifting;
  bool m_multiplace;
  StringList m_placeSounds;
  Maybe<Vec2F> m_lastAimPosition;
  TileCollisionOverride m_collisionOverride;

  mutable Vec2F m_lastTileAreaOriginCache;
  mutable float m_lastTileAreaRadiusCache;
  mutable List<Vec2I> m_tileAreasCache;

  mutable Maybe<List<Drawable>> m_generatedPreviewCache;
};

}


namespace Star {

constexpr int BlockRadiusLimit = 16;
const String BlockRadiusPropertyKey = "building.blockRadius";
const String AltBlockRadiusPropertyKey = "building.altBlockRadius";
const String CollisionOverridePropertyKey = "building.collisionOverride";
const String BlockSwapPropertyKey = "building.blockSwap";

MaterialItem::MaterialItem(Json const& config, String const& directory, Json const& settings)
  : Item(config, directory, settings), FireableItem(config), BeamItem(config) {
  m_material = config.getInt("materialId");
  m_materialHueShift = materialHueFromDegrees(instanceValue("materialHueShift", 0).toFloat());
  auto materialDatabase = Root::singleton().materialDatabase();

  if (materialHueShift() != MaterialHue()) {
    auto drawables = iconDrawables();
    for (auto& d : drawables) {
      if (d.isImage()) {
        String image = strf("?hueshift={}", materialHueToDegrees(m_materialHueShift));
        d.imagePart().addDirectives(image, false);
      }
    }
    setIconDrawables(std::move(drawables));
  }

  setTwoHanded(config.getBool("twoHanded", true));

  auto defaultParameters = Root::singleton().assets()->json("/items/defaultParameters.config");
  setCooldownTime(config.queryFloat("materialItems.cooldown", defaultParameters.queryFloat("materialItems.cooldown")));
  m_blockRadius = config.getFloat("blockRadius", defaultParameters.getFloat("blockRadius"));
  m_altBlockRadius = config.getFloat("altBlockRadius", defaultParameters.getFloat("altBlockRadius"));
  m_collisionOverride = TileCollisionOverrideNames.maybeLeft(config.getString("collisionOverride", "None")).value(TileCollisionOverride::None);
  m_blockSwap = false;

  m_multiplace = config.getBool("allowMultiplace", BlockCollisionSet.contains(materialDatabase->materialCollisionKind(m_material)));
  m_placeSounds = jsonToStringList(config.get("placeSounds", JsonArray()));
  if (m_placeSounds.empty()) {
    auto miningSound = materialDatabase->miningSound(m_material);
    if (!miningSound.empty())
      m_placeSounds.append(std::move(miningSound));
    auto stepSound = materialDatabase->footstepSound(m_material);
    if (!stepSound.empty())
      m_placeSounds.append(std::move(stepSound));
    else if (m_placeSounds.empty())
      m_placeSounds.append(materialDatabase->defaultFootstepSound());
  }
  m_shifting = false;
  m_lastTileAreaRadiusCache = 0.0f;
}

ItemPtr MaterialItem::clone() const {
  return make_shared<MaterialItem>(*this);
}

void MaterialItem::init(ToolUserEntity* owner, ToolHand hand) {
  FireableItem::init(owner, hand);
  BeamItem::init(owner, hand);
  owner->addSound(Random::randValueFrom(m_placeSounds), 1.0f, 2.0f);
  if (auto player = as<Player>(owner))
    updatePropertiesFromPlayer(player);
}

void MaterialItem::uninit() {
  FireableItem::uninit();
  m_lastAimPosition.reset();
}

void MaterialItem::update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) {
  FireableItem::update(dt, fireMode, shifting, moves);
  BeamItem::update(dt, fireMode, shifting, moves);
  float radius = calcRadius(shifting);
  if (radius == 1)
    setEnd(BeamItem::EndType::Tile);
  else
    setEnd(BeamItem::EndType::TileGroup);
  m_shifting = shifting;

  if (Player* player = as<Player>(owner())) {
    if (owner()->isMaster()) {
      Input& input = Input::singleton();
      if (auto presses = input.bindDown("opensb", "materialCollisionCycle")) {
        CollisionKind baseKind = Root::singleton().materialDatabase()->materialCollisionKind(m_material);
        for (size_t i = 0; i != *presses; ++i) {
          constexpr auto limit = (uint8_t)TileCollisionOverride::Block + 1;
          while (true) {
            m_collisionOverride = TileCollisionOverride(((uint8_t)m_collisionOverride + 1) % limit);
            if (collisionKindFromOverride(m_collisionOverride) != baseKind)
              break;
          }
          player->setSecretProperty(CollisionOverridePropertyKey, TileCollisionOverrideNames.getRight(m_collisionOverride));
        }
        owner()->addSound("/sfx/tools/cyclematcollision.ogg", 1.0f, Random::randf(0.9f, 1.1f));
      }

      if (auto presses = input.bindDown("opensb", "buildingRadiusGrow")) {
        m_blockRadius = min(BlockRadiusLimit, int(m_blockRadius + *presses));
        player->setSecretProperty(BlockRadiusPropertyKey, m_blockRadius);
        owner()->addSound("/sfx/tools/buildradiusgrow.wav", 1.0f, 1.0f + m_blockRadius / BlockRadiusLimit);
      }

      if (auto presses = input.bindDown("opensb", "buildingRadiusShrink")) {
        m_blockRadius = max(1, int(m_blockRadius - *presses));
        player->setSecretProperty(BlockRadiusPropertyKey, m_blockRadius);
        owner()->addSound("/sfx/tools/buildradiusshrink.wav", 1.0f, 1.0f + m_blockRadius / BlockRadiusLimit);
      }

      if (auto presses = input.bindDown("opensb", "blockSwapToggle")) {
        if (*presses % 2 != 0)
          m_blockSwap = !m_blockSwap;
        player->setSecretProperty(BlockSwapPropertyKey, m_blockSwap);
        owner()->addSound(m_blockSwap ? "/sfx/interface/button/click.wav" : "/sfx/interface/button/release.wav", 1.0f, Random::randf(0.9f, 1.1f));
      }
    }
    else
      updatePropertiesFromPlayer(player);
  }
}

void MaterialItem::render(RenderCallback* renderCallback, EntityRenderLayer) {
  if (m_blockSwap || m_collisionOverride != TileCollisionOverride::None) {
    float pulse = (float)sin(2 * Constants::pi * 4.0 * Time::monotonicTime());
    float pulseA = 0.85 - pulse * 0.15f;
    float pulseB = 0.85 + pulse * 0.15f;
    Color color = owner()->favoriteColor().mix(Color::White);
    float alpha = color.alphaF();
    color.setAlphaF(alpha * pulseA * 0.95f);
    auto addIndicator = [&](String const& path) {
      Vec2F basePosition = Vec2F(0.5f, 0.5f);
      auto indicator = Drawable::makeImage(path, 1.0f / TilePixels, true, basePosition);
      indicator.fullbright = true;
      indicator.color = color;
      for (auto& tilePos : tileArea(calcRadius(m_shifting), owner()->aimPosition())) {
        indicator.position = basePosition + Vec2F(tilePos);
        renderCallback->addDrawable(indicator, RenderLayerForegroundTile);
      }
    };

    if (m_blockSwap) {
      auto prev = color;
      color.hueShift(0.167f);
      color.setAlphaF(alpha * pulseB * 0.95f);
      addIndicator("/interface/building/blockswap.png");
      color = prev;
    }

    if (m_collisionOverride == TileCollisionOverride::Empty)
      addIndicator("/interface/building/collisionempty.png");
    else if (m_collisionOverride == TileCollisionOverride::Platform)
      addIndicator("/interface/building/collisionplatform.png");
    else if (m_collisionOverride == TileCollisionOverride::Block)
      addIndicator("/interface/building/collisionblock.png");
  }
}

List<Drawable> MaterialItem::preview(PlayerPtr const&) const {
  return generatedPreview();
}

List<Drawable> MaterialItem::dropDrawables() const {
  return generatedPreview();
}

List<Drawable> MaterialItem::nonRotatedDrawables() const {
  return beamDrawables(canPlace(m_shifting));
}

void MaterialItem::fire(FireMode mode, bool shifting, bool edgeTriggered) {
  if (!initialized() || !ready())
    return;

  auto layer = (mode == FireMode::Primary || !twoHanded() ? TileLayer::Foreground : TileLayer::Background);
  TileModificationList modifications;

  float radius = calcRadius(shifting);

  auto geo = world()->geometry();
  auto aimPosition = owner()->aimPosition();

  if (!m_lastAimPosition)
    m_lastAimPosition = aimPosition;

  unsigned steps = 1;
  Vec2F diff = {};
  if (*m_lastAimPosition != aimPosition) {
    diff = geo.diff(*m_lastAimPosition, aimPosition);
    float magnitude = diff.magnitude();
    float limit = max(4.f, 64.f / radius);
    if (magnitude > limit) {
      diff = diff.normalized() * limit;
      magnitude = limit;
    }

    steps = (unsigned)ceil(magnitude * (Constants::pi / 2));
  }

  CollisionKind collisionKind = m_collisionOverride != TileCollisionOverride::None
    ? collisionKindFromOverride(m_collisionOverride)
    : Root::singleton().materialDatabase()->materialCollisionKind(m_material);

  size_t total = 0;

  if (m_blockSwap && owner()->inToolRange(aimPosition))
    total += blockSwap(radius, layer);
  
  for (unsigned i = 0; i != steps; ++i) {
    auto placementOrigin = aimPosition + diff * (1.0f - (static_cast<float>(i) / steps));
    if (!owner()->inToolRange(placementOrigin))
      continue;

    for (Vec2I& pos : tileArea(radius, placementOrigin))
      modifications.emplaceAppend(pos, PlaceMaterial{layer, materialId(), placementHueShift(pos), m_collisionOverride});

    // Make sure not to make any more modifications than we have consumables.
    if (modifications.size() > count())
      modifications.resize(count());
    size_t failed = world()->applyTileModifications(modifications, collisionKind <= CollisionKind::Platform).size();
    if (failed < modifications.size()) {
      size_t placed = modifications.size() - failed;
      consume(placed);
      total += placed;
    }
  }

  if (total) {
    float intensity = clamp(sqrt((float)total) / 16, 0.0f, 1.0f);
    owner()->addSound(Random::randValueFrom(m_placeSounds), 1.0f + intensity, (1.125f - intensity * 0.75f) * Random::randf(0.9f, 1.1f));
    FireableItem::fire(mode, shifting, edgeTriggered);
  }

  m_lastAimPosition = aimPosition;
}

void MaterialItem::endFire(FireMode, bool) {
  m_lastAimPosition.reset();
}

size_t MaterialItem::blockSwap(float radius, TileLayer layer) {
  Player* player = as<Player>(owner());
  if (!player)
    return 0;
  
  ItemPtr beamAxePtr = player->essentialItem(EssentialItem::BeamAxe);
  if (!beamAxePtr)
    return 0;
  
  Item* beamAxe = beamAxePtr.get();

  List<Vec2I> swapPositions;
  for (Vec2I& pos : tileArea(radius, owner()->aimPosition())) {
    if (!world()->isTileConnectable(pos, layer, true))
      continue;
    if (world()->isTileProtected(pos))
      continue;
    if (world()->material(pos, layer) == materialId())
      continue;
    swapPositions.append(pos);
  }

  if (swapPositions.empty())
    return 0;
  if (swapPositions.size() > count())
    swapPositions.resize(count());

  TileDamage damage;
  damage.type = TileDamageType::Beamish;
  damage.amount = beamAxe->instanceValue("tileDamage", beamAxe->instanceValue("primaryAbility.tileDamage", 1.0f)).toFloat();
  damage.harvestLevel = beamAxe->instanceValue("harvestLevel", beamAxe->instanceValue("primaryAbility.harvestLevel", 1)).toUInt();

  TileModificationList toSwap;
  List<Vec2I> willDamage;
  for (auto pos : swapPositions) {
    toSwap.emplaceAppend(pos, PlaceMaterial{layer, materialId(), placementHueShift(pos), m_collisionOverride});
    if (!world()->damageWouldDestroy(pos, layer, damage))
      willDamage.append(pos);
  }
  
  size_t success;
  size_t failed = world()->replaceTiles(toSwap, damage, true).size();

  if (failed < toSwap.size()) {
    success = toSwap.size() - failed;
    consume(success);
  } else {
    List<Vec2I> toDamage;

    for (auto pair : toSwap)
      toDamage.append(pair.first);
    
    world()->damageTiles(toDamage, layer, owner()->position(), damage, owner()->entityId());
  }

  if (willDamage.empty())
    return success;

  auto materialDatabase = Root::singleton().materialDatabase();
  auto assets = Root::singleton().assets();
  String blockSound;

  for (auto pos : willDamage) {
    blockSound = materialDatabase->miningSound(world()->material(pos, layer), world()->mod(pos, layer));
    if (!blockSound.empty())
      break;
  }

  if (blockSound.empty()) {
    for (auto pos : willDamage) {
      blockSound = materialDatabase->footstepSound(world()->material(pos, layer), world()->mod(pos, layer));
      if (!blockSound.empty()
          && blockSound != assets->json("/client.config:defaultFootstepSound").toString())
        break;
    }
  }

  owner()->addSound(blockSound, assets->json("/sfx.config:miningBlockVolume").toFloat());

  auto strikeSounds = beamAxe->instanceValue("strikeSounds");
  if (!strikeSounds.isNull()) {
    owner()->addSound(
        Random::randValueFrom(jsonToStringList(strikeSounds)),
        assets->json("/sfx.config:miningToolVolume").toFloat()
    );
  }

  if (FireableItem* item = as<FireableItem>(beamAxe))
    setFireTimer(item->windupTime() + item->cooldownTime());
  
  return success;
}

MaterialId MaterialItem::materialId() const {
  return m_material;
}

List<Drawable> const& MaterialItem::generatedPreview(Vec2I position) const {
  if (!m_generatedPreviewCache) {
    if (TileDrawer* tileDrawer = TileDrawer::singletonPtr()) {
      auto locker = tileDrawer->lockRenderData();
      WorldRenderData& renderData = tileDrawer->renderData();
      renderData.geometry = WorldGeometry(3, 3);
      renderData.tiles.resize({ 3, 3 });
      renderData.tiles.fill(TileDrawer::DefaultRenderTile);
      renderData.tileMinPosition = { 0, 0 };
      RenderTile& tile = renderData.tiles.at({ 1, 1 });
      tile.foreground = m_material;
      tile.foregroundHueShift = m_materialHueShift;
      tile.foregroundColorVariant = 0;

      List<Drawable> drawables;
      TileDrawer::Drawables tileDrawables;
      bool isBlock = BlockCollisionSet.contains(Root::singleton().materialDatabase()->materialCollisionKind(m_material));
      TileDrawer::TerrainLayer layer = isBlock ? TileDrawer::TerrainLayer::Foreground : TileDrawer::TerrainLayer::Midground;
      for (int x = 0; x != 3; ++x) {
        for (int y = 0; y != 3; ++y)
          tileDrawer->produceTerrainDrawables(tileDrawables, layer, { x, y }, renderData, 1.0f / TilePixels, position - Vec2I(1, 1));
      }

      locker.unlock();
      for (auto& index : tileDrawables.keys())
        drawables.appendAll(tileDrawables.take(index));

      auto boundBox = Drawable::boundBoxAll(drawables, true);
      if (!boundBox.isEmpty()) {
        for (auto& drawable : drawables)
          drawable.translate(-boundBox.center());
      }

      m_generatedPreviewCache.emplace(std::move(drawables));
    }
    else
      m_generatedPreviewCache.emplace(iconDrawables());
  }
  return *m_generatedPreviewCache;
}

void MaterialItem::updatePropertiesFromPlayer(Player* player) {
  auto blockRadius = player->getSecretProperty(BlockRadiusPropertyKey);
  if (blockRadius.isType(Json::Type::Float))
    m_blockRadius = blockRadius.toFloat();

  auto altBlockRadius = player->getSecretProperty(AltBlockRadiusPropertyKey);
  if (altBlockRadius.isType(Json::Type::Float))
    m_altBlockRadius = altBlockRadius.toFloat();

  auto collisionOverride = player->getSecretProperty(CollisionOverridePropertyKey);
  if (collisionOverride.isType(Json::Type::String))
    m_collisionOverride = TileCollisionOverrideNames.maybeLeft(collisionOverride.toString()).value(TileCollisionOverride::None);
  
  auto blockSwap = player->getSecretProperty(BlockSwapPropertyKey);
  if (blockSwap.isType(Json::Type::Bool))
    m_blockSwap = blockSwap.toBool();
}

float MaterialItem::calcRadius(bool shifting) const {
  if (!multiplaceEnabled())
    return 1;
  else
    return !shifting ? m_blockRadius : m_altBlockRadius;
}

List<Vec2I>& MaterialItem::tileArea(float radius, Vec2F const& position) const {
  if (m_lastTileAreaOriginCache != position || m_lastTileAreaRadiusCache != radius) {
    m_lastTileAreaOriginCache = position;
    m_lastTileAreaRadiusCache = radius;
    m_tileAreasCache = tileAreaBrush(radius, position, true);
  }
  return m_tileAreasCache;
}

MaterialHue MaterialItem::materialHueShift() const {
  return m_materialHueShift;
}

bool MaterialItem::canPlace(bool shifting) const {
  if (initialized()) {
    MaterialId material = materialId();

    float radius = calcRadius(shifting);

    for (auto& pos : tileArea(radius, owner()->aimPosition())) {
      MaterialHue hueShift = placementHueShift(pos);
      if (world()->canModifyTile(pos, PlaceMaterial{TileLayer::Foreground, material, hueShift}, false)
          || world()->canModifyTile(pos, PlaceMaterial{TileLayer::Background, material, hueShift}, false))
        return true;
    }
  }
  return false;
}

bool MaterialItem::multiplaceEnabled() const {
  return m_multiplace && count() > 1;
}

float& MaterialItem::blockRadius() {
  return m_blockRadius;
}

float& MaterialItem::altBlockRadius() {
  return m_altBlockRadius;
}

TileCollisionOverride& MaterialItem::collisionOverride() {
  return m_collisionOverride;
}

List<PreviewTile> MaterialItem::previewTiles(bool shifting) const {
  List<PreviewTile> result;
  if (initialized()) {
    Color lightColor = owner()->favoriteColor();
    Vec3B light = lightColor.toRgb();

    auto material = materialId();
    auto color = DefaultMaterialColorVariant;

    size_t c = 0;
    for (auto& pos : tileArea(calcRadius(shifting), owner()->aimPosition())) {
      MaterialHue hueShift = placementHueShift(pos);
      if (c >= count())
        break;
      if (world()->canModifyTile(pos, PlaceMaterial{TileLayer::Foreground, material, hueShift}, false)) {
        result.append({pos, true, material, hueShift, true});
        c++;
      } else if (twoHanded()
          && world()->canModifyTile(pos, PlaceMaterial{TileLayer::Background, material, hueShift}, false)) {
        result.append({pos, true, material, hueShift, true, light, true, color});
        c++;
      } else {
        result.append({pos, true, material, hueShift, true});
      }
    }
  }
  return result;
}

MaterialHue MaterialItem::placementHueShift(Vec2I const& pos) const {
  if (auto hue = instanceValue("materialHueShift")) {
    return materialHueFromDegrees(hue.toFloat());
  } else if (auto worldClient = as<WorldClient>(world())) {
    auto worldTemplate = worldClient->currentTemplate();
    return worldTemplate->biomeMaterialHueShift(worldTemplate->blockBiomeIndex(pos[0], pos[1]), m_material);
  } else {
    return materialHueShift();
  }
}

}
