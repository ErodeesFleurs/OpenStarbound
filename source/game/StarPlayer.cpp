#include "StarPlayer.hpp"
#include "StarEncode.hpp"
#include "StarJsonExtra.hpp"
#include "StarRoot.hpp"
#include "StarSongbook.hpp"
#include "StarSongbookLuaBindings.hpp"
#include "StarEmoteProcessor.hpp"
#include "StarSpeciesDatabase.hpp"
#include "StarDamageManager.hpp"
#include "StarTools.hpp"
#include "StarItemDrop.hpp"
#include "StarMaterialDatabase.hpp"
#include "StarArmors.hpp"
#include "StarPlayerFactory.hpp"
#include "StarAssets.hpp"
#include "StarPlayerInventory.hpp"
#include "StarTechController.hpp"
#include "StarClientContext.hpp"
#include "StarItemDatabase.hpp"
#include "StarItemBag.hpp"
#include "StarEntitySplash.hpp"
#include "StarWorld.hpp"
#include "StarStatusController.hpp"
#include "StarStatusControllerLuaBindings.hpp"
#include "StarPlayerBlueprints.hpp"
#include "StarPlayerUniverseMap.hpp"
#include "StarPlayerCodexes.hpp"
#include "StarPlayerTech.hpp"
#include "StarPlayerCompanions.hpp"
#include "StarPlayerDeployment.hpp"
#include "StarPlayerLog.hpp"
#include "StarPlayerLuaBindings.hpp"
#include "StarQuestManager.hpp"
#include "StarAiDatabase.hpp"
#include "StarStatistics.hpp"
#include "StarInspectionTool.hpp"
#include "StarUtilityLuaBindings.hpp"
#include "StarCelestialLuaBindings.hpp"
#include "StarNetworkedAnimatorLuaBindings.hpp"
#include "StarScriptedAnimatorLuaBindings.hpp"
#include "StarEntityLuaBindings.hpp"
#include "StarDanceDatabase.hpp"
#include "StarPlayerNarrativeQueue.hpp"
#include "StarPlayerChatAndEmotes.hpp"
#include "StarPlayerDamagePipeline.hpp"
#include "StarPlayerTeleporter.hpp"
#include "StarPlayerAppearance.hpp"

namespace Star {

using namespace std::string_literals;

EnumMap<Player::State> const Player::StateNames{
  {Player::State::Idle, "idle"},
  {Player::State::Walk, "walk"},
  {Player::State::Run, "run"},
  {Player::State::Jump, "jump"},
  {Player::State::Fall, "fall"},
  {Player::State::Swim, "swim"},
  {Player::State::SwimIdle, "swimIdle"},
  {Player::State::TeleportIn, "teleportIn"},
  {Player::State::TeleportOut, "teleportOut"},
  {Player::State::Crouch, "crouch"},
  {Player::State::Lounge, "lounge"}
};

Player::Player(PlayerConfigPtr config, Uuid uuid) {
  auto assets = Root::singleton().assets();

  m_config = config;
  m_client = nullptr;

  m_state = State::Idle;

  m_footstepTimer = 0.0f;

  m_shifting = false;

  m_aimPosition = Vec2F();

  setUniqueId(uuid.hex());
  m_appearance.init();

  m_questManager = make_shared<QuestManager>(this);
  m_tools = make_shared<ToolUser>(this);
  m_armor = make_shared<ArmorWearer>();
  m_companions = make_shared<PlayerCompanions>(config->companionsConfig);

  for (auto& p : config->genericScriptContexts) {
    auto scriptComponent = make_shared<GenericScriptComponent>();
    scriptComponent->setScript(p.second);
    m_genericScriptContexts.set(p.first, scriptComponent);
  }

  // all of these are defaults and won't include the correct humanoid config for the species
  auto movementParameters = ActorMovementParameters(jsonMerge(humanoid()->defaultMovementParameters(), humanoid()->playerMovementParameters().value(m_config->movementParameters)));
  if (!movementParameters.physicsEffectCategories)
    movementParameters.physicsEffectCategories = StringSet({"player"});
  m_movementController = make_shared<ActorMovementController>(movementParameters);
  m_zeroGMovementParameters = ActorMovementParameters(m_config->zeroGMovementParameters);

  m_statusController = make_shared<StatusController>(m_config->statusControllerSettings);
  m_techController = make_shared<TechController>(this, m_movementController.get(), m_statusController.get());
  m_deployment = make_shared<PlayerDeployment>(m_config->deploymentConfig);

  m_inventory = make_shared<PlayerInventory>();
  m_inventory->setPlayer(this);

  m_blueprints = make_shared<PlayerBlueprints>();
  m_universeMap = make_shared<PlayerUniverseMap>();
  m_codexes = make_shared<PlayerCodexes>();
  m_techs = make_shared<PlayerTech>();
  m_log = make_shared<PlayerLog>();
  m_narrativeQueue = make_shared<PlayerNarrativeQueue>(this);
  m_chatAndEmotes = make_shared<PlayerChatAndEmotes>(this);
  m_damagePipeline = make_shared<PlayerDamagePipeline>(this);
  m_teleporter = make_shared<PlayerTeleporter>(this);

  setModeType(PlayerMode::Casual);

  m_useDown = false;
  m_edgeTriggeredUse = false;
  setTeam(EntityDamageTeam(TeamType::Friendly));

  m_footstepVolumeVariance = assets->json("/sfx.config:footstepVolumeVariance").toFloat();
  m_landingVolume = assets->json("/sfx.config:landingVolume").toFloat();

  m_effectsAnimator = make_shared<NetworkedAnimator>(assets->fetchJson(m_config->effectsAnimator));
  m_effectEmitter = make_shared<EffectEmitter>();

  m_interactRadius = assets->json("/player.config:interactRadius").toFloat();

  m_walkIntoInteractBias = jsonToVec2F(assets->json("/player.config:walkIntoInteractBias"));

  if (m_landingVolume <= 1)
    m_landingVolume = 6;

  m_isAdmin = false;

  m_chatAndEmotes->init(assets->json("/player.config:emoteCooldown").toFloat(), jsonToVec2F(assets->json("/player.config:blinkInterval")));

  m_songbook = make_shared<Songbook>(species());


  m_ageItemsTimer = GameTimer(assets->json("/player.config:ageItemsEvery").toFloat());

  refreshEquipment();

  m_foodLowThreshold = assets->json("/player.config:foodLowThreshold").toFloat();
  m_foodLowStatusEffects = assets->json("/player.config:foodLowStatusEffects").toArray().transformed(jsonToPersistentStatusEffect);
  m_foodEmptyStatusEffects = assets->json("/player.config:foodEmptyStatusEffects").toArray().transformed(jsonToPersistentStatusEffect);

  m_narrativeQueue->init(assets->json("/player.config:inCinematicStatusEffects").toArray().transformed(jsonToPersistentStatusEffect));

  m_statusController->setPersistentEffects("armor", m_armor->statusEffects());
  m_statusController->setPersistentEffects("tools", m_tools->statusEffects());
  m_statusController->resetAllResources();

  m_landingNoisePending = false;
  m_footstepPending = false;

  setKeepAlive(true);

  m_netGroup.addNetElement(&m_stateNetState);
  m_netGroup.addNetElement(&m_shiftingNetState);
  m_netGroup.addNetElement(&m_xAimPositionNetState);
  m_netGroup.addNetElement(&m_yAimPositionNetState);
  m_netGroup.addNetElement(&m_appearance.identityNetState());
  m_netGroup.addNetElement(&m_teamNetState);
  m_netGroup.addNetElement(&m_landedNetState);
  m_netGroup.addNetElement(&m_chatMessageNetState);
  m_netGroup.addNetElement(&m_newChatMessageNetState);
  m_netGroup.addNetElement(&m_emoteNetState);

  m_xAimPositionNetState.setFixedPointBase(0.003125);
  m_yAimPositionNetState.setFixedPointBase(0.003125);
  m_yAimPositionNetState.setInterpolator(lerp<float, float>);

  m_netGroup.addNetElement(m_inventory.get());
  m_netGroup.addNetElement(m_tools.get());
  m_netGroup.addNetElement(m_armor.get());
  m_netGroup.addNetElement(m_songbook.get());
  m_netGroup.addNetElement(m_movementController.get());
  m_netGroup.addNetElement(m_effectEmitter.get());
  m_netGroup.addNetElement(m_effectsAnimator.get());
  m_netGroup.addNetElement(m_statusController.get());
  m_netGroup.addNetElement(m_techController.get());

  m_appearance.netHumanoid().setCompatibilityVersion(10);
  m_netGroup.addNetElement(&m_appearance.netHumanoid());
  m_appearance.refreshedHumanoidParameters().setCompatibilityVersion(10);
  m_netGroup.addNetElement(&m_appearance.refreshedHumanoidParameters());

  m_appearance.scriptedAnimationParameters().setCompatibilityVersion(10);
  m_netGroup.addNetElement(&m_appearance.scriptedAnimationParameters());

  m_appearance.deathParticleBurst().setCompatibilityVersion(10);
  m_netGroup.addNetElement(&m_appearance.deathParticleBurst());

  m_appearance.humanoidDanceNetState().setCompatibilityVersion(13);
  m_netGroup.addNetElement(&m_appearance.humanoidDanceNetState());

  m_netGroup.setNeedsLoadCallback([this](bool initial) { return getNetStates(initial); });
  m_netGroup.setNeedsStoreCallback([this]() { return setNetStates(); });
}

Player::Player(PlayerConfigPtr config, ByteArray const& netStore, NetCompatibilityRules rules) : Player(config) {
  DataStreamBuffer ds(netStore);
  ds.setStreamCompatibilityVersion(rules);

  setUniqueId(ds.read<String>());

  ds.read(m_description);
  ds.read(m_modeType);
  ds.read(m_appearance.m_identity);
  if (rules.version() >= 10) {
    ds.read(m_appearance.m_humanoidParameters);
  }

  m_appearance.netHumanoid().clearNetElements();
  m_movementController->resetBaseParameters(ActorMovementParameters(jsonMerge(humanoid()->defaultMovementParameters(), humanoid()->playerMovementParameters().value(m_config->movementParameters))));
  m_appearance.deathParticleBurst().set(humanoid()->defaultDeathParticles());
}


Player::Player(PlayerConfigPtr config, Json const& diskStore) : Player(config) {
  diskLoad(diskStore);
}

void Player::diskLoad(Json const& diskStore) {
  setUniqueId(diskStore.getString("uuid"));
  m_description = diskStore.getString("description");
  setModeType(PlayerModeNames.getLeft(diskStore.getString("modeType")));
  m_shipUpgrades = ShipUpgrades(diskStore.get("shipUpgrades"));
  m_blueprints = make_shared<PlayerBlueprints>(diskStore.get("blueprints"));
  m_universeMap = make_shared<PlayerUniverseMap>(diskStore.get("universeMap"));
  if (m_clientContext)
    m_universeMap->setServerUuid(m_clientContext->serverUuid());

  m_codexes = make_shared<PlayerCodexes>(diskStore.get("codexes"));
  m_techs = make_shared<PlayerTech>(diskStore.get("techs"));
  m_appearance.m_identity = HumanoidIdentity(diskStore.get("identity"));
  m_appearance.identityUpdated() = true;

  setTeam(EntityDamageTeam(diskStore.get("team")));

  m_state = State::Idle;

  m_inventory->load(diskStore.get("inventory"));

  m_movementController->loadState(diskStore.get("movementController"));
  m_techController->diskLoad(diskStore.get("techController"));
  m_statusController->diskLoad(diskStore.get("statusController"));

  m_log = make_shared<PlayerLog>(diskStore.get("log"));

  auto speciesDatabase = Root::singleton().speciesDatabase();
  auto speciesDef = speciesDatabase->species(m_appearance.m_identity.species);
  m_shipSpecies = diskStore.getString("shipSpecies", m_appearance.m_identity.species);

  m_questManager->diskLoad(diskStore.get("quests", JsonObject{}));
  m_companions->diskLoad(diskStore.get("companions", JsonObject{}));
  m_deployment->diskLoad(diskStore.get("deployment", JsonObject{}));

  m_appearance.m_humanoidParameters = diskStore.getObject("humanoidParameters", JsonObject());

  m_appearance.netHumanoid().clearNetElements();
  m_movementController->resetBaseParameters(ActorMovementParameters(jsonMerge(humanoid()->defaultMovementParameters(), humanoid()->playerMovementParameters().value(m_config->movementParameters))));
  m_effectsAnimator->setGlobalTag("effectDirectives", speciesDef->effectDirectives());
  m_appearance.deathParticleBurst().set(humanoid()->defaultDeathParticles());

  m_genericProperties = diskStore.getObject("genericProperties");

  m_armor->reset();
  refreshArmor();
  setNetArmorSecrets(true);

  m_codexes->learnInitialCodexes(species());

  m_aiState = AiState(diskStore.get("aiState", JsonObject{}));

  for (auto& script : m_genericScriptContexts)
    script.second->setScriptStorage({});

  for (auto const& p : diskStore.get("genericScriptStorage", JsonObject{}).iterateObject()) {
    if (auto script = m_genericScriptContexts.maybe(p.first).value({})) {
      script->setScriptStorage(p.second.toObject());
    }
  }

  // Make sure to merge the stored player blueprints with what a new player
  // would get as default.
  for (auto const& descriptor : m_config->defaultBlueprints)
    m_blueprints->add(descriptor);
  for (auto const& descriptor : speciesDef->defaultBlueprints())
    m_blueprints->add(descriptor);

  if (m_appearance.m_identity.gender == Gender::Male && m_description == "This gal seems to have nothing to say for herself.")
    m_description = "This guy seems to have nothing to say for himself.";
  else if (m_appearance.m_identity.gender == Gender::Female && m_description == "This guy seems to have nothing to say for himself.")
    m_description = "This gal seems to have nothing to say for herself.";
}

ClientContextPtr Player::clientContext() const {
  return m_clientContext;
}

void Player::setClientContext(ClientContextPtr clientContext) {
  m_clientContext = std::move(clientContext);
  if (m_clientContext && m_universeMap)
    m_universeMap->setServerUuid(m_clientContext->serverUuid());
}

StatisticsPtr Player::statistics() const {
  return m_statistics;
}

void Player::setStatistics(StatisticsPtr statistics) {
  m_statistics = statistics;
}

void Player::setUniverseClient(UniverseClient* client) {
  m_client = client;
  m_questManager->setUniverseClient(client);
}

UniverseClient* Player::universeClient() const {
  return m_client;
}

EntityType Player::entityType() const {
  return EntityType::Player;
}

ClientEntityMode Player::clientEntityMode() const {
  return ClientEntityMode::ClientPresenceMaster;
}

void Player::init(World* world, EntityId entityId, EntityMode mode) {
  Entity::init(world, entityId, mode);


  m_movementController->init(world);
  m_movementController->setIgnorePhysicsEntities({entityId});
  m_statusController->init(this, m_movementController.get());
  auto speciesDefinition = Root::singleton().speciesDatabase()->species(m_appearance.m_identity.species);

  if (mode == EntityMode::Master) {
    m_appearance.scriptedAnimationParameters().clear();
    m_movementController->setRotation(0);
    m_statusController->setStatusProperty("ouchNoise", speciesDefinition->ouchNoise(m_appearance.m_identity.gender));

    m_questManager->init(world);
    m_companions->init(this, world);
    m_deployment->init(this, world);

    m_statusController->setPersistentEffects("species", speciesDefinition->statusEffects());

    for (auto& p : m_genericScriptContexts) {
      p.second->addActorMovementCallbacks(m_movementController.get());
      p.second->addCallbacks("player", LuaBindings::makePlayerCallbacks(this));
      p.second->addCallbacks("status", LuaBindings::makeStatusControllerCallbacks(m_statusController.get()));
      p.second->addCallbacks("songbook", LuaBindings::makeSongbookCallbacks(m_songbook.get()));
      p.second->addCallbacks("animator", LuaBindings::makeNetworkedAnimatorCallbacks(humanoid()->networkedAnimator()));
      if (m_client)
        p.second->addCallbacks("celestial", LuaBindings::makeCelestialCallbacks(m_client));
      p.second->init(world);
    }

    for (auto& p : m_inventory->pullOverflow()) {
      world->addEntity(ItemDrop::createRandomizedDrop(p, m_movementController->position(), true));
    }

    setNetArmorSecrets();
  }

  if (world->isClient()) {
      m_scriptedAnimator.setScripts(humanoid()->animationScripts());
      m_scriptedAnimator.addCallbacks("animationConfig", LuaBindings::makeScriptedAnimatorCallbacks(humanoid()->networkedAnimator(),
        [this](String const& name, Json const& defaultValue) -> Json {
          return m_appearance.scriptedAnimationParameters().value(name, defaultValue);
        }));
      m_scriptedAnimator.addCallbacks("entity", LuaBindings::makeEntityCallbacks(this));
      m_scriptedAnimator.init(world);
  }

  m_xAimPositionNetState.setInterpolator(world->geometry().xLerpFunction());
  refreshEquipment();
}

void Player::uninit() {
  m_techController->uninit();
  m_movementController->uninit();
  m_tools->uninit();
  m_statusController->uninit();

  if (isMaster()) {
    m_questManager->uninit();
    m_companions->uninit();
    m_deployment->uninit();

    for (auto& p : m_genericScriptContexts) {
      p.second->uninit();
      p.second->removeCallbacks("animator");
      p.second->removeCallbacks("entity");
      p.second->removeCallbacks("player");
      p.second->removeCallbacks("mcontroller");
      p.second->removeCallbacks("status");
      p.second->removeCallbacks("songbook");
      p.second->removeCallbacks("world");
      if (m_client)
        p.second->removeCallbacks("celestial");
    }
  }
  if (world()->isClient()) {
    m_scriptedAnimator.uninit();
    m_scriptedAnimator.removeCallbacks("animationConfig");
    m_scriptedAnimator.removeCallbacks("entity");
  }

  Entity::uninit();
}

List<Drawable> Player::drawables() const {
  List<Drawable> drawables;

  if (!isTeleporting()) {
    drawables.appendAll(m_techController->backDrawables());
    if (!m_techController->parentHidden()) {
      m_tools->setupHumanoidHandItemDrawables(*humanoid());

      // Auto-detect any ?scalenearest and apply them as a direct scale on the Humanoid's drawables instead.
      DirectivesGroup humanoidDirectives;
      Vec2F scale = Vec2F::filled(1.f);
      auto extractScale = [&](List<Directives> const& list) {
        for (auto& directives : list) {
          auto result = Humanoid::extractScaleFromDirectives(directives);
          scale = scale.piecewiseMultiply(result.first);
          humanoidDirectives.append(result.second);
        }
      };
      extractScale(m_techController->parentDirectives().list());
      extractScale(m_statusController->parentDirectives().list());
      humanoid()->setScale(scale);

      for (auto& drawable : humanoid()->render()) {
        drawable.translate(position() + m_techController->parentOffset());
        if (drawable.isImage()) {
          drawable.imagePart().addDirectivesGroup(humanoidDirectives, true);

          if (auto anchor = as<LoungeAnchor>(m_movementController->entityAnchor())) {
            if (auto& directives = anchor->directives)
              drawable.imagePart().addDirectives(*directives, true);
          }
        }
        drawables.append(std::move(drawable));
      }
    }
    drawables.appendAll(m_techController->frontDrawables());

    drawables.appendAll(m_statusController->drawables());

    drawables.appendAll(m_tools->renderObjectPreviews(aimPosition(), walkingDirection(), inToolRange(), favoriteColor()));
  }

  drawables.appendAll(m_effectsAnimator->drawables(position()));

  return drawables;
}

List<OverheadBar> Player::bars() const {
  return m_statusController->overheadBars();
}

List<Particle> Player::particles() {
  List<Particle> particles;
  particles.appendAll(m_config->splashConfig.doSplash(position(), m_movementController->velocity(), world()));
  particles.appendAll(take(m_callbackParticles));
  particles.appendAll(m_appearance.humanoidDynamicTarget().pullNewParticles());
  particles.appendAll(m_techController->pullNewParticles());
  particles.appendAll(m_statusController->pullNewParticles());

  return particles;
}

void Player::addParticles(List<Particle> const& particles) {
  m_callbackParticles.appendAll(particles);
}

void Player::addSound(String const& sound, float volume, float pitch) {
  m_callbackSounds.emplaceAppend(sound, volume, pitch);
}

void Player::addEphemeralStatusEffects(List<EphemeralStatusEffect> const& statusEffects) {
  if (isSlave())
    throw PlayerException("Adding status effects to an entity can only be done directly on the master entity.");
  m_statusController->addEphemeralEffects(statusEffects);
}

ActiveUniqueStatusEffectSummary Player::activeUniqueStatusEffectSummary() const {
  return m_statusController->activeUniqueStatusEffectSummary();
}

float Player::powerMultiplier() const {
  return m_statusController->stat("powerMultiplier");
}

bool Player::isDead() const {
  return !m_statusController->resourcePositive("health");
}

void Player::kill() {
  m_statusController->setResource("health", 0);
}

bool Player::wireToolInUse() const {
  return static_cast<bool>(as<WireTool>(m_tools->primaryHandItem()));
}

void Player::setWireConnector(WireConnector* wireConnector) const {
  if (auto wireTool = as<WireTool>(m_tools->primaryHandItem()))
    wireTool->setConnector(wireConnector);
}

List<Drawable> Player::portrait(PortraitMode mode) const {
  return m_appearance.portrait(mode);
}

bool Player::underwater() const {
  return m_appearance.underwater();
}

List<LightSource> Player::lightSources() const {
  List<LightSource> lights;
  lights.appendAll(m_tools->lightSources());
  lights.appendAll(m_statusController->lightSources());
  lights.appendAll(m_techController->lightSources());
  lights.appendAll(humanoid()->networkedAnimator()->lightSources());
  return lights;
}

RectF Player::metaBoundBox() const {
  return m_config->metaBoundBox;
}

Maybe<HitType> Player::queryHit(DamageSource const& source) const {
  return m_damagePipeline->queryHit(source);
}

Maybe<PolyF> Player::hitPoly() const {
  return m_damagePipeline->hitPoly();
}

List<DamageNotification> Player::applyDamage(DamageRequest const& request) {
  return m_damagePipeline->applyDamage(request);
}

List<DamageNotification> Player::selfDamageNotifications() {
  return m_damagePipeline->selfDamageNotifications();
}

void Player::hitOther(EntityId targetEntityId, DamageRequest const& damageRequest) {
  m_damagePipeline->hitOther(targetEntityId, damageRequest);
}

void Player::damagedOther(DamageNotification const& damage) {
  m_damagePipeline->damagedOther(damage);
}

List<DamageSource> Player::damageSources() const {
  return m_damagePipeline->damageSources();
}

bool Player::shouldDestroy() const {
  return isDead();
}

void Player::destroy(RenderCallback* renderCallback) {
  m_state = State::Idle;
    m_chatAndEmotes->setEmoteState(HumanoidEmote::Idle);
  if (renderCallback && m_appearance.deathParticleBurst().get())
    renderCallback->addParticles(humanoid()->particles(*m_appearance.deathParticleBurst().get()), position());

  if (isMaster()) {
    m_log->addDeathCount(1);

    if (!world()->disableDeathDrops()) {
      if (auto dropString = modeConfig().deathDropItemTypes.maybeLeft()) {
        if (*dropString == "all")
          dropEverything();
      } else {
        List<ItemType> dropList = modeConfig().deathDropItemTypes.right().transformed([](String typeName) {
            return ItemTypeNames.getLeft(typeName);
          });
        Set<ItemType> dropSet = Set<ItemType>::from(dropList);
        auto itemDb = Root::singleton().itemDatabase();
        dropSelectedItems([dropSet, itemDb](ItemPtr item) {
            return dropSet.contains(itemDb->itemType(item->name()));
          });
      }
    }
  }

  m_songbook->stop();
}

Maybe<EntityAnchorState> Player::loungingIn() const {
  if (is<LoungeAnchor>(m_movementController->entityAnchor()))
    return m_movementController->anchorState();
  return {};
}

bool Player::lounge(EntityId loungeableEntityId, size_t anchorIndex) {
  if (!canUseTool())
    return false;

  auto loungeableEntity = world()->get<LoungeableEntity>(loungeableEntityId);
  if (!loungeableEntity || anchorIndex >= loungeableEntity->anchorCount()
      || !loungeableEntity->entitiesLoungingIn(anchorIndex).empty()
      || !loungeableEntity->loungeAnchor(anchorIndex))
    return false;

  m_state = State::Lounge;
  m_movementController->setAnchorState({loungeableEntityId, anchorIndex});
  return true;
}

void Player::stopLounging() {
  if (loungingIn()) {
    m_movementController->resetAnchorState();
    m_state = State::Idle;
    m_statusController->setPersistentEffects("lounging", {});
  }
}

Vec2F Player::position() const {
  return m_movementController->position();
}

Vec2F Player::velocity() const {
  return m_movementController->velocity();
}

Vec2F Player::mouthOffset(bool ignoreAdjustments) const {
  return Vec2F(
      humanoid()->mouthOffset(ignoreAdjustments)[0] * numericalDirection(facingDirection()), humanoid()->mouthOffset(ignoreAdjustments)[1]);
}

Vec2F Player::feetOffset() const {
  return Vec2F(humanoid()->feetOffset()[0] * numericalDirection(facingDirection()), humanoid()->feetOffset()[1]);
}

Vec2F Player::headArmorOffset() const {
  return Vec2F(
      humanoid()->headArmorOffset()[0] * numericalDirection(facingDirection()), humanoid()->headArmorOffset()[1]);
}

Vec2F Player::chestArmorOffset() const {
  return Vec2F(
      humanoid()->chestArmorOffset()[0] * numericalDirection(facingDirection()), humanoid()->chestArmorOffset()[1]);
}

Vec2F Player::backArmorOffset() const {
  return Vec2F(
      humanoid()->backArmorOffset()[0] * numericalDirection(facingDirection()), humanoid()->backArmorOffset()[1]);
}

Vec2F Player::legsArmorOffset() const {
  return Vec2F(
      humanoid()->legsArmorOffset()[0] * numericalDirection(facingDirection()), humanoid()->legsArmorOffset()[1]);
}

Vec2F Player::mouthPosition() const {
  return position() + mouthOffset(true);
}

Vec2F Player::mouthPosition(bool ignoreAdjustments) const {
  return position() + mouthOffset(ignoreAdjustments);
}

RectF Player::collisionArea() const {
  return m_movementController->collisionPoly().boundBox();
}

void Player::revive(Vec2F const& footPosition) {
  m_teleporter->revive(footPosition);
}

bool Player::shifting() const {
  return m_shifting;
}

void Player::setShifting(bool shifting) {
  m_shifting = shifting;
}

void Player::special(int specialKey) {
  auto loungeAnchor = as<LoungeAnchor>(m_movementController->entityAnchor());
  if (loungeAnchor && loungeAnchor->controllable) {
    auto anchorState = m_movementController->anchorState();
    if (auto loungeableEntity = world()->get<LoungeableEntity>(anchorState->entityId)) {
      if (specialKey == 1)
        loungeableEntity->loungeControl(anchorState->positionIndex, LoungeControl::Special1);
      else if (specialKey == 2)
        loungeableEntity->loungeControl(anchorState->positionIndex, LoungeControl::Special2);
      else if (specialKey == 3)
        loungeableEntity->loungeControl(anchorState->positionIndex, LoungeControl::Special3);
      return;
    }
  }
  m_techController->special(specialKey);
}

void Player::setMoveVector(Vec2F const& vec) {
  m_moveVector = vec;
}

void Player::moveLeft() {
  m_pendingMoves.add(MoveControlType::Left);
}

void Player::moveRight() {
  m_pendingMoves.add(MoveControlType::Right);
}

void Player::moveUp() {
  m_pendingMoves.add(MoveControlType::Up);
}

void Player::moveDown() {
  m_pendingMoves.add(MoveControlType::Down);
}

void Player::jump() {
  m_pendingMoves.add(MoveControlType::Jump);
}

void Player::dropItem() {
  if (!world())
    return;
  if (!canUseTool())
    return;

  Vec2F throwDirection = world()->geometry().diff(aimPosition(), position());
  for (auto& throwSlot : {m_inventory->primaryHeldSlot(), m_inventory->secondaryHeldSlot()}) {
    if (throwSlot) {
      if (auto drop = m_inventory->takeSlot(*throwSlot)) {
        world()->addEntity(ItemDrop::throwDrop(drop, position(), velocity(), throwDirection));
        break;
      }
    }
  }
}

Maybe<Json> Player::receiveMessage(ConnectionId fromConnection, String const& message, JsonArray const& args) {
  bool localMessage = fromConnection == world()->connection();
  if (message == "queueRadioMessage" && args.size() > 0) {
    float delay = 0;
    if (args.size() > 1 && args.get(1).canConvert(Json::Type::Float))
      delay = args.get(1).toFloat();

    queueRadioMessage(args.get(0), delay);
  } else if (message == "warp") {
    Maybe<String> animation;
    if (args.size() > 1)
      animation = args.get(1).toString();

    bool deploy = false;
    if (args.size() > 2)
      deploy = args.get(2).toBool();

    setPendingWarp(args.get(0).toString(), animation, deploy);
  } else if (message == "interruptRadioMessage") {
    m_narrativeQueue->requestInterrupt();
  } else if (message == "playCinematic" && args.size() > 0) {
    bool unique = false;
    if (args.size() > 1)
      unique = args.get(1).toBool();
    setPendingCinematic(args.get(0), unique);
  } else if (message == "playAltMusic" && args.size() > 0) {
    float fadeTime = args.size() > 1 ? args.get(1).toFloat() : 0.f;
    int loops = args.size() > 2u ? args.get(2).toInt() : -1;
    StringList trackList;
    if (args.get(0).canConvert(Json::Type::Array))
      trackList = jsonToStringList(args.get(0).toArray());
    else
      trackList = StringList();
    m_narrativeQueue->setPendingAltMusic(make_pair(trackList, loops), fadeTime);
  } else if (message == "stopAltMusic") {
    float fadeTime = 0;
    if (args.size() > 0)
      fadeTime = args.get(0).toFloat();
    m_narrativeQueue->setPendingAltMusic({}, fadeTime);
  } else if (message == "recordEvent") {
    statistics()->recordEvent(args.at(0).toString(), args.at(1));
  } else if (message == "addCollectable") {
    auto collection = args.get(0).toString();
    auto collectable = args.get(1).toString();
    if (Root::singleton().collectionDatabase()->hasCollectable(collection, collectable))
      addCollectable(collection, collectable);
  } else {
    Maybe<Json> result = m_tools->receiveMessage(message, localMessage, args);
    if (!result)
      result = m_statusController->receiveMessage(message, localMessage, args);
    if (!result)
      result = m_companions->receiveMessage(message, localMessage, args);
    if (!result)
      result = m_deployment->receiveMessage(message, localMessage, args);
    if (!result)
      result = m_techController->receiveMessage(message, localMessage, args);
    if (!result)
      result = m_questManager->receiveMessage(message, localMessage, args);
    for (auto& p : m_genericScriptContexts) {
      if (result)
        break;
      result = p.second->handleMessage(message, localMessage, args);
    }
    return result;
  }

  return {};
}

void Player::update(float dt, uint64_t) {
  m_movementController->setTimestep(dt);

  if (isMaster()) {
    m_chatAndEmotes->tickChatAndEmotes(dt);
    m_chatAndEmotes->tickBlink(dt);

    m_damagePipeline->tick(dt);

    if (m_movementController->zeroG())
      m_movementController->controlParameters(m_zeroGMovementParameters);

    m_teleporter->tick(dt);

    if (!isTeleporting()) {
      processControls();

      m_questManager->update(dt);
      m_companions->update(dt);
      m_deployment->update(dt);

      bool edgeTriggeredUse = take(m_edgeTriggeredUse);

      m_inventory->cleanup();
      refreshEquipment();

      if (inConflictingLoungeAnchor())
        m_movementController->resetAnchorState();

      if (m_state == State::Lounge) {
        if (auto loungeAnchor = as<LoungeAnchor>(m_movementController->entityAnchor())) {
          m_statusController->setPersistentEffects("lounging", loungeAnchor->statusEffects);
          addEffectEmitters(loungeAnchor->effectEmitters);
          if (loungeAnchor->emote)
            requestEmote(*loungeAnchor->emote);

          auto itemDatabase = Root::singleton().itemDatabase();
          if (auto headOverride = loungeAnchor->armorCosmeticOverrides.maybe("head")) {
            auto overrideItem = itemDatabase->item(ItemDescriptor(*headOverride));
            if (m_inventory->itemAllowedAsEquipment(overrideItem, EquipmentSlot::HeadCosmetic))
              m_armor->setHeadCosmeticItem(as<HeadArmor>(overrideItem));
          }
          if (auto chestOverride = loungeAnchor->armorCosmeticOverrides.maybe("chest")) {
            auto overrideItem = itemDatabase->item(ItemDescriptor(*chestOverride));
            if (m_inventory->itemAllowedAsEquipment(overrideItem, EquipmentSlot::ChestCosmetic))
              m_armor->setChestCosmeticItem(as<ChestArmor>(overrideItem));
          }
          if (auto legsOverride = loungeAnchor->armorCosmeticOverrides.maybe("legs")) {
            auto overrideItem = itemDatabase->item(ItemDescriptor(*legsOverride));
            if (m_inventory->itemAllowedAsEquipment(overrideItem, EquipmentSlot::LegsCosmetic))
              m_armor->setLegsCosmeticItem(as<LegsArmor>(overrideItem));
          }
          if (auto backOverride = loungeAnchor->armorCosmeticOverrides.maybe("back")) {
            auto overrideItem = itemDatabase->item(ItemDescriptor(*backOverride));
            if (m_inventory->itemAllowedAsEquipment(overrideItem, EquipmentSlot::BackCosmetic))
              m_armor->setBackCosmeticItem(as<BackArmor>(overrideItem));
          }
        } else {
          m_state = State::Idle;
          m_movementController->resetAnchorState();
        }
      } else {
        m_movementController->resetAnchorState();
        m_statusController->setPersistentEffects("lounging", {});
      }

      if (!forceNude())
        m_armor->effects(*m_effectEmitter);

      m_tools->effects(*m_effectEmitter);

      auto aimRelative = world()->geometry().diff(m_aimPosition, position()); // dumb, but due to how things are ordered
      m_movementController->tickMaster(dt);
      m_aimPosition = position() + aimRelative;                               // it's gonna have to be like this for now

      m_techController->tickMaster(dt);

      for (auto& p : m_genericScriptContexts)
        p.second->update(p.second->updateDt(dt));

      if (edgeTriggeredUse) {
        auto anchor = as<LoungeAnchor>(m_movementController->entityAnchor());
        bool useTool = canUseTool();
        if (anchor && (!useTool || anchor->controllable))
          m_movementController->resetAnchorState();
        else if (useTool) {
          if (auto ie = bestInteractionEntity(true))
            interactWithEntity(ie);
        }
      }

      m_statusController->setPersistentEffects("armor", m_armor->statusEffects());
      m_statusController->setPersistentEffects("tools", m_tools->statusEffects());

      if (!m_techController->techOverridden())
        m_techController->setLoadedTech(m_techs->equippedTechs().values());

      if (!isDead())
        m_statusController->tickMaster(dt);

      if (!modeConfig().hunger)
        m_statusController->resetResource("food");

      if (!m_statusController->resourcePositive("food"))
        m_statusController->setPersistentEffects("hunger", m_foodEmptyStatusEffects);
      else if (m_statusController->resourcePercentage("food").value() <= m_foodLowThreshold)
        m_statusController->setPersistentEffects("hunger", m_foodLowStatusEffects);
      else
        m_statusController->setPersistentEffects("hunger", {});

      m_narrativeQueue->tickDelayedRadio(dt);
    }

    if (m_isAdmin) {
      m_statusController->resetResource("health");
      m_statusController->resetResource("energy");
      m_statusController->resetResource("food");
      m_statusController->resetResource("breath");
    }

    m_log->addPlayTime(GlobalTimestep);

    if (m_ageItemsTimer.wrapTick(dt)) {
      auto itemDatabase = Root::singleton().itemDatabase();
      m_inventory->forEveryItem([&](InventorySlot const&, ItemPtr& item) {
          itemDatabase->ageItem(item, m_ageItemsTimer.time);
        });
    }

    for (auto& tool : {m_tools->primaryHandItem(), m_tools->altHandItem()}) {
      if (auto inspectionTool = as<InspectionTool>(tool)) {
        for (auto& ir : inspectionTool->pullInspectionResults()) {
          if (ir.objectName) {
            m_questManager->receiveMessage("objectScanned", true, {*ir.objectName, *ir.entityId});
            m_log->addScannedObject(*ir.objectName);
          }

          addChatMessage(ir.message, JsonObject{
            {"message", JsonObject{
              {"context", JsonObject{{"mode", "RadioMessage"}}},
              {"fromConnection", world()->connection()},
              {"text", ir.message}
            }}
          });
        }
      }
    }

    m_interestingObjects = m_questManager->interestingObjects();

  } else {
    m_netGroup.tickNetInterpolation(dt);
    m_movementController->tickSlave(dt);
    m_techController->tickSlave(dt);
    m_statusController->tickSlave(dt);
  }

  humanoid()->setRotation(m_movementController->rotation());

  bool suppressedItems = !canUseTool();

  auto loungeAnchor = as<LoungeAnchor>(m_movementController->entityAnchor());
  if (loungeAnchor && loungeAnchor->dance){
    humanoid()->setDance(*loungeAnchor->dance);
  } else if (m_chatAndEmotes->dance()) {
    humanoid()->setDance(m_chatAndEmotes->dance());
  } else if ((!suppressedItems && (m_tools->primaryHandItem() || m_tools->altHandItem()))
    || humanoid()->danceCyclicOrEnded() || m_movementController->running()) {
    humanoid()->setDance({});
  }
  bool isClient = world()->isClient();

  m_tools->suppressItems(suppressedItems);
  m_tools->tick(dt, m_shifting, m_pendingMoves);

  Direction facingDirection = m_movementController->facingDirection();

  auto overrideFacingDirection = m_tools->setupHumanoidHandItems(*humanoid(), position(), aimPosition());
  if (!loungingIn() && overrideFacingDirection)
    m_movementController->controlFace(facingDirection = *overrideFacingDirection);

  humanoid()->setFacingDirection(facingDirection);
  humanoid()->setMovingBackwards(facingDirection != m_movementController->movingDirection());

  refreshHumanoid();

  auto scale = Mat3F::scaling(Vec2F(facingDirection == Direction::Right ? 1.f : -1.f, 1.f));
  m_effectsAnimator->setTransformationGroup("flip", scale);

  if (m_state == State::Walk || m_state == State::Run) {
    if ((m_footstepTimer += dt) > m_config->footstepTiming) {
      m_footstepPending = true;
      m_footstepTimer = 0.0;
    }
  }

  if (isClient) {
    m_effectsAnimator->update(dt, &m_effectsAnimatorDynamicTarget);
    m_effectsAnimatorDynamicTarget.updatePosition(position() + m_techController->parentOffset());
  } else {
    m_effectsAnimator->update(dt, nullptr);
  }

  if (!isTeleporting())
    processStateChanges(dt);

  m_damagePipeline->tickBuildSources();

  m_songbook->update(*entityMode(), world());

  m_effectEmitter->setSourcePosition("normal", position());
  m_effectEmitter->setSourcePosition("mouth", mouthOffset() + position());
  m_effectEmitter->setSourcePosition("feet", feetOffset() + position());
  m_effectEmitter->setSourcePosition("headArmor", headArmorOffset() + position());
  m_effectEmitter->setSourcePosition("chestArmor", chestArmorOffset() + position());
  m_effectEmitter->setSourcePosition("legsArmor", legsArmorOffset() + position());
  m_effectEmitter->setSourcePosition("backArmor", backArmorOffset() + position());

  m_effectEmitter->setSourcePosition("primary", handPosition(ToolHand::Primary) + position());
  m_effectEmitter->setSourcePosition("alt", handPosition(ToolHand::Alt) + position());

  m_effectEmitter->setDirection(facingDirection);

  m_effectEmitter->tick(dt, *entityMode());

  if (isClient) {
    bool calculateHeadRotation = isMaster();
    if (!calculateHeadRotation) {
      auto headRotationProperty = getSecretProperty("humanoid.headRotation");
      if (headRotationProperty.isType(Json::Type::Float)) {
        humanoid()->setHeadRotation(headRotationProperty.toFloat());
      } else
        calculateHeadRotation = true;
    }
    if (calculateHeadRotation) { // master or not an OpenStarbound player
      float headRotation = 0.f;
      if (Humanoid::globalHeadRotation() && (humanoid()->handHoldingItem(ToolHand::Primary) || humanoid()->handHoldingItem(ToolHand::Alt) || humanoid()->dance())) {
        auto primary = m_tools->primaryHandItem();
        auto alt = m_tools->altHandItem();
        String const disableFlag = "disableHeadRotation";
        auto statusFlag = m_statusController->statusProperty(disableFlag);
        if (!(statusFlag.isType(Json::Type::Bool) && statusFlag.toBool())
         && !(primary && primary->instanceValue(disableFlag))
         && !(alt && alt->instanceValue(disableFlag))) {
          auto diff = world()->geometry().diff(aimPosition(), mouthPosition());
          diff.setX(fabsf(diff.x()));
          headRotation = diff.angle() * .25f * numericalDirection(humanoid()->facingDirection());
        }
      }
      humanoid()->setHeadRotation(headRotation);
      if (isMaster())
        setSecretProperty("humanoid.headRotation", headRotation);
    }
  }
  
  if (isMaster()) {
    for (auto& p : m_genericScriptContexts)
      p.second->invoke("postUpdate");
  }

  m_pendingMoves.clear();

  if (isClient)
    SpatialLogger::logPoly("world", m_movementController->collisionBody(), isMaster() ? Color::Orange.toRgba() : Color::Yellow.toRgba());
}

float Player::timeSinceLastGaveDamage() const {
  return m_damagePipeline->timeSinceLastGaveDamage();
}

EntityId Player::lastDamagedTarget() const {
  return m_damagePipeline->lastDamagedTarget();
}

void Player::render(RenderCallback* renderCallback) {
  if (invisible()) {
    m_techController->pullNewAudios();
    m_techController->pullNewParticles();
    m_statusController->pullNewAudios();
    m_statusController->pullNewParticles();

    m_appearance.humanoidDynamicTarget().pullNewAudios();
    m_appearance.humanoidDynamicTarget().pullNewParticles();
    return;
  }

  Vec2I footstepSensor = Vec2I((m_config->footstepSensor + m_movementController->position()).floor());
  String footstepSound = getFootstepSound(footstepSensor);

  if (!footstepSound.empty() && !m_techController->parentState() && !m_techController->parentHidden()) {
    auto footstepAudio = Root::singleton().assets()->audio(footstepSound);
    if (m_landingNoisePending) {
      auto landingNoise = make_shared<AudioInstance>(*footstepAudio);
      landingNoise->setPosition(position() + feetOffset());
      landingNoise->setVolume(m_landingVolume);
      renderCallback->addAudio(std::move(landingNoise));
    }

    if (m_footstepPending) {
      auto stepNoise = make_shared<AudioInstance>(*footstepAudio);
      stepNoise->setPosition(position() + feetOffset());
      stepNoise->setVolume(1 - Random::randf(0, m_footstepVolumeVariance));
      renderCallback->addAudio(std::move(stepNoise));
    }
  } else {
    m_footstepTimer = m_config->footstepTiming;
  }
  m_footstepPending = false;
  m_landingNoisePending = false;

  renderCallback->addAudios(m_effectsAnimatorDynamicTarget.pullNewAudios());
  renderCallback->addParticles(m_effectsAnimatorDynamicTarget.pullNewParticles());

  renderCallback->addAudios(m_techController->pullNewAudios());
  renderCallback->addAudios(m_statusController->pullNewAudios());
  renderCallback->addAudios(m_appearance.humanoidDynamicTarget().pullNewAudios());

  for (auto const& p : take(m_callbackSounds)) {
    auto audio = make_shared<AudioInstance>(*Root::singleton().assets()->audio(get<0>(p)));
    audio->setVolume(get<1>(p));
    audio->setPitchMultiplier(get<2>(p));
    audio->setPosition(position());
    renderCallback->addAudio(std::move(audio));
  }

  auto loungeAnchor = as<LoungeAnchor>(m_movementController->entityAnchor());
  EntityRenderLayer renderLayer = loungeAnchor ? loungeAnchor->loungeRenderLayer : RenderLayerPlayer;

  renderCallback->addDrawables(drawables(), renderLayer);
  if (!isTeleporting())
    renderCallback->addOverheadBars(bars(), position());
  renderCallback->addParticles(particles());

  m_tools->render(renderCallback, inToolRange(), m_shifting, renderLayer);

  m_effectEmitter->render(renderCallback);
  m_songbook->render(renderCallback);

  if (isMaster())
    m_deployment->render(renderCallback, position());
}

void Player::renderLightSources(RenderCallback* renderCallback) {
  renderCallback->addLightSources(lightSources());
  m_deployment->renderLightSources(renderCallback);
}

Json Player::getGenericProperty(String const& name, Json const& defaultValue) const {
  return m_genericProperties.value(name, defaultValue);
}

void Player::setGenericProperty(String const& name, Json const& value) {
  if (value.isNull())
    m_genericProperties.erase(name);
  else
    m_genericProperties.set(name, value);
}

PlayerInventoryPtr Player::inventory() const {
  return m_inventory;
}

uint64_t Player::itemsCanHold(ItemPtr const& items) const {
  return m_inventory->itemsCanFit(items);
}

ItemPtr Player::pickupItems(ItemPtr const& items, bool silent) {
  if (isDead() || !items || m_inventory->itemsCanFit(items) == 0)
    return items;

  triggerPickupEvents(items);

  if (!silent) {
    if (items->pickupSound().size()) {
      m_effectsAnimator->setSoundPool("pickup", {items->pickupSound()});
      float pitch = 1.f - (static_cast<float>(items->count()) / static_cast<float>(items->maxStack())) * 0.5f;
      m_effectsAnimator->setSoundPitchMultiplier("pickup", clamp(pitch * Random::randf(0.8f, 1.2f), 0.f, 2.f));
      m_effectsAnimator->playSound("pickup");
    }
    auto itemDb = Root::singleton().itemDatabase();
    queueItemPickupMessage(itemDb->itemShared(items->descriptor()));
  }

  return m_inventory->addItems(items);
}

void Player::giveItem(ItemPtr const& item) {
  if (auto spill = pickupItems(item))
    world()->addEntity(ItemDrop::createRandomizedDrop(spill->descriptor(), position()));
}

void Player::triggerPickupEvents(ItemPtr const& item) {
  if (item) {
    for (auto const& b : item->learnBlueprintsOnPickup())
      addBlueprint(b);

    for (auto const& pair : item->collectablesOnPickup())
      addCollectable(pair.first, pair.second);

    for (auto m : item->instanceValue("radioMessagesOnPickup", JsonArray()).iterateArray()) {
      if (m.isType(Json::Type::Array)) {
        if (m.size() >= 2 && m.get(1).canConvert(Json::Type::Float))
          queueRadioMessage(m.get(0), m.get(1).toFloat());
      } else {
        queueRadioMessage(m);
      }
    }

    if (auto cinematic = item->instanceValue("cinematicOnPickup", Json()))
      setPendingCinematic(cinematic, true);

    for (auto const& quest : item->pickupQuestTemplates()) {
      if (m_questManager->canStart(quest))
        m_questManager->offer(make_shared<Quest>(quest, 0, this));
    }

    if (auto consume = item->instanceValue("consumeOnPickup", Json())) {
      if (consume.toBool())
        item->consume(item->count());
    }

    statistics()->recordEvent("item", JsonObject{
        {"itemName", item->name()},
        {"count", item->count()},
        {"category", item->instanceValue("eventCategory", item->category())}
      });
  }
}

ItemPtr Player::essentialItem(EssentialItem essentialItem) const {
  return m_inventory->essentialItem(essentialItem);
}

bool Player::hasItem(ItemDescriptor const& descriptor, bool exactMatch) const {
  return m_inventory->hasItem(descriptor, exactMatch);
}

uint64_t Player::hasCountOfItem(ItemDescriptor const& descriptor, bool exactMatch) const {
  return m_inventory->hasCountOfItem(descriptor, exactMatch);
}

ItemDescriptor Player::takeItem(ItemDescriptor const& descriptor, bool consumePartial, bool exactMatch) {
  return m_inventory->takeItems(descriptor, consumePartial, exactMatch);
}

void Player::giveItem(ItemDescriptor const& descriptor) {
  giveItem(Root::singleton().itemDatabase()->item(descriptor));
}

void Player::clearSwap() {
  // If we cannot put the swap slot back into the bag, then just drop it in the
  // world.
  if (!m_inventory->clearSwap()) {
    if (auto world = worldPtr())
      world->addEntity(ItemDrop::createRandomizedDrop(m_inventory->takeSlot(SwapSlot()), position()));
  }

  // Interrupt all firing in case the item being dropped was in use.
  endPrimaryFire();
  endAltFire();
  endTrigger();
}

void Player::refreshItems() {
  if (isSlave())
    return;

  m_tools->setItems(m_inventory->primaryHeldItem(), m_inventory->secondaryHeldItem());
}

void Player::refreshArmor() {
  if (isSlave())
    return;

  bool shouldSetArmorSecrets = m_clientContext && m_clientContext->netCompatibilityRules().version() < 9;
  for (uint8_t i = 0; i != 20; ++i) {
    auto slot = static_cast<EquipmentSlot>(i);
    auto item = m_inventory->equipment(slot);
    bool visible = m_inventory->equipmentVisibility(slot);
    if (m_armor->setItem(i, item, visible)) {
      if (slot >= EquipmentSlot::Cosmetic1 && shouldSetArmorSecrets)
        setNetArmorSecret(slot, item, visible);
    }
  }
}

void Player::refreshHumanoid() const {
  try {
    if (m_armor->setupHumanoid(*humanoid(), forceNude())) {
      m_movementController->resetBaseParameters(ActorMovementParameters(jsonMerge(humanoid()->defaultMovementParameters(), humanoid()->playerMovementParameters().value(m_config->movementParameters))));
    }
  }
  catch (std::exception const&) {
    if (isMaster()) // it's your problem,
      throw;        // deal with it!
  }
}

void Player::refreshEquipment() {
  refreshArmor();
  refreshItems();
}

PlayerBlueprintsPtr Player::blueprints() const {
  return m_blueprints;
}

bool Player::addBlueprint(ItemDescriptor const& descriptor, bool showFailure) {
  if (descriptor.isNull())
    return false;

  auto itemDb = Root::singleton().itemDatabase();
  auto item = itemDb->item(descriptor);
  auto assets = Root::singleton().assets();
  if (!m_blueprints->isKnown(descriptor)) {
    m_blueprints->add(descriptor);
    queueUIMessage(assets->json("/player.config:blueprintUnlock").toString().replace("<ItemName>", item->friendlyName()));
    return true;
  } else if (showFailure) {
    queueUIMessage(assets->json("/player.config:blueprintAlreadyKnown").toString().replace("<ItemName>", item->friendlyName()));
  }

  return false;
}

bool Player::blueprintKnown(ItemDescriptor const& descriptor) const {
  if (descriptor.isNull())
    return false;

  return m_blueprints->isKnown(descriptor);
}

bool Player::addCollectable(String const& collectionName, String const& collectableName) {
  if (m_log->addCollectable(collectionName, collectableName)) {
    auto collectionDatabase = Root::singleton().collectionDatabase();

    auto collection = collectionDatabase->collection(collectionName);
    auto collectable = collectionDatabase->collectable(collectionName, collectableName);
    queueUIMessage(Root::singleton().assets()->json("/player.config:collectableUnlock").toString().replace("<collectable>", collectable.title).replace("<collection>", collection.title));
    return true;
  } else {
    return false;
  }
}

PlayerUniverseMapPtr Player::universeMap() const {
  return m_universeMap;
}

PlayerCodexesPtr Player::codexes() const {
  return m_codexes;
}

PlayerTechPtr Player::techs() const {
  return m_techs;
}

void Player::overrideTech(Maybe<StringList> const& techModules) {
  if (techModules)
    m_techController->setOverrideTech(*techModules);
  else
    m_techController->clearOverrideTech();
}

bool Player::techOverridden() const {
  return m_techController->techOverridden();
}

PlayerCompanionsPtr Player::companions() const {
  return m_companions;
}

PlayerLogPtr Player::log() const {
  return m_log;
}

InteractiveEntityPtr Player::bestInteractionEntity(bool includeNearby) {
  if (!inWorld())
    return {};

  InteractiveEntityPtr interactiveEntity;
  if (auto entity = world()->getInteractiveInRange(m_aimPosition, isAdmin() ? m_aimPosition : position(), m_interactRadius)) {
    interactiveEntity = entity;
  } else if (includeNearby) {
    Vec2F interactBias = m_walkIntoInteractBias;
    if (facingDirection() == Direction::Left)
      interactBias[0] *= -1;
    Vec2F pos = position() + interactBias;

    if (auto entity = world()->getInteractiveInRange(pos, position(), m_interactRadius))
      interactiveEntity = entity;
  }

  if (interactiveEntity && (isAdmin() || world()->canReachEntity(position(), interactRadius(), interactiveEntity->entityId())))
    return interactiveEntity;
  return {};
}

void Player::interactWithEntity(InteractiveEntityPtr entity) {
  bool questIntercepted = false;
  for (auto const& quest : m_questManager->listActiveQuests()) {
    if (quest->interactWithEntity(entity->entityId()))
      questIntercepted = true;
  }
  if (questIntercepted)
    return;

  bool anyTurnedIn = false;

  for (auto questId : entity->turnInQuests()) {
    if (m_questManager->canTurnIn(questId)) {
      auto const& quest = m_questManager->getQuest(questId);
      quest->setEntityParameter("questReceiver", entity);
      m_questManager->getQuest(questId)->complete();
      anyTurnedIn = true;
    }
  }

  if (anyTurnedIn)
    return;

  for (auto const& questArc : entity->offeredQuests()) {
    if (m_questManager->canStart(questArc)) {
      auto quest = make_shared<Quest>(questArc, 0, this);
      quest->setWorldId(clientContext()->playerWorldId());
      quest->setServerUuid(clientContext()->serverUuid());
      quest->setEntityParameter("questGiver", entity);
      m_questManager->offer(quest);
      return;
    }
  }

  m_pendingInteractActions.append(world()->interact(InteractRequest{
        entityId(), position(), entity->entityId(), aimPosition()}));
}

void Player::aim(Vec2F const& position) {
  m_techController->setAimPosition(position);
  m_aimPosition = position;
}

Vec2F Player::aimPosition() const {
  return m_aimPosition;
}

Vec2F Player::armPosition(ToolHand hand, Direction facingDirection, float armAngle, Vec2F offset) const {
  return m_tools->armPosition(*humanoid(), hand, facingDirection, armAngle, offset);
}

Vec2F Player::handOffset(ToolHand hand, Direction facingDirection) const {
  return m_tools->handOffset(*humanoid(), hand, facingDirection);
}

Vec2F Player::handPosition(ToolHand hand, Vec2F const& handOffset) const {
  return m_tools->handPosition(hand, *humanoid(), handOffset);
}

ItemPtr Player::handItem(ToolHand hand) const {
  if (hand == ToolHand::Primary)
    return m_tools->primaryHandItem();
  else
    return m_tools->altHandItem();
}

Vec2F Player::armAdjustment() const {
  return humanoid()->armAdjustment();
}

void Player::setCameraFocusEntity(Maybe<EntityId> const& cameraFocusEntity) {
  m_cameraFocusEntity = cameraFocusEntity;
}

void Player::playEmote(HumanoidEmote emote) {
  m_chatAndEmotes->playEmote(emote);
}

bool Player::canUseTool() const {
  bool canUse = !isDead() && !isTeleporting() && !m_techController->toolUsageSuppressed() && !m_statusController->toolUsageSuppressed();
  if (canUse) {
    if (auto loungeAnchor = as<LoungeAnchor>(m_movementController->entityAnchor()))
      if (loungeAnchor->suppressTools.value(loungeAnchor->controllable))
        return false;
  }
  return canUse;
}

void Player::beginPrimaryFire() {
  m_techController->beginPrimaryFire();
  m_tools->beginPrimaryFire();
}

void Player::beginAltFire() {
  m_techController->beginAltFire();
  m_tools->beginAltFire();
}

void Player::endPrimaryFire() {
  m_techController->endPrimaryFire();
  m_tools->endPrimaryFire();
}

void Player::endAltFire() {
  m_techController->endAltFire();
  m_tools->endAltFire();
}

void Player::beginTrigger() {
  if (!m_useDown)
    m_edgeTriggeredUse = true;
  m_useDown = true;
}

void Player::endTrigger() {
  m_useDown = false;
}

float Player::toolRadius() const {
  auto radius = m_tools->toolRadius();
  if (radius)
    return *radius;
  return interactRadius();
}

float Player::interactRadius() const {
  return m_interactRadius;
}

void Player::setInteractRadius(float interactRadius) {
  m_interactRadius = interactRadius;
}

List<InteractAction> Player::pullInteractActions() {
  List<InteractAction> results;
  eraseWhere(m_pendingInteractActions, [&results](auto& promise) {
      if (auto res = promise.result())
        results.append(res.take());
      return promise.finished();
    });
  return results;
}

uint64_t Player::currency(String const& currencyType) const {
  return m_inventory->currency(currencyType);
}

float Player::health() const {
  return m_statusController->resource("health");
}

float Player::maxHealth() const {
  return *m_statusController->resourceMax("health");
}

DamageBarType Player::damageBar() const {
  return DamageBarType::Default;
}

float Player::healthPercentage() const {
  return *m_statusController->resourcePercentage("health");
}

float Player::energy() const {
  return m_statusController->resource("energy");
}

float Player::maxEnergy() const {
  return *m_statusController->resourceMax("energy");
}

float Player::energyPercentage() const {
  return *m_statusController->resourcePercentage("energy");
}

float Player::energyRegenBlockPercent() const {
  return *m_statusController->resourcePercentage("energyRegenBlock");
}

bool Player::fullEnergy() const {
  return energy() >= maxEnergy();
}

bool Player::energyLocked() const {
  return m_statusController->resourceLocked("energy");
}

bool Player::consumeEnergy(float energy) {
  if (m_isAdmin)
    return true;
  return m_statusController->overConsumeResource("energy", energy);
}

float Player::foodPercentage() const {
  return *m_statusController->resourcePercentage("food");
}

float Player::breath() const {
  return m_statusController->resource("breath");
}

float Player::maxBreath() const {
  return *m_statusController->resourceMax("breath");
}

float Player::protection() const {
  return m_statusController->stat("protection");
}

bool Player::forceNude() const {
  return m_statusController->statPositive("nude");
}

String Player::description() const {
  return m_description;
}

void Player::setDescription(String const& description) {
  m_description = description;
}

Direction Player::walkingDirection() const {
  return m_movementController->movingDirection();
}

Direction Player::facingDirection() const {
  return m_movementController->facingDirection();
}

pair<ByteArray, uint64_t> Player::writeNetState(uint64_t fromVersion, NetCompatibilityRules rules) {
  return m_netGroup.writeNetState(fromVersion, rules);
}

void Player::readNetState(ByteArray data, float interpolationTime, NetCompatibilityRules rules) {
  m_netGroup.readNetState(data, interpolationTime, rules);
}

void Player::enableInterpolation(float) {
  m_netGroup.enableNetInterpolation();
}

void Player::disableInterpolation() {
  m_netGroup.disableNetInterpolation();
}

void Player::processControls() {
  bool run = !m_shifting && !m_statusController->statPositive("encumberance");

  bool useMoveVector = m_moveVector.x() != 0.0f;
  if (useMoveVector) {
    for (auto move : m_pendingMoves) {
      if (move == MoveControlType::Left || move == MoveControlType::Right) {
        useMoveVector = false;
        break;
      }
    }
  }

  if (useMoveVector) {
    m_pendingMoves.insert(m_moveVector.x() < 0.0f ? MoveControlType::Left : MoveControlType::Right);
    m_movementController->setMoveSpeedMultiplier(clamp(abs(m_moveVector.x()), 0.0f, 1.0f));
  }
  else
    m_movementController->setMoveSpeedMultiplier(1.0f);

  if (auto fireableMain = as<FireableItem>(m_tools->primaryHandItem())) {
    if (fireableMain->inUse() && fireableMain->walkWhileFiring())
      run = false;
  }

  if (auto fireableAlt = as<FireableItem>(m_tools->altHandItem())) {
    if (fireableAlt->inUse() && fireableAlt->walkWhileFiring())
      run = false;
  }

  bool move = true;

  if (auto fireableMain = as<FireableItem>(m_tools->primaryHandItem())) {
    if (fireableMain->inUse() && fireableMain->stopWhileFiring())
      move = false;
  }

  if (auto fireableAlt = as<FireableItem>(m_tools->altHandItem())) {
    if (fireableAlt->inUse() && fireableAlt->stopWhileFiring())
      move = false;
  }

  auto loungeAnchor = as<LoungeAnchor>(m_movementController->entityAnchor());
  if (loungeAnchor && loungeAnchor->controllable) {
    auto anchorState = m_movementController->anchorState();
    if (auto loungeableEntity = world()->get<LoungeableEntity>(anchorState->entityId)) {
      for (auto move : m_pendingMoves) {
        if (move == MoveControlType::Up)
          loungeableEntity->loungeControl(anchorState->positionIndex, LoungeControl::Up);
        else if (move == MoveControlType::Down)
          loungeableEntity->loungeControl(anchorState->positionIndex, LoungeControl::Down);
        else if (move == MoveControlType::Left)
          loungeableEntity->loungeControl(anchorState->positionIndex, LoungeControl::Left);
        else if (move == MoveControlType::Right)
          loungeableEntity->loungeControl(anchorState->positionIndex, LoungeControl::Right);
        else if (move == MoveControlType::Jump)
          loungeableEntity->loungeControl(anchorState->positionIndex, LoungeControl::Jump);
      }
      if (m_tools->firingPrimary())
        loungeableEntity->loungeControl(anchorState->positionIndex, LoungeControl::PrimaryFire);
      if (m_tools->firingAlt())
        loungeableEntity->loungeControl(anchorState->positionIndex, LoungeControl::AltFire);
      if (m_shifting)
        loungeableEntity->loungeControl(anchorState->positionIndex, LoungeControl::Walk);
      loungeableEntity->loungeAim(anchorState->positionIndex, m_aimPosition);
    }
    move = false;
  }

  m_techController->setShouldRun(run);

  if (move) {
    for (auto move : m_pendingMoves) {
      switch (move) {
        case MoveControlType::Right:
          m_techController->moveRight();
          break;
        case MoveControlType::Left:
          m_techController->moveLeft();
          break;
        case MoveControlType::Up:
          m_techController->moveUp();
          break;
        case MoveControlType::Down:
          m_techController->moveDown();
          break;
        case MoveControlType::Jump:
          m_techController->jump();
          break;
      }
    }
  }

  if (m_state == State::Lounge && !m_pendingMoves.empty() && move)
    stopLounging();
}

void Player::processStateChanges(float dt) {
  if (isMaster()) {

    // Set the current player state based on what movement controller tells us
    // we're doing and do some state transition logic
    State oldState = m_state;

    if (m_movementController->zeroG()) {
      if (m_movementController->flying())
        m_state = State::Swim;
      else if (m_state != State::Lounge)
        m_state = State::SwimIdle;
    } else if (m_movementController->groundMovement()) {
      if (m_movementController->running()) {
        m_state = State::Run;
      } else if (m_movementController->walking()) {
        m_state = State::Walk;
      } else if (m_movementController->crouching()) {
        m_state = State::Crouch;
      } else {
        if (m_state != State::Lounge)
          m_state = State::Idle;
      }
    } else if (m_movementController->liquidMovement()) {
      if (m_movementController->jumping()) {
        m_state = State::Swim;
      } else {
        if (m_state != State::Lounge)
          m_state = State::SwimIdle;
      }
    } else {
      if (m_movementController->jumping()) {
        m_state = State::Jump;
      } else {
        if (m_movementController->falling()) {
          m_state = State::Fall;
        }
        if (m_movementController->velocity()[1] > 0) {
          if (m_state != State::Lounge)
            m_state = State::Jump;
        }
      }
    }

    if (m_moveVector.x() != 0.0f && (m_state == State::Run))
        m_state = abs(m_moveVector.x()) > 0.5f ? State::Run : State::Walk;

    if (m_state == State::Jump && (oldState == State::Idle || oldState == State::Run || oldState == State::Walk || oldState == State::Crouch))
      m_effectsAnimator->burstParticleEmitter("jump");

    if (!m_movementController->isNullColliding()) {
      if (oldState == State::Fall && oldState != m_state && m_state != State::Swim && m_state != State::SwimIdle
          && m_state != State::Jump) {
        m_effectsAnimator->burstParticleEmitter("landing");
        m_landedNetState.trigger();
        m_landingNoisePending = true;
      }
    }
  }
  if (world()->isClient()) {
    humanoid()->animate(dt, &m_appearance.humanoidDynamicTarget());
    m_appearance.humanoidDynamicTarget().updatePosition(position() + (m_techController->parentOffset()));
  } else {
    humanoid()->animate(dt, {});
  }
  m_scriptedAnimator.update();

  if (auto techState = m_techController->parentState()) {
    if (techState == TechController::ParentState::Stand) {
      humanoid()->setState(Humanoid::Idle);
    } else if (techState == TechController::ParentState::Fly) {
      humanoid()->setState(Humanoid::Jump);
    } else if (techState == TechController::ParentState::Fall) {
      humanoid()->setState(Humanoid::Fall);
    } else if (techState == TechController::ParentState::Sit) {
      humanoid()->setState(Humanoid::Sit);
    } else if (techState == TechController::ParentState::Lay) {
      humanoid()->setState(Humanoid::Lay);
    } else if (techState == TechController::ParentState::Duck) {
      humanoid()->setState(Humanoid::Duck);
    } else if (techState == TechController::ParentState::Walk) {
      humanoid()->setState(Humanoid::Walk);
    } else if (techState == TechController::ParentState::Run) {
      humanoid()->setState(Humanoid::Run);
    } else if (techState == TechController::ParentState::Swim) {
      humanoid()->setState(Humanoid::Swim);
    } else if (techState == TechController::ParentState::SwimIdle) {
      humanoid()->setState(Humanoid::SwimIdle);
    }
  } else {
    auto loungeAnchor = as<LoungeAnchor>(m_movementController->entityAnchor());
    if (m_state == State::Idle) {
      humanoid()->setState(Humanoid::Idle);
    } else if (m_state == State::Walk) {
      humanoid()->setState(Humanoid::Walk);
    } else if (m_state == State::Run) {
      humanoid()->setState(Humanoid::Run);
    } else if (m_state == State::Jump) {
      humanoid()->setState(Humanoid::Jump);
    } else if (m_state == State::Fall) {
      humanoid()->setState(Humanoid::Fall);
    } else if (m_state == State::Swim) {
      humanoid()->setState(Humanoid::Swim);
    } else if (m_state == State::SwimIdle) {
      humanoid()->setState(Humanoid::SwimIdle);
    } else if (m_state == State::Crouch) {
      humanoid()->setState(Humanoid::Duck);
    } else if (m_state == State::Lounge && loungeAnchor && loungeAnchor->orientation == LoungeOrientation::Sit) {
      humanoid()->setState(Humanoid::Sit);
    } else if (m_state == State::Lounge && loungeAnchor && loungeAnchor->orientation == LoungeOrientation::Lay) {
      humanoid()->setState(Humanoid::Lay);
    } else if (m_state == State::Lounge && loungeAnchor && loungeAnchor->orientation == LoungeOrientation::Stand) {
      humanoid()->setState(Humanoid::Idle);
    }
  }

  humanoid()->setEmoteState(m_chatAndEmotes->emoteState());
}

String Player::getFootstepSound(Vec2I const& sensor) const {
  auto materialDatabase = Root::singleton().materialDatabase();

  String fallback = materialDatabase->defaultFootstepSound();
  List<Vec2I> scanOrder{{0, 0}, {0, -1}, {-1, 0}, {1, 0}, {-1, -1}, {1, -1}};
  for (auto const& subSensor : scanOrder) {
    String footstepSound = materialDatabase->footstepSound(world()->material(sensor + subSensor, TileLayer::Foreground),
        world()->mod(sensor + subSensor, TileLayer::Foreground));
    if (!footstepSound.empty()) {
      if (footstepSound != fallback) {
        return footstepSound;
      }
    }
  }
  return fallback;
}

bool Player::inInteractionRange() const {
  return inInteractionRange(centerOfTile(aimPosition()));
}

bool Player::inInteractionRange(Vec2F aimPos) const {
  return isAdmin() || world()->geometry().diff(aimPos, position()).magnitude() < interactRadius();
}

bool Player::inToolRange() const {
  return inToolRange(centerOfTile(aimPosition()));
}

bool Player::inToolRange(Vec2F const& aimPos) const {
  return isAdmin() || world()->geometry().diff(aimPos, position()).magnitude() < toolRadius();
}

void Player::getNetStates(bool initial) {
  m_state = static_cast<State>(m_stateNetState.get());
  m_shifting = m_shiftingNetState.get();
  m_aimPosition[0] = m_xAimPositionNetState.get();
  m_aimPosition[1] = m_yAimPositionNetState.get();

  if (m_appearance.identityNetState().pullUpdated() && !initial) {
    auto newIdentity = m_appearance.identityNetState().get();
    if ((m_appearance.m_identity.species == newIdentity.species) && (m_appearance.m_identity.imagePath == newIdentity.imagePath)) {
      humanoid()->setIdentity(newIdentity);
    }
    m_appearance.m_identity = newIdentity;
  }
  if (m_appearance.refreshedHumanoidParameters().pullOccurred() && !initial) {
    refreshHumanoidParameters();
  }

  setTeam(m_teamNetState.get());

  if (m_landedNetState.pullOccurred() && !initial)
    m_landingNoisePending = true;

  if (m_newChatMessageNetState.pullOccurred() && !initial) {
    m_chatAndEmotes->setChatMessage(m_chatMessageNetState.get());
  }

  m_chatAndEmotes->setDance(m_appearance.humanoidDanceNetState().get());

  getNetArmorSecrets();
}

void Player::setNetStates() {
  m_stateNetState.set(static_cast<unsigned>(m_state));
  m_shiftingNetState.set(m_shifting);
  m_xAimPositionNetState.set(m_aimPosition[0]);
  m_yAimPositionNetState.set(m_aimPosition[1]);

  if (m_appearance.identityUpdated()) {
    m_appearance.identityNetState().push(m_appearance.m_identity);
    m_appearance.identityUpdated() = false;
  }

  m_teamNetState.set(getTeam());

  if (m_chatAndEmotes->chatMessageChanged()) {
    m_chatAndEmotes->clearChatMessageChanged();
    m_chatMessageNetState.push(m_chatAndEmotes->chatMessage());
    m_newChatMessageNetState.trigger();
  }

  m_emoteNetState.set(HumanoidEmoteNames.getRight(m_chatAndEmotes->emoteState()));
  m_appearance.humanoidDanceNetState().set(m_chatAndEmotes->dance());
}

void Player::setNetArmorSecret(EquipmentSlot slot, ArmorItemPtr const& armor, bool visible) {
  String const& slotName = EquipmentSlotNames.getRight(slot);
  ItemDescriptor descriptor = visible ? itemSafeDescriptor(armor) : ItemDescriptor();
  setSecretProperty(strf("armorWearer.{}.data", slotName), descriptor.diskStore());
  if (m_armorSecretNetVersions.empty())
    setSecretProperty("armorWearer.replicating", true);
  setSecretProperty(strf("armorWearer.{}.version", slotName), ++m_armorSecretNetVersions[slot]);
}

void Player::setNetArmorSecrets(bool includeEmpty) {
  if (m_clientContext && m_clientContext->netCompatibilityRules().version() < 9) {
    for (uint8_t i = 0; i != 12; ++i) {
      auto slot = static_cast<EquipmentSlot>(static_cast<uint8_t>(EquipmentSlot::Cosmetic1) + i);
      auto item = as<ArmorItem>(m_inventory->itemsAt(slot));
      bool visible = m_inventory->equipmentVisibility(slot);
      if ((item && visible) || includeEmpty)
        setNetArmorSecret(slot, item, visible);
    }
  }
}

void Player::getNetArmorSecrets() {
  if (isSlave() && getSecretPropertyPtr("armorWearer.replicating") != nullptr) {
    auto itemDatabase = Root::singleton().itemDatabase();

    for (uint8_t i = 0; i != 12; ++i) {
      auto slot = static_cast<EquipmentSlot>(static_cast<uint8_t>(EquipmentSlot::Cosmetic1) + i);
      String const& slotName = EquipmentSlotNames.getRight(slot);
      auto& curVersion = m_armorSecretNetVersions[slot];
      uint64_t newVersion = 0;
      if (auto jVersion = getSecretProperty(strf("armorWearer.{}.version", slotName), 0); jVersion.isType(Json::Type::Int))
        newVersion = jVersion.toUInt();
      if (newVersion > curVersion) {
        curVersion = newVersion;
        ArmorItemPtr item;
        itemDatabase->diskLoad(getSecretProperty(strf("armorWearer.{}.data", slotName)), item);
        m_inventory->setItem(slot, item);
        m_armor->setCosmeticItem(i, item);
      }
    }
  }
}

void Player::setAdmin(bool isAdmin) {
  m_isAdmin = isAdmin;
}

bool Player::isAdmin() const {
  return m_isAdmin;
}

void Player::setFavoriteColor(Color color) {
  m_appearance.setFavoriteColor(color);
}

Color Player::favoriteColor() const {
  return m_appearance.favoriteColor();
}

bool Player::isTeleporting() const {
  return m_teleporter->isTeleporting();
}

bool Player::isTeleportingOut() const {
  return m_teleporter->isTeleportingOut();
}

bool Player::canDeploy() {
  return m_teleporter->canDeploy();
}

void Player::deployAbort(String const& animationType) {
  m_teleporter->deployAbort(animationType);
}

bool Player::isDeploying() const {
  return m_teleporter->isDeploying();
}

bool Player::isDeployed() const {
  return m_teleporter->isDeployed();
}

void Player::setBusyState(PlayerBusyState busyState) {
  m_teleporter->setBusyState(busyState);
}

void Player::teleportOut(String const& animationType, bool deploy) {
  m_teleporter->teleportOut(animationType, deploy);
}

void Player::teleportIn() {
  m_teleporter->teleportIn();
}

void Player::teleportAbort() {
  m_teleporter->teleportAbort();
}

void Player::moveTo(Vec2F const& footPosition) {
  m_teleporter->moveTo(footPosition);
}

ItemPtr Player::primaryHandItem() const {
  return m_tools->primaryHandItem();
}

ItemPtr Player::altHandItem() const {
  return m_tools->altHandItem();
}

Uuid Player::uuid() const {
  return Uuid(*uniqueId());
}

PlayerMode Player::modeType() const {
  return m_modeType;
}

void Player::setModeType(PlayerMode mode) {
  m_modeType = mode;

  auto assets = Root::singleton().assets();
  m_modeConfig = PlayerModeConfig(assets->json("/playermodes.config").get(PlayerModeNames.getRight(mode)));
}

PlayerModeConfig Player::modeConfig() const {
  return m_modeConfig;
}

ShipUpgrades Player::shipUpgrades() {
  return m_shipUpgrades;
}

void Player::setShipUpgrades(ShipUpgrades shipUpgrades) {
  m_shipUpgrades = std::move(shipUpgrades);
}

void Player::applyShipUpgrades(Json const& upgrades) {
  if (m_clientContext->playerUuid() == uuid())
    m_clientContext->rpcInterface()->invokeRemote("ship.applyShipUpgrades", upgrades);
  else
    m_shipUpgrades.apply(upgrades);
}

void Player::setShipSpecies(String species) {
  m_shipSpecies = std::move(species);
}

String Player::shipSpecies() const {
  return m_shipSpecies;
}

String Player::name() const {
  return m_appearance.name();
}

void Player::setName(String const& name) {
  m_appearance.setName(name);
}

Maybe<String> Player::statusText() const {
  return m_appearance.statusText();
}

bool Player::displayNametag() const {
  return m_appearance.displayNametag();
}

Vec3B Player::nametagColor() const {
  return m_appearance.nametagColor();
}

Vec2F Player::nametagOrigin() const {
  return m_appearance.nametagOrigin();
}

String Player::nametag() const {
  return m_appearance.nametag();
}

void Player::setNametag(Maybe<String> nametag) {
  m_appearance.setNametag(std::move(nametag));
}

void Player::updateIdentity() {
  m_appearance.updateIdentity();
}

void Player::setHumanoidParameter(String key, Maybe<Json> value) {
  m_appearance.setHumanoidParameter(key, value);
}

Maybe<Json> Player::getHumanoidParameter(String key) {
  return m_appearance.getHumanoidParameter(key);
}

void Player::setHumanoidParameters(JsonObject parameters) {
  m_appearance.setHumanoidParameters(parameters);
}

JsonObject Player::getHumanoidParameters() {
  return m_appearance.getHumanoidParameters();
}

void Player::setBodyDirectives(String const& directives) {
  m_appearance.setBodyDirectives(directives);
}

void Player::setEmoteDirectives(String const& directives) {
  m_appearance.setEmoteDirectives(directives);
}

void Player::setHairGroup(String const& group) {
  m_appearance.setHairGroup(group);
}

void Player::setHairType(String const& type) {
  m_appearance.setHairType(type);
}

void Player::setHairDirectives(String const& directives) {
  m_appearance.setHairDirectives(directives);
}

void Player::setFacialHairGroup(String const& group) {
  m_appearance.setFacialHairGroup(group);
}

void Player::setFacialHairType(String const& type) {
  m_appearance.setFacialHairType(type);
}

void Player::setFacialHairDirectives(String const& directives) {
  m_appearance.setFacialHairDirectives(directives);
}

void Player::setFacialMaskGroup(String const& group) {
  m_appearance.setFacialMaskGroup(group);
}

void Player::setFacialMaskType(String const& type) {
  m_appearance.setFacialMaskType(type);
}

void Player::setFacialMaskDirectives(String const& directives) {
  m_appearance.setFacialMaskDirectives(directives);
}

void Player::setHair(String const& group, String const& type, String const& directives) {
  m_appearance.setHair(group, type, directives);
}

void Player::setFacialHair(String const& group, String const& type, String const& directives) {
  m_appearance.setFacialHair(group, type, directives);
}

void Player::setFacialMask(String const& group, String const& type, String const& directives) {
  m_appearance.setFacialMask(group, type, directives);
}

void Player::setSpecies(String const& species) {
  m_appearance.setSpecies(species);
}

Gender Player::gender() const {
  return m_appearance.gender();
}

void Player::setGender(Gender const& gender) {
  m_appearance.setGender(gender);
}

String Player::species() const {
  return m_appearance.species();
}

void Player::setPersonality(Personality const& personality) {
  m_appearance.setPersonality(personality);
}

void Player::setImagePath(Maybe<String> const& imagePath) {
  m_appearance.setImagePath(imagePath);
}

HumanoidPtr Player::humanoid() {
  return m_appearance.humanoid();
}
HumanoidPtr Player::humanoid() const {
  return m_appearance.humanoid();
}

HumanoidIdentity const& Player::identity() const {
  return m_appearance.identity();
}

void Player::setIdentity(HumanoidIdentity identity) {
  m_appearance.setIdentity(std::move(identity));
}

List<String> Player::pullQueuedMessages() {
  return take(m_queuedMessages);
}

List<ItemPtr> Player::pullQueuedItemDrops() {
  return take(m_queuedItemPickups);
}

void Player::queueUIMessage(String const& message) {
  if (!isSlave())
    m_queuedMessages.append(message);
}

void Player::queueItemPickupMessage(ItemPtr const& item) {
  if (!isSlave())
    m_queuedItemPickups.append(item);
}

void Player::addChatMessage(String const& message, Json const& config) {
  m_chatAndEmotes->addChatMessage(message, config);
}

void Player::addEmote(HumanoidEmote const& emote, Maybe<float> emoteCooldown) {
  m_chatAndEmotes->addEmote(emote, emoteCooldown);
}
void Player::setDance(Maybe<String> const& danceName) {
  m_chatAndEmotes->setDance(danceName);
}

pair<HumanoidEmote, float> Player::currentEmote() const {
  return m_chatAndEmotes->currentEmote();
}

Player::State Player::currentState() const {
  return m_state;
}

List<ChatAction> Player::pullPendingChatActions() {
  return m_chatAndEmotes->pullPendingChatActions();
}

Maybe<String> Player::inspectionLogName() const {
  return m_appearance.inspectionLogName();
}

Maybe<String> Player::inspectionDescription(String const& species) const {
  return m_appearance.inspectionDescription(species);
}

float Player::beamGunRadius() const {
  return m_tools->beamGunRadius();
}

bool Player::instrumentPlaying() {
  return m_songbook->instrumentPlaying();
}

void Player::instrumentEquipped(String const& instrumentKind) {
  if (canUseTool())
    m_songbook->keepAlive(instrumentKind, mouthPosition());
}

void Player::interact(InteractAction const& action) {
  starAssert(!isSlave());
  m_pendingInteractActions.append(RpcPromise<InteractAction>::createFulfilled(action));
}

void Player::addEffectEmitters(StringSet const& emitters) {
  starAssert(!isSlave());
  m_effectEmitter->addEffectSources("normal", emitters);
}

void Player::requestEmote(String const& emote) {
  m_chatAndEmotes->requestEmote(emote);
}

ActorMovementController* Player::movementController() {
  return m_movementController.get();
}

StatusController* Player::statusController() {
  return m_statusController.get();
}

List<PhysicsForceRegion> Player::forceRegions() const {
  return m_tools->forceRegions();
}


StatusControllerPtr Player::statusControllerPtr() {
  return m_statusController;
}

ActorMovementControllerPtr Player::movementControllerPtr() {
  return m_movementController;
}

PlayerConfigPtr Player::config() {
  return m_config;
}

SongbookPtr Player::songbook() const {
  return m_songbook;
}

QuestManagerPtr Player::questManager() const {
  return m_questManager;
}

Json Player::diskStore() {
  JsonObject genericScriptStorage;
  for (auto& p : m_genericScriptContexts) {
    auto scriptStorage = p.second->getScriptStorage();
    if (!scriptStorage.empty())
      genericScriptStorage[p.first] = std::move(scriptStorage);
  }

  return JsonObject{
    {"uuid", *uniqueId()},
    {"description", m_description},
    {"modeType", PlayerModeNames.getRight(m_modeType)},
    {"shipUpgrades", m_shipUpgrades.toJson()},
    {"shipSpecies", !m_shipSpecies.empty() ? m_shipSpecies : m_appearance.m_identity.species},
    {"blueprints", m_blueprints->toJson()},
    {"universeMap", m_universeMap->toJson()},
    {"codexes", m_codexes->toJson()},
    {"techs", m_techs->toJson()},
    {"identity", m_appearance.m_identity.toJson()},
    {"team", getTeam().toJson()},
    {"inventory", m_inventory->store()},
    {"movementController", m_movementController->storeState()},
    {"techController", m_techController->diskStore()},
    {"statusController", m_statusController->diskStore()},
    {"log", m_log->toJson()},
    {"aiState", m_aiState.toJson()},
    {"quests", m_questManager->diskStore()},
    {"companions", m_companions->diskStore()},
    {"deployment", m_deployment->diskStore()},
    {"genericProperties", m_genericProperties},
    {"genericScriptStorage", genericScriptStorage},
    {"humanoidParameters", m_appearance.m_humanoidParameters},
  };
}

ByteArray Player::netStore(NetCompatibilityRules rules) {
  DataStreamBuffer ds;
  ds.setStreamCompatibilityVersion(rules);

  ds.write(*uniqueId());
  ds.write(m_description);
  ds.write(m_modeType);
  ds.write(m_appearance.m_identity);
  if (rules.version() >= 10)
    ds.write(m_appearance.m_humanoidParameters);

  return ds.data();
}

void Player::finalizeCreation() {
  m_blueprints = make_shared<PlayerBlueprints>();
  m_techs = make_shared<PlayerTech>();

  auto itemDatabase = Root::singleton().itemDatabase();
  for (auto const& descriptor : m_config->defaultItems)
    m_inventory->addItems(itemDatabase->item(descriptor));

  for (auto const& descriptor : Root::singleton().speciesDatabase()->species(m_appearance.m_identity.species)->defaultItems())
    m_inventory->addItems(itemDatabase->item(descriptor));

  for (auto const& descriptor : m_config->defaultBlueprints)
    m_blueprints->add(descriptor);

  for (auto const& descriptor : Root::singleton().speciesDatabase()->species(m_appearance.m_identity.species)->defaultBlueprints())
    m_blueprints->add(descriptor);

  refreshEquipment();

  m_state = State::Idle;
    m_chatAndEmotes->setEmoteState(HumanoidEmote::Idle);

  m_statusController->setPersistentEffects("armor", m_armor->statusEffects());
  m_statusController->setPersistentEffects("tools", m_tools->statusEffects());
  m_statusController->resetAllResources();

  m_effectEmitter->reset();

  m_description = strf("This {} seems to have nothing to say for {}self.",
    m_appearance.m_identity.gender == Gender::Male ? "guy" : "gal",
    m_appearance.m_identity.gender == Gender::Male ? "him" : "her");
}

bool Player::invisible() const {
  return m_statusController->statPositive("invisible");
}

void Player::animatePortrait(float dt) {
  m_appearance.animatePortrait(dt);
}

bool Player::isOutside() {
  if (!inWorld())
    return false;
  return !world()->isUnderground(position())
      && !world()->tileIsOccupied(Vec2I::floor(mouthPosition()), TileLayer::Background);
}

void Player::dropSelectedItems(function<bool(ItemPtr)> filter) {
  if (!world())
    return;

  m_inventory->forEveryItem([&](InventorySlot const&, ItemPtr& item) {
      if (item && (!filter || filter(item)))
        world()->addEntity(ItemDrop::throwDrop(take(item), position(), velocity(), Vec2F::withAngle(Random::randf(-Constants::pi, Constants::pi)), true));
    });
}

void Player::dropEverything() {
  dropSelectedItems({});
}

bool Player::isPermaDead() const {
  if (!isDead())
    return false;
  return modeConfig().permadeath;
}

bool Player::interruptRadioMessage() {
  return m_narrativeQueue->interruptRadioMessage();
}

Maybe<RadioMessage> Player::pullPendingRadioMessage() {
  return m_narrativeQueue->pullPendingRadioMessage();
}

void Player::queueRadioMessage(Json const& messageConfig, float delay) {
  m_narrativeQueue->queueRadioMessage(messageConfig, delay);
}

void Player::queueRadioMessage(RadioMessage message) {
  m_narrativeQueue->queueRadioMessage(message);
}

Maybe<Json> Player::pullPendingCinematic() {
  return m_narrativeQueue->pullPendingCinematic();
}

void Player::setPendingCinematic(Json const& cinematic, bool unique) {
  m_narrativeQueue->setPendingCinematic(cinematic, unique);
}

void Player::setInCinematic(bool inCinematic) {
  m_narrativeQueue->setInCinematic(inCinematic);
}

Maybe<pair<Maybe<pair<StringList, int>>, float>> Player::pullPendingAltMusic() {
  return m_narrativeQueue->pullPendingAltMusic();
}

Maybe<PlayerWarpRequest> Player::pullPendingWarp() {
  return m_narrativeQueue->pullPendingWarp();
}

void Player::setPendingWarp(String const& action, Maybe<String> const& animation, bool deploy) {
  m_narrativeQueue->setPendingWarp(action, animation, deploy);
}

Maybe<pair<Json, RpcPromiseKeeper<Json>>> Player::pullPendingConfirmation() {
  return m_narrativeQueue->pullPendingConfirmation();
}

void Player::queueConfirmation(Json const& dialogConfig, RpcPromiseKeeper<Json> const& resultPromise) {
  m_narrativeQueue->queueConfirmation(dialogConfig, resultPromise);
}

AiState const& Player::aiState() const {
  return m_aiState;
}

AiState& Player::aiState() {
  return m_aiState;
}

bool Player::inspecting() const {
  return is<InspectionTool>(m_tools->primaryHandItem()) || is<InspectionTool>(m_tools->altHandItem());
}

EntityHighlightEffect Player::inspectionHighlight(InspectableEntityPtr const& inspectableEntity) const {
  auto inspectionTool = as<InspectionTool>(m_tools->primaryHandItem());
  if (!inspectionTool)
    inspectionTool = as<InspectionTool>(m_tools->altHandItem());

  if (!inspectionTool)
    return EntityHighlightEffect();

  if (auto name = inspectableEntity->inspectionLogName()) {
    auto ehe = EntityHighlightEffect();
    ehe.level = inspectionTool->inspectionHighlightLevel(inspectableEntity);
    if (ehe.level > 0) {
      if (m_interestingObjects.contains(*name))
        ehe.type = EntityHighlightEffectType::Interesting;
      else if (m_log->scannedObjects().contains(*name))
        ehe.type = EntityHighlightEffectType::Inspected;
      else
        ehe.type = EntityHighlightEffectType::Inspectable;
    }
    return ehe;
  }

  return EntityHighlightEffect();
}

Vec2F Player::cameraPosition() {
  if (inWorld()) {
    if (auto loungeAnchor = as<LoungeAnchor>(m_movementController->entityAnchor())) {
      if (loungeAnchor->cameraFocus) {
        if (auto anchoredEntity = world()->entity(m_movementController->anchorState()->entityId))
          return anchoredEntity->position();
      }
    }

    if (m_cameraFocusEntity) {
      if (auto focusedEntity = world()->entity(*m_cameraFocusEntity))
        return focusedEntity->position();
      else
        m_cameraFocusEntity = {};
    }
  }
  return position();
}

NetworkedAnimatorPtr Player::effectsAnimator() {
  return m_effectsAnimator;
}

const String secretProprefix = "\0JsonProperty\0"s;

Maybe<StringView> Player::getSecretPropertyView(String const& name) const {
  if (auto tag = m_effectsAnimator->globalTagPtr(secretProprefix + name)) {
    auto& view = tag->utf8();
    DataStreamExternalBuffer buffer(view.data(), view.size());
    try {
      uint8_t typeIndex = buffer.read<uint8_t>() - 1;
      if (static_cast<Json::Type>(typeIndex) == Json::Type::String) {
        size_t len = buffer.readVlqU();
        size_t pos = buffer.pos();
        if (pos + len == buffer.size())
          return StringView(buffer.ptr() + pos, len);
      }
    }
    catch (StarException const& e) {}
  }

  return {};
}

String const* Player::getSecretPropertyPtr(String const& name) const {
  return m_effectsAnimator->globalTagPtr(secretProprefix + name);
}

Json Player::getSecretProperty(String const& name, Json defaultValue) const {
  if (auto tag = m_effectsAnimator->globalTagPtr(secretProprefix + name)) {
    DataStreamExternalBuffer buffer(tag->utf8Ptr(), tag->utf8Size());
    try
      { return buffer.read<Json>(); }
    catch (StarException const& e)
      { Logger::error("Exception reading secret player property '{}': {}", name, e.what()); }
  }

  return defaultValue;
}

void Player::setSecretProperty(String const& name, Json const& value) {
  if (value) {
    DataStreamBuffer ds;
    ds.write(value);
    auto& data = ds.data();
    m_effectsAnimator->setGlobalTag(secretProprefix + name, String(data.ptr(), data.size()));
  }
  else
    m_effectsAnimator->removeGlobalTag(secretProprefix + name);
}

void Player::refreshHumanoidParameters() {
  m_appearance.refreshHumanoidParameters();
}

void Player::setAnimationParameter(String name, Json value) {
  m_appearance.setAnimationParameter(name, value);
}

}
