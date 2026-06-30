#pragma once

#include "StarQuestManager.hpp"
#include "StarQuests.hpp"
#include "StarPane.hpp"
#include "StarIAssets.hpp"

namespace Star {

class QuestManager;
class Player;
using PlayerPtr = SharedPtr<Player>;
class Cinematic;
using CinematicPtr = SharedPtr<Cinematic>;
class UniverseClient;
using UniverseClientPtr = SharedPtr<UniverseClient>;
class PaneManager;
class ItemBag;
using ItemBagPtr = SharedPtr<ItemBag>;

struct QuestInterfaceServices {
  IAssetsConstPtr assets;
};

class QuestLogInterface : public Pane {
public:
  QuestLogInterface(QuestManagerPtr manager, PlayerPtr player, CinematicPtr cinematic, UniverseClientPtr client, QuestInterfaceServices services = {});
  virtual ~QuestLogInterface() = default;

  virtual void displayed() override;
  virtual void tick(float dt) override;
  virtual PanePtr createTooltip(Vec2I const& screenPosition) override;

  void fetchData();

  void pollDialog(PaneManager* paneManager);

private:
  WidgetPtr getSelected();
  void setSelected(WidgetPtr selected);
  void toggleTracking();
  void abandon();
  void showQuests(List<QuestPtr> quests);

  QuestManagerPtr m_manager;
  PlayerPtr m_player;
  CinematicPtr m_cinematic;
  UniverseClientPtr m_client;
  IAssetsConstPtr m_assets;

  String m_trackLabel;
  String m_untrackLabel;

  ItemBagPtr m_rewardItems;
  int m_refreshRate;
  int m_refreshTimer;
};

class QuestPane : public Pane {
protected:
  QuestPane(QuestPtr const& quest, PlayerPtr player, QuestInterfaceServices services = {});

  void commonSetup(Json config, String bodyText, String const& portraitName);
  virtual void close();
  virtual void decline();
  virtual void accept();
  virtual PanePtr createTooltip(Vec2I const& screenPosition) override;

  QuestPtr m_quest;
  PlayerPtr m_player;
  IAssetsConstPtr m_assets;
};

class NewQuestInterface : public QuestPane {
public:
  enum class QuestDecision {
    Declined,
    Accepted,
    Cancelled
  };

  NewQuestInterface(QuestManagerPtr const& manager, QuestPtr const& quest, PlayerPtr player, QuestInterfaceServices services = {});

protected:
  void close() override;
  void decline() override;
  void accept() override;
  void dismissed() override;

private:
  QuestManagerPtr m_manager;
  QuestDecision m_decision;
};

class QuestCompleteInterface : public QuestPane {
public:
  QuestCompleteInterface(QuestPtr const& quest, PlayerPtr player, CinematicPtr cinematic, QuestInterfaceServices services = {});

protected:
  void close() override;

private:
  PlayerPtr m_player;
  CinematicPtr m_cinematic;
};

class QuestFailedInterface : public QuestPane {
public:
  QuestFailedInterface(QuestPtr const& quest, PlayerPtr player, QuestInterfaceServices services = {});
};

}
