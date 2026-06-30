#pragma once

#include "StarAssets.hpp"
#include "StarConfiguration.hpp"
#include "StarInventoryTypes.hpp"
#include "StarMainInterfaceTypes.hpp"

namespace Star {

class Player;
using PlayerPtr = SharedPtr<Player>;
class Item;
using ItemPtr = SharedPtr<Item>;
class ItemSlotWidget;
using ItemSlotWidgetPtr = SharedPtr<ItemSlotWidget>;
class ImageWidget;
using ImageWidgetPtr = SharedPtr<ImageWidget>;

class ActionBar;
using ActionBarPtr = SharedPtr<ActionBar>;
class ObjectDatabase;
using ObjectDatabaseConstPtr = SharedPtr<ObjectDatabase const>;
class StatusEffectDatabase;
using StatusEffectDatabaseConstPtr = SharedPtr<StatusEffectDatabase const>;

struct ActionBarServices {
  AssetsConstPtr assets;
  ConfigurationPtr configuration;
  ObjectDatabaseConstPtr objectDatabase;
  StatusEffectDatabaseConstPtr statusEffectDatabase;
  GuiContext& guiContext;
};

class ActionBar : public Pane {
public:
  ActionBar(MainInterfacePaneManager& paneManager, PlayerPtr player, ActionBarServices services);

  [[nodiscard]] PanePtr createTooltip(Vec2I const& screenPosition) override;
  [[nodiscard]] bool sendEvent(InputEvent const& event) override;

  void update(float dt) override;

  [[nodiscard]] Maybe<String> cursorOverride(Vec2I const& screenPosition) override;

private:
  struct CustomBarEntry {
    WidgetRef<ItemSlotWidget> left;
    WidgetRef<ItemSlotWidget> right;
    WidgetRef<ImageWidget> leftOverlay;
    WidgetRef<ImageWidget> rightOverlay;
  };

  struct CustomBarHover {
    CustomBarIndex index;
    bool secondary;
  };

  void customBarClick(uint8_t index, bool primary);
  void customBarClickRight(uint8_t index, bool primary);
  void essentialBarClick(uint8_t index);
  void swapCustomBar();

  MainInterfacePaneManager& m_paneManager;
  PlayerPtr m_player;
  AssetsConstPtr m_assets;
  ConfigurationPtr m_configuration;
  ObjectDatabaseConstPtr m_objectDatabase;
  StatusEffectDatabaseConstPtr m_statusEffectDatabase;
  Json m_config;

  Vec2I m_actionBarSelectOffset;
  StringList m_switchSounds;

  List<CustomBarEntry> m_customBarWidgets;
  WidgetRef<ImageWidget> m_customSelectedWidget;

  List<WidgetRef<ItemSlotWidget>> m_essentialBarWidgets;
  WidgetRef<ImageWidget> m_essentialSelectedWidget;

  SelectedActionBarLocation m_emptyHandsPreviousActionBarLocation;
  Maybe<CustomBarHover> m_customBarHover;
};

}
