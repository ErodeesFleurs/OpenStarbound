module;
#include "StarJson.hpp"
#include "StarJsonExtra.hpp"
#include "StarIdMap.hpp"
#include "StarNetElementBasicFields.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarGameTypes.hpp"
#include "StarPoly.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"
#include "StarColor.hpp"
#include "StarDirectives.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarNetElementSystem.hpp"
#include "StarVariant.hpp"
#include "StarList.hpp"
#include "StarRpcPromise.hpp"
#include "StarRect.hpp"
#include "StarAStar.hpp"
#include "StarConfig.hpp"
#include "StarDataStreamExtra.hpp"
#include "StarSet.hpp"
#include "StarString.hpp"
#include "StarPeriodicFunction.hpp"
#include "StarOrderedMap.hpp"
#include "StarMatrix3.hpp"
#include "StarAudio.hpp"
#include "StarThread.hpp"
#include "StarMap.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarListener.hpp"
#include "StarVersion.hpp"
#include "StarArray.hpp"
#include "StarOrderedSet.hpp"
#include "StarEither.hpp"
#include "StarNetElement.hpp"


#include "StarLuaRoot.hpp"
#include "StarLuaComponents.hpp"
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.drawable;
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
import star.animated_part_set;
import star.mixer;
import star.networked_animator;
import star.durability_item;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.uuid;
import star.humanoid;
import star.lounging_entities;
import star.chat_action;
import star.chatty_entity;
import star.portrait_entity;
import star.damage_bar_entity;
import star.nametag_entity;
import star.inspectable_entity;
import star.inventory_types;
import star.ai_types;
import star.entity_rendering_types;
import star.entity_rendering;
import star.player_types;
import star.radio_message_database;
import star.player;
import star.emote_entity;


namespace Star {

STAR_CLASS(ActiveItem);

class ActiveItem :
  public Item,
  public DurabilityItem,
  public virtual ToolUserItem,
  public virtual NetElementGroup {
public:
  ActiveItem(Json const& config, String const& directory, Json const& parameters = JsonObject());
  ActiveItem(ActiveItem const& rhs);

  ItemPtr clone() const override;

  void init(ToolUserEntity* owner, ToolHand hand) override;
  void uninit() override;

  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;

  List<DamageSource> damageSources() const override;
  List<PolyF> shieldPolys() const override;

  List<PhysicsForceRegion> forceRegions() const override;

  bool holdingItem() const;
  Maybe<String> backArmFrame() const;
  Maybe<String> frontArmFrame() const;
  bool twoHandedGrip() const;
  bool recoil() const;
  bool outsideOfHand() const;

  float armAngle() const;
  Maybe<Direction> facingDirection() const;

  // Hand drawables are in hand-space, everything else is in world space.
  List<Drawable> handDrawables() const;
  List<pair<Drawable, Maybe<EntityRenderLayer>>> entityDrawables() const;
  List<LightSource> lights() const;
  List<AudioInstancePtr> pullNewAudios();
  List<Particle> pullNewParticles();

  Maybe<String> cursor() const;

  Maybe<ChainableJsonMessageResponse> receiveMessage(String const& message, bool localMessage, JsonArray const& args = {});

  float durabilityStatus() override;

private:
  Vec2F armPosition(Vec2F const& offset) const;
  Vec2F handPosition(Vec2F const& offset) const;

  LuaCallbacks makeActiveItemCallbacks();
  LuaCallbacks makeScriptedAnimationCallbacks();

  mutable LuaMessageHandlingComponent<LuaActorMovementComponent<LuaUpdatableComponent<LuaStorableComponent<LuaWorldComponent<LuaBaseComponent>>>>> m_script;

  NetworkedAnimator m_itemAnimator;
  NetworkedAnimator::DynamicTarget m_itemAnimatorDynamicTarget;

  mutable LuaAnimationComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>> m_scriptedAnimator;

  HashMap<AudioInstancePtr, Vec2F> m_activeAudio;

  FireMode m_currentFireMode;
  Maybe<String> m_cursor;

  NetElementBool m_holdingItem;
  NetElementData<Maybe<String>> m_backArmFrame;
  NetElementData<Maybe<String>> m_frontArmFrame;
  NetElementBool m_twoHandedGrip;
  NetElementBool m_recoil;
  NetElementBool m_outsideOfHand;
  NetElementFloat m_armAngle;
  NetElementData<Maybe<Direction>> m_facingDirection;
  NetElementData<List<DamageSource>> m_damageSources;
  NetElementData<List<DamageSource>> m_itemDamageSources;
  NetElementData<List<PolyF>> m_shieldPolys;
  NetElementData<List<PolyF>> m_itemShieldPolys;
  NetElementData<List<PhysicsForceRegion>> m_forceRegions;
  NetElementData<List<PhysicsForceRegion>> m_itemForceRegions;
  NetElementHashMap<String, Json> m_scriptedAnimationParameters;
};

}

export module star.active_item;

export namespace Star {
  using ::Star::ActiveItem;
  using ::Star::ActiveItemPtr;
  using ::Star::ActiveItemConstPtr;
  using ::Star::ActiveItemWeakPtr;
  using ::Star::ActiveItemConstWeakPtr;
  using ::Star::ActiveItemUPtr;
  using ::Star::ActiveItemConstUPtr;
}

