
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarApplicationController.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarRenderer.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarRoot.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
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
#include "StarArray.hpp"
import star.uuid;


import star.char_selection;
import star.widget_parsing;
import star.gui_reader;
#include "StarPlayer.hpp"
import star.button_group;
import star.button_widget;
import star.portrait_widget;
import star.label_widget;
import star.large_char_plate_widget;
import star.text_box_widget;
#include "StarAssets.hpp"
#include "StarRandom.hpp"

import star.player_storage;

namespace Star {

CharSelectionPane::CharSelectionPane(PlayerStoragePtr playerStorage,
    CreateCharCallback createCallback,
    SelectCharacterCallback selectCallback,
    DeleteCharacterCallback deleteCallback)
  : m_playerStorage(playerStorage),
    m_downScroll(0),
    m_search(""),
    m_filteredList({}),
    m_createCallback(createCallback),
    m_selectCallback(selectCallback),
    m_deleteCallback(deleteCallback) {
  auto& root = Root::singleton();

  GuiReader guiReader;

  guiReader.registerCallback("playerUpButton", [this](Widget*) { shiftCharacters(-1); });
  guiReader.registerCallback("playerDownButton", [this](Widget*) { shiftCharacters(1); });
  guiReader.registerCallback("charSelector1", [this](Widget*) { selectCharacter(0); });
  guiReader.registerCallback("charSelector2", [this](Widget*) { selectCharacter(1); });
  guiReader.registerCallback("charSelector3", [this](Widget*) { selectCharacter(2); });
  guiReader.registerCallback("charSelector4", [this](Widget*) { selectCharacter(3); });
  guiReader.registerCallback("createCharButton", [this](Widget*) { m_createCallback(); });
  guiReader.registerCallback("searchCharacter", [this](Widget* obj) {
    m_downScroll = 0;
    m_search = convert<TextBoxWidget>(obj)->getText().trim().toLower();
    updateCharacterPlates();
  });
  guiReader.registerCallback("clearSearch", [this](Widget*) {
    fetchChild<TextBoxWidget>("searchCharacter")->setText("");
  });
  guiReader.registerCallback("toggleDismissCheckbox", [=](Widget* widget) {
    auto configuration = Root::singleton().configuration();
    configuration->set("characterSwapDismisses", as<ButtonWidget>(widget)->isChecked());
  });

  guiReader.construct(root.assets()->json("/interface/windowconfig/charselection.config"), this);
}

bool CharSelectionPane::sendEvent(InputEvent const& event) {
  if (m_visible) {
    if (auto mouseWheel = event.ptr<MouseWheelEvent>()) {
      if (inMember(*context()->mousePosition(event))) {
        if (mouseWheel->mouseWheel == MouseWheel::Down)
          shiftCharacters(1);
        else if (mouseWheel->mouseWheel == MouseWheel::Up)
          shiftCharacters(-1);
        return true;
      }
    }
  }
  return Pane::sendEvent(event);
}

void CharSelectionPane::show() {
  Pane::show();

  m_downScroll = 0;
  fetchChild<TextBoxWidget>("searchCharacter")->setText("");
  updateCharacterPlates();
}

void CharSelectionPane::shiftCharacters(int shift) {
  m_downScroll = std::max<int>(std::min<int>(m_downScroll + shift, m_filteredList.size() - 3), 0);
  updateCharacterPlates();
}

void CharSelectionPane::selectCharacter(unsigned buttonIndex) {
  if (m_downScroll + buttonIndex < m_filteredList.size()) {
    auto playerUuid = m_filteredList.get(m_downScroll + buttonIndex);
    auto player = m_playerStorage->loadPlayer(playerUuid);
    if (player->isPermaDead() && !player->isAdmin()) {
      auto sound = Random::randValueFrom(
                       Root::singleton().assets()->json("/interface.config:buttonClickFailSound").toArray(), "")
                       .toString();
      if (!sound.empty())
        context()->playAudio(sound);
    } else
      m_selectCallback(player);
  } else
    m_createCallback();
}

void CharSelectionPane::updateCharacterPlates() {

  auto updatePlayerLine = [this](String name, unsigned scrollPosition) {
    m_filteredList = m_playerStorage->playerUuidListByName(m_search);
    auto charSelector = fetchChild<LargeCharPlateWidget>(name);

    if (m_filteredList.size() > 0 && scrollPosition < m_filteredList.size()) {
      auto playerUuid = m_filteredList.get(scrollPosition);
      if (auto player = m_playerStorage->loadPlayer(playerUuid)) {
        charSelector->show();
        player->humanoid()->setFacingDirection(Direction::Right);
        charSelector->setPlayer(player);
        if (!m_readOnly)
          charSelector->enableDelete([this, playerUuid](Widget*) { m_deleteCallback(playerUuid); });
        return;
      }
    }
    charSelector->setPlayer(PlayerPtr());
    charSelector->disableDelete();
    if (m_readOnly)
      charSelector->hide();
  };

  updatePlayerLine("charSelector1", m_downScroll + 0);
  updatePlayerLine("charSelector2", m_downScroll + 1);
  updatePlayerLine("charSelector3", m_downScroll + 2);
  updatePlayerLine("charSelector4", m_downScroll + 3);

  if (m_downScroll > 0)
    fetchChild("playerUpButton")->show();
  else
    fetchChild("playerUpButton")->hide();

  if (m_downScroll < m_filteredList.size() - 3)
    fetchChild("playerDownButton")->show();
  else
    fetchChild("playerDownButton")->hide();
}

void CharSelectionPane::setReadOnly(bool readOnly) {
  findChild("createCharButton")->setVisibility(!(m_readOnly = readOnly));
}

}
