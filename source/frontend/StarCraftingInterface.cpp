#include "StarCraftingInterface.hpp"
#include "StarAlgorithm.hpp"
#include "StarJsonExtra.hpp"
#include "StarGuiReader.hpp"
#include "StarLexicalCast.hpp"
#include "StarItemTooltip.hpp"
#include "StarPlayer.hpp"
#include "StarContainerEntity.hpp"
#include "StarWorldClient.hpp"
#include "StarPlayerBlueprints.hpp"
#include "StarButtonWidget.hpp"
#include "StarPaneManager.hpp"
#include "StarPortraitWidget.hpp"
#include "StarLabelWidget.hpp"
#include "StarTextBoxWidget.hpp"
#include "StarImageWidget.hpp"
#include "StarListWidget.hpp"
#include "StarImageStretchWidget.hpp"
#include "StarItemSlotWidget.hpp"
#include "StarConfiguration.hpp"
#include "StarObjectItem.hpp"
#include "StarAssets.hpp"
#include "StarItemDatabase.hpp"
#include "StarObjectDatabase.hpp"
#include "StarPlayerInventory.hpp"
#include "StarPlayerLog.hpp"
#include "StarMixer.hpp"

namespace Star {

CraftingPane::CraftingPane(WorldClientPtr worldClient,
    PlayerPtr player,
    Json const& settings,
    EntityId sourceEntityId,
    CraftingPaneServices services)
  : Pane(services.guiContext),
    m_worldClient(requireServiceValueAs<StarException>(std::move(worldClient), "CraftingPane", "world client")),
    m_player(requireServiceValueAs<StarException>(std::move(player), "CraftingPane", "player")),
    m_blueprints(m_player->blueprints()),
    m_assets(requireServiceValueAs<StarException>(std::move(services.assets), "CraftingPane", "assets")),
    m_configuration(requireServiceValueAs<StarException>(std::move(services.configuration), "CraftingPane", "configuration")),
    m_itemDatabase(requireServiceValueAs<StarException>(std::move(services.itemDatabase), "CraftingPane", "item database")),
    m_objectDatabase(requireServiceValueAs<StarException>(std::move(services.objectDatabase), "CraftingPane", "object database")),
    m_statusEffectDatabase(requireServiceValueAs<StarException>(std::move(services.statusEffectDatabase), "CraftingPane", "status effect database")),
    m_sourceEntityId(sourceEntityId) {
  // get the config data for this crafting pane, default to "bare hands" crafting
  auto baseConfig = settings.get("config", "/interface/windowconfig/crafting.config");
  m_settings = jsonMerge(m_assets->json("/interface/windowconfig/crafting.config:default"),
               jsonMerge(m_assets->fetchJson(baseConfig), settings));

  m_filter = StringSet::from(jsonToStringList(m_settings.get("filter", JsonArray())));
  m_maxSpinCount = m_settings.getUInt("maxSpinCount", 1000);

  GuiReader reader(context());
  reader.registerCallback("spinCount.up", [=, this](Widget*) {
      if (m_count < maxCraft())
        m_count++;
      else
        m_count = 1;
      countChanged();
    });

  reader.registerCallback("spinCount.down", [=, this](Widget*) {
      if (m_count > 1)
        m_count--;
      else
        m_count = std::max(maxCraft(), 1);
      countChanged();
    });

  reader.registerCallback("tbSpinCount", [=, this](Widget*) { countTextChanged(); });

  reader.registerCallback("close", [=, this](Widget*) { dismiss(); });

  reader.registerCallback("btnCraft", [=, this](Widget*) { toggleCraft(); });
  reader.registerCallback("btnStopCraft", [=, this](Widget*) { toggleCraft(); });

  reader.registerCallback("btnFilterHaveMaterials", [=, this](Widget*) {
      m_configuration->setPath("crafting.filterHaveMaterials", m_filterHaveMaterials->isChecked());
      updateAvailableRecipes();
    });

  reader.registerCallback("filter", [=, this](Widget*) { updateAvailableRecipes(); });

  reader.registerCallback("categories", [=, this](Widget*) { updateAvailableRecipes(); });

  reader.registerCallback("rarities", [=, this](Widget*) { updateAvailableRecipes(); });

  reader.registerCallback("btnUpgrade", [=, this](Widget*) { upgradeTable(); });

  // this is where the GUI gets built and the buttons begin to have existence.
  // all possible callbacks must exist by this point

  Json paneLayout = m_settings.get("paneLayout");
  paneLayout = jsonMerge(paneLayout, m_settings.get("paneLayoutOverride", {}));
  reader.construct(paneLayout, this);

  if (auto upgradeButton = fetchChild<ButtonWidget>("btnUpgrade")) {
    upgradeButton->disable();
    Maybe<JsonArray> recipeData = m_settings.optArray("upgradeMaterials");

    // create a recipe out of the listed upgrade materials.
    // for ease of creating a tooltip later.
    if (recipeData) {
      m_upgradeRecipe = ItemRecipe();
      for (auto ingredient : *recipeData)
        m_upgradeRecipe->inputs.append(ItemDescriptor(ingredient.getString("item"), ingredient.getUInt("count"), {}));
      upgradeButton->setVisibility(true);
    } else {
      upgradeButton->setVisibility(false);
    }
  }

  m_guiList = fetchChild<ListWidget>("scrollArea.itemList").get();
  m_textBox = fetchChild<TextBoxWidget>("tbSpinCount").get();

  m_filterHaveMaterials = fetchChild<ButtonWidget>("btnFilterHaveMaterials").get();
  if (m_filterHaveMaterials)
    m_filterHaveMaterials->setChecked(m_configuration->getPath("crafting.filterHaveMaterials").toBool());

  fetchChild<ButtonWidget>("btnCraft")->disable();
  if (auto spinCountUp = fetchChild<ButtonWidget>("spinCount.up"))
    spinCountUp->disable();
  if (auto spinCountDown = fetchChild<ButtonWidget>("spinCount.down"))
    spinCountDown->disable();

  m_displayedRecipe = NPos;
  updateAvailableRecipes();

  m_crafting = false;
  m_count = 1;
  countChanged();

  if (m_settings.getBool("titleFromEntity", false) && sourceEntityId != NullEntityId) {
    auto entity = m_worldClient->entity(sourceEntityId);

    if (auto container = as<ContainerEntity>(entity)) {
      if (container->iconItem()) {
        auto iconItem = m_itemDatabase->itemShared(container->iconItem());
        auto icon = make_shared<ItemSlotWidget>(context(), iconItem, "/interface/inventory/portrait.png");
        String title = this->title();
        if (title.empty())
          title = container->containerDescription();
        String subTitle = this->subTitle();
        if (subTitle.empty())
          subTitle = container->containerSubTitle();
        icon->showRarity(false);
        setTitle(icon, title, subTitle);
      }
    }
    if (auto portaitEntity = as<PortraitEntity>(entity)) {
      auto portrait = make_shared<PortraitWidget>(context(), portaitEntity, PortraitMode::Bust);
      portrait->setIconMode();
      String title = this->title();
      if (title.empty())
        title = portaitEntity->name();
      String subTitle = this->subTitle();
      setTitle(portrait, title, subTitle);
    }
  }
}

void CraftingPane::displayed() {
  Pane::displayed();

  if (auto filterWidget = fetchChild<TextBoxWidget>("filter")) {
    filterWidget->setText("");
    filterWidget->blur();
  }

  updateAvailableRecipes();

  // unlock any recipes specified
  if (auto recipeUnlocks = m_settings.opt("initialRecipeUnlocks")) {
    for (String itemName : jsonToStringList(*recipeUnlocks))
      m_player->addBlueprint(ItemDescriptor(itemName));
  }
}

void CraftingPane::dismissed() {
  stopCrafting();
  Pane::dismissed();
  m_itemCache.clear();
}

PanePtr CraftingPane::createTooltip(Vec2I const& screenPosition) {
  for (size_t i = 0; i < m_guiList->numChildren(); ++i) {
    auto entry = m_guiList->itemAt(i);
    if (entry->getChildAt(screenPosition)) {
      return setupTooltip(m_recipes[i]);
    }
  }

  if (auto child = getChildAt(screenPosition)) {
    if (child->name() == "btnUpgrade") {
      if (m_upgradeRecipe)
        return setupTooltip(*m_upgradeRecipe);
    }
  }

  return {};
}

EntityId CraftingPane::sourceEntityId() const {
  return m_sourceEntityId;
}

void CraftingPane::upgradeTable() {
  if (m_sourceEntityId != NullEntityId) {
    // Checks that the upgrade path exists
    if (m_upgradeRecipe) {
      if (m_player->isAdmin() || ItemDatabase::canMakeRecipe(*m_upgradeRecipe, m_player->inventory()->availableItems(), m_player->inventory()->availableCurrencies())) {
        if (!m_player->isAdmin())
          consumeIngredients(*m_upgradeRecipe, 1);

        // upgrade the old table
        m_worldClient->sendEntityMessage(m_sourceEntityId, "requestUpgrade");

        // unlock any recipes specified
        if (auto recipeUnlocks = m_settings.opt("upgradeRecipeUnlocks")) {
          for (String itemName : jsonToStringList(*recipeUnlocks))
            m_player->addBlueprint(ItemDescriptor(itemName));
        }

        // this closes the interface window
        dismiss();
      }
    }
  }
}

size_t CraftingPane::itemCount(List<ItemPtr> const& store, ItemDescriptor const& item) {
  return m_itemDatabase->getCountOfItem(store, item);
}

void CraftingPane::update(float dt) {
  // shut down if we can't reach the table anymore.
  if (m_sourceEntityId != NullEntityId) {
    auto sourceEntity = as<TileEntity>(m_worldClient->entity(m_sourceEntityId));
    if (!sourceEntity || !m_worldClient->playerCanReachEntity(m_sourceEntityId) || !sourceEntity->isInteractive()) {
      dismiss();
      return;
    }
  }

  // similarly if the player is dead
  if (m_player->isDead()) {
    dismiss();
    return;
  }

  // has the selected recipe changed ?
  bool changedHighlight = (m_displayedRecipe != m_guiList->selectedItem());

  if (changedHighlight) {
    stopCrafting(); // TODO: allow viewing other recipes without interrupting crafting

    m_displayedRecipe = m_guiList->selectedItem();
    countTextChanged();

    auto recipe = recipeFromSelectedWidget();

    if (recipe.isNull()) {
      fetchChild<Widget>("description")->removeAllChildren();
    } else {
      auto description = fetchChild<Widget>("description");
      description->removeAllChildren();

      auto item = m_itemDatabase->itemShared(recipe.output);
      ItemTooltipBuilder::buildItemDescription(description.get(), item, {m_assets, m_objectDatabase, m_statusEffectDatabase, context()});
    }
  }

  // crafters gonna craft
  while (m_crafting && m_craftTimer.wrapTick()) {
    craft(min(m_count, static_cast<int>(m_settings.getInt("craftCount", 1))));
  }

  // update crafting icon, progress and buttons
  if (auto currentRecipeIcon = fetchChild<ItemSlotWidget>("currentRecipeIcon")) {
    auto recipe = recipeFromSelectedWidget();
    if (recipe.isNull()) {
      currentRecipeIcon->setItem(nullptr);
    } else {
      auto single = recipe.output.singular();
      ItemPtr item = m_itemCache[single];
      currentRecipeIcon->setItem(item);

      if (m_crafting)
        currentRecipeIcon->setProgress(1.0f - m_craftTimer.percent());
      else
        currentRecipeIcon->setProgress(1.0f);
    }
  }

  --m_recipeAutorefreshCooldown;

  // changed recipe or auto update time
  if (changedHighlight || (m_recipeAutorefreshCooldown <= 0)) {
    updateAvailableRecipes();
    updateCraftButtons();
  }

  setLabel("lblPlayerMoney", toString(static_cast<int>(m_player->currency("money"))));

  Pane::update(dt);
}

void CraftingPane::updateCraftButtons() {
  auto normalizedBag = m_player->inventory()->availableItems();
  auto availableCurrencies = m_player->inventory()->availableCurrencies();

  auto recipe = recipeFromSelectedWidget();
  bool recipeAvailable = !recipe.isNull() && (m_player->isAdmin() || ItemDatabase::canMakeRecipe(recipe, normalizedBag, availableCurrencies));

  fetchChild<ButtonWidget>("btnCraft")->setEnabled(recipeAvailable);
  if (auto spinCountUp = fetchChild<ButtonWidget>("spinCount.up"))
    spinCountUp->setEnabled(recipeAvailable);
  if (auto spinCountDown = fetchChild<ButtonWidget>("spinCount.down"))
    spinCountDown->setEnabled(recipeAvailable);

  if (auto stopCraftButton = fetchChild<ButtonWidget>("btnStopCraft")) {
    stopCraftButton->setVisibility(m_crafting);
    fetchChild<ButtonWidget>("btnCraft")->setVisibility(!m_crafting);
  }

  if (auto upgradeButton = fetchChild<ButtonWidget>("btnUpgrade")) {
    bool canUpgrade = (m_upgradeRecipe && (m_player->isAdmin() || ItemDatabase::canMakeRecipe(*m_upgradeRecipe, normalizedBag, availableCurrencies)));
    upgradeButton->setEnabled(canUpgrade);
  }
}

void CraftingPane::updateAvailableRecipes() {
  m_recipeAutorefreshCooldown = 30;

  StringSet categoryFilter;
  if (auto categoriesGroup = fetchChild<ButtonGroupWidget>("categories")) {
    if (auto selectedCategories = categoriesGroup->checkedButton()) {
      for (auto group : selectedCategories->data().getArray("filter"))
        categoryFilter.add(group.toString());
    }
  }

  HashSet<Rarity> rarityFilter;
  if (auto raritiesGroup = fetchChild<ButtonGroupWidget>("rarities")) {
    if (auto selectedRarities = raritiesGroup->checkedButton()) {
      for (auto entry : jsonToStringSet(selectedRarities->data().getArray("rarity")))
        rarityFilter.add(RarityNames.getLeft(entry));
    }
  }

  String filterText;
  if (auto filterWidget = fetchChild<TextBoxWidget>("filter"))
    filterText = filterWidget->getText();

  m_recipes = determineRecipes();

  size_t currentOffset = 0;

  size_t selectedIdx = m_guiList->selectedItem();
  ItemRecipe selectedRecipe;
  if (selectedIdx != NPos)
    selectedRecipe = m_recipes[selectedIdx];

  HashMap<ItemDescriptor, uint64_t> normalizedBag = m_player->inventory()->availableItems();

  m_guiList->clear();

  for (auto const& recipe : m_recipes) {
    auto widget = m_guiList->addItem();
    setupWidget(widget, recipe, normalizedBag);

    if (selectedRecipe == recipe)
      m_guiList->setSelected(currentOffset);

    currentOffset++;
  }
}

void CraftingPane::setupWidget(WidgetRef<Widget> const& widget, ItemRecipe const& recipe, HashMap<ItemDescriptor, uint64_t> const& normalizedBag) {
  auto single = recipe.output.singular();
  ItemPtr item = m_itemCache[single];
  if (!item) {
    item = m_itemDatabase->itemShared(single);
    m_itemCache[single] = item;
  }

  bool unavailable = false;
  size_t price = recipe.currencyInputs.value("money", 0);

  if (!m_player->isAdmin()) {
    for (auto const& [currencyName, currencyCount] : recipe.currencyInputs) {
      if (m_player->currency(currencyName) < currencyCount)
        unavailable = true;
    }

    for (auto const& input : recipe.inputs) {
      if (m_itemDatabase->getCountOfItem(normalizedBag, input, recipe.matchInputParameters) < input.count())
        unavailable = true;
    }
  }

  String name = item->friendlyName();
  if (recipe.output.count() > 1)
    name = strf("{} (x{})", name, recipe.output.count());

  auto itemName = widget->fetchChild<LabelWidget>("itemName");
  auto notcraftableoverlay = widget->fetchChild<ImageWidget>("notcraftableoverlay");

  itemName->setText(name);

  if (unavailable) {
    itemName->setColor(Color::Gray);
    notcraftableoverlay->show();
  } else {
    itemName->setColor(Color::White);
    notcraftableoverlay->hide();
  }

  if (price > 0) {
    widget->setLabel("priceLabel", toString(price));
    if (auto icon = widget->fetchChild<ImageWidget>("moneyIcon"))
      icon->setVisibility(true);
  } else {
    widget->setLabel("priceLabel", "");
    if (auto icon = widget->fetchChild<ImageWidget>("moneyIcon"))
      icon->setVisibility(false);
  }

  if (auto newIndicator = widget->fetchChild<ImageWidget>("newIcon")) {
    if (m_blueprints->isNew(recipe.output.singular())) {
      newIndicator->show();
      widget->setLabel("priceLabel", "");
      if (auto icon = widget->fetchChild<ImageWidget>("moneyIcon"))
        icon->setVisibility(false);
    } else {
      newIndicator->hide();
    }
  }

  widget->fetchChild<ItemSlotWidget>("itemIcon")->setItem(item);
  widget->show();
}

PanePtr CraftingPane::setupTooltip(ItemRecipe const& recipe) {
  auto tooltip = make_shared<Pane>(context());
  GuiReader reader(context());
  reader.construct(m_assets->json("/interface/craftingtooltip/craftingtooltip.config"), tooltip.get());

  auto guiList = tooltip->fetchChild<ListWidget>("itemList");
  guiList->clear();

  auto normalizedBag = m_player->inventory()->availableItems();

  auto addIngredient = [guiList](ItemPtr const& item, size_t availableCount, size_t requiredCount) {
      auto widget = guiList->addItem();
      widget->fetchChild<LabelWidget>("itemName")->setText(item->friendlyName());
      auto countWidget = widget->fetchChild<LabelWidget>("count");
      countWidget->setText(strf("{}/{}", availableCount, requiredCount));
      if (availableCount < requiredCount)
        countWidget->setColor(Color::Red);
      else
        countWidget->setColor(Color::Green);
      widget->fetchChild<ItemSlotWidget>("itemIcon")->setItem(item);
      widget->show();
    };

  auto currenciesConfig = m_assets->json("/currencies.config");
  for (auto const& [currencyName, currencyCount] : recipe.currencyInputs) {
    if (currencyCount > 0) {
      auto currencyItem = m_itemDatabase->itemShared(ItemDescriptor(currenciesConfig.get(currencyName).getString("representativeItem")));
      addIngredient(currencyItem, m_player->currency(currencyName), currencyCount);
    }
  }

  for (auto const& input : recipe.inputs) {
    auto item = m_itemDatabase->itemShared(input.singular());
    size_t itemCount = m_itemDatabase->getCountOfItem(normalizedBag, input, recipe.matchInputParameters);
    addIngredient(item, itemCount, input.count());
  }

  auto background = tooltip->fetchChild<ImageStretchWidget>("background");
  background->setSize(background->size() + Vec2I(0, guiList->size()[1]));

  auto title = tooltip->fetchChild<LabelWidget>("title");
  title->setPosition(title->position() + Vec2I(0, guiList->size()[1]));

  tooltip->setSize(background->size());

  return tooltip;
}

bool CraftingPane::consumeIngredients(ItemRecipe& recipe, int count) {
  auto normalizedBag = m_player->inventory()->availableItems();
  auto availableCurrencies = m_player->inventory()->availableCurrencies();

  // make sure we still have the currencies and items avaialable
  for (auto const& [currencyName, currencyCount] : recipe.currencyInputs) {
    uint64_t countRequired = currencyCount * count;
    if (availableCurrencies.value(currencyName) < countRequired) {
      updateAvailableRecipes();
      return false;
    }
  }
  for (auto input : recipe.inputs) {
    size_t countRequired = input.count() * count;
    if (m_itemDatabase->getCountOfItem(normalizedBag, input, recipe.matchInputParameters) < countRequired) {
      updateAvailableRecipes();
      return false;
    }
  }

  // actually consume the currencies and items
  for (auto const& [currencyName, currencyCount] : recipe.currencyInputs) {
    if (currencyCount > 0)
      m_player->inventory()->consumeCurrency(currencyName, currencyCount * count);
  }
  for (auto input : recipe.inputs) {
    if (count > 0)
      m_player->inventory()->consumeItems(ItemDescriptor(input.name(), input.count() * count, input.parameters()), recipe.matchInputParameters);
  }
  return true;
}

void CraftingPane::stopCrafting() {
  if (m_craftingSound)
    m_craftingSound->stop();
  m_crafting = false;
}

void CraftingPane::toggleCraft() {
  if (m_crafting) {
    stopCrafting();
  } else {
    auto recipe = recipeFromSelectedWidget();
    if (recipe.duration > 0 && !m_settings.getBool("disableTimer", false)) {
      m_crafting = true;
      m_craftTimer = GameTimer(recipe.duration);

      if (auto craftingSound = m_settings.optString("craftingSound")) {
        m_craftingSound = make_shared<AudioInstance>(*m_assets->audio(*craftingSound));
        m_craftingSound->setLoops(-1);
        context().playAudio(m_craftingSound);
      }
    } else {
      craft(m_count);
    }
  }
}

void CraftingPane::craft(int count) {
  if (m_guiList->selectedItem() != NPos) {
    auto recipe = recipeFromSelectedWidget();

    if (!m_player->isAdmin() && !consumeIngredients(recipe, count)) {
      stopCrafting();
      return;
    }

    ItemDescriptor itemDescriptor = recipe.output;
    int remainingItemCount = itemDescriptor.count() * count;
    while (remainingItemCount > 0) {
      auto craftedItem = m_itemDatabase->item(itemDescriptor.singular().multiply(remainingItemCount));
      remainingItemCount -= craftedItem->count();
      m_player->giveItem(craftedItem);

      for (auto const& [collectableName, collectableAmount] : recipe.collectables)
        m_player->addCollectable(collectableName, collectableAmount);
    }

    m_blueprints->markAsRead(recipe.output.singular());
  }

  updateAvailableRecipes();

  m_count -= count;
  if (m_count <= 0) {
    m_count = 1;
    stopCrafting();
  }
  countChanged();

  updateCraftButtons();
}

void CraftingPane::countTextChanged() {
  if (m_textBox) {
    int appropriateDefaultCount = 1;
    try {
      if (!m_textBox->getText().replace("x", "").size()) {
        m_count = appropriateDefaultCount;
      } else {
        m_count = clamp<int>(lexicalCast<int>(m_textBox->getText().replace("x", "")), appropriateDefaultCount, maxCraft());
        countChanged();
      }
    } catch (BadLexicalCast const&) {
      m_count = appropriateDefaultCount;
      countChanged();
    }
  } else {
    m_count = 1;
  }
}

void CraftingPane::countChanged() {
  if (m_textBox)
    m_textBox->setText(strf("x{}", m_count), false);
}

List<ItemRecipe> CraftingPane::determineRecipes() {
  HashSet<ItemRecipe> recipes;

  StringSet categoryFilter;
  if (auto categoriesGroup = fetchChild<ButtonGroupWidget>("categories")) {
    if (auto selectedCategories = categoriesGroup->checkedButton()) {
      for (auto group : selectedCategories->data().getArray("filter"))
        categoryFilter.add(group.toString());
    }
  }

  HashSet<Rarity> rarityFilter;
  if (auto raritiesGroup = fetchChild<ButtonGroupWidget>("rarities")) {
    if (auto selectedRarities = raritiesGroup->checkedButton()) {
      for (auto entry : jsonToStringSet(selectedRarities->data().getArray("rarity")))
        rarityFilter.add(RarityNames.getLeft(entry));
    }
  }

  String filterText;
  if (auto filterWidget = fetchChild<TextBoxWidget>("filter"))
    filterText = filterWidget->getText();

  bool filterHaveMaterials = false;
  if (m_filterHaveMaterials)
    filterHaveMaterials = m_filterHaveMaterials->isChecked();

  if (m_settings.getBool("printer", false)) {
    StringList itemList;
    if (m_player->isAdmin())
      itemList = m_objectDatabase->allObjects();
    else
      itemList = StringList::from(m_player->log()->scannedObjects());

    filter(itemList, [objectDatabase = m_objectDatabase, itemDatabase = m_itemDatabase](String const& itemName) {
        if (objectDatabase->isObject(itemName)) {
          if (auto objectConfig = objectDatabase->getConfig(itemName))
            return objectConfig->printable && itemDatabase->hasItem(itemName);
        }
        return false;
      });

    float printTime = m_settings.getFloat("printTime", 0);
    float printFactor = m_settings.getFloat("printCostFactor", 1.0);
    for (auto& itemName : itemList) {
      ItemRecipe recipe;
      recipe.output = ItemDescriptor(itemName, 1);
      auto recipeItem = m_itemDatabase->itemShared(recipe.output);
      int itemPrice = int(recipeItem->price() * printFactor);
      recipe.currencyInputs["money"] = itemPrice;
      recipe.outputRarity = recipeItem->rarity();
      recipe.duration = printTime;
      recipe.guiFilterString = ItemDatabase::guiFilterString(recipeItem);
      recipe.groups = StringSet{m_objectDatabase->getConfig(itemName)->category};
      recipes.add(recipe);
    }
  } else if (m_settings.contains("recipes")) {
    for (auto& entry : m_settings.getArray("recipes")) {
      if (entry.type() == Json::Type::String)
        recipes.addAll(m_itemDatabase->recipesForOutputItem(entry.toString()));
      else
        recipes.add(m_itemDatabase->parseRecipe(entry));
    }

    if (filterHaveMaterials)
      recipes.addAll(m_itemDatabase->recipesFromSubset(m_player->inventory()->availableItems(), m_player->inventory()->availableCurrencies(), take(recipes), m_filter));
  } else {
    if (filterHaveMaterials)
      recipes.addAll(m_itemDatabase->recipesFromBagContents(m_player->inventory()->availableItems(), m_player->inventory()->availableCurrencies(), m_filter));
    else
      recipes.addAll(m_itemDatabase->allRecipes(m_filter));
  }

  if (!m_player->isAdmin() && m_settings.getBool("requiresBlueprint", true)) {
    auto tempRecipes = take(recipes);
    for (auto const& recipe : tempRecipes) {
      if (m_blueprints->isKnown(recipe.output))
        recipes.add(recipe);
    }
  }

  if (!categoryFilter.empty()) {
    auto temprecipes = take(recipes);
    for (auto const& recipe : temprecipes) {
      if (recipe.groups.hasIntersection(categoryFilter))
        recipes.add(recipe);
    }
  }

  if (!rarityFilter.empty()) {
    auto temprecipes = take(recipes);
    for (auto const& recipe : temprecipes) {
      if (recipe.output) {
        if (rarityFilter.contains(recipe.outputRarity))
          recipes.add(recipe);
      }
    }
  }

  if (!filterText.empty()) {
    auto bits = filterText.toLower().splitAny(" ,.?*\\+/|\t");
    auto temprecipes = take(recipes);
    for (auto const& recipe : temprecipes) {
      if (recipe.output) {
        bool match = true;
        auto guiFilterString = recipe.guiFilterString;
        for (auto const& bit : bits) {
          match &= guiFilterString.contains(bit);
          if (!match)
            break;
        }
        if (match)
          recipes.add(recipe);
      }
    }
  }

  List<ItemRecipe> sortedRecipes = recipes.values();
  sortByComputedValue(sortedRecipes, [itemDatabase = m_itemDatabase](ItemRecipe const& recipe) {
      return tuple<String, String>{itemDatabase->itemFriendlyName(recipe.output.name()).trim().toLower(), recipe.output.name()};
    });

  return sortedRecipes;
}

int CraftingPane::maxCraft() {
  if (m_player->isAdmin())
    return m_maxSpinCount;
  int res = 0;
  if (m_guiList->selectedItem() != NPos && m_guiList->selectedItem() < m_recipes.size()) {
    HashMap<ItemDescriptor, uint64_t> normalizedBag = m_player->inventory()->availableItems();
    auto selectedRecipe = recipeFromSelectedWidget();
    res = m_itemDatabase->maxCraftableInBag(normalizedBag, m_player->inventory()->availableCurrencies(), selectedRecipe);
    res = std::min(res, m_maxSpinCount);
  }
  return res;
}

ItemRecipe CraftingPane::recipeFromSelectedWidget() const {
  auto idx = m_guiList->selectedItem();
  if (idx != NPos)
    return m_recipes[idx];
  return ItemRecipe();
}

}
