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
import star.format;

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

  [[nodiscard]] static std::expected<std::shared_ptr<QuestLogInterface>, StarException> create(GuiContext& guiContext, std::shared_ptr<QuestManager> manager, std::shared_ptr<Player> player, std::shared_ptr<Cinematic> cinematic, std::shared_ptr<UniverseClient> client);
  virtual ~QuestLogInterface() {}

  [[nodiscard]] std::expected<void, StarException> displayed() override;
  [[nodiscard]] std::expected<void, StarException> tick(float dt) override;
  [[nodiscard]] std::expected<std::shared_ptr<Pane>, StarException> createTooltip(Vec2I const& screenPosition) override;

  [[nodiscard]] std::expected<void, StarException> fetchData();

  [[nodiscard]] std::expected<void, StarException> pollDialog(PaneManager& paneManager);

private:
  QuestLogInterface(GuiContext& guiContext, std::shared_ptr<QuestManager> manager, std::shared_ptr<Player> player, std::shared_ptr<Cinematic> cinematic, std::shared_ptr<UniverseClient> client);
  [[nodiscard]] std::expected<void, StarException> initialize();
  [[nodiscard]] std::expected<std::shared_ptr<Widget>, StarException> getSelected();
  [[nodiscard]] std::expected<void, StarException> setSelected(std::shared_ptr<Widget> selected);
  [[nodiscard]] std::expected<void, StarException> toggleTracking();
  [[nodiscard]] std::expected<void, StarException> abandon();
  [[nodiscard]] std::expected<void, StarException> showQuests(List<std::shared_ptr<Quest>> quests);

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

  [[nodiscard]] std::expected<void, StarException> commonSetup(Json config, String bodyText, String const& portraitName);
  [[nodiscard]] virtual std::expected<void, StarException> close();
  [[nodiscard]] virtual std::expected<void, StarException> decline();
  [[nodiscard]] virtual std::expected<void, StarException> accept();
  [[nodiscard]] std::expected<std::shared_ptr<Pane>, StarException> createTooltip(Vec2I const& screenPosition) override;

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

  [[nodiscard]] static std::expected<std::shared_ptr<NewQuestInterface>, StarException> create(GuiContext& guiContext, std::shared_ptr<QuestManager> const& manager, std::shared_ptr<Quest> const& quest, std::shared_ptr<Player> player);

protected:
  [[nodiscard]] std::expected<void, StarException> close() override;
  [[nodiscard]] std::expected<void, StarException> decline() override;
  [[nodiscard]] std::expected<void, StarException> accept() override;
  [[nodiscard]] std::expected<void, StarException> dismissed() override;

private:
  NewQuestInterface(GuiContext& guiContext, std::shared_ptr<QuestManager> const& manager, std::shared_ptr<Quest> const& quest, std::shared_ptr<Player> player);
  [[nodiscard]] std::expected<void, StarException> initialize();
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

  [[nodiscard]] static std::expected<std::shared_ptr<QuestCompleteInterface>, StarException> create(GuiContext& guiContext, std::shared_ptr<Quest> const& quest, std::shared_ptr<Player> player, std::shared_ptr<Cinematic> cinematic);

protected:
  [[nodiscard]] std::expected<void, StarException> close() override;

private:
  QuestCompleteInterface(GuiContext& guiContext, std::shared_ptr<Quest> const& quest, std::shared_ptr<Player> player, std::shared_ptr<Cinematic> cinematic);
  [[nodiscard]] std::expected<void, StarException> initialize();
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

  [[nodiscard]] static std::expected<std::shared_ptr<QuestFailedInterface>, StarException> create(GuiContext& guiContext, std::shared_ptr<Quest> const& quest, std::shared_ptr<Player> player);

private:
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
