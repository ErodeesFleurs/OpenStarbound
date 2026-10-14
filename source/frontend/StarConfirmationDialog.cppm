module;
#include "StarConfig.hpp"
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
import star.rpc_promise;


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
