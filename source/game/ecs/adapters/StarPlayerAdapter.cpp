#include "StarPlayerAdapter.hpp"
#include "StarRoot.hpp"
#include "StarWorld.hpp"
#include "StarPlayerDatabase.hpp"
#include "StarDamageDatabase.hpp"
#include "StarDamageManager.hpp"
#include "StarItemDrop.hpp"
#include "StarItemDatabase.hpp"
#include "StarEmoteProcessor.hpp"
#include "StarEntityRendering.hpp"
#include "StarConfigLuaBindings.hpp"
#include "StarEntityLuaBindings.hpp"
#include "StarWorldLuaBindings.hpp"
#include "StarStatusControllerLuaBindings.hpp"
#include "StarNetworkedAnimatorLuaBindings.hpp"
#include "StarScriptedAnimatorLuaBindings.hpp"
#include "StarArmors.hpp"
#include "StarFireableItem.hpp"
#include "StarJsonExtra.hpp"
#include "StarPlayerInventory.hpp"
#include "StarPlayerBlueprints.hpp"
#include "StarPlayerTech.hpp"
#include "StarPlayerCompanions.hpp"
#include "StarPlayerDeployment.hpp"
#include "StarPlayerLog.hpp"
#include "StarPlayerCodexes.hpp"
#include "StarQuestManager.hpp"
#include "StarPlayerUniverseMap.hpp"
#include "StarTechController.hpp"
#include "StarSpeciesDatabase.hpp"

namespace Star {
namespace ECS {

shared_ptr<PlayerAdapter> PlayerAdapter::create(World& ecsWorld, PlayerConfigPtr config, Uuid uuid) {
  auto entity = ecsWorld.createEntity();
  
  // Add tag and data components
  ecsWorld.addComponent<PlayerTag>(entity);
  auto& data = ecsWorld.addComponent<PlayerDataComponent>(entity);
  
  data.config = config;
  data.uuid = uuid;
  
  // Initialize humanoid with default identity
  data.identity = HumanoidIdentity();
  data.humanoid = make_shared<Humanoid>(data.identity);
  
  data.emoteState = HumanoidEmote::Idle;
  data.chatMessageUpdated = false;

  auto assets = Root::singleton().assets();

  data.emoteCooldownTimer = GameTimer(assets->json("/player.config:emoteCooldown").toFloat());
  data.danceCooldownTimer = GameTimer(0.0f);
  data.blinkInterval = jsonToVec2F(assets->json("/player.config:blinkInterval"));
  data.blinkCooldownTimer = GameTimer();

  // Initialize subsystems
  auto movementParameters = ActorMovementParameters(config->movementParameters);
  if (!movementParameters.physicsEffectCategories)
    movementParameters.physicsEffectCategories = StringSet({"player"});
  data.movementController = make_shared<ActorMovementController>(movementParameters);

  data.statusController = make_shared<StatusController>(config->statusControllerSettings);

  data.effectEmitter = make_shared<EffectEmitter>();
  data.effectsAnimator = make_shared<NetworkedAnimator>();

  data.armor = make_shared<ArmorWearer>();
  data.tools = make_shared<ToolUser>();
  data.songbook = make_shared<Songbook>(data.identity.species);

  data.inventory = make_shared<PlayerInventory>();
  data.blueprints = make_shared<PlayerBlueprints>();
  data.universeMap = make_shared<PlayerUniverseMap>();
  data.codexes = make_shared<PlayerCodexes>();
  data.techs = make_shared<PlayerTech>();
  data.companions = make_shared<PlayerCompanions>();
  data.deployment = make_shared<PlayerDeployment>();
  data.log = make_shared<PlayerLog>();
  data.questManager = make_shared<QuestManager>(nullptr);  // Player pointer set on init

  data.techController = make_shared<TechController>();

  data.hitDamageNotificationLimiter = 0;
  data.hitDamageNotificationLimit = assets->json("/player.config:hitDamageNotificationLimit").toInt();

  data.modeType = PlayerMode::Casual;
  data.isAdmin = false;
  data.interactRadius = config->interactRadius;
  data.walkIntoInteractBias = jsonToVec2F(config->walkIntoInteractBias);
  
  data.state = PlayerState::Idle;

  auto adapter = make_shared<PlayerAdapter>(ecsWorld, entity);
  adapter->setTeam(EntityDamageTeam(TeamType::Friendly));
  
  return adapter;
}

shared_ptr<PlayerAdapter> PlayerAdapter::createFromDiskStore(World& ecsWorld, PlayerConfigPtr config, Json const& diskStore) {
  auto adapter = create(ecsWorld, config, Uuid(diskStore.getString("uuid")));
  adapter->diskLoad(diskStore);
  return adapter;
}

shared_ptr<PlayerAdapter> PlayerAdapter::createFromNetStore(World& ecsWorld, PlayerConfigPtr config, ByteArray const& netStore, NetCompatibilityRules rules) {
  DataStreamBuffer ds(netStore);
  
  Uuid uuid = ds.read<Uuid>();
  auto adapter = create(ecsWorld, config, uuid);
  
  auto* data = adapter->getData();
  data->identity = ds.read<HumanoidIdentity>();
  data->humanoid->setIdentity(data->identity);
  data->modeType = PlayerModeNames.getLeft(ds.read<String>());
  
  return adapter;
}

PlayerAdapter::PlayerAdapter(World& ecsWorld, Entity entity)
  : EntityAdapter(ecsWorld, entity) {
  setupNetStates();
}

void PlayerAdapter::diskLoad(Json const& diskStore) {
  auto* data = getData();
  
  data->identity = HumanoidIdentity(diskStore.get("identity"));
  data->humanoid->setIdentity(data->identity);
  
  data->modeType = PlayerModeNames.getLeft(diskStore.getString("modeType", "casual"));
  
  if (diskStore.contains("movementController"))
    data->movementController->loadState(diskStore.get("movementController"));
  if (diskStore.contains("statusController"))
    data->statusController->diskLoad(diskStore.get("statusController"));
  
  if (diskStore.contains("inventory"))
    data->inventory->diskLoad(diskStore.get("inventory"));
  if (diskStore.contains("blueprints"))
    data->blueprints->diskLoad(diskStore.get("blueprints"));
  if (diskStore.contains("techs"))
    data->techs->diskLoad(diskStore.get("techs"));
  if (diskStore.contains("log"))
    data->log->diskLoad(diskStore.get("log"));
  
  if (diskStore.contains("armor"))
    data->armor->diskLoad(diskStore.get("armor"));
  
  if (diskStore.contains("shipUpgrades"))
    data->shipUpgrades = ShipUpgrades(diskStore.get("shipUpgrades"));
  
  data->description = diskStore.getString("description", "");
  data->isAdmin = diskStore.getBool("isAdmin", false);
  
  if (diskStore.contains("aiState"))
    data->aiState = AiState(diskStore.get("aiState"));
  
  if (diskStore.contains("genericProperties"))
    data->genericProperties = diskStore.getObject("genericProperties");
  
  setUniqueId(diskStore.optString("uniqueId"));
  if (diskStore.contains("team"))
    setTeam(EntityDamageTeam(diskStore.get("team")));
}

Json PlayerAdapter::diskStore() {
  auto const* data = getData();
  
  return JsonObject{
    {"uuid", data->uuid.hex()},
    {"identity", data->identity.toJson()},
    {"modeType", PlayerModeNames.getRight(data->modeType)},
    {"movementController", data->movementController->storeState()},
    {"statusController", data->statusController->diskStore()},
    {"inventory", data->inventory->diskStore()},
    {"blueprints", data->blueprints->diskStore()},
    {"techs", data->techs->diskStore()},
    {"log", data->log->diskStore()},
    {"armor", data->armor->diskStore()},
    {"shipUpgrades", data->shipUpgrades.toJson()},
    {"description", data->description},
    {"isAdmin", data->isAdmin},
    {"aiState", data->aiState.toJson()},
    {"genericProperties", data->genericProperties},
    {"uniqueId", jsonFromMaybe(uniqueId())},
    {"team", getTeam().toJson()}
  };
}

ByteArray PlayerAdapter::netStore(NetCompatibilityRules rules) {
  auto const* data = getData();
  
  DataStreamBuffer ds;
  ds.write(data->uuid);
  ds.write(data->identity);
  ds.write(PlayerModeNames.getRight(data->modeType));
  
  return ds.takeData();
}

ClientContextPtr PlayerAdapter::clientContext() const {
  return getData()->clientContext;
}

void PlayerAdapter::setClientContext(ClientContextPtr clientContext) {
  getData()->clientContext = std::move(clientContext);
}

StatisticsPtr PlayerAdapter::statistics() const {
  return getData()->statistics;
}

void PlayerAdapter::setStatistics(StatisticsPtr statistics) {
  getData()->statistics = std::move(statistics);
}

void PlayerAdapter::setUniverseClient(UniverseClient* universeClient) {
  getData()->client = universeClient;
}

UniverseClient* PlayerAdapter::universeClient() const {
  return getData()->client;
}

QuestManagerPtr PlayerAdapter::questManager() const {
  return getData()->questManager;
}

EntityType PlayerAdapter::entityType() const {
  return EntityType::Player;
}

ClientEntityMode PlayerAdapter::clientEntityMode() const {
  return ClientEntityMode::ClientPresenceMaster;
}

void PlayerAdapter::init(::Star::World* world, EntityId entityId, EntityMode mode) {
  EntityAdapter::init(world, entityId, mode);
  
  auto* data = getData();
  data->movementController->init(world);
  data->movementController->setIgnorePhysicsEntities({entityId});
  data->statusController->init(this, data->movementController.get());
  data->tools->init(this);
  data->techController->init(this, data->movementController.get(), data->statusController.get());

  data->armor->setupHumanoid(*data->humanoid, forceNude());

  if (isMaster()) {
    data->movementController->resetAnchorState();
  }
}

void PlayerAdapter::uninit() {
  auto* data = getData();
  
  if (isMaster()) {
    data->movementController->resetAnchorState();
  }
  
  data->techController->uninit();
  data->tools->uninit();
  data->statusController->uninit();
  data->movementController->uninit();
  
  EntityAdapter::uninit();
}

Vec2F PlayerAdapter::position() const {
  return getData()->movementController->position();
}

Vec2F PlayerAdapter::velocity() const {
  return getData()->movementController->velocity();
}

Vec2F PlayerAdapter::mouthPosition() const {
  return mouthOffset(true) + position();
}

Vec2F PlayerAdapter::mouthPosition(bool ignoreAdjustments) const {
  return mouthOffset(ignoreAdjustments) + position();
}

Vec2F PlayerAdapter::mouthOffset(bool ignoreAdjustments) const {
  auto const* data = getData();
  return Vec2F{data->humanoid->mouthOffset(ignoreAdjustments)[0] * numericalDirection(data->humanoid->facingDirection()),
      data->humanoid->mouthOffset(ignoreAdjustments)[1]};
}

Vec2F PlayerAdapter::feetOffset() const {
  auto const* data = getData();
  return {data->humanoid->feetOffset()[0] * numericalDirection(data->humanoid->facingDirection()), data->humanoid->feetOffset()[1]};
}

Vec2F PlayerAdapter::headArmorOffset() const {
  auto const* data = getData();
  return {data->humanoid->headArmorOffset()[0] * numericalDirection(data->humanoid->facingDirection()), data->humanoid->headArmorOffset()[1]};
}

Vec2F PlayerAdapter::chestArmorOffset() const {
  auto const* data = getData();
  return {data->humanoid->chestArmorOffset()[0] * numericalDirection(data->humanoid->facingDirection()), data->humanoid->chestArmorOffset()[1]};
}

Vec2F PlayerAdapter::legsArmorOffset() const {
  auto const* data = getData();
  return {data->humanoid->legsArmorOffset()[0] * numericalDirection(data->humanoid->facingDirection()), data->humanoid->legsArmorOffset()[1]};
}

Vec2F PlayerAdapter::backArmorOffset() const {
  auto const* data = getData();
  return {data->humanoid->backArmorOffset()[0] * numericalDirection(data->humanoid->facingDirection()), data->humanoid->backArmorOffset()[1]};
}

RectF PlayerAdapter::metaBoundBox() const {
  return RectF(-4, -4, 4, 4);
}

RectF PlayerAdapter::collisionArea() const {
  return getData()->movementController->collisionPoly().boundBox();
}

pair<ByteArray, uint64_t> PlayerAdapter::writeNetState(uint64_t fromVersion, NetCompatibilityRules rules) {
  return m_netGroup.writeNetState(fromVersion, rules);
}

void PlayerAdapter::readNetState(ByteArray data, float interpolationTime, NetCompatibilityRules rules) {
  m_netGroup.readNetState(std::move(data), interpolationTime, rules);
}

void PlayerAdapter::enableInterpolation(float extrapolationHint) {
  m_netGroup.enableNetInterpolation(extrapolationHint);
}

void PlayerAdapter::disableInterpolation() {
  m_netGroup.disableNetInterpolation();
}

Maybe<HitType> PlayerAdapter::queryHit(DamageSource const& source) const {
  auto const* data = getData();
  if (!inWorld() || !data->statusController->resourcePositive("health") || data->statusController->statPositive("invulnerable"))
    return {};

  if (data->tools->queryShieldHit(source))
    return HitType::ShieldHit;

  if (source.intersectsWithPoly(world()->geometry(), data->movementController->collisionBody()))
    return HitType::Hit;

  return {};
}

Maybe<PolyF> PlayerAdapter::hitPoly() const {
  return getData()->movementController->collisionBody();
}

List<DamageNotification> PlayerAdapter::applyDamage(DamageRequest const& damage) {
  if (!inWorld())
    return {};

  auto* data = getData();
  auto notifications = data->statusController->applyDamageRequest(damage);

  float totalDamage = 0.0f;
  for (auto const& notification : notifications)
    totalDamage += notification.healthLost;

  if (totalDamage > 0 && data->hitDamageNotificationLimiter < data->hitDamageNotificationLimit) {
    data->hitDamageNotificationLimiter++;
  }

  return notifications;
}

List<DamageNotification> PlayerAdapter::selfDamageNotifications() {
  return getData()->statusController->pullSelfDamageNotifications();
}

void PlayerAdapter::hitOther(EntityId targetEntityId, DamageRequest const& damageRequest) {
  // Stats tracking, etc.
}

void PlayerAdapter::damagedOther(DamageNotification const& damage) {
  auto* data = getData();
  if (inWorld() && isMaster()) {
    data->statusController->damagedOther(damage);
    data->lastDamagedOtherTimer = 0.0f;
    data->lastDamagedTarget = damage.targetEntityId;
  }
}

List<DamageSource> PlayerAdapter::damageSources() const {
  auto const* data = getData();
  auto damageSources = data->tools->damageSources();
  
  for (auto& damageSource : damageSources) {
    damageSource.sourceEntityId = entityId();
    damageSource.team = getTeam();
  }

  return damageSources;
}

bool PlayerAdapter::shouldDestroy() const {
  return false;  // Players are never destroyed, only killed/respawned
}

void PlayerAdapter::destroy(RenderCallback* renderCallback) {
  auto* data = getData();
  
  if (renderCallback && data->deathParticleBurst)
    renderCallback->addParticles(data->humanoid->particles(*data->deathParticleBurst), position());

  data->songbook->stop();
}

Maybe<EntityAnchorState> PlayerAdapter::loungingIn() const {
  auto const* data = getData();
  if (is<LoungeAnchor>(data->movementController->entityAnchor()))
    return data->movementController->anchorState();
  return {};
}

bool PlayerAdapter::lounge(EntityId loungeableEntityId, size_t anchorIndex) {
  auto* data = getData();
  auto loungeableEntity = world()->get<LoungeableEntity>(loungeableEntityId);
  if (!loungeableEntity || anchorIndex >= loungeableEntity->anchorCount()
      || !loungeableEntity->entitiesLoungingIn(anchorIndex).empty()
      || !loungeableEntity->loungeAnchor(anchorIndex))
    return false;

  data->movementController->setAnchorState({loungeableEntityId, anchorIndex});
  return true;
}

void PlayerAdapter::stopLounging() {
  getData()->movementController->resetAnchorState();
}

void PlayerAdapter::revive(Vec2F const& footPosition) {
  auto* data = getData();
  data->movementController->setPosition(footPosition);
  data->statusController->revive();
  data->state = PlayerState::Idle;
}

List<Drawable> PlayerAdapter::portrait(PortraitMode mode) const {
  return getData()->humanoid->renderPortrait(mode);
}

bool PlayerAdapter::underwater() const {
  return getData()->movementController->liquidMovement();
}

bool PlayerAdapter::shifting() const {
  return getData()->shifting;
}

void PlayerAdapter::setShifting(bool shifting) {
  getData()->shifting = shifting;
}

void PlayerAdapter::special(int specialKey) {
  getData()->techController->special(specialKey);
}

void PlayerAdapter::setMoveVector(Vec2F const& vec) {
  getData()->moveVector = vec;
}

void PlayerAdapter::moveLeft() {
  getData()->pendingMoves.add(MoveControlType::Left);
}

void PlayerAdapter::moveRight() {
  getData()->pendingMoves.add(MoveControlType::Right);
}

void PlayerAdapter::moveUp() {
  getData()->pendingMoves.add(MoveControlType::Up);
}

void PlayerAdapter::moveDown() {
  getData()->pendingMoves.add(MoveControlType::Down);
}

void PlayerAdapter::jump() {
  getData()->pendingMoves.add(MoveControlType::Jump);
}

void PlayerAdapter::dropItem() {
  // Drop selected item from inventory
}

float PlayerAdapter::toolRadius() const {
  return getData()->tools->toolRadius();
}

float PlayerAdapter::interactRadius() const {
  return getData()->interactRadius;
}

void PlayerAdapter::setInteractRadius(float interactRadius) {
  getData()->interactRadius = interactRadius;
}

List<InteractAction> PlayerAdapter::pullInteractActions() {
  List<InteractAction> result;
  auto& pending = getData()->pendingInteractActions;
  for (auto it = pending.begin(); it != pending.end();) {
    if (it->finished()) {
      if (auto action = it->result())
        result.append(*action);
      it = pending.erase(it);
    } else {
      ++it;
    }
  }
  return result;
}

uint64_t PlayerAdapter::currency(String const& currencyType) const {
  return getData()->inventory->currency(currencyType);
}

float PlayerAdapter::health() const {
  return getData()->statusController->resource("health");
}

float PlayerAdapter::maxHealth() const {
  return *getData()->statusController->resourceMax("health");
}

DamageBarType PlayerAdapter::damageBar() const {
  return DamageBarType::Default;
}

float PlayerAdapter::healthPercentage() const {
  return *getData()->statusController->resourcePercentage("health");
}

float PlayerAdapter::energy() const {
  return getData()->statusController->resource("energy");
}

float PlayerAdapter::maxEnergy() const {
  return *getData()->statusController->resourceMax("energy");
}

float PlayerAdapter::energyPercentage() const {
  return *getData()->statusController->resourcePercentage("energy");
}

float PlayerAdapter::energyRegenBlockPercent() const {
  return getData()->statusController->stat("energyRegenBlockPercent");
}

bool PlayerAdapter::energyLocked() const {
  return getData()->statusController->resourceLocked("energy");
}

bool PlayerAdapter::fullEnergy() const {
  return *getData()->statusController->resourcePercentage("energy") >= 1.0;
}

bool PlayerAdapter::consumeEnergy(float energy) {
  return getData()->statusController->overConsumeResource("energy", energy);
}

float PlayerAdapter::foodPercentage() const {
  if (auto max = getData()->statusController->resourceMax("food"))
    return getData()->statusController->resource("food") / *max;
  return 1.0f;
}

float PlayerAdapter::breath() const {
  return getData()->statusController->resource("breath");
}

float PlayerAdapter::maxBreath() const {
  return *getData()->statusController->resourceMax("breath");
}

float PlayerAdapter::protection() const {
  return getData()->statusController->stat("protection");
}

bool PlayerAdapter::forceNude() const {
  return getData()->statusController->statPositive("nude");
}

String PlayerAdapter::description() const {
  return getData()->description;
}

void PlayerAdapter::setDescription(String const& description) {
  getData()->description = description;
}

List<LightSource> PlayerAdapter::lightSources() const {
  auto const* data = getData();
  List<LightSource> lights;
  lights.appendAll(data->tools->lightSources());
  lights.appendAll(data->statusController->lightSources());
  lights.appendAll(data->techController->lightSources());
  lights.appendAll(data->effectsAnimator->lightSources(position()));
  return lights;
}

Direction PlayerAdapter::walkingDirection() const {
  return getData()->movementController->movingDirection();
}

Direction PlayerAdapter::facingDirection() const {
  return getData()->movementController->facingDirection();
}

Maybe<Json> PlayerAdapter::receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) {
  auto* data = getData();
  Maybe<Json> result = data->statusController->receiveMessage(message, world()->connection() == sendingConnection, args);
  return result;
}

void PlayerAdapter::update(float dt, uint64_t) {
  if (!inWorld())
    return;

  auto* data = getData();
  data->movementController->setTimestep(dt);

  if (isMaster()) {
    processControls();
    
    data->movementController->tickMaster(dt);
    data->statusController->tickMaster(dt);
    data->techController->tickMaster(dt);

    tickShared(dt);

    processStateChanges(dt);
    
    setNetStates();
  } else {
    m_netGroup.tickNetInterpolation(dt);
    getNetStates(false);
    data->movementController->tickSlave(dt);
    data->statusController->tickSlave(dt);
    data->techController->tickSlave(dt);

    tickShared(dt);
  }

  if (data->hitDamageNotificationLimiter)
    data->hitDamageNotificationLimiter--;

  if (world()->isClient())
    SpatialLogger::logPoly("world", data->movementController->collisionBody(), {0, 255, 0, 255});
}

void PlayerAdapter::render(RenderCallback* renderCallback) {
  auto* data = getData();
  EntityRenderLayer renderLayer = RenderLayerPlayer;
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

  data->tools->render(renderCallback, inToolRange(), data->shifting, renderLayer);

  renderCallback->addDrawables(data->tools->renderObjectPreviews(aimPosition(), walkingDirection(), inToolRange(), favoriteColor()), renderLayer);

  data->effectEmitter->render(renderCallback);
  data->songbook->render(renderCallback);
}

void PlayerAdapter::renderLightSources(RenderCallback* renderCallback) {
  renderCallback->addLightSources(lightSources());
}

Json PlayerAdapter::getGenericProperty(String const& name, Json const& defaultValue) const {
  return getData()->genericProperties.value(name, defaultValue);
}

void PlayerAdapter::setGenericProperty(String const& name, Json const& value) {
  if (value.isNull())
    getData()->genericProperties.erase(name);
  else
    getData()->genericProperties[name] = value;
}

PlayerInventoryPtr PlayerAdapter::inventory() const {
  return getData()->inventory;
}

uint64_t PlayerAdapter::itemsCanHold(ItemPtr const& items) const {
  return getData()->inventory->itemsCanFit(items);
}

ItemPtr PlayerAdapter::pickupItems(ItemPtr const& items, bool silent) {
  return getData()->inventory->pickupItems(items);
}

void PlayerAdapter::giveItem(ItemPtr const& item) {
  auto overflow = pickupItems(item);
  if (overflow && !overflow->empty())
    world()->addEntity(ItemDrop::createRandomizedDrop(overflow, position()));
}

void PlayerAdapter::triggerPickupEvents(ItemPtr const& item) {
  // Fire pickup events
}

ItemPtr PlayerAdapter::essentialItem(EssentialItem essentialItem) const {
  return getData()->inventory->essentialItem(essentialItem);
}

bool PlayerAdapter::hasItem(ItemDescriptor const& descriptor, bool exactMatch) const {
  return getData()->inventory->hasItem(descriptor, exactMatch);
}

uint64_t PlayerAdapter::hasCountOfItem(ItemDescriptor const& descriptor, bool exactMatch) const {
  return getData()->inventory->hasCountOfItem(descriptor, exactMatch);
}

ItemDescriptor PlayerAdapter::takeItem(ItemDescriptor const& descriptor, bool consumePartial, bool exactMatch) {
  return getData()->inventory->takeItem(descriptor, consumePartial, exactMatch);
}

void PlayerAdapter::giveItem(ItemDescriptor const& descriptor) {
  auto item = Root::singleton().itemDatabase()->item(descriptor);
  giveItem(item);
}

void PlayerAdapter::clearSwap() {
  getData()->inventory->clearSwap();
}

void PlayerAdapter::refreshItems() {
  auto* data = getData();
  data->armor->setItems(
    data->inventory->headArmor(),
    data->inventory->chestArmor(),
    data->inventory->legsArmor(),
    data->inventory->backArmor()
  );
}

void PlayerAdapter::refreshArmor() {
  auto* data = getData();
  data->armor->setupHumanoid(*data->humanoid, forceNude());
}

void PlayerAdapter::refreshHumanoid() const {
  // Refresh humanoid parameters from species
}

void PlayerAdapter::refreshEquipment() {
  refreshItems();
  refreshArmor();
}

PlayerBlueprintsPtr PlayerAdapter::blueprints() const {
  return getData()->blueprints;
}

bool PlayerAdapter::addBlueprint(ItemDescriptor const& descriptor, bool showFailure) {
  return getData()->blueprints->add(descriptor);
}

bool PlayerAdapter::blueprintKnown(ItemDescriptor const& descriptor) const {
  return getData()->blueprints->isKnown(descriptor);
}

bool PlayerAdapter::addCollectable(String const& collectionName, String const& collectableName) {
  return getData()->log->addCollectable(collectionName, collectableName);
}

PlayerUniverseMapPtr PlayerAdapter::universeMap() const {
  return getData()->universeMap;
}

PlayerCodexesPtr PlayerAdapter::codexes() const {
  return getData()->codexes;
}

PlayerTechPtr PlayerAdapter::techs() const {
  return getData()->techs;
}

void PlayerAdapter::overrideTech(Maybe<StringList> const& techModules) {
  getData()->techController->setOverrideTech(techModules);
}

bool PlayerAdapter::techOverridden() const {
  return getData()->techController->techOverridden();
}

PlayerCompanionsPtr PlayerAdapter::companions() const {
  return getData()->companions;
}

PlayerLogPtr PlayerAdapter::log() const {
  return getData()->log;
}

InteractiveEntityPtr PlayerAdapter::bestInteractionEntity(bool includeNearby) {
  // Find best entity to interact with
  return {};
}

void PlayerAdapter::interactWithEntity(InteractiveEntityPtr entity) {
  // Start interaction
}

void PlayerAdapter::aim(Vec2F const& position) {
  getData()->aimPosition = position;
}

Vec2F PlayerAdapter::aimPosition() const {
  return getData()->aimPosition;
}

Vec2F PlayerAdapter::armPosition(ToolHand hand, Direction facingDirection, float armAngle, Vec2F offset) const {
  auto const* data = getData();
  return data->tools->armPosition(*data->humanoid, hand, facingDirection, armAngle, offset);
}

Vec2F PlayerAdapter::handOffset(ToolHand hand, Direction facingDirection) const {
  return getData()->tools->handOffset(*getData()->humanoid, hand, facingDirection);
}

Vec2F PlayerAdapter::handPosition(ToolHand hand, Vec2F const& handOffset) const {
  return getData()->tools->handPosition(hand, *getData()->humanoid, handOffset);
}

ItemPtr PlayerAdapter::handItem(ToolHand hand) const {
  auto const* data = getData();
  if (hand == ToolHand::Primary)
    return data->tools->primaryHandItem();
  return data->tools->altHandItem();
}

Vec2F PlayerAdapter::armAdjustment() const {
  return getData()->humanoid->armAdjustment();
}

void PlayerAdapter::setCameraFocusEntity(Maybe<EntityId> const& cameraFocusEntity) {
  getData()->cameraFocusEntity = cameraFocusEntity;
}

void PlayerAdapter::playEmote(HumanoidEmote emote) {
  addEmote(emote);
}

bool PlayerAdapter::canUseTool() const {
  auto const* data = getData();
  bool canUse = !isDead() && !data->statusController->toolUsageSuppressed();
  if (canUse) {
    if (auto loungeAnchor = as<LoungeAnchor>(data->movementController->entityAnchor()))
      if (loungeAnchor->suppressTools.value(loungeAnchor->controllable))
        return false;
  }
  return canUse;
}

void PlayerAdapter::beginPrimaryFire() {
  getData()->tools->beginPrimaryFire();
}

void PlayerAdapter::beginAltFire() {
  getData()->tools->beginAltFire();
}

void PlayerAdapter::endPrimaryFire() {
  getData()->tools->endPrimaryFire();
}

void PlayerAdapter::endAltFire() {
  getData()->tools->endAltFire();
}

void PlayerAdapter::beginTrigger() {
  auto* data = getData();
  if (!data->useDown) {
    data->useDown = true;
    data->edgeTriggeredUse = true;
  }
}

void PlayerAdapter::endTrigger() {
  getData()->useDown = false;
}

ItemPtr PlayerAdapter::primaryHandItem() const {
  return getData()->tools->primaryHandItem();
}

ItemPtr PlayerAdapter::altHandItem() const {
  return getData()->tools->altHandItem();
}

Uuid PlayerAdapter::uuid() const {
  return getData()->uuid;
}

PlayerMode PlayerAdapter::modeType() const {
  return getData()->modeType;
}

void PlayerAdapter::setModeType(PlayerMode mode) {
  getData()->modeType = mode;
}

PlayerModeConfig PlayerAdapter::modeConfig() const {
  return getData()->modeConfig;
}

ShipUpgrades PlayerAdapter::shipUpgrades() {
  return getData()->shipUpgrades;
}

void PlayerAdapter::setShipUpgrades(ShipUpgrades shipUpgrades) {
  getData()->shipUpgrades = std::move(shipUpgrades);
}

void PlayerAdapter::applyShipUpgrades(Json const& upgrades) {
  getData()->shipUpgrades.apply(upgrades);
}

String PlayerAdapter::name() const {
  return getData()->identity.name;
}

void PlayerAdapter::setName(String const& name) {
  getData()->identity.name = name;
  updateIdentity();
}

Maybe<String> PlayerAdapter::statusText() const {
  return {};
}

bool PlayerAdapter::displayNametag() const {
  return true;
}

Vec3B PlayerAdapter::nametagColor() const {
  return Vec3B(255, 255, 255);
}

Vec2F PlayerAdapter::nametagOrigin() const {
  return mouthPosition(false);
}

String PlayerAdapter::nametag() const {
  if (getData()->nametagOverride)
    return *getData()->nametagOverride;
  return name();
}

void PlayerAdapter::setNametag(Maybe<String> nametag) {
  getData()->nametagOverride = std::move(nametag);
}

void PlayerAdapter::updateIdentity() {
  auto* data = getData();
  data->identityUpdated = true;
  data->humanoid->setIdentity(data->identity);
}

HumanoidPtr PlayerAdapter::humanoid() {
  return getData()->humanoid;
}

HumanoidPtr PlayerAdapter::humanoid() const {
  return getData()->humanoid;
}

HumanoidIdentity const& PlayerAdapter::identity() const {
  return getData()->identity;
}

void PlayerAdapter::setIdentity(HumanoidIdentity identity) {
  getData()->identity = std::move(identity);
  updateIdentity();
}

void PlayerAdapter::setAdmin(bool isAdmin) {
  getData()->isAdmin = isAdmin;
}

bool PlayerAdapter::isAdmin() const {
  return getData()->isAdmin;
}

bool PlayerAdapter::inToolRange() const {
  return inToolRange(aimPosition());
}

bool PlayerAdapter::inToolRange(Vec2F const& aimPos) const {
  auto distance = world()->geometry().diff(aimPos, position()).magnitude();
  return distance <= toolRadius();
}

bool PlayerAdapter::inInteractionRange() const {
  return inInteractionRange(aimPosition());
}

bool PlayerAdapter::inInteractionRange(Vec2F aimPos) const {
  auto distance = world()->geometry().diff(aimPos, position()).magnitude();
  return distance <= interactRadius();
}

void PlayerAdapter::addParticles(List<Particle> const& particles) {
  getData()->callbackParticles.appendAll(particles);
}

void PlayerAdapter::addSound(String const& sound, float volume, float pitch) {
  getData()->callbackSounds.append({sound, volume, pitch});
}

bool PlayerAdapter::wireToolInUse() const {
  return false;  // TODO: Implement wire tool detection
}

void PlayerAdapter::setWireConnector(WireConnector* wireConnector) const {
  // TODO: Implement wire connector
}

void PlayerAdapter::addEphemeralStatusEffects(List<EphemeralStatusEffect> const& statusEffects) {
  getData()->statusController->addEphemeralEffects(statusEffects);
}

ActiveUniqueStatusEffectSummary PlayerAdapter::activeUniqueStatusEffectSummary() const {
  return getData()->statusController->activeUniqueStatusEffectSummary();
}

float PlayerAdapter::powerMultiplier() const {
  return getData()->statusController->stat("powerMultiplier");
}

bool PlayerAdapter::isDead() const {
  return !getData()->statusController->resourcePositive("health");
}

void PlayerAdapter::kill() {
  getData()->statusController->setResource("health", 0);
}

void PlayerAdapter::setFavoriteColor(Color color) {
  getData()->identity.color = color.toRgba();
  updateIdentity();
}

Color PlayerAdapter::favoriteColor() const {
  return Color::rgba(getData()->identity.color);
}

void PlayerAdapter::teleportOut(String const& animationType, bool deploy) {
  auto* data = getData();
  data->teleportAnimationType = animationType;
  data->state = PlayerState::TeleportOut;
}

void PlayerAdapter::teleportIn() {
  auto* data = getData();
  data->state = PlayerState::TeleportIn;
}

void PlayerAdapter::teleportAbort() {
  auto* data = getData();
  if (data->state == PlayerState::TeleportOut || data->state == PlayerState::TeleportIn)
    data->state = PlayerState::Idle;
}

bool PlayerAdapter::isTeleporting() const {
  auto state = getData()->state;
  return state == PlayerState::TeleportIn || state == PlayerState::TeleportOut;
}

bool PlayerAdapter::isTeleportingOut() const {
  return getData()->state == PlayerState::TeleportOut;
}

bool PlayerAdapter::canDeploy() {
  return getData()->deployment && getData()->deployment->canDeploy();
}

void PlayerAdapter::deployAbort(String const& animationType) {
  teleportAbort();
}

bool PlayerAdapter::isDeploying() const {
  return getData()->deployment && getData()->deployment->isDeploying();
}

bool PlayerAdapter::isDeployed() const {
  return getData()->deployment && getData()->deployment->isDeployed();
}

void PlayerAdapter::setBusyState(PlayerBusyState busyState) {
  // Set player busy state
}

void PlayerAdapter::moveTo(Vec2F const& footPosition) {
  getData()->movementController->setPosition(footPosition);
}

List<String> PlayerAdapter::pullQueuedMessages() {
  return take(getData()->queuedMessages);
}

List<ItemPtr> PlayerAdapter::pullQueuedItemDrops() {
  return take(getData()->queuedItemPickups);
}

void PlayerAdapter::queueUIMessage(String const& message) {
  getData()->queuedMessages.append(message);
}

void PlayerAdapter::queueItemPickupMessage(ItemPtr const& item) {
  getData()->queuedItemPickups.append(item);
}

void PlayerAdapter::addChatMessage(String const& message, Json const& config) {
  auto* data = getData();
  data->chatMessage = message;
  data->chatMessageChanged = true;
  data->chatMessageUpdated = true;
  data->pendingChatActions.append(SayChatAction{entityId(), message, mouthPosition(), config});
}

void PlayerAdapter::addEmote(HumanoidEmote const& emote, Maybe<float> emoteCooldown) {
  auto* data = getData();
  data->emoteState = emote;
  if (emoteCooldown)
    data->emoteCooldownTimer = GameTimer(*emoteCooldown);
  else
    data->emoteCooldownTimer.reset();
}

void PlayerAdapter::setDance(Maybe<String> const& danceName) {
  auto* data = getData();
  data->dance = danceName;
  if (danceName)
    data->danceCooldownTimer = GameTimer(2.0f);  // Default dance duration
}

pair<HumanoidEmote, float> PlayerAdapter::currentEmote() const {
  auto const* data = getData();
  return {data->emoteState, data->emoteCooldownTimer.percent()};
}

PlayerState PlayerAdapter::currentState() const {
  return getData()->state;
}

List<ChatAction> PlayerAdapter::pullPendingChatActions() {
  return take(getData()->pendingChatActions);
}

Maybe<String> PlayerAdapter::inspectionLogName() const {
  return name();
}

Maybe<String> PlayerAdapter::inspectionDescription(String const& species) const {
  return description();
}

float PlayerAdapter::beamGunRadius() const {
  return getData()->tools->beamGunRadius();
}

bool PlayerAdapter::instrumentPlaying() {
  return getData()->songbook->instrumentPlaying();
}

void PlayerAdapter::instrumentEquipped(String const& instrumentKind) {
  auto* data = getData();
  if (canUseTool())
    data->songbook->keepAlive(instrumentKind, mouthPosition());
}

void PlayerAdapter::interact(InteractAction const& action) {
  // Process interaction action
}

void PlayerAdapter::addEffectEmitters(StringSet const& emitters) {
  getData()->effectEmitter->addEffectSources("normal", emitters);
}

void PlayerAdapter::requestEmote(String const& emote) {
  auto* data = getData();
  if (!emote.empty()) {
    auto state = HumanoidEmoteNames.getLeft(emote);
    if (state != HumanoidEmote::Idle && (data->emoteState == HumanoidEmote::Idle || data->emoteState == HumanoidEmote::Blink))
      addEmote(state);
  }
}

ActorMovementController* PlayerAdapter::movementController() {
  return getData()->movementController.get();
}

StatusController* PlayerAdapter::statusController() {
  return getData()->statusController.get();
}

List<PhysicsForceRegion> PlayerAdapter::forceRegions() const {
  return getData()->tools->forceRegions();
}

StatusControllerPtr PlayerAdapter::statusControllerPtr() {
  return getData()->statusController;
}

ActorMovementControllerPtr PlayerAdapter::movementControllerPtr() {
  return getData()->movementController;
}

PlayerConfigPtr PlayerAdapter::playerConfig() {
  return getData()->config;
}

SongbookPtr PlayerAdapter::songbook() const {
  return getData()->songbook;
}

void PlayerAdapter::finalizeCreation() {
  // Finalize player creation
}

float PlayerAdapter::timeSinceLastGaveDamage() const {
  return getData()->lastDamagedOtherTimer;
}

EntityId PlayerAdapter::lastDamagedTarget() const {
  return getData()->lastDamagedTarget;
}

bool PlayerAdapter::invisible() const {
  return getData()->statusController->statPositive("invisible");
}

void PlayerAdapter::animatePortrait(float dt) {
  getData()->humanoid->animate(dt, nullptr);
}

bool PlayerAdapter::isOutside() {
  return world()->isOutside(position());
}

void PlayerAdapter::dropSelectedItems(function<bool(ItemPtr)> filter) {
  // Drop items matching filter
}

void PlayerAdapter::dropEverything() {
  // Drop all items
}

bool PlayerAdapter::isPermaDead() const {
  return getData()->modeType == PlayerMode::Hardcore && isDead();
}

bool PlayerAdapter::interruptRadioMessage() {
  auto* data = getData();
  bool hadMessage = !data->pendingRadioMessages.empty();
  data->interruptRadioMessage = true;
  return hadMessage;
}

Maybe<RadioMessage> PlayerAdapter::pullPendingRadioMessage() {
  auto* data = getData();
  if (data->pendingRadioMessages.empty())
    return {};
  return data->pendingRadioMessages.takeFirst();
}

void PlayerAdapter::queueRadioMessage(Json const& messageConfig, float delay) {
  auto message = Root::singleton().radioMessageDatabase()->createRadioMessage(messageConfig);
  if (delay > 0)
    getData()->delayedRadioMessages.append({GameTimer(delay), message});
  else
    queueRadioMessage(message);
}

void PlayerAdapter::queueRadioMessage(RadioMessage message) {
  getData()->pendingRadioMessages.append(std::move(message));
}

Maybe<Json> PlayerAdapter::pullPendingCinematic() {
  return take(getData()->pendingCinematic);
}

void PlayerAdapter::setPendingCinematic(Json const& cinematic, bool unique) {
  getData()->pendingCinematic = cinematic;
}

void PlayerAdapter::setInCinematic(bool inCinematic) {
  // Set cinematic state
}

Maybe<pair<Maybe<pair<StringList, int>>, float>> PlayerAdapter::pullPendingAltMusic() {
  return take(getData()->pendingAltMusic);
}

Maybe<PlayerWarpRequest> PlayerAdapter::pullPendingWarp() {
  return take(getData()->pendingWarp);
}

void PlayerAdapter::setPendingWarp(String const& action, Maybe<String> const& animation, bool deploy) {
  getData()->pendingWarp = PlayerWarpRequest{action, animation, deploy};
}

Maybe<pair<Json, RpcPromiseKeeper<Json>>> PlayerAdapter::pullPendingConfirmation() {
  auto& confirmations = getData()->pendingConfirmations;
  if (confirmations.empty())
    return {};
  return confirmations.takeFirst();
}

void PlayerAdapter::queueConfirmation(Json const& dialogConfig, RpcPromiseKeeper<Json> const& resultPromise) {
  getData()->pendingConfirmations.append({dialogConfig, resultPromise});
}

AiState const& PlayerAdapter::aiState() const {
  return getData()->aiState;
}

AiState& PlayerAdapter::aiState() {
  return getData()->aiState;
}

bool PlayerAdapter::inspecting() const {
  // Check if player is in inspection mode
  return false;
}

EntityHighlightEffect PlayerAdapter::inspectionHighlight(InspectableEntityPtr const& inspectableEntity) const {
  return EntityHighlightEffect();
}

Vec2F PlayerAdapter::cameraPosition() {
  return position();
}

NetworkedAnimatorPtr PlayerAdapter::effectsAnimator() {
  return getData()->effectsAnimator;
}

Maybe<StringView> PlayerAdapter::getSecretPropertyView(String const& name) const {
  // TODO: Implement secret properties via effects animator tags
  return {};
}

String const* PlayerAdapter::getSecretPropertyPtr(String const& name) const {
  return nullptr;
}

Json PlayerAdapter::getSecretProperty(String const& name, Json defaultValue) const {
  // TODO: Implement
  return defaultValue;
}

void PlayerAdapter::setSecretProperty(String const& name, Json const& value) {
  // TODO: Implement
}

void PlayerAdapter::setAnimationParameter(String name, Json value) {
  getData()->scriptedAnimationParameters[std::move(name)] = std::move(value);
}

PlayerDataComponent* PlayerAdapter::getData() {
  return m_world.getComponentPtr<PlayerDataComponent>(m_entity);
}

PlayerDataComponent const* PlayerAdapter::getData() const {
  return m_world.getComponentPtr<PlayerDataComponent>(m_entity);
}

void PlayerAdapter::processControls() {
  auto* data = getData();
  
  // Process pending moves
  for (auto move : data->pendingMoves) {
    switch (move) {
      case MoveControlType::Left:
        data->movementController->controlMove(Direction::Left, data->shifting);
        break;
      case MoveControlType::Right:
        data->movementController->controlMove(Direction::Right, data->shifting);
        break;
      case MoveControlType::Up:
        data->movementController->controlModifiers(ActorMovementModifiers().withMovingUp(true));
        break;
      case MoveControlType::Down:
        data->movementController->controlModifiers(ActorMovementModifiers().withMovingDown(true));
        break;
      case MoveControlType::Jump:
        data->movementController->controlJump();
        break;
    }
  }
  data->pendingMoves.clear();
  
  // Apply move vector
  if (data->moveVector != Vec2F()) {
    data->movementController->controlMove(data->moveVector.x() < 0 ? Direction::Left : Direction::Right, data->shifting);
  }
  
  // Face aim position
  auto aimDirection = world()->geometry().diff(aimPosition(), position());
  if (aimDirection.x() != 0)
    data->movementController->controlFace(aimDirection.x() > 0 ? Direction::Right : Direction::Left);
  
  data->edgeTriggeredUse = false;
}

void PlayerAdapter::processStateChanges(float dt) {
  auto* data = getData();
  
  // Update state based on movement
  if (!is<LoungeAnchor>(data->movementController->entityAnchor())) {
    if (data->movementController->groundMovement()) {
      if (data->movementController->running())
        data->state = PlayerState::Run;
      else if (data->movementController->walking())
        data->state = PlayerState::Walk;
      else if (data->movementController->crouching())
        data->state = PlayerState::Crouch;
      else
        data->state = PlayerState::Idle;
    } else if (data->movementController->liquidMovement()) {
      if (abs(data->movementController->xVelocity()) > 0)
        data->state = PlayerState::Swim;
      else
        data->state = PlayerState::SwimIdle;
    } else {
      if (data->movementController->yVelocity() > 0)
        data->state = PlayerState::Jump;
      else
        data->state = PlayerState::Fall;
    }
  } else {
    data->state = PlayerState::Lounge;
  }
  
  // Update humanoid state
  switch (data->state) {
    case PlayerState::Idle: data->humanoid->setState(Humanoid::Idle); break;
    case PlayerState::Walk: data->humanoid->setState(Humanoid::Walk); break;
    case PlayerState::Run: data->humanoid->setState(Humanoid::Run); break;
    case PlayerState::Jump: data->humanoid->setState(Humanoid::Jump); break;
    case PlayerState::Fall: data->humanoid->setState(Humanoid::Fall); break;
    case PlayerState::Swim: data->humanoid->setState(Humanoid::Swim); break;
    case PlayerState::SwimIdle: data->humanoid->setState(Humanoid::SwimIdle); break;
    case PlayerState::TeleportIn: data->humanoid->setState(Humanoid::Idle); break;
    case PlayerState::TeleportOut: data->humanoid->setState(Humanoid::Idle); break;
    case PlayerState::Crouch: data->humanoid->setState(Humanoid::Duck); break;
    case PlayerState::Lounge:
      if (auto loungeAnchor = as<LoungeAnchor>(data->movementController->entityAnchor())) {
        switch (loungeAnchor->orientation) {
          case LoungeOrientation::Sit: data->humanoid->setState(Humanoid::Sit); break;
          case LoungeOrientation::Lay: data->humanoid->setState(Humanoid::Lay); break;
          default: data->humanoid->setState(Humanoid::Idle); break;
        }
      }
      break;
  }
  
  // Handle emotes
  if (data->emoteCooldownTimer.tick(dt))
    data->emoteState = HumanoidEmote::Idle;
  if (data->danceCooldownTimer.tick(dt))
    data->dance = {};
    
  if (data->blinkCooldownTimer.tick(dt)) {
    data->blinkCooldownTimer = GameTimer(Random::randf(data->blinkInterval[0], data->blinkInterval[1]));
    if (data->emoteState == HumanoidEmote::Idle)
      addEmote(HumanoidEmote::Blink);
  }

  data->humanoid->setEmoteState(data->emoteState);
  data->humanoid->setDance(data->dance);
  
  // Update damage timer
  data->lastDamagedOtherTimer += dt;
}

void PlayerAdapter::getNetStates(bool initial) {
  auto* data = getData();
  setTeam(m_teamNetState.get());
  
  data->state = (PlayerState)m_stateNetState.get();
  data->shifting = m_shiftingNetState.get();
  data->aimPosition = Vec2F(m_xAimPositionNetState.get(), m_yAimPositionNetState.get());
  
  if (m_identityNetState.pullUpdated() && !initial) {
    data->identity = m_identityNetState.get();
    data->humanoid->setIdentity(data->identity);
  }
  
  data->dance = m_humanoidDanceNetState.get();
  data->humanoid->setDance(data->dance);
  
  if (m_newChatMessageNetState.pullOccurred() && !initial) {
    data->chatMessageUpdated = true;
    data->pendingChatActions.append(SayChatAction{entityId(), m_chatMessageNetState.get(), mouthPosition(), {}});
  }
  
  if (!m_emoteNetState.get().empty()) {
    data->emoteState = HumanoidEmoteNames.getLeft(m_emoteNetState.get());
    data->humanoid->setEmoteState(data->emoteState);
  }
  
  data->deathParticleBurst = m_deathParticleBurst.get();
}

void PlayerAdapter::setNetStates() {
  auto const* data = getData();
  m_teamNetState.set(getTeam());
  
  m_stateNetState.set((unsigned)data->state);
  m_shiftingNetState.set(data->shifting);
  m_xAimPositionNetState.set(data->aimPosition.x());
  m_yAimPositionNetState.set(data->aimPosition.y());
  
  if (data->identityUpdated) {
    m_identityNetState.push(data->identity);
    getData()->identityUpdated = false;
  }
  
  m_humanoidDanceNetState.set(data->dance);
  
  if (data->chatMessageChanged) {
    m_chatMessageNetState.set(data->chatMessage);
    m_newChatMessageNetState.trigger();
    getData()->chatMessageChanged = false;
  }
  
  m_emoteNetState.set(HumanoidEmoteNames.getRight(data->emoteState));
  m_deathParticleBurst.set(data->deathParticleBurst);
}

List<Drawable> PlayerAdapter::drawables() const {
  return getData()->humanoid->render();
}

List<OverheadBar> PlayerAdapter::bars() const {
  return {};
}

List<Particle> PlayerAdapter::particles() {
  return take(getData()->callbackParticles);
}

String PlayerAdapter::getFootstepSound(Vec2I const& sensor) const {
  // Get footstep sound based on material
  return "";
}

void PlayerAdapter::tickShared(float dt) {
  auto* data = getData();

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

  data->armor->setupHumanoid(*data->humanoid, forceNude());

  data->tools->suppressItems(!canUseTool());
  data->tools->tick(dt, data->shifting, {});

  if (auto overrideDirection = data->tools->setupHumanoidHandItems(*data->humanoid, position(), aimPosition()))
    data->movementController->controlFace(*overrideDirection);

  if (world()->isClient()) {
    data->humanoid->animate(dt, nullptr);
  } else {
    data->humanoid->animate(dt, nullptr);
  }
}

HumanoidEmote PlayerAdapter::detectEmotes(String const& chatter) {
  return Root::singleton().emoteProcessor()->detectEmotes(chatter);
}

void PlayerAdapter::setupNetStates() {
  m_netGroup.addNetElement(&m_stateNetState);
  m_netGroup.addNetElement(&m_shiftingNetState);
  m_netGroup.addNetElement(&m_xAimPositionNetState);
  m_netGroup.addNetElement(&m_yAimPositionNetState);

  m_xAimPositionNetState.setFixedPointBase(0.0625);
  m_yAimPositionNetState.setFixedPointBase(0.0625);
  m_xAimPositionNetState.setInterpolator(lerp<float, float>);
  m_yAimPositionNetState.setInterpolator(lerp<float, float>);

  m_netGroup.addNetElement(&m_identityNetState);
  m_netGroup.addNetElement(&m_refreshedHumanoidParameters);
  m_netGroup.addNetElement(&m_teamNetState);
  m_netGroup.addNetElement(&m_landedNetState);
  m_netGroup.addNetElement(&m_chatMessageNetState);
  m_netGroup.addNetElement(&m_newChatMessageNetState);
  m_netGroup.addNetElement(&m_emoteNetState);
  m_netGroup.addNetElement(&m_humanoidDanceNetState);
  m_netGroup.addNetElement(&m_deathParticleBurst);
  m_netGroup.addNetElement(&m_scriptedAnimationParameters);
}

} // namespace ECS
} // namespace Star
