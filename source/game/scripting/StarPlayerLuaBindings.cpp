#include "StarPlayerLuaBindings.hpp"

#include "StarClientContext.hpp"// IWYU pragma: export
#include "StarCodex.hpp"
#include "StarItem.hpp"
#include "StarItemBag.hpp"
#include "StarItemDatabase.hpp"
#include "StarJsonExtra.hpp"
#include "StarNetworkedAnimatorLuaBindings.hpp"
#include "StarPlayer.hpp"
#include "StarPlayerCodexes.hpp"
#include "StarPlayerInventory.hpp"
#include "StarPlayerLog.hpp" // IWYU pragma: export
#include "StarPlayerTech.hpp"// IWYU pragma: export
#include "StarPlayerUniverseMap.hpp"
#include "StarQuestManager.hpp"// IWYU pragma: export
#include "StarQuests.hpp"
#include "StarStatistics.hpp"// IWYU pragma: export
#include "StarTeamClient.hpp"
#include "StarTechDatabase.hpp"
#include "StarUniverseClient.hpp"
#include "StarWarping.hpp"

import std;

namespace Star {

auto LuaBindings::makePlayerCallbacks(Player* player) -> LuaCallbacks {
  LuaCallbacks callbacks;

  callbacks.registerCallback("save", [player]() -> Json { return player->diskStore(); });
  callbacks.registerCallback("load", [player](Json const& data) -> void {
    auto saved = player->diskStore();
    try {
      player->diskLoad(data);
    } catch (StarException const&) {
      player->diskLoad(saved);
      throw;
    }
  });

  callbacks.registerCallback("effectsAnimator", [player]() -> LuaCallbacks {
    return LuaBindings::makeNetworkedAnimatorCallbacks(player->effectsAnimator().get());
  });

  callbacks.registerCallback("teamMembers", [player]() -> std::optional<JsonArray> {
    if (auto client = player->universeClient()) {
      return client->teamClient()->members().transformed([](TeamClient::Member const& member) -> Json {
        return JsonObject{
          {"name", member.name},
          {"uuid", member.uuid.hex()},
          {"entity", member.entity},
          {"healthPercentage", member.healthPercentage},
          {"energyPercentage", member.energyPercentage}};
      });
    }
    return {};
  });

  callbacks.registerCallback("humanoidIdentity", [player]() -> Json { return player->humanoid()->identity().toJson(); });
  callbacks.registerCallback("setHumanoidIdentity", [player](Json const& id) -> void { player->setIdentity(HumanoidIdentity(id)); });
  callbacks.registerCallback("setHumanoidParameter", [player](String key, std::optional<Json> value) -> void { player->setHumanoidParameter(key, value); });
  callbacks.registerCallback("getHumanoidParameter", [player](String key) -> std::optional<Json> { return player->getHumanoidParameter(key); });
  callbacks.registerCallback("setHumanoidParameters", [player](JsonObject parameters) -> void { player->setHumanoidParameters(parameters); });
  callbacks.registerCallback("getHumanoidParameters", [player]() -> JsonObject { return player->getHumanoidParameters(); });
  callbacks.registerCallback("refreshHumanoidParameters", [player]() -> void { player->refreshHumanoidParameters(); });
  callbacks.registerCallback("humanoidConfig", [player](bool withOverrides) -> Json { return player->humanoid()->humanoidConfig(withOverrides); });

  callbacks.registerCallback("bodyDirectives", [player]() -> Directives { return player->identity().bodyDirectives; });
  callbacks.registerCallback("setBodyDirectives", [player](String const& str) -> void { player->setBodyDirectives(str); });

  callbacks.registerCallback("emoteDirectives", [player]() -> Directives { return player->identity().emoteDirectives; });
  callbacks.registerCallback("setEmoteDirectives", [player](String const& str) -> void { player->setEmoteDirectives(str); });

  callbacks.registerCallback("hairGroup", [player]() -> String { return player->identity().hairGroup; });
  callbacks.registerCallback("setHairGroup", [player](String const& str) -> void { player->setHairGroup(str); });
  callbacks.registerCallback("hairType", [player]() -> String { return player->identity().hairType; });
  callbacks.registerCallback("setHairType", [player](String const& str) -> void { player->setHairType(str); });
  callbacks.registerCallback("hairDirectives", [player]() -> Directives { return player->identity().hairDirectives; });
  callbacks.registerCallback("setHairDirectives", [player](String const& str) -> void { player->setHairDirectives(str); });

  callbacks.registerCallback("facialHairGroup", [player]() -> String { return player->identity().facialHairGroup; });
  callbacks.registerCallback("setFacialHairGroup", [player](String const& str) -> void { player->setFacialHairGroup(str); });
  callbacks.registerCallback("facialHairType", [player]() -> String { return player->identity().facialHairType; });
  callbacks.registerCallback("setFacialHairType", [player](String const& str) -> void { player->setFacialHairType(str); });
  callbacks.registerCallback("facialHairDirectives", [player]() -> Directives { return player->identity().facialHairDirectives; });
  callbacks.registerCallback("setFacialHairDirectives", [player](String const& str) -> void { player->setFacialHairDirectives(str); });

  callbacks.registerCallback("facialMaskGroup", [player]() -> String { return player->identity().facialMaskGroup; });
  callbacks.registerCallback("setFacialMaskGroup", [player](String const& str) -> void { player->setFacialMaskGroup(str); });
  callbacks.registerCallback("facialMaskType", [player]() -> String { return player->identity().facialMaskType; });
  callbacks.registerCallback("setFacialMaskType", [player](String const& str) -> void { player->setFacialMaskType(str); });
  callbacks.registerCallback("facialMaskDirectives", [player]() -> Directives { return player->identity().facialMaskDirectives; });
  callbacks.registerCallback("setFacialMaskDirectives", [player](String const& str) -> void { player->setFacialMaskDirectives(str); });

  callbacks.registerCallback("hair", [player]() -> LuaTupleReturn<std::decay_t<const Star::String&>, std::decay_t<const Star::String&>, std::decay_t<const Star::Directives&>> {
    HumanoidIdentity const& identity = player->identity();
    return luaTupleReturn(identity.hairGroup, identity.hairType, identity.hairDirectives);
  });

  callbacks.registerCallback("facialHair", [player]() -> LuaTupleReturn<std::decay_t<const Star::String&>, std::decay_t<const Star::String&>, std::decay_t<const Star::Directives&>> {
    HumanoidIdentity const& identity = player->identity();
    return luaTupleReturn(identity.facialHairGroup, identity.facialHairType, identity.facialHairDirectives);
  });

  callbacks.registerCallback("facialMask", [player]() -> LuaTupleReturn<std::decay_t<const Star::String&>, std::decay_t<const Star::String&>, std::decay_t<const Star::Directives&>> {
    HumanoidIdentity const& identity = player->identity();
    return luaTupleReturn(identity.facialMaskGroup, identity.facialMaskType, identity.facialMaskDirectives);
  });

  callbacks.registerCallback("setFacialHair", [player](std::optional<String> const& group, std::optional<String> const& type, std::optional<String> const& directives) -> void {
    if (group && type && directives)
      player->setFacialHair(*group, *type, *directives);
    else {
      if (group)
        player->setFacialHairGroup(*group);
      if (type)
        player->setFacialHairType(*type);
      if (directives)
        player->setFacialHairDirectives(*directives);
    }
  });

  callbacks.registerCallback("setFacialMask", [player](std::optional<String> const& group, std::optional<String> const& type, std::optional<String> const& directives) -> void {
    if (group && type && directives)
      player->setFacialMask(*group, *type, *directives);
    else {
      if (group)
        player->setFacialMaskGroup(*group);
      if (type)
        player->setFacialMaskType(*type);
      if (directives)
        player->setFacialMaskDirectives(*directives);
    }
  });

  callbacks.registerCallback("setHair", [player](std::optional<String> const& group, std::optional<String> const& type, std::optional<String> const& directives) -> void {
    if (group && type && directives)
      player->setHair(*group, *type, *directives);
    else {
      if (group)
        player->setHairGroup(*group);
      if (type)
        player->setHairType(*type);
      if (directives)
        player->setHairDirectives(*directives);
    }
  });

  callbacks.registerCallback("description", [player]() -> String { return player->description(); });
  callbacks.registerCallback("setDescription", [player](String const& description) -> void { player->setDescription(description); });

  callbacks.registerCallback("name", [player]() -> String { return player->name(); });
  callbacks.registerCallback("setName", [player](String const& name) -> void { player->setName(name); });

  callbacks.registerCallback("nametag", [player]() -> String { return player->nametag(); });
  callbacks.registerCallback("setNametag", [player](std::optional<String> const& nametag) -> void { player->setNametag(nametag); });

  callbacks.registerCallback("species", [player]() -> String { return player->species(); });
  callbacks.registerCallback("setSpecies", [player](String const& species) -> void { player->setSpecies(species); });

  callbacks.registerCallback("imagePath", [player]() -> std::optional<String> { return player->identity().imagePath; });
  callbacks.registerCallback("setImagePath", [player](std::optional<String> const& imagePath) -> void { player->setImagePath(imagePath); });

  callbacks.registerCallback("gender", [player]() -> String { return GenderNames.getRight(player->gender()); });
  callbacks.registerCallback("setGender", [player](String const& gender) -> void { player->setGender(GenderNames.getLeft(gender)); });

  callbacks.registerCallback("personality", [player]() -> Json { return jsonFromPersonality(player->identity().personality); });
  callbacks.registerCallback("setPersonality", [player](Json const& personalityConfig) -> void {
    Personality const& oldPersonality = player->identity().personality;
    Personality newPersonality = oldPersonality;
    player->setPersonality(parsePersonality(newPersonality, personalityConfig));
  });

  callbacks.registerCallback("favoriteColor", [player]() -> Color { return player->favoriteColor(); });
  callbacks.registerCallback("setFavoriteColor", [player](Color color) -> void { player->setFavoriteColor(color); });

  callbacks.registerCallback("mode", [player]() -> String { return PlayerModeNames.getRight(player->modeType()); });
  callbacks.registerCallback("setMode", [player](String const& modeName) -> void { player->setModeType(PlayerModeNames.getLeft(modeName)); });

  callbacks.registerCallback("interactRadius", [player]() -> float { return player->interactRadius(); });
  callbacks.registerCallback("setInteractRadius", [player](float radius) -> void { player->setInteractRadius(radius); });

  callbacks.registerCallback("actionBarGroup", [player]() -> LuaTupleReturn<std::decay_t<int>, std::decay_t<unsigned char>> {
    return luaTupleReturn(player->inventory()->customBarGroup() + 1, player->inventory()->customBarGroups());
  });

  callbacks.registerCallback("setActionBarGroup", [player](int group) -> void {
    player->inventory()->setCustomBarGroup((group - 1) % (unsigned)player->inventory()->customBarGroups());
  });

  callbacks.registerCallback("selectedActionBarSlot", [player](LuaEngine& engine) -> std::optional<LuaValue> {
    if (auto barLocation = player->inventory()->selectedActionBarLocation()) {
      if (auto index = barLocation.ptr<CustomBarIndex>())
        return engine.luaFrom<CustomBarIndex>(*index + 1);
      else
        return engine.luaFrom<String>(EssentialItemNames.getRight(barLocation.get<EssentialItem>()));
    } else {
      return {};
    }
  });

  callbacks.registerCallback("setSelectedActionBarSlot", [player](MVariant<int, String> const& slot) -> void {
    auto inventory = player->inventory();
    if (!slot)
      inventory->selectActionBarLocation(SelectedActionBarLocation());
    else if (auto index = slot.ptr<int>()) {
      CustomBarIndex wrapped = (*index - 1) % (unsigned)inventory->customBarIndexes();
      inventory->selectActionBarLocation(SelectedActionBarLocation(wrapped));
    } else {
      EssentialItem const& item = EssentialItemNames.getLeft(slot.get<String>());
      inventory->selectActionBarLocation(SelectedActionBarLocation(item));
    }
  });

  callbacks.registerCallback("actionBarSlotLink", [player](int slot, String const& handName) -> std::optional<InventorySlot> {
    auto inventory = player->inventory();
    CustomBarIndex wrapped = (slot - 1) % (unsigned)inventory->customBarIndexes();
    if (handName == "primary")
      return inventory->customBarPrimarySlot(wrapped);
    else if (handName == "alt")
      return inventory->customBarSecondarySlot(wrapped);
    else
      throw StarException(strf("Unknown tool hand {}", handName));
  });

  callbacks.registerCallback("setActionBarSlotLink", [player](int slot, String const& handName, std::optional<InventorySlot> inventorySlot) -> void {
    auto inventory = player->inventory();
    CustomBarIndex wrapped = (slot - 1) % (unsigned)inventory->customBarIndexes();
    if (inventorySlot && !inventory->slotValid(*inventorySlot))
      inventorySlot.reset();

    if (handName == "primary")
      inventory->setCustomBarPrimarySlot(wrapped, inventorySlot);
    else if (handName == "alt")
      inventory->setCustomBarSecondarySlot(wrapped, inventorySlot);
    else
      throw StarException(strf("Unknown tool hand {}", handName));
  });

  callbacks.registerCallback("itemBagSize", [player](String const& bagName) -> std::optional<unsigned> {
    if (ConstPtr<ItemBag> bag = player->inventory()->bagContents(bagName))
      return (unsigned)bag->size();
    else
      return {};
  });

  callbacks.registerCallback("itemAllowedInBag", [player](String const& bagName, Json const& item) -> bool {
    auto inventory = player->inventory();
    ConstPtr<ItemDatabase> itemDatabase = Root::singleton().itemDatabase();
    if (!inventory->bagContents(bagName))
      return false;
    else
      return inventory->itemAllowedInBag(itemDatabase->item(ItemDescriptor(item)), bagName);
  });

  callbacks.registerCallback("item", [player](InventorySlot const& slot) -> std::optional<Json> {
    if (!player->inventory()->slotValid(slot))
      return {};
    if (auto item = player->inventory()->itemsAt(slot))
      return itemSafeDescriptor(item).toJson();
    else
      return {};
  });

  callbacks.registerCallback("setItem", [player](InventorySlot const& slot, Json const& item) -> void {
    if (!player->inventory()->slotValid(slot))
      return;
    auto itemDatabase = Root::singleton().itemDatabase();
    player->inventory()->setItem(slot, itemDatabase->item(ItemDescriptor(item)));
  });

  callbacks.registerCallback("setDamageTeam", [player](String const& typeName, std::optional<std::uint16_t> teamNumber) -> void {
    player->setTeam(EntityDamageTeam(TeamTypeNames.getLeft(typeName), teamNumber.value_or(0)));
  });

  callbacks.registerCallback("say", [player](String const& message) -> void { player->addChatMessage(message); });

  callbacks.registerCallback("emote", [player](String const& emote, std::optional<float> cooldown) -> void {
    player->addEmote(HumanoidEmoteNames.getLeft(emote), cooldown);
  });

  callbacks.registerCallback("currentEmote", [player]() -> LuaTupleReturn<std::decay_t<const Star::String&>, std::decay_t<float&>> {
    auto currentEmote = player->currentEmote();
    return luaTupleReturn(HumanoidEmoteNames.getRight(currentEmote.first), currentEmote.second);
  });

  callbacks.registerCallback("dance", [player](std::optional<String> const& dance) -> void {
    player->setDance(dance);
  });

  callbacks.registerCallback("currentState", [player]() -> String {
    return Player::StateNames.getRight(player->currentState());
  });

  callbacks.registerCallback("aimPosition", [player]() -> Vec2F { return player->aimPosition(); });

  callbacks.registerCallback("id", [player]() -> EntityId { return player->entityId(); });
  callbacks.registerCallback("uniqueId", [player]() -> std::optional<String> { return player->uniqueId(); });
  callbacks.registerCallback("isAdmin", [player]() -> bool { return player->isAdmin(); });

  callbacks.registerCallback("interact", [player](String const& type, Json const& configData, std::optional<EntityId> const& sourceEntityId) -> void {
    player->interact(InteractAction(type, sourceEntityId.value_or(NullEntityId), configData));
  });

  callbacks.registerCallback("shipUpgrades", [player]() -> Json { return player->shipUpgrades().toJson(); });
  callbacks.registerCallback("upgradeShip", [player](Json const& upgrades) -> void { player->applyShipUpgrades(upgrades); });

  callbacks.registerCallback("setUniverseFlag", [player](String const& flagName) -> void {
    player->clientContext()->rpcInterface()->invokeRemote("universe.setFlag", flagName);
  });

  callbacks.registerCallback("giveBlueprint", [player](Json const& item) -> void { player->addBlueprint(ItemDescriptor(item)); });

  callbacks.registerCallback("blueprintKnown", [player](Json const& item) -> bool { return player->blueprintKnown(ItemDescriptor(item)); });

  callbacks.registerCallback("availableRecipes", [player](std::optional<StringSet> const& filter) -> JsonArray {
    auto itemDatabase = Root::singleton().itemDatabase();
    auto inventory = player->inventory();
    auto recipes = itemDatabase->recipesFromBagContents(inventory->availableItems(), inventory->availableCurrencies(), filter.value());
    JsonArray result;
    result.reserve(recipes.size());
    for (auto& recipe : recipes)
      result.append(recipe.toJson());
    return result;
  });

  callbacks.registerCallback("makeTechAvailable", [player](String const& tech) -> void {
    player->techs()->makeAvailable(tech);
  });
  callbacks.registerCallback("makeTechUnavailable", [player](String const& tech) -> void {
    player->techs()->makeUnavailable(tech);
  });
  callbacks.registerCallback("enableTech", [player](String const& tech) -> void {
    player->techs()->enable(tech);
  });
  callbacks.registerCallback("equipTech", [player](String const& tech) -> void {
    player->techs()->equip(tech);
  });
  callbacks.registerCallback("unequipTech", [player](String const& tech) -> void {
    player->techs()->unequip(tech);
  });
  callbacks.registerCallback("availableTechs", [player]() -> StringSet {
    return player->techs()->availableTechs();
  });
  callbacks.registerCallback("enabledTechs", [player]() -> StringSet {
    return player->techs()->enabledTechs();
  });
  callbacks.registerCallback("equippedTech", [player](String typeName) -> std::optional<String> {
    return player->techs()->equippedTechs().maybe(TechTypeNames.getLeft(typeName));
  });

  callbacks.registerCallback("currency", [player](String const& currencyType) -> std::uint64_t { return player->currency(currencyType); });
  callbacks.registerCallback("addCurrency", [player](String const& currencyType, std::uint64_t amount) -> void {
    player->inventory()->addCurrency(currencyType, amount);
  });
  callbacks.registerCallback("consumeCurrency", [player](String const& currencyType, std::uint64_t amount) -> bool {
    return player->inventory()->consumeCurrency(currencyType, amount);
  });

  callbacks.registerCallback("cleanupItems", [player]() -> void {
    player->inventory()->cleanup();
  });

  callbacks.registerCallback("giveItem", [player](Json const& item) -> void {
    player->giveItem(ItemDescriptor(item));
  });

  callbacks.registerCallback("giveEssentialItem", [player](String const& slotName, Json const& item) -> void {
    auto itemDatabase = Root::singleton().itemDatabase();
    player->inventory()->setEssentialItem(EssentialItemNames.getLeft(slotName), itemDatabase->item(ItemDescriptor(item)));
  });

  callbacks.registerCallback("essentialItem", [player](String const& slotName) -> Json {
    return itemSafeDescriptor(player->inventory()->essentialItem(EssentialItemNames.getLeft(slotName))).toJson();
  });

  callbacks.registerCallback("removeEssentialItem", [player](String const& slotName) -> void {
    player->inventory()->setEssentialItem(EssentialItemNames.getLeft(slotName), {});
  });

  callbacks.registerCallback("setEquippedItem", [player](String const& slotName, Json const& item) -> void {
    auto itemDatabase = Root::singleton().itemDatabase();
    auto slot = InventorySlot(EquipmentSlotNames.getLeft(slotName));
    player->inventory()->setItem(slot, itemDatabase->item(ItemDescriptor(item)));
  });

  callbacks.registerCallback("equippedItem", [player](String const& slotName) -> Json {
    auto slot = InventorySlot(EquipmentSlotNames.getLeft(slotName));
    if (auto item = player->inventory()->itemsAt(slot))
      return item->descriptor().toJson();
    return {};
  });

  callbacks.registerCallback("hasItem", [player](Json const& item, std::optional<bool> exactMatch) -> bool {
    return player->hasItem(ItemDescriptor(item), exactMatch.value_or(false));
  });

  callbacks.registerCallback("hasCountOfItem", [player](Json const& item, std::optional<bool> exactMatch) -> std::uint64_t {
    return player->hasCountOfItem(ItemDescriptor(item), exactMatch.value_or(false));
  });

  callbacks.registerCallback("consumeItem", [player](Json const& item, std::optional<bool> consumePartial, std::optional<bool> exactMatch) -> Json {
    return player->takeItem(ItemDescriptor(item), consumePartial.value_or(false), exactMatch.value_or(false)).toJson();
  });

  callbacks.registerCallback("inventoryTags", [player]() -> StringMap<size_t> {
    StringMap<size_t> inventoryTags;
    for (auto const& item : player->inventory()->allItems()) {
      for (auto tag : item->itemTags())
        ++inventoryTags[tag];
    }
    return inventoryTags;
  });

  callbacks.registerCallback("itemsWithTag", [player](String const& tag) -> JsonArray {
    JsonArray items;
    for (auto const& item : player->inventory()->allItems()) {
      for (auto itemTag : item->itemTags()) {
        if (itemTag == tag)
          items.append(item->descriptor().toJson());
      }
    }
    return items;
  });

  callbacks.registerCallback("consumeTaggedItem", [player](String const& itemTag, std::uint64_t count) -> void {
    for (auto const& item : player->inventory()->allItems()) {
      if (item->hasItemTag(itemTag)) {
        std::uint64_t takeCount = std::min(item->count(), count);
        player->takeItem(item->descriptor().singular().multiply(takeCount));
        count -= takeCount;
        if (count == 0)
          break;
      }
    }
  });

  callbacks.registerCallback("hasItemWithParameter", [player](String const& parameterName, Json const& parameterValue) -> bool {
    for (auto const& item : player->inventory()->allItems()) {
      if (item->instanceValue(parameterName, Json()) == parameterValue)
        return true;
    }
    return false;
  });

  callbacks.registerCallback("consumeItemWithParameter", [player](String const& parameterName, Json const& parameterValue, std::uint64_t count) -> void {
    for (auto const& item : player->inventory()->allItems()) {
      if (item->instanceValue(parameterName, Json()) == parameterValue) {
        std::uint64_t takeCount = std::min(item->count(), count);
        player->takeItem(item->descriptor().singular().multiply(takeCount));
        count -= takeCount;
        if (count == 0)
          break;
      }
    }
  });

  callbacks.registerCallback("getItemWithParameter", [player](String const& parameterName, Json const& parameterValue) -> Json {
    for (auto const& item : player->inventory()->allItems()) {
      if (item->instanceValue(parameterName, Json()) == parameterValue)
        return item->descriptor().toJson();
    }
    return {};
  });

  callbacks.registerCallback("primaryHandItem", [player]() -> std::optional<Json> {
    if (!player->primaryHandItem())
      return {};
    return player->primaryHandItem()->descriptor().toJson();
  });

  callbacks.registerCallback("altHandItem", [player]() -> std::optional<Json> {
    if (!player->altHandItem())
      return {};
    return player->altHandItem()->descriptor().toJson();
  });

  callbacks.registerCallback("primaryHandItemTags", [player]() -> StringSet {
    if (!player->primaryHandItem())
      return {};
    return player->primaryHandItem()->itemTags();
  });

  callbacks.registerCallback("altHandItemTags", [player]() -> StringSet {
    if (!player->altHandItem())
      return {};
    return player->altHandItem()->itemTags();
  });

  callbacks.registerCallback("swapSlotItem", [player]() -> std::optional<Json> {
    if (!player->inventory()->swapSlotItem())
      return {};
    return player->inventory()->swapSlotItem()->descriptor().toJson();
  });

  callbacks.registerCallback("setSwapSlotItem", [player](Json const& item) -> void {
    auto itemDatabase = Root::singleton().itemDatabase();
    player->inventory()->setSwapSlotItem(itemDatabase->item(ItemDescriptor(item)));
  });

  callbacks.registerCallback("canStartQuest",
                             [player](Json const& quest) -> bool { return player->questManager()->canStart(QuestArcDescriptor::fromJson(quest)); });

  callbacks.registerCallback("startQuest", [player](Json const& quest, std::optional<String> const& serverUuid, std::optional<String> const& worldId) -> String {
    auto questArc = QuestArcDescriptor::fromJson(quest);
    auto followUp = std::make_shared<Quest>(questArc, 0, player);
    if (serverUuid)
      followUp->setServerUuid(Uuid(*serverUuid));
    if (worldId)
      followUp->setWorldId(parseWorldId(*worldId));
    player->questManager()->offer(followUp);
    return followUp->questId();
  });

  callbacks.registerCallback("questIds", [player]() -> List<String> {
    return player->questManager()->quests().keys();
  });

  callbacks.registerCallback("serverQuestIds", [player]() -> List<String> {
    return player->questManager()->serverQuests().keys();
  });

  callbacks.registerCallback("quest", [player](String const& questId) -> Json {
    if (!player->questManager()->hasQuest(questId))
      return {};
    return player->questManager()->getQuest(questId)->diskStore();
  });

  callbacks.registerCallback("questPortrait", [player](String const& questId, String const& portraitName) -> std::optional<List<Drawable>> {
    if (!player->questManager()->hasQuest(questId))
      return {};
    return player->questManager()->getQuest(questId)->portrait(portraitName);
  });

  callbacks.registerCallback("questState", [player](String const& questId) -> std::optional<String> {
    if (!player->questManager()->hasQuest(questId))
      return {};
    return QuestStateNames.getRight(player->questManager()->getQuest(questId)->state());
  });

  callbacks.registerCallback("questObjectives", [player](String const& questId) -> std::optional<JsonArray> {
    return player->questManager()->getQuest(questId)->objectiveList();
  });

  callbacks.registerCallback("callQuest", [player](String const& questId, String const& func, LuaVariadic<LuaValue> const& args) -> std::optional<LuaValue> {
    if (!player->questManager()->hasQuest(questId))
      return {};
    return player->questManager()->getQuest(questId)->callScript(func, args);
  });

  callbacks.registerCallback("hasQuest", [player](String const& questId) -> bool {
    return player->questManager()->hasQuest(questId);
  });

  callbacks.registerCallback("hasAcceptedQuest", [player](String const& questId) -> bool {
    return player->questManager()->hasAcceptedQuest(questId);
  });

  callbacks.registerCallback("hasActiveQuest", [player](String const& questId) -> bool {
    return player->questManager()->isActive(questId);
  });

  callbacks.registerCallback("hasCompletedQuest", [player](String const& questId) -> bool {
    return player->questManager()->hasCompleted(questId);
  });

  callbacks.registerCallback("trackedQuestId", [player]() -> std::optional<String> {
    return player->questManager()->trackedQuestId();
  });

  callbacks.registerCallback("setTrackedQuest", [player](String const& questId) -> void {
    if (!player->questManager()->isCurrent(questId))
      return player->questManager()->setAsTracked(questId);
    else
      return player->questManager()->setAsTracked({});
  });

  callbacks.registerCallback("canTurnInQuest", [player](String const& questId) -> bool {
    return player->questManager()->canTurnIn(questId);
  });

  callbacks.registerCallback("currentQuestId", [player]() -> std::optional<String> {
    return player->questManager()->currentQuestId();
  });

  callbacks.registerCallback("currentQuest", [player]() -> Json {
    auto maybeQuest = player->questManager()->currentQuest();
    if (maybeQuest) {
      return (*maybeQuest)->diskStore();
    }
    return {};
  });

  callbacks.registerCallback("currentQuestWorld", [player]() -> std::optional<String> {
    auto maybeQuest = player->questManager()->currentQuest();
    if (maybeQuest) {
      if (auto worldId = (*maybeQuest)->worldId())
        return printWorldId(*worldId);
    }
    return {};
  });

  callbacks.registerCallback("questWorlds", [player]() -> List<std::pair<String, bool>> {
    List<std::pair<String, bool>> res;
    auto maybeCurrentQuest = player->questManager()->currentQuest();
    for (auto q : player->questManager()->listActiveQuests()) {
      if (auto worldId = q->worldId()) {
        bool isCurrentQuest = maybeCurrentQuest && maybeCurrentQuest.value()->questId() == q->questId();
        res.append(std::pair<String, bool>(printWorldId(*worldId), isCurrentQuest));
      }
    }
    return res;
  });

  callbacks.registerCallback("currentQuestLocation", [player]() -> Json {
    auto maybeQuest = player->questManager()->currentQuest();
    if (maybeQuest) {
      if (auto questLocation = maybeQuest.value()->location())
        return JsonObject{{"system", jsonFromVec3I(questLocation->first)}, {"location", jsonFromSystemLocation(questLocation->second)}};
    }
    return {};
  });

  callbacks.registerCallback("questLocations", [player]() -> List<std::pair<Json, bool>> {
    List<std::pair<Json, bool>> res;
    auto maybeCurrentQuest = player->questManager()->currentQuest();
    for (auto q : player->questManager()->listActiveQuests()) {
      if (auto questLocation = q->location()) {
        bool isCurrentQuest = maybeCurrentQuest && maybeCurrentQuest.value()->questId() == q->questId();
        auto locationJson = JsonObject{{"system", jsonFromVec3I(questLocation->first)}, {"location", jsonFromSystemLocation(questLocation->second)}};
        res.append(std::pair<Json, bool>(locationJson, isCurrentQuest));
      }
    }
    return res;
  });

  callbacks.registerCallback("enableMission", [player](String const& mission) -> void {
    AiState& aiState = player->aiState();
    if (!aiState.completedMissions.contains(mission))
      aiState.availableMissions.add(mission);
  });

  callbacks.registerCallback("completeMission", [player](String const& mission) -> void {
    AiState& aiState = player->aiState();
    aiState.availableMissions.remove(mission);
    aiState.completedMissions.add(mission);
  });

  callbacks.registerCallback("hasCompletedMission", [player](String const& mission) -> bool {
    return player->aiState().completedMissions.contains(mission);
  });

  callbacks.registerCallback("radioMessage", [player](Json const& messageConfig, std::optional<float> const& delay) -> void {
    player->queueRadioMessage(messageConfig, delay.value_or(0));
  });

  callbacks.registerCallback("worldId", [player]() -> String { return printWorldId(player->clientContext()->playerWorldId()); });

  callbacks.registerCallback("serverUuid", [player]() -> String { return player->clientContext()->serverUuid().hex(); });

  callbacks.registerCallback("ownShipWorldId", [player]() -> String { return printWorldId(ClientShipWorldId(player->uuid())); });

  callbacks.registerCallback("lounge", [player](EntityId entityId, std::optional<size_t> anchorIndex) -> bool {
    return player->lounge(entityId, anchorIndex.value_or(0));
  });
  callbacks.registerCallback("isLounging", [player]() -> bool { return (bool)player->loungingIn(); });
  callbacks.registerCallback("loungingIn", [player]() -> std::optional<EntityId> {
    if (auto anchorState = player->loungingIn())
      return anchorState->entityId;
    return {};
  });
  callbacks.registerCallback("stopLounging", [player]() -> void { player->stopLounging(); });

  callbacks.registerCallback("playTime", [player]() -> double { return player->log()->playTime(); });

  callbacks.registerCallback("introComplete", [player]() -> bool { return player->log()->introComplete(); });
  callbacks.registerCallback("setIntroComplete", [player](bool complete) -> void {
    return player->log()->setIntroComplete(complete);
  });

  callbacks.registerCallback("warp", [player](String action, std::optional<String> animation, std::optional<bool> deploy) -> void {
    player->setPendingWarp(action, animation, deploy.value_or(false));
  });

  callbacks.registerCallback("canDeploy", [player]() -> bool {
    return player->canDeploy();
  });

  callbacks.registerCallback("isDeployed", [player]() -> bool {
    return player->isDeployed();
  });

  callbacks.registerCallback("confirm", [player](Json dialogConfig) -> Star::RpcPromise<Star::Json> {
    auto pair = RpcPromise<Json>::createPair();
    player->queueConfirmation(dialogConfig, pair.second);
    return pair.first;
  });

  callbacks.registerCallback("playCinematic", [player](Json const& cinematic, std::optional<bool> unique) -> void {
    player->setPendingCinematic(cinematic, unique.value_or(false));
  });

  callbacks.registerCallback("recordEvent", [player](String const& eventName, Json const& fields) -> void {
    player->statistics()->recordEvent(eventName, fields);
  });

  callbacks.registerCallback("worldHasOrbitBookmark", [player](Json const& coords) -> bool {
    auto coordinate = CelestialCoordinate(coords);
    return player->universeMap()->worldBookmark(coordinate).has_value();
  });

  callbacks.registerCallback("orbitBookmarks", [player]() -> List<std::pair<Vec3I, Json>> {
    return player->universeMap()->orbitBookmarks().transformed([](std::pair<Vec3I, OrbitBookmark> const& p) -> std::pair<Vec3I, Json> {
      return {p.first, p.second.toJson()};
    });
  });

  callbacks.registerCallback("systemBookmarks", [player](Json const& coords) -> List<Json> {
    auto coordinate = CelestialCoordinate(coords);
    return player->universeMap()->systemBookmarks(coordinate).transformed([](OrbitBookmark const& bookmark) -> Json {
      return bookmark.toJson();
    });
  });

  callbacks.registerCallback("addOrbitBookmark", [player](Json const& system, Json const& bookmarkConfig) -> bool {
    auto coordinate = CelestialCoordinate(system);
    return player->universeMap()->addOrbitBookmark(coordinate, OrbitBookmark::fromJson(bookmarkConfig));
  });

  callbacks.registerCallback("removeOrbitBookmark", [player](Json const& system, Json const& bookmarkConfig) -> bool {
    auto coordinate = CelestialCoordinate(system);
    return player->universeMap()->removeOrbitBookmark(coordinate, OrbitBookmark::fromJson(bookmarkConfig));
  });

  callbacks.registerCallback("teleportBookmarks", [player]() -> List<Json> {
    return player->universeMap()->teleportBookmarks().transformed([](TeleportBookmark const& bookmark) -> Json {
      return bookmark.toJson();
    });
  });

  callbacks.registerCallback("addTeleportBookmark", [player](Json const& bookmarkConfig) -> bool {
    return player->universeMap()->addTeleportBookmark(TeleportBookmark::fromJson(bookmarkConfig));
  });

  callbacks.registerCallback("removeTeleportBookmark", [player](Json const& bookmarkConfig) -> bool {
    return player->universeMap()->removeTeleportBookmark(TeleportBookmark::fromJson(bookmarkConfig));
  });

  callbacks.registerCallback("isMapped", [player](Json const& coords) -> bool {
    auto coordinate = CelestialCoordinate(coords);
    return player->universeMap()->isMapped(coordinate);
  });

  callbacks.registerCallback("mappedObjects", [player](Json const& coords) -> Json {
    auto coordinate = CelestialCoordinate(coords);
    JsonObject json;
    for (auto p : player->universeMap()->mappedObjects(coordinate)) {
      JsonObject object = {
        {"typeName", p.second.typeName},
        {"orbit", jsonFromMaybe<CelestialOrbit>(p.second.orbit, [](CelestialOrbit const& o) -> Json { return o.toJson(); })},
        {"parameters", p.second.parameters}};
      json.set(p.first.hex(), object);
    }
    return json;
  });

  callbacks.registerCallback("collectables", [player](String const& collection) -> StringSet {
    return player->log()->collectables(collection);
  });

  callbacks.registerCallback("getProperty", [player](String const& name, std::optional<Json> const& defaultValue) -> Json {
    return player->getGenericProperty(name, defaultValue.value_or(Json()));
  });

  callbacks.registerCallback("setProperty", [player](String const& name, Json const& value) -> void {
    player->setGenericProperty(name, value);
  });

  callbacks.registerCallback("addScannedObject", [player](String const& objectName) -> bool {
    return player->log()->addScannedObject(objectName);
  });

  callbacks.registerCallback("removeScannedObject", [player](String const& objectName) -> void {
    player->log()->removeScannedObject(objectName);
  });

  // codex bindings
  callbacks.registerCallback("isCodexKnown", [player](String const& codexId) -> bool {
    return player->codexes()->codexKnown(codexId);
  });

  callbacks.registerCallback("isCodexRead", [player](String const& codexId) -> bool {
    return player->codexes()->codexRead(codexId);
  });

  callbacks.registerCallback("markCodexRead", [player](String const& codexId) -> bool {
    return player->codexes()->markCodexRead(codexId);
  });

  callbacks.registerCallback("markCodexUnread", [player](String const& codexId) -> bool {
    return player->codexes()->markCodexUnread(codexId);
  });

  callbacks.registerCallback("learnCodex", [player](String const& codexId, std::optional<bool> markRead) -> void {
    player->codexes()->learnCodex(codexId, markRead.value_or(false));
  });

  callbacks.registerCallback("getCodexes", [player]() -> Json {
    return player->codexes()->toJson();
  });

  callbacks.registerCallback("getNewCodex", [player]() -> std::optional<String> {
    ConstPtr<Codex> codexPtr = player->codexes()->firstNewCodex();
    if (codexPtr)
      return codexPtr->title();

    return {};
  });

  callbacks.registerCallback("setAnimationParameter", [player](String name, Json value) -> void {
    player->setAnimationParameter(name, value);
  });

  callbacks.registerCallback("setCameraFocusEntity", [player](std::optional<EntityId> const& entityId) -> void {
    player->setCameraFocusEntity(entityId);
  });

  return callbacks;
}

}// namespace Star
