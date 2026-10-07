module;

#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
#include "StarJson.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarString.hpp"
import star.text;
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"


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

namespace Star {

STAR_CLASS(QuestManager);
STAR_CLASS(Player);
STAR_CLASS(Cinematic);
STAR_CLASS(UniverseClient);
STAR_CLASS(PaneManager);
STAR_CLASS(ItemBag);
STAR_CLASS(Quest);

class QuestLogInterface : public Pane {
public:
  QuestLogInterface(QuestManagerPtr manager, PlayerPtr player, CinematicPtr cinematic, UniverseClientPtr client);
  virtual ~QuestLogInterface() {}

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

  String m_trackLabel;
  String m_untrackLabel;

  ItemBagPtr m_rewardItems;
  int m_refreshRate;
  int m_refreshTimer;
};

class QuestPane : public Pane {
protected:
  QuestPane(QuestPtr const& quest, PlayerPtr player);

  void commonSetup(Json config, String bodyText, String const& portraitName);
  virtual void close();
  virtual void decline();
  virtual void accept();
  virtual PanePtr createTooltip(Vec2I const& screenPosition) override;

  QuestPtr m_quest;
  PlayerPtr m_player;
};

class NewQuestInterface : public QuestPane {
public:
  enum class QuestDecision {
    Declined,
    Accepted,
    Cancelled
  };

  NewQuestInterface(QuestManagerPtr const& manager, QuestPtr const& quest, PlayerPtr player);

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
  QuestCompleteInterface(QuestPtr const& quest, PlayerPtr player, CinematicPtr cinematic);

protected:
  void close() override;

private:
  PlayerPtr m_player;
  CinematicPtr m_cinematic;
};

class QuestFailedInterface : public QuestPane {
public:
  QuestFailedInterface(QuestPtr const& quest, PlayerPtr player);
};

}

export module star.quest_interface;

export namespace Star {
  using ::Star::QuestManager;
  using ::Star::QuestManagerPtr;
  using ::Star::QuestManagerConstPtr;
  using ::Star::QuestManagerWeakPtr;
  using ::Star::QuestManagerConstWeakPtr;
  using ::Star::QuestManagerUPtr;
  using ::Star::QuestManagerConstUPtr;
  using ::Star::Player;
  using ::Star::PlayerPtr;
  using ::Star::PlayerConstPtr;
  using ::Star::PlayerWeakPtr;
  using ::Star::PlayerConstWeakPtr;
  using ::Star::PlayerUPtr;
  using ::Star::PlayerConstUPtr;
  using ::Star::Cinematic;
  using ::Star::CinematicPtr;
  using ::Star::CinematicConstPtr;
  using ::Star::CinematicWeakPtr;
  using ::Star::CinematicConstWeakPtr;
  using ::Star::CinematicUPtr;
  using ::Star::CinematicConstUPtr;
  using ::Star::UniverseClient;
  using ::Star::UniverseClientPtr;
  using ::Star::UniverseClientConstPtr;
  using ::Star::UniverseClientWeakPtr;
  using ::Star::UniverseClientConstWeakPtr;
  using ::Star::UniverseClientUPtr;
  using ::Star::UniverseClientConstUPtr;
  using ::Star::PaneManager;
  using ::Star::PaneManagerPtr;
  using ::Star::PaneManagerConstPtr;
  using ::Star::PaneManagerWeakPtr;
  using ::Star::PaneManagerConstWeakPtr;
  using ::Star::PaneManagerUPtr;
  using ::Star::PaneManagerConstUPtr;
  using ::Star::ItemBag;
  using ::Star::ItemBagPtr;
  using ::Star::ItemBagConstPtr;
  using ::Star::ItemBagWeakPtr;
  using ::Star::ItemBagConstWeakPtr;
  using ::Star::ItemBagUPtr;
  using ::Star::ItemBagConstUPtr;
  using ::Star::Quest;
  using ::Star::QuestPtr;
  using ::Star::QuestConstPtr;
  using ::Star::QuestWeakPtr;
  using ::Star::QuestConstWeakPtr;
  using ::Star::QuestUPtr;
  using ::Star::QuestConstUPtr;
  using ::Star::QuestLogInterface;
  using ::Star::QuestPane;
  using ::Star::NewQuestInterface;
  using ::Star::QuestCompleteInterface;
  using ::Star::QuestFailedInterface;
}
