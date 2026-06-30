#pragma once

#include "StarQuestManager.hpp"
#include "StarQuests.hpp"
#include "StarPane.hpp"
#include "StarAssets.hpp"

namespace Star {

class QuestManager;
class Player;
using PlayerPtr = SharedPtr<Player>;
class Cinematic;
using CinematicPtr = SharedPtr<Cinematic>;
class UniverseClient;
using UniverseClientPtr = SharedPtr<UniverseClient>;
class PaneManager;
class GuiContext;
class ItemBag;
using ItemBagPtr = SharedPtr<ItemBag>;
class ObjectDatabase;
using ObjectDatabaseConstPtr = SharedPtr<ObjectDatabase const>;
class StatusEffectDatabase;
using StatusEffectDatabaseConstPtr = SharedPtr<StatusEffectDatabase const>;

struct QuestInterfaceServices {
  AssetsConstPtr assets;
  ObjectDatabaseConstPtr objectDatabase;
  StatusEffectDatabaseConstPtr statusEffectDatabase;
  GuiContext& guiContext;
};

class QuestLogInterface : public Pane {
public:
  QuestLogInterface(QuestManagerPtr manager, PlayerPtr player, CinematicPtr cinematic, UniverseClientPtr client, QuestInterfaceServices services);
  virtual ~QuestLogInterface() = default;

  void displayed() override;
  void tick(float dt) override;
  [[nodiscard]] UniquePtr<Pane> createTooltip(Vec2I const& screenPosition) override;

  void fetchData();

  void pollDialog(PaneManager& paneManager);

private:
  [[nodiscard]] WidgetRef<Widget> getSelected();
  void setSelected(WidgetRef<Widget> selected);
  void toggleTracking();
  void abandon();
  void showQuests(List<QuestPtr> quests);

  QuestManagerPtr m_manager;
  PlayerPtr m_player;
  CinematicPtr m_cinematic;
  UniverseClientPtr m_client;
  AssetsConstPtr m_assets;
  ObjectDatabaseConstPtr m_objectDatabase;
  StatusEffectDatabaseConstPtr m_statusEffectDatabase;

  String m_trackLabel;
  String m_untrackLabel;

  ItemBagPtr m_rewardItems;
  int m_refreshRate = 30;
  int m_refreshTimer = 0;
};

class QuestPane : public Pane {
protected:
  QuestPane(QuestPtr const& quest, PlayerPtr player, QuestInterfaceServices services);

  void commonSetup(Json config, String bodyText, String const& portraitName);
  virtual void close();
  virtual void decline();
  virtual void accept();
  [[nodiscard]] UniquePtr<Pane> createTooltip(Vec2I const& screenPosition) override;

  QuestPtr m_quest;
  PlayerPtr m_player;
  AssetsConstPtr m_assets;
  ObjectDatabaseConstPtr m_objectDatabase;
  StatusEffectDatabaseConstPtr m_statusEffectDatabase;
};

class NewQuestInterface : public QuestPane {
public:
  enum class QuestDecision {
    Declined,
    Accepted,
    Cancelled
  };

  NewQuestInterface(QuestManagerPtr const& manager, QuestPtr const& quest, PlayerPtr player, QuestInterfaceServices services);

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
  QuestCompleteInterface(QuestPtr const& quest, PlayerPtr player, CinematicPtr cinematic, QuestInterfaceServices services);

protected:
  void close() override;

private:
  CinematicPtr m_cinematic;
};

class QuestFailedInterface : public QuestPane {
public:
  QuestFailedInterface(QuestPtr const& quest, PlayerPtr player, QuestInterfaceServices services);
};

}
