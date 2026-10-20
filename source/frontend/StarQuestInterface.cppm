module;
import std;
import star.type_identity;
import star.json;

import star.vector;
import star.casting;
import star.input_event;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.bi_map;
import star.string;
import star.text;
import star.asset_path;
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;


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

class QuestManager;
class Player;
class Cinematic;
class UniverseClient;
class PaneManager;
class ItemBag;
class Quest;

class QuestLogInterface : public Pane {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<QuestLogInterface, Pane>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<QuestLogInterface>();
  }

  QuestLogInterface(GuiContext& guiContext, std::shared_ptr<QuestManager> manager, std::shared_ptr<Player> player, std::shared_ptr<Cinematic> cinematic, std::shared_ptr<UniverseClient> client);
  virtual ~QuestLogInterface() {}

  virtual void displayed() override;
  virtual void tick(float dt) override;
  virtual std::shared_ptr<Pane> createTooltip(Vec2I const& screenPosition) override;

  void fetchData();

  void pollDialog(PaneManager* paneManager);

private:
  std::shared_ptr<Widget> getSelected();
  void setSelected(std::shared_ptr<Widget> selected);
  void toggleTracking();
  void abandon();
  void showQuests(List<std::shared_ptr<Quest>> quests);

  std::shared_ptr<QuestManager> m_manager;
  std::shared_ptr<Player> m_player;
  std::shared_ptr<Cinematic> m_cinematic;
  std::shared_ptr<UniverseClient> m_client;

  String m_trackLabel;
  String m_untrackLabel;

  std::shared_ptr<ItemBag> m_rewardItems;
  int m_refreshRate;
  int m_refreshTimer;
};

class QuestPane : public Pane {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<QuestPane, Pane>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<QuestPane>();
  }

protected:
  QuestPane(GuiContext& guiContext, std::shared_ptr<Quest> const& quest, std::shared_ptr<Player> player);

  void commonSetup(Json config, String bodyText, String const& portraitName);
  virtual void close();
  virtual void decline();
  virtual void accept();
  virtual std::shared_ptr<Pane> createTooltip(Vec2I const& screenPosition) override;

  std::shared_ptr<Quest> m_quest;
  std::shared_ptr<Player> m_player;
};

class NewQuestInterface : public QuestPane {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<NewQuestInterface, QuestPane>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<NewQuestInterface>();
  }

  enum class QuestDecision {
    Declined,
    Accepted,
    Cancelled
  };

  NewQuestInterface(GuiContext& guiContext, std::shared_ptr<QuestManager> const& manager, std::shared_ptr<Quest> const& quest, std::shared_ptr<Player> player);

protected:
  void close() override;
  void decline() override;
  void accept() override;
  void dismissed() override;

private:
  std::shared_ptr<QuestManager> m_manager;
  QuestDecision m_decision;
};

class QuestCompleteInterface : public QuestPane {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<QuestCompleteInterface, QuestPane>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<QuestCompleteInterface>();
  }

  QuestCompleteInterface(GuiContext& guiContext, std::shared_ptr<Quest> const& quest, std::shared_ptr<Player> player, std::shared_ptr<Cinematic> cinematic);

protected:
  void close() override;

private:
  std::shared_ptr<Player> m_player;
  std::shared_ptr<Cinematic> m_cinematic;
};

class QuestFailedInterface : public QuestPane {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<QuestFailedInterface, QuestPane>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<QuestFailedInterface>();
  }

  QuestFailedInterface(GuiContext& guiContext, std::shared_ptr<Quest> const& quest, std::shared_ptr<Player> player);
};

}

export module star.quest_interface;

export namespace Star {
  using ::Star::QuestManager;
  using ::Star::Player;
  using ::Star::Cinematic;
  using ::Star::UniverseClient;
  using ::Star::PaneManager;
  using ::Star::ItemBag;
  using ::Star::Quest;
  using ::Star::QuestLogInterface;
  using ::Star::QuestPane;
  using ::Star::NewQuestInterface;
  using ::Star::QuestCompleteInterface;
  using ::Star::QuestFailedInterface;
}
