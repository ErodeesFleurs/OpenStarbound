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

namespace Star {

STAR_CLASS(LabelWidget);
STAR_CLASS(ButtonWidget);
STAR_CLASS(ListWidget);

class ModsMenu : public Pane {
public:
  ModsMenu();

  void update(float dt) override;

private:
  static String bestModName(JsonObject const& metadata, String const& sourcePath);

  void openLink();
  void openWorkshop();

  StringList m_assetsSources;

  ListWidgetPtr m_modList;
  LabelWidgetPtr m_modName;
  LabelWidgetPtr m_modAuthor;
  LabelWidgetPtr m_modVersion;
  LabelWidgetPtr m_modPath;
  LabelWidgetPtr m_modDescription;

  ButtonWidgetPtr m_linkButton;
  ButtonWidgetPtr m_copyLinkButton;
};

}

export module star.mods_menu;

export namespace Star {
  using ::Star::LabelWidget;
  using ::Star::LabelWidgetPtr;
  using ::Star::LabelWidgetConstPtr;
  using ::Star::LabelWidgetWeakPtr;
  using ::Star::LabelWidgetConstWeakPtr;
  using ::Star::LabelWidgetUPtr;
  using ::Star::LabelWidgetConstUPtr;
  using ::Star::ButtonWidget;
  using ::Star::ButtonWidgetPtr;
  using ::Star::ButtonWidgetConstPtr;
  using ::Star::ButtonWidgetWeakPtr;
  using ::Star::ButtonWidgetConstWeakPtr;
  using ::Star::ButtonWidgetUPtr;
  using ::Star::ButtonWidgetConstUPtr;
  using ::Star::ListWidget;
  using ::Star::ListWidgetPtr;
  using ::Star::ListWidgetConstPtr;
  using ::Star::ListWidgetWeakPtr;
  using ::Star::ListWidgetConstWeakPtr;
  using ::Star::ListWidgetUPtr;
  using ::Star::ListWidgetConstUPtr;
  using ::Star::ModsMenu;
}
