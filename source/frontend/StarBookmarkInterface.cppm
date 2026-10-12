module;
#include "StarJson.hpp"
#include "StarStrongTypedef.hpp"
#include "StarDataStream.hpp"
import star.game_types;
#include "StarVariant.hpp"
import star.random;
import star.weighted_pool;
import star.either;
#include "StarCasting.hpp"
import star.input_event;
import star.color;
#include "StarList.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
#include "StarBiMap.hpp"
#include "StarString.hpp"
import star.text;
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
#include "StarSet.hpp"
import star.audio;
#include "StarMap.hpp"


// Parse foundations before importing global bookmark value types on GCC.
import star.sky_types;
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.sky_parameters;
import star.system_world;


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
