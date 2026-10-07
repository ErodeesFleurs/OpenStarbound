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
#include "StarRpcPromise.hpp"


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

STAR_CLASS(ConfirmationDialog);

class ConfirmationDialog : public Pane {
public:
  ConfirmationDialog();

  virtual ~ConfirmationDialog() {}

  void displayConfirmation(Json const& dialogConfig, RpcPromiseKeeper<Json> resultPromise);
  void displayConfirmation(Json const& dialogConfig, WidgetCallbackFunc okCallback, WidgetCallbackFunc cancelCallback);
  
  Maybe<EntityId> sourceEntityId();

  void dismissed() override;

private:
  void ok();

  WidgetCallbackFunc m_okCallback;
  WidgetCallbackFunc m_cancelCallback;
  bool m_confirmed;

  Maybe<EntityId> m_sourceEntityId;

  Maybe<RpcPromiseKeeper<Json>> m_resultPromise;
};

}

export module star.confirmation_dialog;

export namespace Star {
  using ::Star::ConfirmationDialog;
  using ::Star::ConfirmationDialogPtr;
  using ::Star::ConfirmationDialogConstPtr;
  using ::Star::ConfirmationDialogWeakPtr;
  using ::Star::ConfirmationDialogConstWeakPtr;
  using ::Star::ConfirmationDialogUPtr;
  using ::Star::ConfirmationDialogConstUPtr;
}
