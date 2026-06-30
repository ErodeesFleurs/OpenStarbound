#pragma once

#include "StarPane.hpp"
#include "StarLuaComponents.hpp"
#include "StarContainerInteractor.hpp"
#include "StarGuiReader.hpp"
#include "StarAssets.hpp"
#include "StarItemDatabase.hpp"

namespace Star {

class Player;
using PlayerPtr = SharedPtr<Player>;
class WorldClient;
using WorldClientPtr = SharedPtr<WorldClient>;
class ItemGridWidget;
class ItemBag;
using ItemBagPtr = SharedPtr<ItemBag>;
class ContainerPane;
using ContainerPanePtr = SharedPtr<ContainerPane>;
class ObjectDatabase;
using ObjectDatabaseConstPtr = SharedPtr<ObjectDatabase const>;
class StatusEffectDatabase;
using StatusEffectDatabaseConstPtr = SharedPtr<StatusEffectDatabase const>;

struct ContainerPaneServices {
  ItemDatabaseConstPtr itemDatabase;
  AssetsConstPtr assets;
  ObjectDatabaseConstPtr objectDatabase;
  StatusEffectDatabaseConstPtr statusEffectDatabase;
  function<bool()> takeAllPressed;
  GuiContext& guiContext;
};

class ContainerPane : public Pane {
public:
  ContainerPane(WorldClientPtr worldClient, PlayerPtr player, ContainerInteractorPtr containerInteractor, ContainerPaneServices services);

  void displayed() override;
  void dismissed() override;
  [[nodiscard]] PanePtr createTooltip(Vec2I const& screenPosition) override;

  [[nodiscard]] bool giveContainerResult(ContainerResult result);

protected:
  void update(float dt) override;

private:
  enum class ExpectingSwap {
    None,
    Inventory,
    SwapSlot,
    SwapSlotStack
  };

  void swapSlot(ItemGridWidget* grid);
  void startCrafting();
  void stopCrafting();
  void toggleCrafting();
  void clear();
  void burn();

  WorldClientPtr m_worldClient;
  PlayerPtr m_player;
  ContainerInteractorPtr m_containerInteractor;
  ItemDatabaseConstPtr m_itemDatabase;
  AssetsConstPtr m_assets;
  ObjectDatabaseConstPtr m_objectDatabase;
  StatusEffectDatabaseConstPtr m_statusEffectDatabase;
  function<bool()> m_takeAllPressed;
  ItemBagPtr m_itemBag;

  ExpectingSwap m_expectingSwap;

  GuiReader m_reader;

  Maybe<LuaWorldComponent<LuaUpdatableComponent<LuaBaseComponent>>> m_script;
};

}
