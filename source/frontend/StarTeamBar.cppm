module;
import std;
import star.type_identity;
import star.format;
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

import star.ordered_map;




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
import star.uuid;
import star.game_timers;
import star.pane_manager;
import star.registered_pane_manager;
import star.main_interface_types;
import star.progress_widget;
import star.label_widget;

namespace Star {

class TeamBar;
class MainInterface;
class UniverseClient;
class Player;

class TeamBar;

class TeamInvite : public Pane {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<TeamInvite, Pane>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<TeamInvite>();
  }

  [[nodiscard]] static std::expected<std::shared_ptr<TeamInvite>, StarException> create(GuiContext& guiContext, TeamBar& owner);

  [[nodiscard]] std::expected<void, StarException> show() override;

private:
  TeamInvite(GuiContext& guiContext, TeamBar& owner);
  [[nodiscard]] std::expected<void, StarException> initialize();
  std::weak_ptr<TeamBar> m_owner;

  [[nodiscard]] std::expected<void, StarException> ok();
  void close();
};

class TeamInvitation : public Pane {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<TeamInvitation, Pane>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<TeamInvitation>();
  }

  [[nodiscard]] static std::expected<std::shared_ptr<TeamInvitation>, StarException> create(GuiContext& guiContext, TeamBar& owner);

  [[nodiscard]] std::expected<void, StarException> open(Uuid const& inviterUuid, String const& inviterName);

private:
  TeamInvitation(GuiContext& guiContext, TeamBar& owner);
  [[nodiscard]] std::expected<void, StarException> initialize();
  std::weak_ptr<TeamBar> m_owner;
  Uuid m_inviterUuid;

  [[nodiscard]] std::expected<void, StarException> ok();
  void close();
};

class TeamMemberMenu : public Pane {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<TeamMemberMenu, Pane>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<TeamMemberMenu>();
  }

  [[nodiscard]] static std::expected<std::shared_ptr<TeamMemberMenu>, StarException> create(GuiContext& guiContext, TeamBar& owner);

  [[nodiscard]] std::expected<void, StarException> open(Uuid memberUuid, Vec2I position);

  [[nodiscard]] std::expected<void, StarException> update(float dt) override;

private:
  TeamMemberMenu(GuiContext& guiContext, TeamBar& owner);
  [[nodiscard]] std::expected<void, StarException> initialize();
  [[nodiscard]] std::expected<void, StarException> updateWidgets();

  void close();
  [[nodiscard]] std::expected<void, StarException> beamToShip();
  [[nodiscard]] std::expected<void, StarException> makeLeader();
  [[nodiscard]] std::expected<void, StarException> removeFromTeam();

  std::weak_ptr<TeamBar> m_owner;
  Uuid m_memberUuid;
  bool m_canBeam;
};

class TeamBar : public Pane {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<TeamBar, Pane>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<TeamBar>();
  }

  [[nodiscard]] static std::expected<std::shared_ptr<TeamBar>, StarException> create(GuiContext& guiContext, MainInterface& mainInterface, std::shared_ptr<UniverseClient> client);
  void detachInterface() { m_mainInterface.reset(); }

  [[nodiscard]] std::expected<bool, StarException> sendEvent(InputEvent const& event) override;

  [[nodiscard]] std::expected<void, StarException> invitePlayer(String const& playerName);
  [[nodiscard]] std::expected<void, StarException> acceptInvitation(Uuid const& inviterUuid);

protected:
  [[nodiscard]] std::expected<void, StarException> update(float dt) override;

private:
  TeamBar(GuiContext& guiContext, MainInterface& mainInterface, std::shared_ptr<UniverseClient> client);
  [[nodiscard]] std::expected<void, StarException> initialize();
  [[nodiscard]] std::expected<void, StarException> updatePlayerResources();

  [[nodiscard]] std::expected<void, StarException> inviteButton();

  [[nodiscard]] std::expected<void, StarException> buildTeamBar();

  [[nodiscard]] std::expected<void, StarException> showMemberMenu(Uuid memberUuid, Vec2I position);

  std::optional<std::reference_wrapper<MainInterface>> m_mainInterface;
  std::shared_ptr<UniverseClient> m_client;

  std::shared_ptr<TeamInvite> m_teamInvite;
  std::shared_ptr<TeamInvitation> m_teamInvitation;
  std::shared_ptr<TeamMemberMenu> m_teamMemberMenu;

  std::shared_ptr<ProgressWidget> m_healthBar;
  std::shared_ptr<ProgressWidget> m_energyBar;
  std::shared_ptr<ProgressWidget> m_foodBar;

  std::shared_ptr<LabelWidget> m_nameLabel;

  Color m_energyBarColor;
  Color m_energyBarRegenMixColor;
  Color m_energyBarUnusableColor;

  friend class TeamMemberMenu;
};

}

export module star.team_bar;

export namespace Star {
  using ::Star::TeamBar;
  using ::Star::MainInterface;
  using ::Star::UniverseClient;
  using ::Star::Player;
  using ::Star::TeamInvite;
  using ::Star::TeamInvitation;
  using ::Star::TeamMemberMenu;
}
