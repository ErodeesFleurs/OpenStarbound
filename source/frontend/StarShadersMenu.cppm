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
STAR_CLASS(UniverseClient);
}

namespace Star {

STAR_CLASS(ShadersMenu);

class ShadersMenu : public BaseScriptPane {
public:
  ShadersMenu(Json const& config, UniverseClientPtr client);

  virtual void show() override;
  void displayed() override;
  void dismissed() override;

private:
  UniverseClientPtr m_client;
};

}

export module star.shaders_menu;

export namespace Star {
  using ::Star::ShadersMenu;
  using ::Star::ShadersMenuPtr;
  using ::Star::ShadersMenuConstPtr;
  using ::Star::ShadersMenuWeakPtr;
  using ::Star::ShadersMenuConstWeakPtr;
  using ::Star::ShadersMenuUPtr;
  using ::Star::ShadersMenuConstUPtr;
}
