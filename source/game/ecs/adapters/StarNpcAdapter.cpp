#include "StarNpcAdapter.hpp"
#include "StarRoot.hpp"
#include "StarWorld.hpp"
#include "StarNpcDatabase.hpp"
#include "StarDamageDatabase.hpp"
#include "StarDamageManager.hpp"
#include "StarTreasure.hpp"
#include "StarItemDrop.hpp"
#include "StarItemDatabase.hpp"
#include "StarEmoteProcessor.hpp"
#include "StarDanceDatabase.hpp"
#include "StarSpeciesDatabase.hpp"
#include "StarEntityRendering.hpp"
#include "StarConfigLuaBindings.hpp"
#include "StarEntityLuaBindings.hpp"
#include "StarWorldLuaBindings.hpp"
#include "StarStatusControllerLuaBindings.hpp"
#include "StarBehaviorLuaBindings.hpp"
#include "StarNetworkedAnimatorLuaBindings.hpp"
#include "StarScriptedAnimatorLuaBindings.hpp"
#include "StarSongbookLuaBindings.hpp"
#include "StarArmors.hpp"
#include "StarFireableItem.hpp"
#include "StarJsonExtra.hpp"

namespace Star {
namespace ECS {

shared_ptr<NpcAdapter> NpcAdapter::create(World& ecsWorld, NpcVariant const& variant) {
  auto entity = ecsWorld.createEntity();
  
  // Add tag and data components
  ecsWorld.addComponent<NpcTag>(entity);
  auto& data = ecsWorld.addComponent<NpcDataComponent>(entity);
  
  data.npcVariant = variant;
  
  // Initialize humanoid
  data.humanoid = make_shared<Humanoid>(variant.humanoidIdentity);
  data.humanoid->setHeadArmorDirectives(variant.humanoidConfig.getString("headArmorDirectives", ""));
  data.humanoid->setChestArmorDirectives(variant.humanoidConfig.getString("chestArmorDirectives", ""));
  data.humanoid->setLegsArmorDirectives(variant.humanoidConfig.getString("legsArmorDirectives", ""));
  data.humanoid->setBackArmorDirectives(variant.humanoidConfig.getString("backArmorDirectives", ""));

  data.emoteState = HumanoidEmote::Idle;
  data.chatMessageUpdated = false;

  auto assets = Root::singleton().assets();

  data.emoteCooldownTimer = GameTimer(assets->json("/npcs/npc.config:emoteCooldown").toFloat());
  data.danceCooldownTimer = GameTimer(0.0f);
  data.blinkInterval = jsonToVec2F(assets->json("/npcs/npc.config:blinkInterval"));

  data.questIndicatorOffset = jsonToVec2F(assets->json("/quests/quests.config:defaultIndicatorOffset"));

  if (variant.overrides)
    data.clientEntityMode = ClientEntityModeNames.getLeft(variant.overrides.getString("clientEntityMode", "ClientSlaveOnly"));

  data.isInteractive = false;
  data.shifting = false;
  data.damageOnTouch = false;

  data.dropPools = variant.dropPools;

  data.scriptComponent.setScripts(variant.scripts);
  data.scriptComponent.setUpdateDelta(variant.initialScriptDelta);

  auto movementParameters = ActorMovementParameters(jsonMerge(data.humanoid->defaultMovementParameters(), variant.movementParameters));
  if (!movementParameters.physicsEffectCategories)
    movementParameters.physicsEffectCategories = StringSet({"npc"});
  data.movementController = make_shared<ActorMovementController>(movementParameters);
  
  data.identityUpdated = false;
  data.deathParticleBurst = data.humanoid->defaultDeathParticles();

  data.statusController = make_shared<StatusController>(variant.statusControllerSettings);
  data.statusController->setPersistentEffects("innate", variant.innateStatusEffects);
  auto speciesDefinition = Root::singleton().speciesDatabase()->species(variant.humanoidIdentity.species);
  data.statusController->setPersistentEffects("species", speciesDefinition->statusEffects());
  data.statusController->setStatusProperty("species", variant.humanoidIdentity.species);
  if (!data.statusController->statusProperty("effectDirectives"))
    data.statusController->setStatusProperty("effectDirectives", speciesDefinition->effectDirectives());

  data.songbook = make_shared<Songbook>(variant.humanoidIdentity.species);

  data.effectEmitter = make_shared<EffectEmitter>();

  data.hitDamageNotificationLimiter = 0;
  data.hitDamageNotificationLimit = assets->json("/npcs/npc.config:hitDamageNotificationLimit").toInt();

  data.blinkCooldownTimer = GameTimer();

  data.armor = make_shared<ArmorWearer>();
  data.tools = make_shared<ToolUser>();

  data.aggressive = false;

  auto adapter = make_shared<NpcAdapter>(ecsWorld, entity);
  adapter->setPersistent(variant.persistent);
  adapter->setKeepAlive(variant.keepAlive);
  adapter->setTeam(EntityDamageTeam(variant.damageTeamType, variant.damageTeam));
  
  return adapter;
}

shared_ptr<NpcAdapter> NpcAdapter::createFromDiskStore(World& ecsWorld, Json const& diskStore) {
  auto npcDatabase = Root::singleton().npcDatabase();
  NpcVariant variant = npcDatabase->generateNpcVariant(diskStore.get("npcVariant"));
  
  auto adapter = create(ecsWorld, variant);
  auto* data = adapter->getData();
  
  data->movementController->loadState(diskStore.get("movementController"));
  data->statusController->diskLoad(diskStore.get("statusController"));
  
  auto aimPosition = jsonToVec2F(diskStore.get("aimPosition"));
  data->xAimPosition = aimPosition[0];
  data->yAimPosition = aimPosition[1];
  
  data->humanoid->setState(Humanoid::StateNames.getLeft(diskStore.getString("humanoidState")));
  data->humanoid->setEmoteState(HumanoidEmoteNames.getLeft(diskStore.getString("humanoidEmoteState")));
  data->isInteractive = diskStore.getBool("isInteractive");
  data->shifting = diskStore.getBool("shifting");
  data->damageOnTouch = diskStore.getBool("damageOnTouch", false);

  data->effectEmitter->fromJson(diskStore.get("effectEmitter"));

  data->armor->diskLoad(diskStore.get("armor"));
  data->tools->diskLoad(diskStore.get("tools"));

  data->disableWornArmor = diskStore.getBool("disableWornArmor");

  data->scriptComponent.setScriptStorage(diskStore.getObject("scriptStorage"));

  adapter->setUniqueId(diskStore.optString("uniqueId"));
  if (diskStore.contains("team"))
    adapter->setTeam(EntityDamageTeam(diskStore.get("team")));

  data->deathParticleBurst = diskStore.optString("deathParticleBurst");

  data->dropPools = diskStore.getArray("dropPools").transformed(mem_fn(&Json::toString));

  data->blinkCooldownTimer = GameTimer();

  data->aggressive = diskStore.getBool("aggressive");
  
  return adapter;
}

shared_ptr<NpcAdapter> NpcAdapter::createFromNetStore(World& ecsWorld, ByteArray const& netStore, NetCompatibilityRules rules) {
  NpcVariant variant = Root::singleton().npcDatabase()->readNpcVariant(netStore, rules);
  return create(ecsWorld, variant);
}

NpcAdapter::NpcAdapter(World& ecsWorld, Entity entity)
  : EntityAdapter(ecsWorld, entity) {
  setupNetStates();
}

Json NpcAdapter::diskStore() const {
  auto const* data = getData();
  return JsonObject{
    {"npcVariant", Root::singleton().npcDatabase()->writeNpcVariantToJson(data->npcVariant)},
    {"movementController", data->movementController->storeState()},
    {"statusController", data->statusController->diskStore()},
    {"armor", data->armor->diskStore()},
    {"tools", data->tools->diskStore()},
    {"aimPosition", jsonFromVec2F({data->xAimPosition, data->yAimPosition})},
    {"humanoidState", Humanoid::StateNames.getRight(data->humanoid->state())},
    {"humanoidEmoteState", HumanoidEmoteNames.getRight(data->humanoid->emoteState())},
    {"isInteractive", data->isInteractive},
    {"shifting", data->shifting},
    {"damageOnTouch", data->damageOnTouch},
    {"effectEmitter", data->effectEmitter->toJson()},
    {"disableWornArmor", data->disableWornArmor},
    {"scriptStorage", data->scriptComponent.getScriptStorage()},
    {"uniqueId", jsonFromMaybe(uniqueId())},
    {"team", getTeam().toJson()},
    {"deathParticleBurst", jsonFromMaybe(data->deathParticleBurst)},
    {"dropPools", data->dropPools.transformed(construct<Json>())},
    {"aggressive", data->aggressive}
  };
}

ByteArray NpcAdapter::netStore(NetCompatibilityRules rules) {
  return Root::singleton().npcDatabase()->writeNpcVariant(getData()->npcVariant, rules);
}

EntityType NpcAdapter::entityType() const {
  return EntityType::Npc;
}

ClientEntityMode NpcAdapter::clientEntityMode() const {
  return getData()->clientEntityMode;
}

void NpcAdapter::init(::Star::World* world, EntityId entityId, EntityMode mode) {
  EntityAdapter::init(world, entityId, mode);
  
  auto* data = getData();
  data->movementController->init(world);
  data->movementController->setIgnorePhysicsEntities({entityId});
  data->statusController->init(this, data->movementController.get());
  data->tools->init(this);

  data->armor->setupHumanoid(*data->humanoid, forceNude());

  if (isMaster()) {
    data->movementController->resetAnchorState();

    auto itemDatabase = Root::singleton().itemDatabase();
    for (auto const& item : data->npcVariant.items)
      setItemSlot(item.first, item.second);
    
    data->scriptComponent.addCallbacks("npc", makeNpcCallbacks());
    data->scriptComponent.addCallbacks("config",
        LuaBindings::makeConfigCallbacks([data](String const& name, Json const& def)
            { return data->npcVariant.scriptConfig.query(name, def); }));
    data->scriptComponent.addCallbacks("entity", LuaBindings::makeEntityCallbacks(this));
    data->scriptComponent.addCallbacks("status", LuaBindings::makeStatusControllerCallbacks(data->statusController.get()));
    data->scriptComponent.addCallbacks("behavior", LuaBindings::makeBehaviorCallbacks(&data->behaviors));
    data->scriptComponent.addCallbacks("songbook", LuaBindings::makeSongbookCallbacks(data->songbook.get()));
    data->scriptComponent.addCallbacks("animator", LuaBindings::makeNetworkedAnimatorCallbacks(data->humanoid->networkedAnimator()));
    data->scriptComponent.addActorMovementCallbacks(data->movementController.get());
    data->scriptComponent.init(world);
  }
  if (world->isClient()) {
    data->scriptedAnimator.setScripts(data->humanoid->animationScripts());
    data->scriptedAnimator.addCallbacks("animationConfig", LuaBindings::makeScriptedAnimatorCallbacks(data->humanoid->networkedAnimator(),
      [data](String const& name, Json const& defaultValue) -> Json {
        return data->scriptedAnimationParameters.value(name, defaultValue);
      }));
    data->scriptedAnimator.addCallbacks("config",
        LuaBindings::makeConfigCallbacks([data](String const& name, Json const& def)
            { return data->npcVariant.scriptConfig.query(name, def); }));
    data->scriptedAnimator.addCallbacks("entity", LuaBindings::makeEntityCallbacks(this));
    data->scriptedAnimator.init(world);
  }
}

void NpcAdapter::uninit() {
  auto* data = getData();
  
  if (isMaster()) {
    data->movementController->resetAnchorState();
    data->scriptComponent.uninit();
    data->scriptComponent.removeCallbacks("npc");
    data->scriptComponent.removeCallbacks("config");
    data->scriptComponent.removeCallbacks("entity");
    data->scriptComponent.removeCallbacks("status");
    data->scriptComponent.removeCallbacks("behavior");
    data->scriptComponent.removeCallbacks("songbook");
    data->scriptComponent.removeCallbacks("animator");
    data->scriptComponent.removeActorMovementCallbacks();
  }
  if (world()->isClient()) {
    data->scriptedAnimator.uninit();
    data->scriptedAnimator.removeCallbacks("animationConfig");
    data->scriptedAnimator.removeCallbacks("config");
    data->scriptedAnimator.removeCallbacks("entity");
  }
  data->tools->uninit();
  data->statusController->uninit();
  data->movementController->uninit();
  
  EntityAdapter::uninit();
}

Vec2F NpcAdapter::position() const {
  return getData()->movementController->position();
}

RectF NpcAdapter::metaBoundBox() const {
  return RectF(-4, -4, 4, 4);
}

RectF NpcAdapter::collisionArea() const {
  return getData()->movementController->collisionPoly().boundBox();
}

Vec2F NpcAdapter::velocity() const {
  return getData()->movementController->velocity();
}

Vec2F NpcAdapter::mouthOffset(bool ignoreAdjustments) const {
  auto const* data = getData();
  return Vec2F{data->humanoid->mouthOffset(ignoreAdjustments)[0] * numericalDirection(data->humanoid->facingDirection()),
      data->humanoid->mouthOffset(ignoreAdjustments)[1]};
}

Vec2F NpcAdapter::feetOffset() const {
  auto const* data = getData();
  return {data->humanoid->feetOffset()[0] * numericalDirection(data->humanoid->facingDirection()), data->humanoid->feetOffset()[1]};
}

Vec2F NpcAdapter::headArmorOffset() const {
  auto const* data = getData();
  return {data->humanoid->headArmorOffset()[0] * numericalDirection(data->humanoid->facingDirection()), data->humanoid->headArmorOffset()[1]};
}

Vec2F NpcAdapter::chestArmorOffset() const {
  auto const* data = getData();
  return {data->humanoid->chestArmorOffset()[0] * numericalDirection(data->humanoid->facingDirection()), data->humanoid->chestArmorOffset()[1]};
}

Vec2F NpcAdapter::legsArmorOffset() const {
  auto const* data = getData();
  return {data->humanoid->legsArmorOffset()[0] * numericalDirection(data->humanoid->facingDirection()), data->humanoid->legsArmorOffset()[1]};
}

Vec2F NpcAdapter::backArmorOffset() const {
  auto const* data = getData();
  return {data->humanoid->backArmorOffset()[0] * numericalDirection(data->humanoid->facingDirection()), data->humanoid->backArmorOffset()[1]};
}

pair<ByteArray, uint64_t> NpcAdapter::writeNetState(uint64_t fromVersion, NetCompatibilityRules rules) {
  return m_netGroup.writeNetState(fromVersion, rules);
}

void NpcAdapter::readNetState(ByteArray data, float interpolationTime, NetCompatibilityRules rules) {
  m_netGroup.readNetState(std::move(data), interpolationTime, rules);
}

void NpcAdapter::enableInterpolation(float extrapolationHint) {
  m_netGroup.enableNetInterpolation(extrapolationHint);
}

void NpcAdapter::disableInterpolation() {
  m_netGroup.disableNetInterpolation();
}

String NpcAdapter::description() const {
  return getData()->npcVariant.description.value("Some funny looking person");
}

String NpcAdapter::species() const {
  return getData()->npcVariant.humanoidIdentity.species;
}

Gender NpcAdapter::gender() const {
  return getData()->npcVariant.humanoidIdentity.gender;
}

String NpcAdapter::npcType() const {
  return getData()->npcVariant.typeName;
}

Json NpcAdapter::scriptConfigParameter(String const& parameterName, Json const& defaultValue) const {
  return getData()->npcVariant.scriptConfig.query(parameterName, defaultValue);
}

float NpcAdapter::maxHealth() const {
  return *getData()->statusController->resourceMax("health");
}

float NpcAdapter::health() const {
  return getData()->statusController->resource("health");
}

DamageBarType NpcAdapter::damageBar() const {
  return DamageBarType::Default;
}

Maybe<HitType> NpcAdapter::queryHit(DamageSource const& source) const {
  auto const* data = getData();
  if (!inWorld() || !data->statusController->resourcePositive("health") || data->statusController->statPositive("invulnerable"))
    return {};

  if (data->tools->queryShieldHit(source))
    return HitType::ShieldHit;

  if (source.intersectsWithPoly(world()->geometry(), data->movementController->collisionBody()))
    return HitType::Hit;

  return {};
}

Maybe<PolyF> NpcAdapter::hitPoly() const {
  return getData()->movementController->collisionBody();
}

void NpcAdapter::damagedOther(DamageNotification const& damage) {
  if (inWorld() && isMaster())
    getData()->statusController->damagedOther(damage);
}

List<DamageNotification> NpcAdapter::applyDamage(DamageRequest const& damage) {
  if (!inWorld())
    return {};

  auto* data = getData();
  auto notifications = data->statusController->applyDamageRequest(damage);

  float totalDamage = 0.0f;
  for (auto const& notification : notifications)
    totalDamage += notification.healthLost;

  if (totalDamage > 0 && data->hitDamageNotificationLimiter < data->hitDamageNotificationLimit) {
    data->scriptComponent.invoke("damage", JsonObject{
        {"sourceId", damage.sourceEntityId},
        {"damage", totalDamage},
        {"sourceDamage", damage.damage},
        {"sourceKind", damage.damageSourceKind}
      });
    data->hitDamageNotificationLimiter++;
  }

  return notifications;
}

List<DamageNotification> NpcAdapter::selfDamageNotifications() {
  return getData()->statusController->pullSelfDamageNotifications();
}

bool NpcAdapter::shouldDestroy() const {
  auto const* data = getData();
  if (auto res = data->scriptComponent.invoke<bool>("shouldDie"))
    return *res;
  else if (!data->statusController->resourcePositive("health") || data->scriptComponent.error())
    return true;
  else
    return false;
}

void NpcAdapter::destroy(RenderCallback* renderCallback) {
  auto* data = getData();
  data->scriptComponent.invoke("die");

  if (isMaster() && !data->dropPools.empty()) {
    auto treasureDatabase = Root::singleton().treasureDatabase();
    for (auto const& treasureItem :
        treasureDatabase->createTreasure(staticRandomFrom(data->dropPools, data->npcVariant.seed), data->npcVariant.level))
      world()->addEntity(ItemDrop::createRandomizedDrop(treasureItem, position()));
  }

  if (renderCallback && data->deathParticleBurst)
    renderCallback->addParticles(data->humanoid->particles(*data->deathParticleBurst), position());

  data->songbook->stop();
}

void NpcAdapter::update(float dt, uint64_t) {
  if (!inWorld())
    return;

  auto* data = getData();
  data->movementController->setTimestep(dt);

  if (isMaster()) {
    data->scriptComponent.update(data->scriptComponent.updateDt(dt));

    if (inConflictingLoungeAnchor())
      data->movementController->resetAnchorState();

    if (auto loungeAnchor = as<LoungeAnchor>(data->movementController->entityAnchor())) {
      if (loungeAnchor->emote)
        requestEmote(*loungeAnchor->emote);
      if (loungeAnchor->dance)
        setDance(loungeAnchor->dance);
      data->statusController->setPersistentEffects("lounging", loungeAnchor->statusEffects);
      data->effectEmitter->addEffectSources("normal", loungeAnchor->effectEmitters);
      switch (loungeAnchor->orientation) {
        case LoungeOrientation::Sit:
          data->humanoid->setState(Humanoid::Sit);
          break;
        case LoungeOrientation::Lay:
          data->humanoid->setState(Humanoid::Lay);
          break;
        case LoungeOrientation::Stand:
          data->humanoid->setState(Humanoid::Idle);
          break;
        default:
          data->humanoid->setState(Humanoid::Idle);
      }
    } else {
      data->statusController->setPersistentEffects("lounging", {});
    }

    data->armor->effects(*data->effectEmitter);
    data->tools->effects(*data->effectEmitter);

    data->statusController->setPersistentEffects("armor", data->armor->statusEffects(data->disableWornArmor));
    data->statusController->setPersistentEffects("tools", data->tools->statusEffects());

    data->movementController->tickMaster(dt);
    data->statusController->tickMaster(dt);

    tickShared(dt);

    if (!is<LoungeAnchor>(data->movementController->entityAnchor())) {
      if (data->movementController->groundMovement()) {
        if (data->movementController->running())
          data->humanoid->setState(Humanoid::Run);
        else if (data->movementController->walking())
          data->humanoid->setState(Humanoid::Walk);
        else if (data->movementController->crouching())
          data->humanoid->setState(Humanoid::Duck);
        else
          data->humanoid->setState(Humanoid::Idle);
      } else if (data->movementController->liquidMovement()) {
        if (abs(data->movementController->xVelocity()) > 0)
          data->humanoid->setState(Humanoid::Swim);
        else
          data->humanoid->setState(Humanoid::SwimIdle);
      } else {
        if (data->movementController->yVelocity() > 0)
          data->humanoid->setState(Humanoid::Jump);
        else
          data->humanoid->setState(Humanoid::Fall);
      }
    }

    if (data->emoteCooldownTimer.tick(dt))
      data->emoteState = HumanoidEmote::Idle;
    if (data->danceCooldownTimer.tick(dt))
      data->dance = {};

    if (data->chatMessageUpdated) {
      auto state = Root::singleton().emoteProcessor()->detectEmotes(data->chatMessage);
      if (state != HumanoidEmote::Idle)
        addEmote(state);
      data->chatMessageUpdated = false;
    }

    if (data->blinkCooldownTimer.tick(dt)) {
      data->blinkCooldownTimer = GameTimer(Random::randf(data->blinkInterval[0], data->blinkInterval[1]));
      if (data->emoteState == HumanoidEmote::Idle)
        addEmote(HumanoidEmote::Blink);
    }

    data->humanoid->setEmoteState(data->emoteState);
    data->humanoid->setDance(data->dance);

    setNetStates();
  } else {
    m_netGroup.tickNetInterpolation(dt);
    getNetStates(false);
    data->movementController->tickSlave(dt);
    data->statusController->tickSlave(dt);

    tickShared(dt);
  }

  if (world()->isClient())
    SpatialLogger::logPoly("world", data->movementController->collisionBody(), {0, 255, 0, 255});
}

void NpcAdapter::render(RenderCallback* renderCallback) {
  auto* data = getData();
  EntityRenderLayer renderLayer = RenderLayerNpc;
  if (auto loungeAnchor = as<LoungeAnchor>(data->movementController->entityAnchor()))
    renderLayer = loungeAnchor->loungeRenderLayer;

  data->tools->setupHumanoidHandItemDrawables(*data->humanoid);

  DirectivesGroup humanoidDirectives;
  Vec2F scale = Vec2F::filled(1.f);
  for (auto& directives : data->statusController->parentDirectives().list()) {
    auto result = Humanoid::extractScaleFromDirectives(directives);
    scale = scale.piecewiseMultiply(result.first);
    humanoidDirectives.append(result.second);
  }
  data->humanoid->setScale(scale);

  for (auto& drawable : data->humanoid->render()) {
    drawable.translate(position());
    if (drawable.isImage())
      drawable.imagePart().addDirectivesGroup(humanoidDirectives, true);
    renderCallback->addDrawable(std::move(drawable), renderLayer);
  }

  renderCallback->addDrawables(data->statusController->drawables(), renderLayer);
  renderCallback->addParticles(data->statusController->pullNewParticles());
  renderCallback->addAudios(data->statusController->pullNewAudios());

  renderCallback->addParticles(data->npcVariant.splashConfig.doSplash(position(), data->movementController->velocity(), world()));

  data->tools->render(renderCallback, inToolRange(), data->shifting, renderLayer);

  renderCallback->addDrawables(data->tools->renderObjectPreviews(aimPosition(), walkingDirection(), inToolRange(), favoriteColor()), renderLayer);

  data->effectEmitter->render(renderCallback);
  data->songbook->render(renderCallback);
}

void NpcAdapter::renderLightSources(RenderCallback* renderCallback) {
  renderCallback->addLightSources(lightSources());
}

void NpcAdapter::setPosition(Vec2F const& pos) {
  getData()->movementController->setPosition(pos);
}

List<Drawable> NpcAdapter::portrait(PortraitMode mode) const {
  return getData()->humanoid->renderPortrait(mode);
}

String NpcAdapter::name() const {
  return getData()->npcVariant.humanoidIdentity.name;
}

Maybe<String> NpcAdapter::statusText() const {
  return getData()->statusText;
}

bool NpcAdapter::displayNametag() const {
  return getData()->displayNametag;
}

Vec3B NpcAdapter::nametagColor() const {
  return getData()->npcVariant.nametagColor;
}

Vec2F NpcAdapter::nametagOrigin() const {
  return mouthPosition(false);
}

String NpcAdapter::nametag() const {
  return name();
}

bool NpcAdapter::aggressive() const {
  return getData()->aggressive;
}

Maybe<LuaValue> NpcAdapter::callScript(String const& func, LuaVariadic<LuaValue> const& args) {
  return getData()->scriptComponent.invoke(func, args);
}

Maybe<LuaValue> NpcAdapter::evalScript(String const& code) {
  return getData()->scriptComponent.eval(code);
}

Vec2F NpcAdapter::mouthPosition() const {
  return mouthOffset(true) + position();
}

Vec2F NpcAdapter::mouthPosition(bool ignoreAdjustments) const {
  return mouthOffset(ignoreAdjustments) + position();
}

List<ChatAction> NpcAdapter::pullPendingChatActions() {
  return std::move(getData()->pendingChatActions);
}

bool NpcAdapter::isInteractive() const {
  return getData()->isInteractive;
}

InteractAction NpcAdapter::interact(InteractRequest const& request) {
  auto const* data = getData();
  auto result = data->scriptComponent.invoke<Json>("interact",
      JsonObject{{"sourceId", request.sourceId}, {"sourcePosition", jsonFromVec2F(request.sourcePosition)}}).value();

  if (result.isNull())
    return {};

  if (result.isType(Json::Type::String))
    return InteractAction(result.toString(), entityId(), Json());

  return InteractAction(result.getString(0), entityId(), result.get(1));
}

RectF NpcAdapter::interactiveBoundBox() const {
  return getData()->movementController->collisionPoly().boundBox();
}

Maybe<EntityAnchorState> NpcAdapter::loungingIn() const {
  auto const* data = getData();
  if (is<LoungeAnchor>(data->movementController->entityAnchor()))
    return data->movementController->anchorState();
  return {};
}

List<QuestArcDescriptor> NpcAdapter::offeredQuests() const {
  return getData()->offeredQuests;
}

StringSet NpcAdapter::turnInQuests() const {
  return getData()->turnInQuests;
}

Vec2F NpcAdapter::questIndicatorPosition() const {
  auto const* data = getData();
  Vec2F pos = position() + data->questIndicatorOffset;
  pos[1] += interactiveBoundBox().yMax();
  return pos;
}

List<LightSource> NpcAdapter::lightSources() const {
  auto const* data = getData();
  List<LightSource> lights;
  lights.appendAll(data->tools->lightSources());
  lights.appendAll(data->statusController->lightSources());
  lights.appendAll(data->humanoid->networkedAnimator()->lightSources());
  return lights;
}

Maybe<Json> NpcAdapter::receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) {
  auto* data = getData();
  Maybe<Json> result = data->scriptComponent.handleMessage(message, world()->connection() == sendingConnection, args);
  if (!result)
    result = data->statusController->receiveMessage(message, world()->connection() == sendingConnection, args);
  return result;
}

Vec2F NpcAdapter::armPosition(ToolHand hand, Direction facingDirection, float armAngle, Vec2F offset) const {
  auto const* data = getData();
  return data->tools->armPosition(*data->humanoid, hand, facingDirection, armAngle, offset);
}

Vec2F NpcAdapter::handOffset(ToolHand hand, Direction facingDirection) const {
  return getData()->tools->handOffset(*getData()->humanoid, hand, facingDirection);
}

Vec2F NpcAdapter::handPosition(ToolHand hand, Vec2F const& handOffset) const {
  return getData()->tools->handPosition(hand, *getData()->humanoid, handOffset);
}

ItemPtr NpcAdapter::handItem(ToolHand hand) const {
  auto const* data = getData();
  if (hand == ToolHand::Primary)
    return data->tools->primaryHandItem();
  return data->tools->altHandItem();
}

Vec2F NpcAdapter::armAdjustment() const {
  return getData()->humanoid->armAdjustment();
}

Vec2F NpcAdapter::aimPosition() const {
  auto const* data = getData();
  return world()->geometry().xwrap(Vec2F(data->xAimPosition, data->yAimPosition) + position());
}

float NpcAdapter::interactRadius() const {
  return 9999;
}

Direction NpcAdapter::facingDirection() const {
  return getData()->movementController->facingDirection();
}

Direction NpcAdapter::walkingDirection() const {
  return getData()->movementController->movingDirection();
}

bool NpcAdapter::isAdmin() const {
  return false;
}

Color NpcAdapter::favoriteColor() const {
  return Color::White;
}

float NpcAdapter::beamGunRadius() const {
  return getData()->tools->beamGunRadius();
}

void NpcAdapter::addParticles(List<Particle> const&) {}

void NpcAdapter::addSound(String const&, float, float) {}

bool NpcAdapter::inToolRange() const {
  return true;
}

bool NpcAdapter::inToolRange(Vec2F const&) const {
  return true;
}

void NpcAdapter::addEphemeralStatusEffects(List<EphemeralStatusEffect> const& statusEffects) {
  getData()->statusController->addEphemeralEffects(statusEffects);
}

ActiveUniqueStatusEffectSummary NpcAdapter::activeUniqueStatusEffectSummary() const {
  return getData()->statusController->activeUniqueStatusEffectSummary();
}

float NpcAdapter::powerMultiplier() const {
  return getData()->statusController->stat("powerMultiplier");
}

bool NpcAdapter::fullEnergy() const {
  return *getData()->statusController->resourcePercentage("energy") >= 1.0;
}

float NpcAdapter::energy() const {
  return getData()->statusController->resource("energy");
}

bool NpcAdapter::energyLocked() const {
  return getData()->statusController->resourceLocked("energy");
}

bool NpcAdapter::consumeEnergy(float energy) {
  return getData()->statusController->overConsumeResource("energy", energy);
}

void NpcAdapter::queueUIMessage(String const&) {}

bool NpcAdapter::instrumentPlaying() {
  return getData()->songbook->instrumentPlaying();
}

void NpcAdapter::instrumentEquipped(String const& instrumentKind) {
  auto* data = getData();
  if (canUseTool())
    data->songbook->keepAlive(instrumentKind, mouthPosition());
}

void NpcAdapter::interact(InteractAction const&) {}

void NpcAdapter::addEffectEmitters(StringSet const& emitters) {
  getData()->effectEmitter->addEffectSources("normal", emitters);
}

void NpcAdapter::requestEmote(String const& emote) {
  auto* data = getData();
  if (!emote.empty()) {
    auto state = HumanoidEmoteNames.getLeft(emote);
    if (state != HumanoidEmote::Idle && (data->emoteState == HumanoidEmote::Idle || data->emoteState == HumanoidEmote::Blink))
      addEmote(state);
  }
}

ActorMovementController* NpcAdapter::movementController() {
  return getData()->movementController.get();
}

StatusController* NpcAdapter::statusController() {
  return getData()->statusController.get();
}

void NpcAdapter::setCameraFocusEntity(Maybe<EntityId> const&) {
  // players only
}

void NpcAdapter::playEmote(HumanoidEmote emote) {
  addEmote(emote);
}

List<DamageSource> NpcAdapter::damageSources() const {
  auto const* data = getData();
  auto damageSources = data->tools->damageSources();

  if (data->damageOnTouch && !data->npcVariant.touchDamageConfig.isNull()) {
    Json config = data->npcVariant.touchDamageConfig;
    if (!config.contains("poly") && !config.contains("line")) {
      config = config.set("poly", jsonFromPolyF(data->movementController->collisionPoly()));
    }
    DamageSource damageSource(config);
    if (auto damagePoly = damageSource.damageArea.ptr<PolyF>())
      damagePoly->rotate(data->movementController->rotation());
    damageSource.damage *= data->statusController->stat("powerMultiplier");
    damageSources.append(damageSource);
  }

  for (auto& damageSource : damageSources) {
    damageSource.sourceEntityId = entityId();
    damageSource.team = getTeam();
  }

  return damageSources;
}

List<PhysicsForceRegion> NpcAdapter::forceRegions() const {
  return getData()->tools->forceRegions();
}

HumanoidIdentity const& NpcAdapter::identity() const {
  return getData()->npcVariant.humanoidIdentity;
}

void NpcAdapter::updateIdentity() {
  auto* data = getData();
  data->identityUpdated = true;
  data->humanoid->setIdentity(data->npcVariant.humanoidIdentity);
}

void NpcAdapter::setIdentity(HumanoidIdentity identity) {
  getData()->npcVariant.humanoidIdentity = std::move(identity);
  updateIdentity();
}

void NpcAdapter::setBodyDirectives(String const& directives) {
  getData()->npcVariant.humanoidIdentity.bodyDirectives = directives;
  updateIdentity();
}

void NpcAdapter::setEmoteDirectives(String const& directives) {
  getData()->npcVariant.humanoidIdentity.emoteDirectives = directives;
  updateIdentity();
}

void NpcAdapter::setHairGroup(String const& group) {
  getData()->npcVariant.humanoidIdentity.hairGroup = group;
  updateIdentity();
}

void NpcAdapter::setHairType(String const& type) {
  getData()->npcVariant.humanoidIdentity.hairType = type;
  updateIdentity();
}

void NpcAdapter::setHairDirectives(String const& directives) {
  getData()->npcVariant.humanoidIdentity.hairDirectives = directives;
  updateIdentity();
}

void NpcAdapter::setFacialHairGroup(String const& group) {
  getData()->npcVariant.humanoidIdentity.facialHairGroup = group;
  updateIdentity();
}

void NpcAdapter::setFacialHairType(String const& type) {
  getData()->npcVariant.humanoidIdentity.facialHairType = type;
  updateIdentity();
}

void NpcAdapter::setFacialHairDirectives(String const& directives) {
  getData()->npcVariant.humanoidIdentity.facialHairDirectives = directives;
  updateIdentity();
}

void NpcAdapter::setFacialMaskGroup(String const& group) {
  getData()->npcVariant.humanoidIdentity.facialMaskGroup = group;
  updateIdentity();
}

void NpcAdapter::setFacialMaskType(String const& type) {
  getData()->npcVariant.humanoidIdentity.facialMaskType = type;
  updateIdentity();
}

void NpcAdapter::setFacialMaskDirectives(String const& directives) {
  getData()->npcVariant.humanoidIdentity.facialMaskDirectives = directives;
  updateIdentity();
}

void NpcAdapter::setHair(String const& group, String const& type, String const& directives) {
  auto* data = getData();
  data->npcVariant.humanoidIdentity.hairGroup = group;
  data->npcVariant.humanoidIdentity.hairType = type;
  data->npcVariant.humanoidIdentity.hairDirectives = directives;
  updateIdentity();
}

void NpcAdapter::setFacialHair(String const& group, String const& type, String const& directives) {
  auto* data = getData();
  data->npcVariant.humanoidIdentity.facialHairGroup = group;
  data->npcVariant.humanoidIdentity.facialHairType = type;
  data->npcVariant.humanoidIdentity.facialHairDirectives = directives;
  updateIdentity();
}

void NpcAdapter::setFacialMask(String const& group, String const& type, String const& directives) {
  auto* data = getData();
  data->npcVariant.humanoidIdentity.facialMaskGroup = group;
  data->npcVariant.humanoidIdentity.facialMaskType = type;
  data->npcVariant.humanoidIdentity.facialMaskDirectives = directives;
  updateIdentity();
}

void NpcAdapter::setSpecies(String const& species) {
  getData()->npcVariant.humanoidIdentity.species = species;
  updateIdentity();
}

void NpcAdapter::setGender(Gender const& gender) {
  getData()->npcVariant.humanoidIdentity.gender = gender;
  updateIdentity();
}

void NpcAdapter::setPersonality(Personality const& personality) {
  getData()->npcVariant.humanoidIdentity.personality = personality;
  updateIdentity();
}

void NpcAdapter::setImagePath(Maybe<String> const& imagePath) {
  getData()->npcVariant.humanoidIdentity.imagePath = imagePath;
  updateIdentity();
}

void NpcAdapter::setFavoriteColor(Color color) {
  getData()->npcVariant.humanoidIdentity.color = color.toRgba();
  updateIdentity();
}

void NpcAdapter::setName(String const& name) {
  getData()->npcVariant.humanoidIdentity.name = name;
  updateIdentity();
}

void NpcAdapter::setDescription(String const& description) {
  getData()->npcVariant.description = description;
}

HumanoidPtr NpcAdapter::humanoid() {
  return getData()->humanoid;
}

HumanoidPtr NpcAdapter::humanoid() const {
  return getData()->humanoid;
}

bool NpcAdapter::forceNude() const {
  return getData()->statusController->statPositive("nude");
}

Songbook* NpcAdapter::songbook() {
  return getData()->songbook.get();
}

NpcDataComponent* NpcAdapter::getData() {
  return m_world.getComponentPtr<NpcDataComponent>(m_entity);
}

NpcDataComponent const* NpcAdapter::getData() const {
  return m_world.getComponentPtr<NpcDataComponent>(m_entity);
}

Vec2F NpcAdapter::getAbsolutePosition(Vec2F relativePosition) const {
  auto const* data = getData();
  if (data->humanoid->facingDirection() == Direction::Left)
    relativePosition[0] *= -1;
  return data->movementController->position() + relativePosition;
}

void NpcAdapter::tickShared(float dt) {
  auto* data = getData();

  if (data->hitDamageNotificationLimiter)
    data->hitDamageNotificationLimiter--;

  data->songbook->update(*entityMode(), world());

  data->effectEmitter->setSourcePosition("normal", position());
  data->effectEmitter->setSourcePosition("mouth", position() + mouthOffset());
  data->effectEmitter->setSourcePosition("feet", position() + feetOffset());
  data->effectEmitter->setSourcePosition("headArmor", headArmorOffset() + position());
  data->effectEmitter->setSourcePosition("chestArmor", chestArmorOffset() + position());
  data->effectEmitter->setSourcePosition("legsArmor", legsArmorOffset() + position());
  data->effectEmitter->setSourcePosition("backArmor", backArmorOffset() + position());

  data->effectEmitter->setDirection(data->humanoid->facingDirection());
  data->effectEmitter->tick(dt, *entityMode());

  data->humanoid->setMovingBackwards(data->movementController->movingDirection() != data->movementController->facingDirection());
  data->humanoid->setFacingDirection(data->movementController->facingDirection());
  data->humanoid->setRotation(data->movementController->rotation());

  ActorMovementModifiers firingModifiers;
  if (auto fireableMain = as<FireableItem>(handItem(ToolHand::Primary))) {
    if (fireableMain->firing()) {
      if (fireableMain->stopWhileFiring())
        firingModifiers.movementSuppressed = true;
      else if (fireableMain->walkWhileFiring())
        firingModifiers.runningSuppressed = true;
    }
  }

  if (auto fireableAlt = as<FireableItem>(handItem(ToolHand::Alt))) {
    if (fireableAlt->firing()) {
      if (fireableAlt->stopWhileFiring())
        firingModifiers.movementSuppressed = true;
      else if (fireableAlt->walkWhileFiring())
        firingModifiers.runningSuppressed = true;
    }
  }

  data->armor->setupHumanoid(*data->humanoid, forceNude());

  data->tools->suppressItems(!canUseTool());
  data->tools->tick(dt, data->shifting, {});

  if (auto overrideDirection = data->tools->setupHumanoidHandItems(*data->humanoid, position(), aimPosition()))
    data->movementController->controlFace(*overrideDirection);

  if (world()->isClient()) {
    // TODO: Handle dynamic target for humanoid animation
    data->humanoid->animate(dt, nullptr);
  } else {
    data->humanoid->animate(dt, {});
  }
  data->scriptedAnimator.update();
}

LuaCallbacks NpcAdapter::makeNpcCallbacks() {
  LuaCallbacks callbacks;

  callbacks.registerCallback("toAbsolutePosition", [this](Vec2F const& p) { return getAbsolutePosition(p); });

  callbacks.registerCallback("species", [this]() { return getData()->npcVariant.species; });

  callbacks.registerCallback("gender", [this]() { return GenderNames.getRight(getData()->humanoid->identity().gender); });

  callbacks.registerCallback("humanoidIdentity", [this]() { return getData()->humanoid->identity().toJson(); });
  callbacks.registerCallback("setHumanoidIdentity", [this](Json const& id) { setIdentity(HumanoidIdentity(id)); });

  callbacks.registerCallback("npcType", [this]() { return npcType(); });

  callbacks.registerCallback("seed", [this]() { return getData()->npcVariant.seed; });

  callbacks.registerCallback("level", [this]() { return getData()->npcVariant.level; });

  callbacks.registerCallback("dropPools", [this]() { return getData()->dropPools; });

  callbacks.registerCallback("setDropPools", [this](StringList const& dropPools) { getData()->dropPools = dropPools; });

  callbacks.registerCallback("energy", [this]() { return getData()->statusController->resource("energy"); });

  callbacks.registerCallback("maxEnergy", [this]() { return getData()->statusController->resourceMax("energy"); });

  callbacks.registerCallback("say", [this](String line, Maybe<StringMap<String>> const& tags, Json const& config) {
      if (tags)
        line = line.replaceTags(*tags, false);

      if (!line.empty()) {
        addChatMessage(line, config);
        return true;
      }

      return false;
    });

  callbacks.registerCallback("sayPortrait", [this](String line, String portrait, Maybe<StringMap<String>> const& tags, Json const& config) {
      if (tags)
        line = line.replaceTags(*tags, false);

      if (!line.empty()) {
        addChatMessage(line, config, portrait);
        return true;
      }

      return false;
    });

  callbacks.registerCallback("emote", [this](String const& arg1) { addEmote(HumanoidEmoteNames.getLeft(arg1)); });

  callbacks.registerCallback("dance", [this](Maybe<String> const& danceName) { setDance(danceName); });

  callbacks.registerCallback("setInteractive", [this](bool interactive) { getData()->isInteractive = interactive; });

  callbacks.registerCallback("setAggressive", [this](bool aggressive) { getData()->aggressive = aggressive; });

  callbacks.registerCallback("setDamageOnTouch", [this](bool damageOnTouch) { getData()->damageOnTouch = damageOnTouch; });

  callbacks.registerCallback("aimPosition", [this]() { return jsonFromVec2F(aimPosition()); });

  callbacks.registerCallback("setAimPosition", [this](Vec2F const& pos) {
      auto* data = getData();
      auto aimPosition = world()->geometry().diff(pos, position());
      data->xAimPosition = aimPosition[0];
      data->yAimPosition = aimPosition[1];
    });

  callbacks.registerCallback("setDeathParticleBurst", [this](Maybe<String> const& deathParticleBurst) {
      getData()->deathParticleBurst = deathParticleBurst;
    });

  callbacks.registerCallback("setStatusText", [this](Maybe<String> const& status) { getData()->statusText = status; });
  callbacks.registerCallback("setDisplayNametag", [this](bool display) { getData()->displayNametag = display; });

  callbacks.registerCallback("setPersistent", [this](bool persistent) { setPersistent(persistent); });

  callbacks.registerCallback("setKeepAlive", [this](bool keepAlive) { setKeepAlive(keepAlive); });

  callbacks.registerCallback("setDamageTeam", [this](Json const& team) { setTeam(EntityDamageTeam(team)); });

  callbacks.registerCallback("setUniqueId", [this](Maybe<String> uniqueId) { setUniqueId(uniqueId); });

  return callbacks;
}

void NpcAdapter::setupNetStates() {
  m_netGroup.addNetElement(&m_xAimPosition);
  m_netGroup.addNetElement(&m_yAimPosition);

  m_xAimPosition.setFixedPointBase(0.0625);
  m_yAimPosition.setFixedPointBase(0.0625);
  m_xAimPosition.setInterpolator(lerp<float, float>);
  m_yAimPosition.setInterpolator(lerp<float, float>);

  m_netGroup.addNetElement(&m_uniqueIdNetState);
  m_netGroup.addNetElement(&m_teamNetState);
  m_netGroup.addNetElement(&m_humanoidStateNetState);
  m_netGroup.addNetElement(&m_humanoidEmoteStateNetState);
  m_netGroup.addNetElement(&m_humanoidDanceNetState);

  m_netGroup.addNetElement(&m_newChatMessageEvent);
  m_netGroup.addNetElement(&m_chatMessage);
  m_netGroup.addNetElement(&m_chatPortrait);
  m_netGroup.addNetElement(&m_chatConfig);

  m_netGroup.addNetElement(&m_statusText);
  m_netGroup.addNetElement(&m_displayNametag);

  m_netGroup.addNetElement(&m_isInteractive);

  m_netGroup.addNetElement(&m_offeredQuests);
  m_netGroup.addNetElement(&m_turnInQuests);

  m_netGroup.addNetElement(&m_shifting);
  m_netGroup.addNetElement(&m_damageOnTouch);

  m_netGroup.addNetElement(&m_disableWornArmor);

  m_netGroup.addNetElement(&m_deathParticleBurst);

  m_netGroup.addNetElement(&m_dropPools);
  m_netGroup.addNetElement(&m_aggressive);

  m_netGroup.addNetElement(&m_identityNetState);

  m_netGroup.addNetElement(&m_scriptedAnimationParameters);
}

void NpcAdapter::getNetStates(bool initial) {
  auto* data = getData();
  setUniqueId(m_uniqueIdNetState.get());
  setTeam(m_teamNetState.get());
  data->humanoid->setState(m_humanoidStateNetState.get());
  data->humanoid->setEmoteState(m_humanoidEmoteStateNetState.get());
  data->humanoid->setDance(m_humanoidDanceNetState.get());

  if (m_identityNetState.pullUpdated() && !initial) {
    auto newIdentity = m_identityNetState.get();
    data->npcVariant.humanoidIdentity = newIdentity;
    data->humanoid->setIdentity(newIdentity);
  }

  if (m_newChatMessageEvent.pullOccurred() && !initial) {
    data->chatMessageUpdated = true;
    if (m_chatPortrait.get().empty())
      data->pendingChatActions.append(SayChatAction{entityId(), m_chatMessage.get(), mouthPosition(), m_chatConfig.get()});
    else
      data->pendingChatActions.append(PortraitChatAction{
          entityId(), m_chatPortrait.get(), m_chatMessage.get(), mouthPosition(), m_chatConfig.get()});
  }

  data->statusText = m_statusText.get();
  data->displayNametag = m_displayNametag.get();
  data->isInteractive = m_isInteractive.get();
  data->shifting = m_shifting.get();
  data->damageOnTouch = m_damageOnTouch.get();
  data->disableWornArmor = m_disableWornArmor.get();
  data->dropPools = m_dropPools.get();
  data->aggressive = m_aggressive.get();
  data->offeredQuests = m_offeredQuests.get();
  data->turnInQuests = m_turnInQuests.get();
  data->deathParticleBurst = m_deathParticleBurst.get();
}

void NpcAdapter::setNetStates() {
  auto const* data = getData();
  m_uniqueIdNetState.set(uniqueId());
  m_teamNetState.set(getTeam());
  m_humanoidStateNetState.set(data->humanoid->state());
  m_humanoidEmoteStateNetState.set(data->humanoid->emoteState());
  m_humanoidDanceNetState.set(data->humanoid->dance());

  if (data->identityUpdated) {
    m_identityNetState.push(data->npcVariant.humanoidIdentity);
    getData()->identityUpdated = false;
  }

  m_statusText.set(data->statusText);
  m_displayNametag.set(data->displayNametag);
  m_isInteractive.set(data->isInteractive);
  m_shifting.set(data->shifting);
  m_damageOnTouch.set(data->damageOnTouch);
  m_disableWornArmor.set(data->disableWornArmor);
  m_dropPools.set(data->dropPools);
  m_aggressive.set(data->aggressive);
  m_offeredQuests.set(data->offeredQuests);
  m_turnInQuests.set(data->turnInQuests);
  m_deathParticleBurst.set(data->deathParticleBurst);

  m_xAimPosition.set(data->xAimPosition);
  m_yAimPosition.set(data->yAimPosition);
}

void NpcAdapter::addChatMessage(String const& message, Json const& config, String const& portrait) {
  auto* data = getData();
  data->chatMessage = message;
  data->chatPortrait = portrait;
  data->chatConfig = config;
  data->chatMessageUpdated = true;
  m_newChatMessageEvent.trigger();
  m_chatMessage.set(message);
  m_chatPortrait.set(portrait);
  m_chatConfig.set(config);
  if (portrait.empty())
    data->pendingChatActions.append(SayChatAction{entityId(), message, mouthPosition(), config});
  else
    data->pendingChatActions.append(PortraitChatAction{entityId(), portrait, message, mouthPosition(), config});
}

void NpcAdapter::addEmote(HumanoidEmote const& emote) {
  auto* data = getData();
  data->emoteState = emote;
  data->emoteCooldownTimer.reset();
}

void NpcAdapter::setDance(Maybe<String> const& danceName) {
  auto* data = getData();
  data->dance = danceName;

  if (danceName.isValid()) {
    auto danceDatabase = Root::singleton().danceDatabase();
    DancePtr dance = danceDatabase->getDance(*danceName);
    data->danceCooldownTimer = GameTimer(dance->duration);
  }
}

bool NpcAdapter::setItemSlot(String const& slot, ItemDescriptor itemDescriptor) {
  auto* data = getData();
  auto item = Root::singleton().itemDatabase()->item(ItemDescriptor(itemDescriptor), data->npcVariant.level, data->npcVariant.seed);

  if (auto equipmentSlot = EquipmentSlotNames.leftPtr(slot)) {
    data->armor->setItem((uint8_t)*equipmentSlot, as<ArmorItem>(item));
  } else if (slot.equalsIgnoreCase("primary"))
    data->tools->setItems(item, data->tools->altHandItem());
  else if (slot.equalsIgnoreCase("alt"))
    data->tools->setItems(data->tools->primaryHandItem(), item);
  else
    return false;

  return true;
}

bool NpcAdapter::canUseTool() const {
  auto const* data = getData();
  bool canUse = !shouldDestroy() && !data->statusController->toolUsageSuppressed();
  if (canUse) {
    if (auto loungeAnchor = as<LoungeAnchor>(data->movementController->entityAnchor()))
      if (loungeAnchor->suppressTools.value(loungeAnchor->controllable))
        return false;
  }
  return canUse;
}

void NpcAdapter::disableWornArmor(bool disable) {
  getData()->disableWornArmor = disable;
}

} // namespace ECS
} // namespace Star
