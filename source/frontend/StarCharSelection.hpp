#pragma once

#include "StarPane.hpp"
#include "StarUuid.hpp"

namespace Star {

STAR_CLASS(PlayerStorage);
STAR_CLASS(Player);

class CharSelectionPane : public Pane {
public:
  typedef function<void()> CreateCharCallback;
  typedef function<void(PlayerPtr const&)> SelectCharacterCallback;
  typedef function<void(Uuid)> DeleteCharacterCallback;

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
typedef shared_ptr<CharSelectionPane> CharSelectionPanePtr;
}
