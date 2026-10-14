module;
import std;
import star.json;

import star.vector;
#include "StarCasting.hpp"
import star.input_event;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.bi_map;
import star.string;
import star.text;
import star.asset_path;
import star.maybe;
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;

import star.ordered_map;




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
class UniverseClient;
using UniverseClientPtr = std::shared_ptr<UniverseClient>;
using UniverseClientConstPtr = std::shared_ptr<UniverseClient const>;
using UniverseClientWeakPtr = std::weak_ptr<UniverseClient>;
using UniverseClientConstWeakPtr = std::weak_ptr<UniverseClient const>;
using UniverseClientUPtr = std::unique_ptr<UniverseClient>;
using UniverseClientConstUPtr = std::unique_ptr<UniverseClient const>;
}

namespace Star {

class GraphicsMenu;
using GraphicsMenuPtr = std::shared_ptr<GraphicsMenu>;
using GraphicsMenuConstPtr = std::shared_ptr<GraphicsMenu const>;
using GraphicsMenuWeakPtr = std::weak_ptr<GraphicsMenu>;
using GraphicsMenuConstWeakPtr = std::weak_ptr<GraphicsMenu const>;
using GraphicsMenuUPtr = std::unique_ptr<GraphicsMenu>;
using GraphicsMenuConstUPtr = std::unique_ptr<GraphicsMenu const>;
class ShadersMenu;
using ShadersMenuPtr = std::shared_ptr<ShadersMenu>;
using ShadersMenuConstPtr = std::shared_ptr<ShadersMenu const>;
using ShadersMenuWeakPtr = std::weak_ptr<ShadersMenu>;
using ShadersMenuConstWeakPtr = std::weak_ptr<ShadersMenu const>;
using ShadersMenuUPtr = std::unique_ptr<ShadersMenu>;
using ShadersMenuConstUPtr = std::unique_ptr<ShadersMenu const>;

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
