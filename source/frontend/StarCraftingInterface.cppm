module;
import std;
import star.json;

import star.data_stream;
import star.bi_map;
import star.game_types;
import star.vector;
#include "StarCasting.hpp"
import star.input_event;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.string;
import star.text;
import star.asset_path;
import star.maybe;
import star.listener;
import star.set;
import star.audio;
import star.map;


import star.item_descriptor;
import star.item_recipe;
import star.font_texture_group;
import star.anchor_types;
import star.text_painter;
import star.drawable;
import star.asset_texture_group;
import star.drawable_painter;
import star.gui_types;
import star.key_bindings;
import star.mixer;
import star.gui_context;
import star.widget;
import star.pane;
import star.game_timers;

namespace Star {
class Player;
using PlayerPtr = std::shared_ptr<Player>;
using PlayerConstPtr = std::shared_ptr<Player const>;
using PlayerWeakPtr = std::weak_ptr<Player>;
using PlayerConstWeakPtr = std::weak_ptr<Player const>;
using PlayerUPtr = std::unique_ptr<Player>;
using PlayerConstUPtr = std::unique_ptr<Player const>;
}

namespace Star {

class WorldClient;
using WorldClientPtr = std::shared_ptr<WorldClient>;
using WorldClientConstPtr = std::shared_ptr<WorldClient const>;
using WorldClientWeakPtr = std::weak_ptr<WorldClient>;
using WorldClientConstWeakPtr = std::weak_ptr<WorldClient const>;
using WorldClientUPtr = std::unique_ptr<WorldClient>;
using WorldClientConstUPtr = std::unique_ptr<WorldClient const>;
class PlayerBlueprints;
using PlayerBlueprintsPtr = std::shared_ptr<PlayerBlueprints>;
using PlayerBlueprintsConstPtr = std::shared_ptr<PlayerBlueprints const>;
using PlayerBlueprintsWeakPtr = std::weak_ptr<PlayerBlueprints>;
using PlayerBlueprintsConstWeakPtr = std::weak_ptr<PlayerBlueprints const>;
using PlayerBlueprintsUPtr = std::unique_ptr<PlayerBlueprints>;
using PlayerBlueprintsConstUPtr = std::unique_ptr<PlayerBlueprints const>;
class ListWidget;
using ListWidgetPtr = std::shared_ptr<ListWidget>;
using ListWidgetConstPtr = std::shared_ptr<ListWidget const>;
using ListWidgetWeakPtr = std::weak_ptr<ListWidget>;
using ListWidgetConstWeakPtr = std::weak_ptr<ListWidget const>;
using ListWidgetUPtr = std::unique_ptr<ListWidget>;
using ListWidgetConstUPtr = std::unique_ptr<ListWidget const>;
class TextBoxWidget;
using TextBoxWidgetPtr = std::shared_ptr<TextBoxWidget>;
using TextBoxWidgetConstPtr = std::shared_ptr<TextBoxWidget const>;
using TextBoxWidgetWeakPtr = std::weak_ptr<TextBoxWidget>;
using TextBoxWidgetConstWeakPtr = std::weak_ptr<TextBoxWidget const>;
using TextBoxWidgetUPtr = std::unique_ptr<TextBoxWidget>;
using TextBoxWidgetConstUPtr = std::unique_ptr<TextBoxWidget const>;
class ButtonWidget;
using ButtonWidgetPtr = std::shared_ptr<ButtonWidget>;
using ButtonWidgetConstPtr = std::shared_ptr<ButtonWidget const>;
using ButtonWidgetWeakPtr = std::weak_ptr<ButtonWidget>;
using ButtonWidgetConstWeakPtr = std::weak_ptr<ButtonWidget const>;
using ButtonWidgetUPtr = std::unique_ptr<ButtonWidget>;
using ButtonWidgetConstUPtr = std::unique_ptr<ButtonWidget const>;
class LabelWidget;
using LabelWidgetPtr = std::shared_ptr<LabelWidget>;
using LabelWidgetConstPtr = std::shared_ptr<LabelWidget const>;
using LabelWidgetWeakPtr = std::weak_ptr<LabelWidget>;
using LabelWidgetConstWeakPtr = std::weak_ptr<LabelWidget const>;
using LabelWidgetUPtr = std::unique_ptr<LabelWidget>;
using LabelWidgetConstUPtr = std::unique_ptr<LabelWidget const>;
class AudioInstance;
using AudioInstancePtr = std::shared_ptr<AudioInstance>;
using AudioInstanceConstPtr = std::shared_ptr<AudioInstance const>;
using AudioInstanceWeakPtr = std::weak_ptr<AudioInstance>;
using AudioInstanceConstWeakPtr = std::weak_ptr<AudioInstance const>;
using AudioInstanceUPtr = std::unique_ptr<AudioInstance>;
using AudioInstanceConstUPtr = std::unique_ptr<AudioInstance const>;

class CraftingPane;
using CraftingPanePtr = std::shared_ptr<CraftingPane>;
using CraftingPaneConstPtr = std::shared_ptr<CraftingPane const>;
using CraftingPaneWeakPtr = std::weak_ptr<CraftingPane>;
using CraftingPaneConstWeakPtr = std::weak_ptr<CraftingPane const>;
using CraftingPaneUPtr = std::unique_ptr<CraftingPane>;
using CraftingPaneConstUPtr = std::unique_ptr<CraftingPane const>;

class CraftingPane : public Pane {
public:
  CraftingPane(
      WorldClientPtr worldClient, PlayerPtr player, Json const& settings, EntityId sourceEntityId = NullEntityId);

  void displayed() override;
  void dismissed() override;
  PanePtr createTooltip(Vec2I const& screenPosition) override;

  EntityId sourceEntityId() const;

private:
  void upgradeTable();

  List<ItemRecipe> determineRecipes();

  virtual void update(float dt) override;
  void updateCraftButtons();
  void updateAvailableRecipes();
  bool consumeIngredients(ItemRecipe& recipe, int count);
  void stopCrafting();
  void toggleCraft();
  void craft(int count);
  void countChanged();
  void countTextChanged();
  int maxCraft();
  void setupList(WidgetPtr widget, ItemRecipe const& recipe);
  ItemRecipe recipeFromSelectedWidget() const;
  void setupWidget(WidgetPtr const& widget, ItemRecipe const& recipe, HashMap<ItemDescriptor, std::uint64_t> const& normalizedBag);

  PanePtr setupTooltip(ItemRecipe const& recipe);

  std::size_t itemCount(List<ItemPtr> const& store, ItemDescriptor const& item);

  WorldClientPtr m_worldClient;
  PlayerPtr m_player;
  PlayerBlueprintsPtr m_blueprints;

  bool m_crafting;
  GameTimer m_craftTimer;
  AudioInstancePtr m_craftingSound;
  int m_count;
  List<ItemRecipe> m_recipes;
  BiHashMap<ItemRecipe, WidgetPtr> m_recipesWidgetMap; // maps ItemRecipe to guiList WidgetPtrs

  ListWidgetPtr m_guiList;
  TextBoxWidgetPtr m_textBox;
  ButtonWidgetPtr m_filterHaveMaterials;
  std::size_t m_displayedRecipe;

  StringSet m_filter;

  int m_maxSpinCount;

  int m_recipeAutorefreshCooldown;

  HashMap<ItemDescriptor, ItemPtr> m_itemCache;

  EntityId m_sourceEntityId;
  Json m_settings;

  Maybe<ItemRecipe> m_upgradeRecipe;
};

}

export module star.crafting_interface;

export namespace Star {
  using ::Star::WorldClient;
  using ::Star::WorldClientPtr;
  using ::Star::WorldClientConstPtr;
  using ::Star::WorldClientWeakPtr;
  using ::Star::WorldClientConstWeakPtr;
  using ::Star::WorldClientUPtr;
  using ::Star::WorldClientConstUPtr;
  using ::Star::PlayerBlueprints;
  using ::Star::PlayerBlueprintsPtr;
  using ::Star::PlayerBlueprintsConstPtr;
  using ::Star::PlayerBlueprintsWeakPtr;
  using ::Star::PlayerBlueprintsConstWeakPtr;
  using ::Star::PlayerBlueprintsUPtr;
  using ::Star::PlayerBlueprintsConstUPtr;
  using ::Star::ListWidget;
  using ::Star::ListWidgetPtr;
  using ::Star::ListWidgetConstPtr;
  using ::Star::ListWidgetWeakPtr;
  using ::Star::ListWidgetConstWeakPtr;
  using ::Star::ListWidgetUPtr;
  using ::Star::ListWidgetConstUPtr;
  using ::Star::TextBoxWidget;
  using ::Star::TextBoxWidgetPtr;
  using ::Star::TextBoxWidgetConstPtr;
  using ::Star::TextBoxWidgetWeakPtr;
  using ::Star::TextBoxWidgetConstWeakPtr;
  using ::Star::TextBoxWidgetUPtr;
  using ::Star::TextBoxWidgetConstUPtr;
  using ::Star::ButtonWidget;
  using ::Star::ButtonWidgetPtr;
  using ::Star::ButtonWidgetConstPtr;
  using ::Star::ButtonWidgetWeakPtr;
  using ::Star::ButtonWidgetConstWeakPtr;
  using ::Star::ButtonWidgetUPtr;
  using ::Star::ButtonWidgetConstUPtr;
  using ::Star::LabelWidget;
  using ::Star::LabelWidgetPtr;
  using ::Star::LabelWidgetConstPtr;
  using ::Star::LabelWidgetWeakPtr;
  using ::Star::LabelWidgetConstWeakPtr;
  using ::Star::LabelWidgetUPtr;
  using ::Star::LabelWidgetConstUPtr;
  using ::Star::AudioInstance;
  using ::Star::AudioInstancePtr;
  using ::Star::AudioInstanceConstPtr;
  using ::Star::AudioInstanceWeakPtr;
  using ::Star::AudioInstanceConstWeakPtr;
  using ::Star::AudioInstanceUPtr;
  using ::Star::AudioInstanceConstUPtr;
  using ::Star::CraftingPane;
  using ::Star::CraftingPanePtr;
  using ::Star::CraftingPaneConstPtr;
  using ::Star::CraftingPaneWeakPtr;
  using ::Star::CraftingPaneConstWeakPtr;
  using ::Star::CraftingPaneUPtr;
  using ::Star::CraftingPaneConstUPtr;
}
