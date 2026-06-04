#pragma once

#include "StarPane.hpp"
#include "StarPlayerStorage.hpp"

namespace Star {

STAR_CLASS(PlayerStorage);

class CharSelectionPane : public Pane {
public:
  using CreateCharCallback = function<void()>;
  using SelectCharacterCallback = function<void(PlayerPtr const&)>;
  using DeleteCharacterCallback = function<void(Uuid)>;

  CharSelectionPane(PlayerStoragePtr playerStorage, CreateCharCallback createCallback,
      SelectCharacterCallback selectCallback, DeleteCharacterCallback deleteCallback);

  bool sendEvent(InputEvent const& event) override;
  void show() override;
  void updateCharacterPlates();
  void setReadOnly(bool readOnly);

private:
  void shiftCharacters(int movement);
  void selectCharacter(unsigned buttonIndex);

  PlayerStoragePtr m_playerStorage;
  unsigned m_downScroll;
  String m_search;
  List<Uuid> m_filteredList;
  bool m_readOnly = false;

  CreateCharCallback m_createCallback;
  SelectCharacterCallback m_selectCallback;
  DeleteCharacterCallback m_deleteCallback;
};
using CharSelectionPanePtr = shared_ptr<CharSelectionPane>;
}
