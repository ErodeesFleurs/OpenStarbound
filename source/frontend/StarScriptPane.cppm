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

STAR_CLASS(CanvasWidget);
STAR_CLASS(ScriptPane);
STAR_CLASS(UniverseClient);

class ScriptPane : public BaseScriptPane {
public:
  ScriptPane(UniverseClientPtr client, Json config, EntityId sourceEntityId = NullEntityId);

  void displayed() override;
  void dismissed() override;

  void tick(float dt) override;

  PanePtr createTooltip(Vec2I const& screenPosition) override;

  bool openWithInventory() const;
  bool closeWithInventory() const;

  EntityId sourceEntityId() const;

  LuaCallbacks makePaneCallbacks() override;
private:
  UniverseClientPtr m_client;
  EntityId m_sourceEntityId;
};

}

export module star.script_pane;

export namespace Star {
  using ::Star::CanvasWidget;
  using ::Star::CanvasWidgetPtr;
  using ::Star::CanvasWidgetConstPtr;
  using ::Star::CanvasWidgetWeakPtr;
  using ::Star::CanvasWidgetConstWeakPtr;
  using ::Star::CanvasWidgetUPtr;
  using ::Star::CanvasWidgetConstUPtr;
  using ::Star::ScriptPane;
  using ::Star::ScriptPanePtr;
  using ::Star::ScriptPaneConstPtr;
  using ::Star::ScriptPaneWeakPtr;
  using ::Star::ScriptPaneConstWeakPtr;
  using ::Star::ScriptPaneUPtr;
  using ::Star::ScriptPaneConstUPtr;
  using ::Star::UniverseClient;
  using ::Star::UniverseClientPtr;
  using ::Star::UniverseClientConstPtr;
  using ::Star::UniverseClientWeakPtr;
  using ::Star::UniverseClientConstWeakPtr;
  using ::Star::UniverseClientUPtr;
  using ::Star::UniverseClientConstUPtr;
}
