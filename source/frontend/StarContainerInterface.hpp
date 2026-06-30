#pragma once

#include "StarPane.hpp"
#include "StarLuaComponents.hpp"
#include "StarContainerInteractor.hpp"
#include "StarGuiReader.hpp"
#include "StarIAssets.hpp"
#include "StarIItemDatabase.hpp"

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

struct ContainerPaneServices {
  IItemDatabaseConstPtr itemDatabase;
  IAssetsConstPtr assets;
};

class ContainerPane : public Pane {
public:
  ContainerPane(WorldClientPtr worldClient, PlayerPtr player, ContainerInteractorPtr containerInteractor, ContainerPaneServices services = {});

  void displayed() override;
  void dismissed() override;
  PanePtr createTooltip(Vec2I const& screenPosition) override;

  bool giveContainerResult(ContainerResult result);

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
  IItemDatabaseConstPtr m_itemDatabase;
  IAssetsConstPtr m_assets;
  ItemBagPtr m_itemBag;

  ExpectingSwap m_expectingSwap;

  GuiReader m_reader;

  Maybe<LuaWorldComponent<LuaUpdatableComponent<LuaBaseComponent>>> m_script;
};

}
