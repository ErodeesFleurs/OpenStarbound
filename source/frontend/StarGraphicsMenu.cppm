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
import star.game_timers;
import star.pane_manager;
import star.registered_pane_manager;
import star.main_interface_types;

namespace Star {
STAR_CLASS(UniverseClient);
}

namespace Star {

STAR_CLASS(GraphicsMenu);
STAR_CLASS(ShadersMenu);

class GraphicsMenu : public Pane {
public:
  GraphicsMenu(PaneManager* manager,UniverseClientPtr client);

  void show() override;
  void dismissed() override;

  void toggleFullscreen();

private:
  static StringList const ConfigKeys;

  void initConfig();
  void syncGui();

  void apply();
  void applyWindowSettings();
  
  void displayShaders();

  List<Vec2U> m_resList;
  List<float> m_interfaceScaleList;
  List<float> m_zoomList;
  List<float> m_cameraSpeedList;

  JsonObject m_localChanges;
  
  ShadersMenuPtr m_shadersMenu;
  PaneManager* m_paneManager;
};

}

export module star.graphics_menu;

export namespace Star {
  using ::Star::GraphicsMenu;
  using ::Star::GraphicsMenuPtr;
  using ::Star::GraphicsMenuConstPtr;
  using ::Star::GraphicsMenuWeakPtr;
  using ::Star::GraphicsMenuConstWeakPtr;
  using ::Star::GraphicsMenuUPtr;
  using ::Star::GraphicsMenuConstUPtr;
  using ::Star::ShadersMenu;
  using ::Star::ShadersMenuPtr;
  using ::Star::ShadersMenuConstPtr;
  using ::Star::ShadersMenuWeakPtr;
  using ::Star::ShadersMenuConstWeakPtr;
  using ::Star::ShadersMenuUPtr;
  using ::Star::ShadersMenuConstUPtr;
}
