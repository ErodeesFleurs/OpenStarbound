#include "StarItemDrop.hpp"

#include "StarCasting.hpp"
#include "StarConfig.hpp"
#include "StarConfigLuaBindings.hpp"
#include "StarEntityLuaBindings.hpp"
#include "StarEntityRendering.hpp"
#include "StarItemDatabase.hpp"
#include "StarItemLuaBindings.hpp"
#include "StarJsonExtra.hpp"
#include "StarMaterialItem.hpp"
#include "StarMovementControllerLuaBindings.hpp"
#include "StarPlayer.hpp"
#include "StarRandom.hpp"
#include "StarRoot.hpp"
#include "StarWorld.hpp"

import std;

namespace Star {

auto ItemDrop::createRandomizedDrop(Ptr<Item> const& item, Vec2F const& position, bool eternal) -> Ptr<ItemDrop> {
  if (!item)
    return {};

  auto idconfig = Root::singleton().assets()->json("/itemdrop.config");

  Ptr<ItemDrop> itemDrop = std::make_shared<ItemDrop>(item);
  auto offset = Vec2F(idconfig.getFloat("randomizedDistance"), 0).rotate(Constants::pi * 2.0 * Random::randf());
  offset[1] = std::fabs(offset[1]);
  itemDrop->setPosition(position + offset / TilePixels);
  itemDrop->setVelocity(offset * idconfig.getFloat("randomizedSpeed"));
  itemDrop->setEternal(eternal);

  return itemDrop;
}

auto ItemDrop::createRandomizedDrop(ItemDescriptor const& descriptor, Vec2F const& position, bool eternal) -> Ptr<ItemDrop> {
  if (!descriptor || descriptor.isEmpty())
    return {};

  ConstPtr<ItemDatabase> itemDatabase = Root::singleton().itemDatabase();
  auto itemDrop = createRandomizedDrop(itemDatabase->item(descriptor), position);
  itemDrop->setEternal(eternal);

  return itemDrop;
}

auto ItemDrop::throwDrop(Ptr<Item> const& item, Vec2F const& position, Vec2F const& velocity, Vec2F const& direction, bool eternal) -> Ptr<ItemDrop> {
  if (!item)
    return {};

  auto idconfig = Root::singleton().assets()->json("/itemdrop.config");

  Ptr<ItemDrop> itemDrop = make_shared<ItemDrop>(item);
  itemDrop->setPosition(position);
  if (direction != Vec2F())
    itemDrop->setVelocity(velocity + vnorm(direction) * idconfig.getFloat("throwSpeed"));

  itemDrop->setEternal(eternal);
  itemDrop->setIntangibleTime(idconfig.getFloat("throwIntangibleTime"));

  return itemDrop;
}

auto ItemDrop::throwDrop(ItemDescriptor const& itemDescriptor, Vec2F const& position, Vec2F const& velocity, Vec2F const& direction, bool eternal) -> Ptr<ItemDrop> {
  if (!itemDescriptor || itemDescriptor.isEmpty())
    return {};

  auto itemDatabase = Root::singleton().itemDatabase();
  auto itemDrop = throwDrop(itemDatabase->item(itemDescriptor), position, velocity, direction);
  itemDrop->setEternal(eternal);

  return itemDrop;
}

ItemDrop::ItemDrop(Ptr<Item> item)
    : ItemDrop() {
  m_item = std::move(item);

  m_parameters = m_item->instanceValueOfType("itemDrop", Json::Type::Object, JsonObject{});

  updateCollisionPoly();

  m_owningEntity.set(NullEntityId);
  m_mode.set(Mode::Available);
  m_itemDescriptor.set(m_item->descriptor());
  m_clientEntityMode = ClientEntityModeNames.getLeft(configValue("clientEntityMode", "ClientSlaveOnly").toString());
}

ItemDrop::ItemDrop(Json const& diskStore)
    : ItemDrop() {
  Root::singleton().itemDatabase()->diskLoad(diskStore.get("item"), m_item);
  m_parameters = m_item->instanceValueOfType("itemDrop", Json::Type::Object, JsonObject{});
  m_movementController.setPosition(jsonToVec2F(diskStore.get("position")));
  m_mode.set(ModeNames.getLeft(diskStore.getString("mode")));
  m_eternal = diskStore.getBool("eternal");
  m_dropAge = EpochTimer(diskStore.get("dropAge"));
  m_ageItemsTimer = EpochTimer(diskStore.get("ageItemsTimer"));

  updateCollisionPoly();
  m_owningEntity.set(NullEntityId);
  m_itemDescriptor.set(m_item->descriptor());
  m_clientEntityMode = ClientEntityModeNames.getLeft(configValue("clientEntityMode", "ClientSlaveOnly").toString());
}

ItemDrop::ItemDrop(ByteArray store, NetCompatibilityRules rules) : ItemDrop() {
  DataStreamBuffer ds(std::move(store));
  ds.setStreamCompatibilityVersion(rules);

  Root::singleton().itemDatabase()->loadItem(ds.read<ItemDescriptor>(), m_item);
  m_parameters = m_item->instanceValueOfType("itemDrop", Json::Type::Object, JsonObject{});
  ds.read(m_eternal);
  ds.read(m_dropAge);
  ds.read(m_intangibleTimer);

  updateCollisionPoly();
  m_itemDescriptor.set(m_item->descriptor());
  m_clientEntityMode = ClientEntityModeNames.getLeft(configValue("clientEntityMode", "ClientSlaveOnly").toString());
}

auto ItemDrop::diskStore() const -> Json {
  auto itemDatabase = Root::singleton().itemDatabase();
  return JsonObject{
    {"item", itemDatabase->diskStore(m_item)},
    {"position", jsonFromVec2F(m_movementController.position())},
    {"mode", ModeNames.getRight(m_mode.get())},
    {"eternal", m_eternal},
    {"dropAge", m_dropAge.toJson()},
    {"ageItemsTimer", m_ageItemsTimer.toJson()}};
}

auto ItemDrop::netStore(NetCompatibilityRules rules) const -> ByteArray {
  DataStreamBuffer ds;
  ds.setStreamCompatibilityVersion(rules);

  ds.write(itemSafeDescriptor(m_item));
  ds.write(m_eternal);
  ds.write(m_dropAge);
  ds.write(m_intangibleTimer);

  return ds.takeData();
}

auto ItemDrop::entityType() const -> EntityType {
  return EntityType::ItemDrop;
}

void ItemDrop::init(World* world, EntityId entityId, EntityMode mode) {
  Entity::init(world, entityId, mode);

  m_movementController.init(world);
  if (isMaster()) {
    auto scripts = configValue("scripts").optArray().transform(jsonToStringList);
    if (scripts && !(*scripts).empty()) {
      m_scriptComponent.setScripts(*scripts);
      m_scriptComponent.setUpdateDelta(configValue("scriptDelta", 1).toUInt());

      m_scriptComponent.addCallbacks("itemDrop", makeItemDropCallbacks());
      m_scriptComponent.addCallbacks("item", LuaBindings::makeItemCallbacks(m_item.get()));
      m_scriptComponent.addCallbacks("config", LuaBindings::makeConfigCallbacks([this](auto&& PH1, auto&& PH2) -> auto { return configValue(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); }));
      m_scriptComponent.addCallbacks("entity", LuaBindings::makeEntityCallbacks(this));
      m_scriptComponent.addCallbacks("mcontroller", LuaBindings::makeMovementControllerCallbacks(&m_movementController));
      m_scriptComponent.init(world);
    }
  }
}

auto ItemDrop::clientEntityMode() const -> ClientEntityMode {
  return m_clientEntityMode;
}

void ItemDrop::uninit() {
  Entity::uninit();
  m_movementController.uninit();
  if (isMaster()) {
    auto scripts = configValue("scripts").optArray().transform(jsonToStringList);
    if (scripts && !(*scripts).empty()) {
      m_scriptComponent.uninit();
      m_scriptComponent.removeCallbacks("itemDrop");
      m_scriptComponent.removeCallbacks("item");
      m_scriptComponent.removeCallbacks("config");
      m_scriptComponent.removeCallbacks("entity");
      m_scriptComponent.removeCallbacks("mcontroller");
    }
  }
}

auto ItemDrop::name() const -> String {
  if (m_item)
    return m_item->name();
  return Entity::name();
}

auto ItemDrop::description() const -> String {
  return m_item->description();
}

auto ItemDrop::writeNetState(std::uint64_t fromVersion, NetCompatibilityRules rules) -> std::pair<ByteArray, std::uint64_t> {
  return m_netGroup.writeNetState(fromVersion, rules);
}

void ItemDrop::readNetState(ByteArray data, float interpolationTime, NetCompatibilityRules rules) {
  m_netGroup.readNetState(data, interpolationTime, rules);
}

void ItemDrop::enableInterpolation(float extrapolationHint) {
  m_netGroup.enableNetInterpolation(extrapolationHint);
  m_mode.disableNetInterpolation();
  m_owningEntity.disableNetInterpolation();
}

void ItemDrop::disableInterpolation() {
  m_netGroup.disableNetInterpolation();
}

auto ItemDrop::position() const -> Vec2F {
  return m_movementController.position();
}

auto ItemDrop::metaBoundBox() const -> RectF {
  return m_boundBox;
}

auto ItemDrop::ephemeral() const -> bool {
  return true;
}

auto ItemDrop::collisionArea() const -> RectF {
  return m_boundBox;
}

void ItemDrop::update(float dt, std::uint64_t) {
  m_dropAge.update(world()->epochTime());

  if (isMaster()) {
    m_scriptComponent.update(m_scriptComponent.updateDt(dt));

    if (m_owningEntity.get() != NullEntityId) {
      updateTaken(true);
    } else {
      // Rarely, check for other drops near us and combine with them if possible.
      if (canTake() && m_mode.get() == Mode::Available && Random::randf() < m_combineChance) {
        auto _ = world()->findEntity(RectF::withCenter(position(), Vec2F::filled(m_combineRadius)), [&](Ptr<Entity> const& entity) -> bool {
          if (auto closeDrop = as<ItemDrop>(entity)) {
            // Make sure not to try to merge with ourselves here.
            if (closeDrop.get() != this && closeDrop->canTake()
                && vmag(position() - closeDrop->position()) < m_combineRadius
                && closeDrop->isMaster()) {
              if (m_item->couldStack(closeDrop->item()) == closeDrop->item()->count()) {
                m_item->stackWith(closeDrop->take());
                m_dropAge.setElapsedTime(std::min(m_dropAge.elapsedTime(), closeDrop->m_dropAge.elapsedTime()));

                // Average the position and velocity of the drop we merged
                // with
                m_movementController.setPosition(m_movementController.position()
                                                 + world()->geometry().diff(closeDrop->position(), m_movementController.position()) / 2.0f);
                m_movementController.setVelocity((m_movementController.velocity() + closeDrop->velocity()) / 2.0f);
                return true;
              }
            }
          }
          return false;
        });
      }

      MovementParameters parameters;
      parameters.collisionEnabled = true;
      parameters.gravityEnabled = true;
      m_movementController.applyParameters(parameters);
    }
    m_movementController.tickMaster(dt);

    m_intangibleTimer.tick(dt);
    m_ageItemsTimer.update(world()->epochTime());

    if ((m_mode.get() == Mode::Intangible || m_mode.get() == Mode::Available) && m_movementController.atWorldLimit())
      m_mode.set(Mode::Dead);
    if (m_mode.get() == Mode::Intangible && m_intangibleTimer.ready())
      m_mode.set(Mode::Available);
    if (!m_eternal && m_mode.get() == Mode::Available && m_dropAge.elapsedTime() > m_item->timeToLive())
      m_mode.set(Mode::Dead);
    if (m_mode.get() == Mode::Taken && m_dropAge.elapsedTime() > m_afterTakenLife)
      m_mode.set(Mode::Dead);

    if (m_overrideMode) {
      m_mode.set(*m_overrideMode);
    }
    if (m_clientEntityMode != ClientEntityMode::ClientSlaveOnly) {
      if (m_mode.get() == Mode::Available) {
        m_mode.set(Mode::Intangible);
      }
    }

    if (m_mode.get() <= Mode::Available && m_ageItemsTimer.elapsedTime() > m_ageItemsEvery) {
      if (Root::singleton().itemDatabase()->ageItem(m_item, m_ageItemsTimer.elapsedTime())) {
        m_itemDescriptor.set(m_item->descriptor());
        updateCollisionPoly();
      }
      m_ageItemsTimer.setElapsedTime(0.0);
    }
  } else {
    if (m_itemDescriptor.pullUpdated())
      Root::singleton().itemDatabase()->loadItem(m_itemDescriptor.get(), m_item);
    m_netGroup.tickNetInterpolation(dt);
    if (m_owningEntity.get() != NullEntityId) {
      if (!isMaster() && m_dropAge.elapsedTime() > 1.0f)
        m_owningEntity.set(NullEntityId);
      else {
        updateTaken(false);
        m_movementController.tickMaster(dt);
      }
    } else {
      m_movementController.tickSlave(dt);
    }
  }

  if (world()->isClient()) {
    SpatialLogger::logPoly("world",
                           m_movementController.collisionBody(),
                           (canTake() ? Color::Green : Color::Red).toRgba());
  }
}

auto ItemDrop::shouldDestroy() const -> bool {
  return m_mode.get() == Mode::Dead || (m_item->empty() && m_owningEntity.get() == NullEntityId);
}

void ItemDrop::render(RenderCallback* renderCallback) {
  if (m_mode.get() != Mode::Taken && m_drawRarityBeam) {
    Color beamColor;
    switch (m_item->rarity()) {
    case Rarity::Uncommon:
      beamColor = Color::rgb(87, 255, 81);
      break;
    case Rarity::Rare:
      beamColor = Color::rgb(87, 220, 255);
      break;
    case Rarity::Legendary:
      beamColor = Color::rgb(176, 81, 255);
      break;
    case Rarity::Essential:
      beamColor = Color::rgb(255, 255, 81);
      break;
    default:
      beamColor = Color::White;
    }

    beamColor.setAlphaF(0.8f);
    Line2F line = {Vec2F(), Vec2F(0.0f, 1.0f + m_boundBox.height() / 2)};
    float width = std::min(2.0f, m_boundBox.width() * TilePixels);
    auto drawable = Drawable::makeLine(line, width, beamColor, position());
    (drawable.linePart().endColor = beamColor)->setAlphaF(0.0f);
    drawable.fullbright = true;
    renderCallback->addDrawable(std::move(drawable), RenderLayerItemDrop);
  }

  if (!m_drawables) {
    if (auto mat = as<MaterialItem>(m_item.get())) {
      m_drawables = mat->generatedPreview(Vec2I(position().floor()));
      m_overForeground = true;
    } else
      m_drawables = m_item->dropDrawables();

    if (Directives dropDirectives = m_config.getString("directives", "")) {
      for (auto& drawable : *m_drawables) {
        if (drawable.isImage())
          drawable.imagePart().addDirectives(dropDirectives, true);
      }
    }
  }
  EntityRenderLayer renderLayer = m_mode.get() == Mode::Taken || m_overForeground ? RenderLayerForegroundTile : RenderLayerItemDrop;
  Vec2F dropPosition = position();
  for (Drawable drawable : *m_drawables) {
    drawable.position += dropPosition;
    renderCallback->addDrawable(std::move(drawable), renderLayer);
  }
}

void ItemDrop::renderLightSources(RenderCallback* renderCallback) {
  LightSource light;
  light.type = LightType::Spread;
  light.color = Vec3F::filled(20.f / 255.f);
  light.position = position();
  renderCallback->addLightSource(std::move(light));
}

auto ItemDrop::item() const -> Ptr<Item> {
  return m_item;
}

void ItemDrop::setEternal(bool eternal) {
  m_eternal = eternal;
}

void ItemDrop::setIntangibleTime(float intangibleTime) {
  m_intangibleTimer = GameTimer(intangibleTime);
  if (m_mode.get() == Mode::Available)
    m_mode.set(Mode::Intangible);
}

auto ItemDrop::canTake() const -> bool {
  if (m_mode.get() == Mode::Available && m_owningEntity.get() == NullEntityId && !m_item->empty()) {
    if (isMaster())
      if (auto res = m_scriptComponent.invoke<bool>("canTake"))
        return *res;
    return true;
  }
  return false;
}

auto ItemDrop::takeBy(EntityId entityId, float timeOffset) -> Ptr<Item> {
  if (canTake()) {
    m_owningEntity.set(entityId);
    m_dropAge.setElapsedTime(timeOffset);
    m_mode.set(Mode::Taken);
    setPersistent(false);

    return m_item->take();
  } else {
    return {};
  }
}

auto ItemDrop::take() -> Ptr<Item> {
  if (canTake()) {
    m_mode.set(Mode::Taken);
    return m_item->take();
  } else {
    return {};
  }
}

void ItemDrop::setPosition(Vec2F const& position) {
  m_movementController.setPosition(position);
}

auto ItemDrop::velocity() const -> Vec2F {
  return m_movementController.velocity();
}

void ItemDrop::setVelocity(Vec2F const& velocity) {
  m_movementController.setVelocity(velocity);
}

EnumMap<ItemDrop::Mode> const ItemDrop::ModeNames{
  {ItemDrop::Mode::Intangible, "Intangible"},
  {ItemDrop::Mode::Available, "Available"},
  {ItemDrop::Mode::Taken, "Taken"},
  {ItemDrop::Mode::Dead, "Dead"}};

auto ItemDrop::configValue(String const& name, Json const& def) const -> Json {
  return m_parameters.query(name, m_config.query(name, def));
}

ItemDrop::ItemDrop() {
  setPersistent(true);

  m_config = Root::singleton().assets()->json("/itemdrop.config");
  m_parameters = JsonObject{};

  MovementParameters parameters = MovementParameters(m_config.get("movementSettings", JsonObject()));
  if (!parameters.physicsEffectCategories)
    parameters.physicsEffectCategories = StringSet({"itemdrop"});
  if (parameters.collisionPoly)
    m_defaultBoundBox = parameters.collisionPoly->boundBox();
  else
    m_defaultBoundBox = RectF{-0.499, -0.499, 0.499, 0.499};
  m_boundBox = m_defaultBoundBox;

  m_movementController.applyParameters(parameters);

  m_netGroup.addNetElement(&m_mode);
  m_netGroup.addNetElement(&m_owningEntity);
  m_netGroup.addNetElement(&m_movementController);
  m_netGroup.addNetElement(&m_itemDescriptor);

  m_afterTakenLife = m_config.getFloat("afterTakenLife");
  m_overheadTime = m_config.getFloat("overheadTime");
  m_pickupDistance = m_config.getFloat("pickupDistance");
  m_velocity = m_config.getFloat("velocity");
  m_velocityApproach = m_config.getFloat("velocityApproach");
  m_overheadApproach = m_config.getFloat("overheadApproach");
  m_overheadOffset = Vec2F(m_config.getFloat("overheadRandomizedDistance"), 0).rotate(Constants::pi * 2.0 * Random::randf());

  m_combineChance = m_config.getFloat("combineChance");
  m_combineRadius = m_config.getFloat("combineRadius");
  m_ageItemsEvery = m_config.getDouble("ageItemsEvery", 10);

  m_drawRarityBeam = m_config.getBool("drawRarityBeam", false);
  m_eternal = false;
  m_overForeground = false;
  m_clientEntityMode = ClientEntityMode::ClientSlaveOnly;
}

void ItemDrop::updateCollisionPoly() {
  /* // currently disabled due to causing items to get stuck
  if (!as<MaterialItem>(m_item.get())) {
    m_boundBox = Drawable::boundBoxAll(m_item->dropDrawables(), true);
    m_boundBox.rangeSetIfEmpty(m_defaultBoundBox);
    MovementParameters parameters;
    parameters.collisionPoly = PolyF(collisionArea());
    m_movementController.applyParameters(parameters);
  }
  */
}

void ItemDrop::updateTaken(bool master) {
  if (auto owningEntity = world()->entity(m_owningEntity.get())) {
    Vec2F position = m_movementController.position();
    bool overhead = m_dropAge.elapsedTime() < m_overheadTime;
    Vec2F targetPosition = owningEntity->position();
    if (overhead) {
      targetPosition += m_overheadOffset;
      auto rect = owningEntity->collisionArea();
      if (!rect.isNull())
        targetPosition[1] += rect.yMax() + 1.5f;
      else
        targetPosition[1] += 1.5f;
    }
    Vec2F diff = world()->geometry().diff(targetPosition, position);
    float magnitude = diff.magnitude();
    Vec2F velocity = diff.normalized() * m_velocity * std::min(1.0f, magnitude);
    if (auto playerEntity = as<Player>(owningEntity))
      velocity += playerEntity->velocity();
    m_movementController.approachVelocity(velocity, overhead ? m_overheadApproach : m_velocityApproach);
    if (master && !overhead && magnitude < m_pickupDistance)
      m_mode.set(Mode::Dead);

  } else if (master) {
    // Our owning entity left, disappear quickly
    m_mode.set(Mode::Dead);
  }

  MovementParameters parameters;
  parameters.maxMovementPerStep = 1000.0f;
  parameters.collisionEnabled = false;
  parameters.gravityEnabled = false;
  m_movementController.applyParameters(parameters);
}

auto ItemDrop::callScript(String const& func, LuaVariadic<LuaValue> const& args) -> std::optional<LuaValue> {
  return m_scriptComponent.invoke(func, args);
}

auto ItemDrop::evalScript(String const& code) -> std::optional<LuaValue> {
  return m_scriptComponent.eval(code);
}

auto ItemDrop::makeItemDropCallbacks() -> LuaCallbacks {
  LuaCallbacks callbacks;
  callbacks.registerCallback("takingEntity", [this]() -> std::optional<EntityId> {
    if (m_owningEntity.get() == NullEntityId)
      return std::nullopt;
    else
      return m_owningEntity.get();
  });
  callbacks.registerCallback("setEternal", [this](bool const& eternal) -> void { setEternal(eternal); });
  callbacks.registerCallback("eternal", [this]() -> bool { return m_eternal; });
  callbacks.registerCallback("setIntangibleTime", [this](float const& intangibleTime) -> void { setIntangibleTime(intangibleTime); });
  callbacks.registerCallback("intangibleTime", [this]() -> float { return m_intangibleTimer.timer; });
  callbacks.registerCallback("setOverrideMode", [this](std::optional<String> const& mode) -> void {
    if (mode)
      m_overrideMode = ModeNames.getLeft(*mode);
    else
      m_overrideMode = std::nullopt;
  });
  callbacks.registerCallback("overrideMode", [this]() -> std::optional<String> {
    if (m_overrideMode)
      return ModeNames.getRight(*m_overrideMode);
    else
      return std::nullopt;
  });
  return callbacks;
}

}// namespace Star
