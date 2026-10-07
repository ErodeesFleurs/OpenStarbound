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
#include "StarArray.hpp"

#include "StarOrderedMap.hpp"




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

STAR_CLASS(TeamBar);
STAR_CLASS(MainInterface);
STAR_CLASS(UniverseClient);
STAR_CLASS(Player);

STAR_CLASS(TeamInvite);
STAR_CLASS(TeamInvitation);
STAR_CLASS(TeamMemberMenu);
STAR_CLASS(TeamBar);

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
