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

export module star.char_selection;

export namespace Star {
  using ::Star::PlayerStorage;
  using ::Star::PlayerStoragePtr;
  using ::Star::PlayerStorageConstPtr;
  using ::Star::PlayerStorageWeakPtr;
  using ::Star::PlayerStorageConstWeakPtr;
  using ::Star::PlayerStorageUPtr;
  using ::Star::PlayerStorageConstUPtr;
  using ::Star::Player;
  using ::Star::PlayerPtr;
  using ::Star::PlayerConstPtr;
  using ::Star::PlayerWeakPtr;
  using ::Star::PlayerConstWeakPtr;
  using ::Star::PlayerUPtr;
  using ::Star::PlayerConstUPtr;
  using ::Star::CharSelectionPane;
  using ::Star::CharSelectionPanePtr;
}
