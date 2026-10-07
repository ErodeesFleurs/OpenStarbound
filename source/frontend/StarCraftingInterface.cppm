module;

#include "StarJson.hpp"
#include "StarDataStream.hpp"
#include "StarBiMap.hpp"
#include "StarGameTypes.hpp"
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
#include "StarStringView.hpp"
#include "StarString.hpp"
import star.text;
#include "StarString.hpp"
#include "StarPoly.hpp"
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
#include "StarThread.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"


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
STAR_CLASS(Player);
}

namespace Star {

STAR_CLASS(WorldClient);
STAR_CLASS(PlayerBlueprints);
STAR_CLASS(ListWidget);
STAR_CLASS(TextBoxWidget);
STAR_CLASS(ButtonWidget);
STAR_CLASS(LabelWidget);
STAR_CLASS(AudioInstance);

STAR_CLASS(CraftingPane);

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
  void setupWidget(WidgetPtr const& widget, ItemRecipe const& recipe, HashMap<ItemDescriptor, uint64_t> const& normalizedBag);

  PanePtr setupTooltip(ItemRecipe const& recipe);

  size_t itemCount(List<ItemPtr> const& store, ItemDescriptor const& item);

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
  size_t m_displayedRecipe;

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
