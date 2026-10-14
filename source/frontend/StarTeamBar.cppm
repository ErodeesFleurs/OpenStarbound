module;
import std;
import star.json;

import star.vector;
#include "StarCasting.hpp"
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
using TeamBarPtr = std::shared_ptr<TeamBar>;
using TeamBarConstPtr = std::shared_ptr<TeamBar const>;
using TeamBarWeakPtr = std::weak_ptr<TeamBar>;
using TeamBarConstWeakPtr = std::weak_ptr<TeamBar const>;
using TeamBarUPtr = std::unique_ptr<TeamBar>;
using TeamBarConstUPtr = std::unique_ptr<TeamBar const>;
class MainInterface;
using MainInterfacePtr = std::shared_ptr<MainInterface>;
using MainInterfaceConstPtr = std::shared_ptr<MainInterface const>;
using MainInterfaceWeakPtr = std::weak_ptr<MainInterface>;
using MainInterfaceConstWeakPtr = std::weak_ptr<MainInterface const>;
using MainInterfaceUPtr = std::unique_ptr<MainInterface>;
using MainInterfaceConstUPtr = std::unique_ptr<MainInterface const>;
class UniverseClient;
using UniverseClientPtr = std::shared_ptr<UniverseClient>;
using UniverseClientConstPtr = std::shared_ptr<UniverseClient const>;
using UniverseClientWeakPtr = std::weak_ptr<UniverseClient>;
using UniverseClientConstWeakPtr = std::weak_ptr<UniverseClient const>;
using UniverseClientUPtr = std::unique_ptr<UniverseClient>;
using UniverseClientConstUPtr = std::unique_ptr<UniverseClient const>;
class Player;
using PlayerPtr = std::shared_ptr<Player>;
using PlayerConstPtr = std::shared_ptr<Player const>;
using PlayerWeakPtr = std::weak_ptr<Player>;
using PlayerConstWeakPtr = std::weak_ptr<Player const>;
using PlayerUPtr = std::unique_ptr<Player>;
using PlayerConstUPtr = std::unique_ptr<Player const>;

class TeamInvite;
using TeamInvitePtr = std::shared_ptr<TeamInvite>;
using TeamInviteConstPtr = std::shared_ptr<TeamInvite const>;
using TeamInviteWeakPtr = std::weak_ptr<TeamInvite>;
using TeamInviteConstWeakPtr = std::weak_ptr<TeamInvite const>;
using TeamInviteUPtr = std::unique_ptr<TeamInvite>;
using TeamInviteConstUPtr = std::unique_ptr<TeamInvite const>;
class TeamInvitation;
using TeamInvitationPtr = std::shared_ptr<TeamInvitation>;
using TeamInvitationConstPtr = std::shared_ptr<TeamInvitation const>;
using TeamInvitationWeakPtr = std::weak_ptr<TeamInvitation>;
using TeamInvitationConstWeakPtr = std::weak_ptr<TeamInvitation const>;
using TeamInvitationUPtr = std::unique_ptr<TeamInvitation>;
using TeamInvitationConstUPtr = std::unique_ptr<TeamInvitation const>;
class TeamMemberMenu;
using TeamMemberMenuPtr = std::shared_ptr<TeamMemberMenu>;
using TeamMemberMenuConstPtr = std::shared_ptr<TeamMemberMenu const>;
using TeamMemberMenuWeakPtr = std::weak_ptr<TeamMemberMenu>;
using TeamMemberMenuConstWeakPtr = std::weak_ptr<TeamMemberMenu const>;
using TeamMemberMenuUPtr = std::unique_ptr<TeamMemberMenu>;
using TeamMemberMenuConstUPtr = std::unique_ptr<TeamMemberMenu const>;
class TeamBar;
using TeamBarPtr = std::shared_ptr<TeamBar>;
using TeamBarConstPtr = std::shared_ptr<TeamBar const>;
using TeamBarWeakPtr = std::weak_ptr<TeamBar>;
using TeamBarConstWeakPtr = std::weak_ptr<TeamBar const>;
using TeamBarUPtr = std::unique_ptr<TeamBar>;
using TeamBarConstUPtr = std::unique_ptr<TeamBar const>;

class TeamInvite : public Pane {
public:
  TeamInvite(TeamBar* owner);

  virtual void show() override;

private:
  TeamBar* m_owner;

  void ok();
  void close();
};

class TeamInvitation : public Pane {
public:
  TeamInvitation(TeamBar* owner);

  void open(Uuid const& inviterUuid, String const& inviterName);

private:
  TeamBar* m_owner;
  Uuid m_inviterUuid;

  void ok();
  void close();
};

class TeamMemberMenu : public Pane {
public:
  TeamMemberMenu(TeamBar* owner);

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
  TeamBar(MainInterface* mainInterface, UniverseClientPtr client);

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
  UniverseClientPtr m_client;

  GuiContext* m_guiContext;

  TextStyle m_nameStyle;
  Vec2F m_nameOffset;

  TeamInvitePtr m_teamInvite;
  TeamInvitationPtr m_teamInvitation;
  TeamMemberMenuPtr m_teamMemberMenu;

  ProgressWidgetPtr m_healthBar;
  ProgressWidgetPtr m_energyBar;
  ProgressWidgetPtr m_foodBar;

  LabelWidgetPtr m_nameLabel;

  Color m_energyBarColor;
  Color m_energyBarRegenMixColor;
  Color m_energyBarUnusableColor;

  friend class TeamMemberMenu;
};

}

export module star.team_bar;

export namespace Star {
  using ::Star::TeamBar;
  using ::Star::TeamBarPtr;
  using ::Star::TeamBarConstPtr;
  using ::Star::TeamBarWeakPtr;
  using ::Star::TeamBarConstWeakPtr;
  using ::Star::TeamBarUPtr;
  using ::Star::TeamBarConstUPtr;
  using ::Star::MainInterface;
  using ::Star::MainInterfacePtr;
  using ::Star::MainInterfaceConstPtr;
  using ::Star::MainInterfaceWeakPtr;
  using ::Star::MainInterfaceConstWeakPtr;
  using ::Star::MainInterfaceUPtr;
  using ::Star::MainInterfaceConstUPtr;
  using ::Star::UniverseClient;
  using ::Star::UniverseClientPtr;
  using ::Star::UniverseClientConstPtr;
  using ::Star::UniverseClientWeakPtr;
  using ::Star::UniverseClientConstWeakPtr;
  using ::Star::UniverseClientUPtr;
  using ::Star::UniverseClientConstUPtr;
  using ::Star::Player;
  using ::Star::PlayerPtr;
  using ::Star::PlayerConstPtr;
  using ::Star::PlayerWeakPtr;
  using ::Star::PlayerConstWeakPtr;
  using ::Star::PlayerUPtr;
  using ::Star::PlayerConstUPtr;
  using ::Star::TeamInvite;
  using ::Star::TeamInvitePtr;
  using ::Star::TeamInviteConstPtr;
  using ::Star::TeamInviteWeakPtr;
  using ::Star::TeamInviteConstWeakPtr;
  using ::Star::TeamInviteUPtr;
  using ::Star::TeamInviteConstUPtr;
  using ::Star::TeamInvitation;
  using ::Star::TeamInvitationPtr;
  using ::Star::TeamInvitationConstPtr;
  using ::Star::TeamInvitationWeakPtr;
  using ::Star::TeamInvitationConstWeakPtr;
  using ::Star::TeamInvitationUPtr;
  using ::Star::TeamInvitationConstUPtr;
  using ::Star::TeamMemberMenu;
  using ::Star::TeamMemberMenuPtr;
  using ::Star::TeamMemberMenuConstPtr;
  using ::Star::TeamMemberMenuWeakPtr;
  using ::Star::TeamMemberMenuConstWeakPtr;
  using ::Star::TeamMemberMenuUPtr;
  using ::Star::TeamMemberMenuConstUPtr;
}
