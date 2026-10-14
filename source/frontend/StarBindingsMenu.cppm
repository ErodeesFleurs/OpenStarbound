module;
import std;
import star.json;

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
#include "StarLuaComponents.hpp"




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
import star.widget_parsing;
import star.gui_reader;
import star.base_script_pane;

namespace Star {

class BindingsMenu;
using BindingsMenuPtr = std::shared_ptr<BindingsMenu>;
using BindingsMenuConstPtr = std::shared_ptr<BindingsMenu const>;
using BindingsMenuWeakPtr = std::weak_ptr<BindingsMenu>;
using BindingsMenuConstWeakPtr = std::weak_ptr<BindingsMenu const>;
using BindingsMenuUPtr = std::unique_ptr<BindingsMenu>;
using BindingsMenuConstUPtr = std::unique_ptr<BindingsMenu const>;

class BindingsMenu : public BaseScriptPane {
public:
  BindingsMenu(Json const& config);

  virtual void show() override;
  void displayed() override;
  void dismissed() override;

private:

};

}

export module star.bindings_menu;

export namespace Star {
  using ::Star::BindingsMenu;
  using ::Star::BindingsMenuPtr;
  using ::Star::BindingsMenuConstPtr;
  using ::Star::BindingsMenuWeakPtr;
  using ::Star::BindingsMenuConstWeakPtr;
  using ::Star::BindingsMenuUPtr;
  using ::Star::BindingsMenuConstUPtr;
}
