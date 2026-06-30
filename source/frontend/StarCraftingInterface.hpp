#pragma once

#include "StarAssets.hpp"
#include "StarConfiguration.hpp"
#include "StarItemDatabase.hpp"
#include "StarItemRecipe.hpp"
#include "StarObjectDatabase.hpp"
#include "StarObserverPtr.hpp"
#include "StarPane.hpp"
#include "StarStatusEffectDatabase.hpp"
#include "StarWorldClient.hpp"
#include "StarWorldPainter.hpp"

namespace Star {

class WorldClient;
using WorldClientPtr = SharedPtr<WorldClient>;
class Player;
using PlayerPtr = SharedPtr<Player>;
class PlayerBlueprints;
using PlayerBlueprintsPtr = SharedPtr<PlayerBlueprints>;
class ListWidget;
using ListWidgetPtr = SharedPtr<ListWidget>;
class TextBoxWidget;
using TextBoxWidgetPtr = SharedPtr<TextBoxWidget>;
class ButtonWidget;
using ButtonWidgetPtr = SharedPtr<ButtonWidget>;
class AudioInstance;
using AudioInstancePtr = SharedPtr<AudioInstance>;

class CraftingPane;
using CraftingPanePtr = SharedPtr<CraftingPane>;

struct CraftingPaneServices {
  AssetsConstPtr assets;
  ConfigurationPtr configuration;
  ItemDatabaseConstPtr itemDatabase;
  ObjectDatabaseConstPtr objectDatabase;
  StatusEffectDatabaseConstPtr statusEffectDatabase;
  GuiContext& guiContext;
};

class CraftingPane : public Pane {
public:
  CraftingPane(
    WorldClientPtr worldClient,
    PlayerPtr player,
    Json const& settings,
    EntityId sourceEntityId,
    CraftingPaneServices services);

  void displayed() override;
  void dismissed() override;
  [[nodiscard]] UniquePtr<Pane> createTooltip(Vec2I const& screenPosition) override;

  [[nodiscard]] EntityId sourceEntityId() const;

private:
  void upgradeTable();

  [[nodiscard]] List<ItemRecipe> determineRecipes();

  void update(float dt) override;
  void updateCraftButtons();
  void updateAvailableRecipes();
  [[nodiscard]] bool consumeIngredients(ItemRecipe& recipe, int count);
  void stopCrafting();
  void toggleCraft();
  void craft(int count);
  void countChanged();
  void countTextChanged();
  [[nodiscard]] int maxCraft();
  [[nodiscard]] ItemRecipe recipeFromSelectedWidget() const;
  void setupWidget(WidgetRef<Widget> const& widget, ItemRecipe const& recipe, HashMap<ItemDescriptor, uint64_t> const& normalizedBag);

  [[nodiscard]] UniquePtr<Pane> setupTooltip(ItemRecipe const& recipe);

  WorldClientPtr m_worldClient;
  PlayerPtr m_player;
  PlayerBlueprintsPtr m_blueprints;
  AssetsConstPtr m_assets;
  ConfigurationPtr m_configuration;
  ItemDatabaseConstPtr m_itemDatabase;
  ObjectDatabaseConstPtr m_objectDatabase;
  StatusEffectDatabaseConstPtr m_statusEffectDatabase;

  bool m_crafting;
  GameTimer m_craftTimer;
  AudioInstancePtr m_craftingSound;
  int m_count;
  List<ItemRecipe> m_recipes;

  observer_ptr<ListWidget> m_guiList;
  observer_ptr<TextBoxWidget> m_textBox;
  observer_ptr<ButtonWidget> m_filterHaveMaterials;
  size_t m_displayedRecipe;

  StringSet m_filter;

  int m_maxSpinCount;

  int m_recipeAutorefreshCooldown = 0;

  HashMap<ItemDescriptor, ItemPtr> m_itemCache;

  EntityId m_sourceEntityId;
  Json m_settings;

  Maybe<ItemRecipe> m_upgradeRecipe;
};

}// namespace Star
