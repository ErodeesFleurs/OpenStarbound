#include "StarTeamBar.hpp"
#include "StarAlgorithm.hpp"
#include "StarAssets.hpp"
#include "StarConfiguration.hpp"
#include "StarMainInterface.hpp"
#include "StarJsonExtra.hpp"
#include "StarUniverseClient.hpp"
#include "StarGuiReader.hpp"
#include "StarButtonWidget.hpp"
#include "StarTeamClient.hpp"
#include "StarImageWidget.hpp"
#include "StarProgressWidget.hpp"
#include "StarTextBoxWidget.hpp"
#include "StarLabelWidget.hpp"
#include "StarPlayer.hpp"
#include "StarWorldClient.hpp"
#include "StarPortraitWidget.hpp"
#include "StarMathCommon.hpp"

namespace Star {

TeamBar::TeamBar(MainInterface& mainInterface, UniverseClientPtr client, Services services)
  : Pane(services.guiContext),
    m_mainInterface(mainInterface),
    m_client(std::move(client)),
    m_assets(requireServiceValueAs<StarException>(std::move(services.assets), "TeamBar", "assets")),
    m_configuration(requireServiceValueAs<StarException>(std::move(services.configuration), "TeamBar", "configuration")),
    m_guiContext(services.guiContext) {
  m_teamInvite = make_shared<TeamInvite>(*this);
  m_teamInvitation = make_shared<TeamInvitation>(*this);
  m_teamMemberMenu = make_shared<TeamMemberMenu>(*this);

  m_nameStyle = m_assets->json("/interface.config:teamBarNameStyle");
  m_nameStyle.fontSize = m_assets->json("/interface.config:font.nameSize").toInt();
  m_nameOffset = jsonToVec2F(m_assets->json("/interface.config:nameOffset"));

  GuiReader reader(m_guiContext);

  reader.registerCallback("inviteButton", [this](Widget*) { inviteButton(); });
  reader.registerCallback("showSelfMenu", [this](Widget*) {
      if (!m_client->teamClient()->isMemberOfTeam())
        return;
      auto position = jsonToVec2I(m_assets->json("/interface/windowconfig/teambar.config:selfMenuOffset"));
      position[1] += windowHeight() / m_guiContext.interfaceScale();
      showMemberMenu(m_client->mainPlayer()->clientContext()->playerUuid(), position);
    });

  reader.construct(m_assets->json("/interface/windowconfig/teambar.config:paneLayout"), this);

  m_healthBar = fetchChild<ProgressWidget>("healthBar");
  m_energyBar = fetchChild<ProgressWidget>("energyBar");
  m_foodBar = fetchChild<ProgressWidget>("foodBar");
  m_nameLabel = fetchChild<LabelWidget>("name");

  m_energyBarColor = jsonToColor(m_assets->json("/interface/windowconfig/teambar.config:energyBarColor"));
  m_energyBarRegenMixColor = jsonToColor(m_assets->json("/interface/windowconfig/teambar.config:energyBarRegenMixColor"));
  m_energyBarUnusableColor = jsonToColor(m_assets->json("/interface/windowconfig/teambar.config:energyBarUnusableColor"));

  m_energyBar->setColor(m_energyBarColor);

  auto playerPortrait = fetchChild<PortraitWidget>("portrait");
  playerPortrait->setEntity(as<PortraitEntity>(m_client->mainPlayer()));

  updatePlayerResources();
  disableScissoring();
}

bool TeamBar::sendEvent(InputEvent const& event) {
  if (event.is<MouseButtonDownEvent>()
      && (event.get<MouseButtonDownEvent>().mouseButton == MouseButton::Left || event.get<MouseButtonDownEvent>().mouseButton == MouseButton::Right)) {
    if (m_teamMemberMenu->isDisplayed() && !m_teamMemberMenu->inMember(*context().mousePosition(event)))
      m_teamMemberMenu->dismiss();
  }
  return Pane::sendEvent(event);
}

void TeamBar::invitePlayer(String const& playerName) {
  m_client->teamClient()->invitePlayer(playerName);
}

void TeamBar::acceptInvitation(Uuid const& inviterUuid) {
  m_client->teamClient()->acceptInvitation(inviterUuid);
}

void TeamBar::update(float dt) {
  Pane::update(dt);

  updatePlayerResources();

  auto teamClient = m_client->teamClient();

  if (!m_teamInvitation->active()) {
    if (teamClient->hasInvitationPending()) {
      auto invitation = teamClient->pullInvitation();
      m_teamInvitation->open(invitation.first, invitation.second);
      if (!m_teamInvitation->isDisplayed())
        m_mainInterface.paneManager().displayPane(PaneLayer::Window, m_teamInvitation);
    }
  }

  if (teamClient->currentTeam() && !teamClient->isTeamLeader())
    m_teamInvite->dismiss();

  fetchChild<ImageWidget>("leader")->setVisibility(teamClient->isTeamLeader());

  buildTeamBar();
}

void TeamBar::updatePlayerResources() {
  auto player = m_client->mainPlayer();

  m_healthBar->setCurrentProgressLevel(player->healthPercentage());
  m_energyBar->setCurrentProgressLevel(player->energyPercentage());

  if (player->modeConfig().hunger) {
    m_foodBar->setCurrentProgressLevel(player->foodPercentage());
    if (player->foodPercentage() <= m_assets->json("/player.config:foodLowThreshold").toFloat()) {
      float flashTime = m_assets->json("/interface/windowconfig/teambar.config:foodBarFlashTime").toFloat();
      if (fmod(Time::monotonicTime(), flashTime * 2) < flashTime)
        m_foodBar->setOverlay(m_assets->json("/interface/windowconfig/teambar.config:foodBarFlashOverlay").toString());
      else
        m_foodBar->setOverlay("");
    } else {
      m_foodBar->setOverlay("");
    }
    m_foodBar->show();
  } else {
    m_foodBar->hide();
  }

  if (player->energyLocked()) {
    m_energyBar->setColor(m_energyBarUnusableColor);
  } else {
    m_energyBar->setColor(m_energyBarColor.mix(m_energyBarRegenMixColor, player->energyRegenBlockPercent()));
  }

  m_nameLabel->setText(player->name());
}

void TeamBar::inviteButton() {
  if (!m_teamInvite->isDisplayed())
    m_mainInterface.paneManager().displayPane(PaneLayer::Window, m_teamInvite);
}

void TeamBar::buildTeamBar() {
  auto teamClient = m_client->teamClient();
  auto player = m_client->mainPlayer();

  auto list = fetchChild("list");

  Vec2I offset;
  size_t controlIndex = 0;

  float portraitScale = m_assets->json("/interface/windowconfig/teambar.config:memberPortraitScale").toFloat();
  int memberSize = m_assets->json("/interface/windowconfig/teambar.config:memberSize").toInt();
  int memberSpacing = m_assets->json("/interface/windowconfig/teambar.config:memberSpacing").toInt();

  Uuid myUuid = player->clientContext()->playerUuid();
  for (auto member : teamClient->members()) {
    if (member.uuid == myUuid) {
      continue;
    }

    String cellName = toString(controlIndex);
    WidgetPtr cell = list->fetchChild(cellName);

    if (!cell) {
      GuiReader reader(m_guiContext);
      cell = make_shared<Widget>(m_guiContext);
      cell->disableScissoring();
      cell->markAsContainer();

      reader.registerCallback("showMemberMenu", [this](Widget* widget) {
          auto position = widget->screenPosition() + jsonToVec2I(m_assets->json("/interface/windowconfig/teambar.config:memberMenuOffset"));
          showMemberMenu(Uuid(widget->parent()->data().toString()), position);
        });

      reader.construct(m_assets->json("/interface/windowconfig/teambar.config:entry"), cell.get());

      list->addChild(cellName, cell);
    }

    offset[1] -= memberSize;
    cell->setPosition(offset);

    cell->setData(member.uuid.hex());

    cell->show();

    if (!teamClient->isTeamLeader(member.uuid))
      cell->fetchChild<ImageWidget>("leader")->hide();
    else
      cell->fetchChild<ImageWidget>("leader")->show();

    List<Drawable> drawables = member.portrait;
    Drawable::scaleAll(drawables, portraitScale);
    cell->fetchChild<ImageWidget>("portrait")->setDrawables(std::move(drawables));

    if (member.world == m_client->playerWorld() && m_client->worldClient()) {
      auto mpos = member.position;
      if (auto entity = m_client->worldClient()->entity(member.entity))
        mpos = entity->position();
      auto direction = m_client->worldClient()->geometry().diff(mpos, player->position());

      auto compassImage = cell->fetchChild<ImageWidget>("compass");
      compassImage->setRotation(direction.angle() - Constants::pi / 2.0f);
      compassImage->show();
      cell->fetchChild<ImageWidget>("compassoffworld")->hide();
    } else {
      cell->fetchChild<ImageWidget>("compass")->hide();
      cell->fetchChild<ImageWidget>("compassoffworld")->show();
    }

    cell->fetchChild<ProgressWidget>("healthBar")->setCurrentProgressLevel(member.healthPercentage);
    cell->fetchChild<ProgressWidget>("energyBar")->setCurrentProgressLevel(member.energyPercentage);

    offset[1] -= memberSpacing;
    controlIndex++;
  }

  auto inviteButton = fetchChild<ButtonWidget>("inviteButton");
  auto noInviteImage = fetchChild<ImageWidget>("noInviteImage");

  Vec2I inviteOffset = list->position() + offset;
  inviteButton->setPosition(inviteOffset - Vec2I{0, inviteButton->size()[1]});
  noInviteImage->setPosition(inviteOffset - Vec2I{0, noInviteImage->size()[1]});

  bool couldInvite = (!teamClient->currentTeam() || teamClient->isTeamLeader())
      && m_client->teamClient()->members().size() < m_configuration->get("maxTeamSize").toUInt();
  inviteButton->setVisibility(couldInvite);
  inviteButton->setEnabled(!m_teamInvitation->active());
  noInviteImage->setVisibility(!couldInvite);

  while (true) {
    String cellName = toString(controlIndex);
    WidgetPtr cell = list->fetchChild(cellName);
    if (!cell)
      break;
    cell->hide();
    controlIndex++;
  }
}

void TeamBar::showMemberMenu(Uuid memberUuid, Vec2I position) {
  m_teamMemberMenu->open(memberUuid, position);
  if (!m_teamMemberMenu->isDisplayed())
    m_mainInterface.paneManager().displayPane(PaneLayer::Window, m_teamMemberMenu);
}

TeamInvite::TeamInvite(TeamBar& owner)
  : Pane(owner.context()), m_owner(owner) {
  GuiReader reader(context());
  reader.registerCallback("ok", [this](Widget*) { ok(); });
  reader.registerCallback("close", [this](Widget*) { close(); });
  reader.registerCallback("name", [](Widget*) {});
  reader.construct(m_owner.m_assets->json("/interface/windowconfig/teaminvite.config:paneLayout"), this);
  dismiss();
}

void TeamInvite::show() {
  Pane::show();
  fetchChild<TextBoxWidget>("name")->setText("", false);
  fetchChild<TextBoxWidget>("name")->focus();
}

void TeamInvite::ok() {
  m_owner.invitePlayer(fetchChild<TextBoxWidget>("name")->getText());
  dismiss();
}

void TeamInvite::close() {
  dismiss();
}

TeamInvitation::TeamInvitation(TeamBar& owner)
  : Pane(owner.context()), m_owner(owner) {
  GuiReader reader(context());

  reader.registerCallback("ok", [this](Widget*) { ok(); });
  reader.registerCallback("close", [this](Widget*) { close(); });

  reader.construct(m_owner.m_assets->json("/interface/windowconfig/teaminvitation.config:paneLayout"), this);
  dismiss();
}

void TeamInvitation::open(Uuid const& inviterUuid, String const& inviterName) {
  if (active())
    return;
  m_inviterUuid = inviterUuid;
  fetchChild<LabelWidget>("inviterName")->setText(inviterName);
  Pane::show();
}

void TeamInvitation::ok() {
  m_owner.acceptInvitation(m_inviterUuid);
  dismiss();
}

void TeamInvitation::close() {
  dismiss();
}

TeamMemberMenu::TeamMemberMenu(TeamBar& owner)
  : Pane(owner.context()), m_owner(owner) {
  GuiReader reader(context());
  reader.registerCallback("beamToShip", [this](Widget*) { beamToShip(); });
  reader.registerCallback("close", [this](Widget*) { close(); });
  reader.registerCallback("makeLeader", [this](Widget*) { makeLeader(); });
  reader.registerCallback("removeFromTeam", [this](Widget*) { removeFromTeam(); });
  reader.construct(m_owner.m_assets->json("/interface/windowconfig/teammembermenu.config:paneLayout"), this);
}

void TeamMemberMenu::open(Uuid memberUuid, Vec2I position) {
  if (active())
    return;

  setPosition(position);

  m_memberUuid = memberUuid;
  auto members = m_owner.m_client->teamClient()->members();
  for (auto member : members) {
    if (member.uuid == m_memberUuid) {
      fetchChild<LabelWidget>("name")->setText(member.name);
      break;
    }
  }

  updateWidgets();

  Pane::show();
}

void TeamMemberMenu::update(float dt) {
  auto stillValid = false;
  auto members = m_owner.m_client->teamClient()->members();
  for (auto member : members) {
    if (member.uuid == m_memberUuid) {
      stillValid = true;
      m_canBeam = member.warpMode != WarpMode::None && m_owner.m_client->canBeamToTeamShip();
    }
  }

  if (!stillValid) {
    close();
    return;
  }

  updateWidgets();

  Pane::update(dt);
}

void TeamMemberMenu::updateWidgets() {
  bool isLeader = m_owner.m_client->teamClient()->isTeamLeader();
  bool isSelf = m_owner.m_client->mainPlayer()->clientContext()->playerUuid() == m_memberUuid;

  fetchChild<ButtonWidget>("beamToShip")->setEnabled(m_canBeam);
  fetchChild<ButtonWidget>("makeLeader")->setEnabled(isLeader && !isSelf);
  fetchChild<ButtonWidget>("removeFromTeam")->setEnabled(isLeader || isSelf);

  if (isSelf)
    fetchChild<ButtonWidget>("removeFromTeam")->setText(m_owner.m_assets->json("/interface/windowconfig/teammembermenu.config:removeSelfText").toString());
  else
    fetchChild<ButtonWidget>("removeFromTeam")->setText(m_owner.m_assets->json("/interface/windowconfig/teammembermenu.config:removeOtherText").toString());
}

void TeamMemberMenu::beamToShip() {
  if (m_canBeam)
    m_owner.m_mainInterface.warpTo(WarpToWorld{ClientShipWorldId(m_memberUuid), {}});
  dismiss();
}

void TeamMemberMenu::close() {
  dismiss();
}

void TeamMemberMenu::makeLeader() {
  m_owner.m_client->teamClient()->makeLeader(m_memberUuid);
  dismiss();
}

void TeamMemberMenu::removeFromTeam() {
  m_owner.m_client->teamClient()->removeFromTeam(m_memberUuid);
  dismiss();
}

}
