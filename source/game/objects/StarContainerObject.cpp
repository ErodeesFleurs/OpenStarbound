import star.json_extra;
import star.json;
import star.id_map;
import star.ordered_map;
import star.lru_cache;
import star.thread;
import star.time;
import star.random;
import star.ttl_cache;
import star.casting;
import star.game_types;
import star.variant;
import star.strong_typedef;
import star.rpc_promise;
import star.bi_map;
import star.a_star;
import star.data_stream;
import star.lexical_cast;
import star.weighted_pool;
import star.interpolation;
import star.parametric_function;
import star.logging;
import star.audio;
import star.list;
import star.map;
import star.set;
import star.color;
import star.string;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.math_common;
import star.periodic;
import star.periodic_function;
import star.lua;
import star.io_device;
import star.ref_ptr;
import star.listener;
import std;
import star.format;
import star.version;

import star.lua_root;
import star.lua_components;
import star.lua_animation_component;

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.entity;
import star.tile_damage;
import star.interaction_types;
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
import star.item;
import star.item_descriptor;
import star.item_database;
import star.item_recipe;
import star.status_effect_entity;
import star.scripted_entity;
import star.chat_action;
import star.chatty_entity;
import star.wiring;
import star.wire_entity;
import star.inspectable_entity;
import star.animated_part_set;
import star.networked_animator;
import star.object;
import star.container_entity;
import star.container_object;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.mobile_entity;
import star.item_drop;
import star.mixer;
import star.drawable;
import star.entity_rendering_types;
import star.animation;
import star.particle;

import star.light_source;
import star.entity_rendering;
import star.augment_item;
import star.treasure;


import star.object_database;

import star.item_bag;

import star.data_stream_devices;

import star.algorithm;
namespace Star {

ContainerObject::ContainerObject(Root& rootContext, std::shared_ptr<ObjectConfig const> config, Json const& parameters) : Entity(rootContext), Object(rootContext, config, parameters) {
  m_opened.set(0);
  m_count = 0;
  m_currentState = 0;
  m_animationFrameCooldown = 0;
  m_autoCloseCooldown = 0;

  m_crafting.set(false);
  m_craftingProgress.set(0);

  m_initialized = false;
  m_itemsUpdated = true;
  m_runUpdatedCallback = true;

  m_items = std::make_shared<ItemBag>(root(), configValue("slotCount").toInt());

  m_netGroup.addNetElement(&m_opened);
  m_netGroup.addNetElement(&m_crafting);
  m_netGroup.addNetElement(&m_craftingProgress);
  m_netGroup.addNetElement(&m_itemsNetState);

  m_craftingProgress.setInterpolator(lerp<float, float>);
}

void ContainerObject::init(World* world, EntityId entityId, EntityMode mode) {
  if (mode == EntityMode::Master)
    m_interactive.set(true);

  Object::init(world, entityId, mode);
  if (mode == EntityMode::Master) {
    if (!m_initialized) {
      m_initialized = true;
      float level = world->threatLevel();
      std::uint64_t seed = configValue("treasureSeed", this->root().random().randu64()).toUInt();
      level = configValue("level", level).toFloat();
      level += configValue("levelAdjustment", 0).toFloat();
      if (!configValue("initialItems").isNull()) {
        List<ItemDescriptor> items;
        for (auto const& spec : configValue("initialItems").iterateArray())
          m_items->addItems({this->root().itemDatabase()->item(ItemDescriptor(spec), level, ++seed)});
      }
      if (!configValue("treasurePools").isNull()) {
        String treasurePool = this->root().random().randValueFrom(configValue("treasurePools").toArray()).toString();
        this->root().treasureDatabase()->fillWithTreasure(m_items, treasurePool, level, ++seed);
      }
      itemsUpdated();
    }
  }
}

void ContainerObject::update(float dt, std::uint64_t currentStep) {
  Object::update(dt, currentStep);

  if (isMaster()) {
    for (auto const& drop : take(m_lostItems))
      world()->addEntity(ItemDrop::createRandomizedDrop(root(), drop, position()));

    if (m_crafting.get())
      tickCrafting(dt);

    if (m_autoCloseCooldown > 0) {
      m_autoCloseCooldown -= 1;
      if (m_autoCloseCooldown <= 0) {
        --m_count;
        if (m_count <= 0) {
          m_count = 0;
          m_opened.set(0);
        } else {
          m_autoCloseCooldown = configValue("autoCloseCooldown").toInt();
        }
      }
    }

    m_ageItemsTimer.update(world()->epochTime());
    if (m_ageItemsTimer.elapsedTime() > configValue("ageItemsEvery", 10).toDouble()) {
      double elapsedTime = m_ageItemsTimer.elapsedTime() * configValue("itemAgeMultiplier", 1.0f).toDouble();
      for (auto& item : m_items->items()) {
        if (this->root().itemDatabase()->ageItem(item, elapsedTime))
          itemsUpdated();
      }
      m_ageItemsTimer.setElapsedTime(0.0);
    }

    if (take(m_runUpdatedCallback))
      m_scriptComponent.invoke("containerCallback");

  } else {
    setImageKey("key", toString(m_currentState));
    setImageKey("state", m_crafting.get() ? "crafting" : "idle");

    m_animationFrameCooldown -= 1;
  }
}

void ContainerObject::render(RenderCallback* renderCallback) {
  auto assets = this->root().assets();

  if (m_animationFrameCooldown <= 0) {
    if (m_opened.get() != m_currentState) {
      if (m_currentState == 0) {
        // opening, or flipping to the other side
        if (!configValue("openSounds").isNull()) {
          auto audio = std::make_shared<AudioInstance>(*assets->audio(this->root().random().randValueFrom(configValue("openSounds").toArray()).toString()));
          audio->setPosition(position());
          audio->setRangeMultiplier(config()->soundEffectRangeMultiplier);
          renderCallback->addAudio(std::move(audio));
        }
      }
      if (m_currentState == configValue("openFrameIndex", 2).toInt()) {
        // closing
        if (!configValue("closeSounds").isNull()) {
          auto audio = std::make_shared<AudioInstance>(*assets->audio(this->root().random().randValueFrom(configValue("closeSounds").toArray()).toString()));
          audio->setPosition(position());
          audio->setRangeMultiplier(config()->soundEffectRangeMultiplier);
          renderCallback->addAudio(std::move(audio));
        }
      }
      if (m_opened.get() < m_currentState) {
        m_currentState -= 1;
      } else {
        m_currentState += 1;
      }
      m_animationFrameCooldown = configValue("frameCooldown").toInt();
    } else {
      m_animationFrameCooldown = 0;
    }
  }

  Object::render(renderCallback);
}

void ContainerObject::destroy(RenderCallback* renderCallback) {
  Object::destroy(renderCallback);
  if (isMaster()) {
    for (auto const& drop : m_items->items())
      world()->addEntity(ItemDrop::createRandomizedDrop(root(), drop, position()));
  }
}

std::optional<ChainableJsonMessageResponse> ContainerObject::receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) {
  auto itemDb = this->root().itemDatabase();

  if (message.equalsIgnoreCase("startCrafting")) {
    startCrafting();
    return ChainableJsonMessageResponse(Json());

  } else if (message.equalsIgnoreCase("stopCrafting")) {
    stopCrafting();
    return ChainableJsonMessageResponse(Json());

  } else if (message.equalsIgnoreCase("burnContainerContents")) {
    burnContainerContents();
    return ChainableJsonMessageResponse(Json());

  } else if (message.equalsIgnoreCase("addItems")) {
    return ChainableJsonMessageResponse(itemSafeDescriptor(doAddItems(itemDb->fromJson(args.at(0)))).toJson());

  } else if (message.equalsIgnoreCase("putItems")) {
    return ChainableJsonMessageResponse(itemSafeDescriptor(doPutItems(args.at(0).toUInt(), itemDb->fromJson(args.at(1)))).toJson());

  } else if (message.equalsIgnoreCase("takeItems")) {
    return ChainableJsonMessageResponse(itemSafeDescriptor(doTakeItems(args.at(0).toUInt(), args.at(1).toUInt())).toJson());

  } else if (message.equalsIgnoreCase("swapItems")) {
    return ChainableJsonMessageResponse(itemSafeDescriptor(doSwapItems(args.at(0).toUInt(), itemDb->fromJson(args.at(1)), args.get(2).optBool().value_or(true))).toJson());

  } else if (message.equalsIgnoreCase("applyAugment")) {
    return ChainableJsonMessageResponse(itemSafeDescriptor(doApplyAugment(args.at(0).toUInt(), itemDb->fromJson(args.at(1)))).toJson());

  } else if (message.equalsIgnoreCase("consumeItems")) {
    return ChainableJsonMessageResponse(Json(doConsumeItems(ItemDescriptor(args.at(0)))));

  } else if (message.equalsIgnoreCase("consumeItemsAt")) {
    return ChainableJsonMessageResponse(Json(doConsumeItems(args.at(0).toUInt(), args.at(1).toUInt())));

  } else if (message.equalsIgnoreCase("clearContainer")) {
    return ChainableJsonMessageResponse(Json(transform<JsonArray>(doClearContainer(), [](auto const& item) {
        return itemSafeDescriptor(item).toJson();
      })));

  } else {
    return Object::receiveMessage(sendingConnection, message, args);
  }
}

InteractAction ContainerObject::interact(InteractRequest const&) {
  return InteractAction(InteractActionType::OpenContainer, entityId(), Json());
}

Json ContainerObject::containerGuiConfig() const {
  return this->root().assets()->json(configValue("uiConfig").toString().replace("<slots>", toString(m_items->size())));
}

String ContainerObject::containerDescription() const {
  return Object::shortDescription();
}

String ContainerObject::containerSubTitle() const {
  Json categories = this->root().assets()->json("/items/categories.config:labels");
  return categories.getString(Object::category(), Object::category());
}

ItemDescriptor ContainerObject::iconItem() const {
  if (configValue("hasWindowIcon", true).toBool())
    return ItemDescriptor(name(), 1);
  return {};
}

std::shared_ptr<ItemBag const> ContainerObject::itemBag() const {
  return m_items;
}

void ContainerObject::containerOpen() {
  m_opened.set(configValue("openFrameIndex", 2).toInt());
  m_count++;
  m_autoCloseCooldown = configValue("autoCloseCooldown").toInt();
}

void ContainerObject::containerClose() {
  --m_count;
  if (m_count <= 0) {
    m_count = 0;
    m_opened.set(0);
  }
}

RpcPromise<std::shared_ptr<Item>> ContainerObject::addItems(std::shared_ptr<Item> const& items) {
  if (isSlave()) {
    return world()->sendEntityMessage(entityId(), "addItems", {itemSafeDescriptor(items).toJson()}).wrap([owner = &root()](Json res) {
        return owner->itemDatabase()->item(ItemDescriptor(res));
      });
  } else {
    return RpcPromise<std::shared_ptr<Item>>::createFulfilled(doAddItems(items));
  }
}

RpcPromise<std::shared_ptr<Item>> ContainerObject::putItems(std::size_t pos, std::shared_ptr<Item> const& items) {
  if (isSlave()) {
    return world()->sendEntityMessage(entityId(), "putItems", {itemSafeDescriptor(items).toJson()}).wrap([owner = &root()](Json res) {
        return owner->itemDatabase()->item(ItemDescriptor(res));
      });
  } else {
    return RpcPromise<std::shared_ptr<Item>>::createFulfilled(doPutItems(pos, items));
  }
}

RpcPromise<std::shared_ptr<Item>> ContainerObject::takeItems(std::size_t slot, std::size_t count) {
  if (isSlave()) {
    return world()->sendEntityMessage(entityId(), "takeItems", {slot, count}).wrap([owner = &root()](Json res) {
        return owner->itemDatabase()->item(ItemDescriptor(res));
      });
  } else {
    return RpcPromise<std::shared_ptr<Item>>::createFulfilled(doTakeItems(slot, count));
  }
}

RpcPromise<std::shared_ptr<Item>> ContainerObject::swapItems(std::size_t slot, std::shared_ptr<Item> const& items, bool tryCombine) {
  if (isSlave()) {
    return world()->sendEntityMessage(entityId(), "swapItems", {slot, itemSafeDescriptor(items).toJson(), tryCombine}).wrap([owner = &root()](Json res) {
        return owner->itemDatabase()->item(ItemDescriptor(res));
      });
  } else {
    return RpcPromise<std::shared_ptr<Item>>::createFulfilled(doSwapItems(slot, items, tryCombine));
  }
}

RpcPromise<std::shared_ptr<Item>> ContainerObject::applyAugment(std::size_t slot, std::shared_ptr<Item> const& augment) {
  if (isSlave()) {
    return world()->sendEntityMessage(entityId(), "applyAugment", {slot, itemSafeDescriptor(augment).toJson()}).wrap([owner = &root()](Json res) {
        return owner->itemDatabase()->item(ItemDescriptor(res));
      });
  } else {
    return RpcPromise<std::shared_ptr<Item>>::createFulfilled(doApplyAugment(slot, augment));
  }
}

RpcPromise<bool> ContainerObject::consumeItems(ItemDescriptor const& descriptor) {
  if (isSlave()) {
    return world()->sendEntityMessage(entityId(), "consumeItems", {descriptor.toJson()}).wrap([](Json res) {
        return res.toBool();
      });
  } else {
    return RpcPromise<bool>::createFulfilled(doConsumeItems(descriptor));
  }
}

RpcPromise<bool> ContainerObject::consumeItems(std::size_t pos, std::size_t count) {
  if (isSlave()) {
    return world()->sendEntityMessage(entityId(), "consumeItemsAt", {pos, count}).wrap([](Json res) {
        return res.toBool();
      });
  } else {
    return RpcPromise<bool>::createFulfilled(doConsumeItems(pos, count));
  }
}

RpcPromise<List<std::shared_ptr<Item>>> ContainerObject::clearContainer() {
  if (isSlave()) {
    return world()->sendEntityMessage(entityId(), "clearContainer", {}).wrap([owner = &root()](Json res) {
        auto itemDb = owner->itemDatabase();
        return res.toArray().transformed([itemDb](Json const& item) {
            return itemDb->item(ItemDescriptor(item));
          });
      });
  } else {
    return RpcPromise<List<std::shared_ptr<Item>>>::createFulfilled(doClearContainer());
  }
}

bool ContainerObject::isCrafting() const {
  return m_crafting.get();
}

void ContainerObject::startCrafting() {
  if (isSlave()) {
    world()->sendEntityMessage(entityId(), "startCrafting");
  } else {
    if (m_crafting.get())
      return;
    auto inputItems = m_items->items();
    inputItems.removeLast();
    m_goalRecipe = recipeForMaterials(inputItems);
    m_crafting.set(true);
    itemsUpdated();
    // tickCrafting validates
  }
}

void ContainerObject::stopCrafting() {
  if (isSlave()) {
    world()->sendEntityMessage(entityId(), "stopCrafting");
  } else {
    if (!m_crafting.get())
      return;
    m_crafting.set(false);
    m_craftingProgress.set(0);
    m_goalRecipe = ItemRecipe();
  }
}

float ContainerObject::craftingProgress() const {
  if (!isCrafting())
    return 1;
  return clamp(m_craftingProgress.get(), 0.0f, 1.0f);
}

void ContainerObject::burnContainerContents() {
  if (isSlave()) {
    world()->sendEntityMessage(entityId(), "burnContainerContents");
  } else {
    stopCrafting();
    auto level = world()->getProperty("ship.fuel", 0).toUInt();
    auto maxLevel = world()->getProperty("ship.maxFuel", 0).toUInt();
    for (auto& item : m_items->items()) {
      if (level > maxLevel)
        level = maxLevel;
      if (maxLevel == level)
        break;

      auto leftToFill = maxLevel - level;

      if (item) {
        auto fuelSingle = item->instanceValue("fuelAmount", 0).toUInt();
        if (fuelSingle > 0) {
          auto itemsToConsume = std::min<std::uint64_t>((leftToFill + fuelSingle - 1) / fuelSingle, item->count());
          level = std::min(maxLevel, level + fuelSingle * itemsToConsume);
          auto consumed = item->consume(itemsToConsume);
          if constexpr (Star::DebugEnabled) {
            Star::starAssertFailed((consumed));
          }
          ((void)(consumed));
        }
      }
    }
    itemsUpdated();
    world()->setProperty("ship.fuel", level);
  }
}

void ContainerObject::getNetStates(bool initial) {
  Object::getNetStates(initial);
  if (m_itemsNetState.pullUpdated()) {
    DataStreamBuffer ds(m_itemsNetState.get());
    m_items->read(ds);
    itemsUpdated();
  }
}

void ContainerObject::setNetStates() {
  Object::setNetStates();
  if (take(m_itemsUpdated)) {
    DataStreamBuffer ds;
    m_items->write(ds);
    m_itemsNetState.set(ds.takeData());
  }
}

void ContainerObject::readStoredData(Json const& diskStore) {
  Object::readStoredData(diskStore);

  m_opened.set(diskStore.getInt("opened"));
  m_currentState = diskStore.getInt("currentState");
  m_crafting.set(diskStore.getBool("crafting"));
  m_craftingProgress.set(diskStore.getFloat("craftingProgress"));
  m_initialized = diskStore.getBool("initialized");
  m_items = std::make_shared<ItemBag>(ItemBag::loadStore(root(), diskStore.get("items")));
  m_ageItemsTimer = EpochTimer(diskStore.get("ageItemsTimer"));

  m_lostItems.appendAll(m_items->resize(configValue("slotCount").toUInt()));
}

Json ContainerObject::writeStoredData() const {
  return Object::writeStoredData().setAll({
      {"opened", m_opened.get()},
      {"currentState", m_currentState},
      {"crafting", m_crafting.get()},
      {"craftingProgress", m_craftingProgress.get()},
      {"initialized", m_initialized},
      {"items", m_items->diskStore()},
      {"ageItemsTimer", m_ageItemsTimer.toJson()}
    });
}

ItemRecipe ContainerObject::recipeForMaterials(List<std::shared_ptr<Item>> const& inputItems) {
  auto& root = this->root();
  auto itemDatabase = root.itemDatabase();

  Json recipeGroup = configValue("recipeGroup");
  if (!recipeGroup.isNull())
    return itemDatabase->getPreciseRecipeForMaterials(recipeGroup.toString(), inputItems, {});

  std::optional<Json> result = m_scriptComponent.invoke<Json>("craftingRecipe", inputItems.filtered([](std::shared_ptr<Item> const& item) {
      return (bool)item;
    }).transformed([](std::shared_ptr<Item> const& item) {
      return item->descriptor().toJson();
    }));
  if (!result || result->isNull())
    return ItemRecipe();
  return itemDatabase->parseRecipe(*result);
}

void ContainerObject::tickCrafting(float dt) {
  if (!m_crafting.get())
    return;

  auto inputItems = m_items->items();
  inputItems.removeLast();
  auto recipe = recipeForMaterials(inputItems);
  bool craftingFail = false;
  if (recipe.isNull() || m_goalRecipe != recipe)
    craftingFail = true;
  std::shared_ptr<Item> targetItem = m_items->at(m_items->size() - 1);
  if (targetItem) {
    if (!targetItem->matches(m_goalRecipe.output, true))
      craftingFail = true;
    else if (targetItem->count() + m_goalRecipe.output.count() > targetItem->maxStack())
      craftingFail = true;
  }
  if (craftingFail) {
    m_crafting.set(false);
    m_craftingProgress.set(0);
    m_goalRecipe = ItemRecipe();
    return;
  }
  if (m_goalRecipe.duration > 0)
    m_craftingProgress.set(m_craftingProgress.get() + dt / m_goalRecipe.duration);
  else
    m_craftingProgress.set(1.0f);
  if (m_craftingProgress.get() >= 1.0f) {
    m_craftingProgress.set(0);
    for (auto const& input : m_goalRecipe.inputs) {
      bool consumed = m_items->consumeItems(input);
      ((void)(consumed));
      if constexpr (Star::DebugEnabled) {
        Star::starAssertFailed((consumed));
      }
    }
    std::shared_ptr<Item> overflow =
        m_items->putItems(m_items->size() - 1, root().itemDatabase()->item(m_goalRecipe.output));
    if (overflow)
      world()->addEntity(ItemDrop::createRandomizedDrop(root(), overflow, position()));
    itemsUpdated();
  }
}

std::shared_ptr<Item> ContainerObject::doAddItems(std::shared_ptr<Item> const& items) {
  itemsUpdated();
  return m_items->addItems(items);
}

std::shared_ptr<Item> ContainerObject::doPutItems(std::size_t slot, std::shared_ptr<Item> const& items) {
  itemsUpdated();
  return m_items->putItems(slot, items);
}

std::shared_ptr<Item> ContainerObject::doTakeItems(std::size_t slot, std::size_t count) {
  itemsUpdated();
  return m_items->takeItems(slot, count);
}

std::shared_ptr<Item> ContainerObject::doSwapItems(std::size_t slot, std::shared_ptr<Item> const& items, bool tryCombine) {
  itemsUpdated();
  return m_items->swapItems(slot, items, tryCombine);
}

std::shared_ptr<Item> ContainerObject::doApplyAugment(std::size_t slot, std::shared_ptr<Item> const& item) {
  itemsUpdated();
  if (auto augment = as<AugmentItem>(item))
    if (auto slotItem = m_items->at(slot))
      m_items->setItem(slot, augment->applyTo(slotItem));
  return item;
}

bool ContainerObject::doConsumeItems(ItemDescriptor const& descriptor) {
  if (m_items->consumeItems(descriptor)) {
    itemsUpdated();
    return true;
  }

  return false;
}

bool ContainerObject::doConsumeItems(std::size_t slot, std::size_t count) {
  if (m_items->consumeItems(slot, count)) {
    itemsUpdated();
    return true;
  }

  return false;
}

List<std::shared_ptr<Item>> ContainerObject::doClearContainer() {
  stopCrafting();
  List<std::shared_ptr<Item>> result = m_items->takeAll();
  m_items->clearItems();
  itemsUpdated();
  return result;
}

void ContainerObject::itemsUpdated() {
  m_itemsUpdated = true;
  m_runUpdatedCallback = true;
}

}
