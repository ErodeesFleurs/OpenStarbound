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
import star.maybe;
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

  TeamInvite(GuiContext& guiContext, TeamBar* owner);

  virtual void show() override;

private:
  TeamBar* m_owner;

  void ok();
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

  TeamInvitation(GuiContext& guiContext, TeamBar* owner);

  void open(Uuid const& inviterUuid, String const& inviterName);

private:
  TeamBar* m_owner;
  Uuid m_inviterUuid;

  void ok();
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

  TeamMemberMenu(GuiContext& guiContext, TeamBar* owner);

  void open(Uuid memberUuid, Vec2I position);

  virtual void update(float dt) override;

private:
  void updateWidgets();

  void close();
  void beamToShip();
  void makeLeader();
  void removeFromTeam();

  TeamBar* m_owner;
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

  TeamBar(GuiContext& guiContext, MainInterface* mainInterface, std::shared_ptr<UniverseClient> client);

  bool sendEvent(InputEvent const& event) override;

  void invitePlayer(String const& playerName);
  void acceptInvitation(Uuid const& inviterUuid);

protected:
  virtual void update(float dt) override;

private:
  void updatePlayerResources();

  void inviteButton();

  void buildTeamBar();

  void showMemberMenu(Uuid memberUuid, Vec2I position);

  MainInterface* m_mainInterface;
  std::shared_ptr<UniverseClient> m_client;



  TextStyle m_nameStyle;
  Vec2F m_nameOffset;

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
