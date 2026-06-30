#pragma once

#include "StarAssets.hpp"
#include "StarConfiguration.hpp"
#include "StarPane.hpp"
#include "StarPlayerStorage.hpp"

namespace Star {

using PlayerStoragePtr = SharedPtr<PlayerStorage>;

struct CharSelectionServices {
  AssetsConstPtr assets;
  ConfigurationPtr configuration;
  GuiContext& guiContext;
};

class CharSelectionPane : public Pane {
public:
  using CreateCharCallback = function<void()>;
  using SelectCharacterCallback = function<void(PlayerPtr const&)>;
  using DeleteCharacterCallback = function<void(Uuid)>;

  CharSelectionPane(PlayerStoragePtr playerStorage, CreateCharCallback createCallback,
      SelectCharacterCallback selectCallback, DeleteCharacterCallback deleteCallback,
      CharSelectionServices services);

  bool sendEvent(InputEvent const& event) override;
  void show() override;
  void updateCharacterPlates();
  void setReadOnly(bool readOnly);

private:
  void shiftCharacters(int movement);
  void selectCharacter(unsigned buttonIndex);

  PlayerStoragePtr m_playerStorage;
  AssetsConstPtr m_assets;
  ConfigurationPtr m_configuration;
  unsigned m_downScroll;
  String m_search;
  List<Uuid> m_filteredList;
  bool m_readOnly = false;

  CreateCharCallback m_createCallback;
  SelectCharacterCallback m_selectCallback;
  DeleteCharacterCallback m_deleteCallback;
};
using CharSelectionPanePtr = SharedPtr<CharSelectionPane>;
}
