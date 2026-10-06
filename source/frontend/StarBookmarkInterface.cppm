module;

// Parse foundations before importing global bookmark value types on GCC.
#include "StarJson.hpp"
#include "StarStrongTypedef.hpp"
#include "StarArray.hpp"
#include "StarDataStream.hpp"
#include "StarVector.hpp"
#include "StarGameTypes.hpp"
#include "StarSystemWorld.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarPoly.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"


import star.uuid;
import star.celestial_coordinate;
import star.warping;
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
import star.player_universe_map;

namespace Star {

STAR_CLASS(EditBookmarkDialog);

class EditBookmarkDialog : public Pane {
public:
  EditBookmarkDialog(PlayerUniverseMapPtr playerUniverseMap);

  virtual void show() override;

  void setBookmark(TeleportBookmark bookmark);

  void ok();
  void remove();
  void close();

private:
  PlayerUniverseMapPtr m_playerUniverseMap;
  TeleportBookmark m_bookmark;

  bool m_isNew;
};

void setupBookmarkEntry(WidgetPtr const& entry, TeleportBookmark const& bookmark);
}

export module star.bookmark_interface;

export namespace Star {
  using ::Star::EditBookmarkDialog;
  using ::Star::EditBookmarkDialogPtr;
  using ::Star::EditBookmarkDialogConstPtr;
  using ::Star::EditBookmarkDialogWeakPtr;
  using ::Star::EditBookmarkDialogConstWeakPtr;
  using ::Star::EditBookmarkDialogUPtr;
  using ::Star::EditBookmarkDialogConstUPtr;
  using ::Star::setupBookmarkEntry;
}
