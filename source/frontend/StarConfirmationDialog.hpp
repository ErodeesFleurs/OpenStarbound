#pragma once

#include "StarAssets.hpp"
#include "StarPane.hpp"
#include "StarRpcPromise.hpp"

namespace Star {

class ConfirmationDialog;
using ConfirmationDialogPtr = SharedPtr<ConfirmationDialog>;

class ConfirmationDialog : public Pane {
public:
  struct Services {
    AssetsConstPtr assets;
    GuiContext& guiContext;
  };

  explicit ConfirmationDialog(Services services);

  virtual ~ConfirmationDialog() = default;

  void displayConfirmation(Json const& dialogConfig, RpcPromiseKeeper<Json> resultPromise);
  void displayConfirmation(Json const& dialogConfig, WidgetCallbackFunc okCallback, WidgetCallbackFunc cancelCallback);
  
  Maybe<EntityId> sourceEntityId();

  void dismissed() override;

private:
  void ok();

  WidgetCallbackFunc m_okCallback;
  WidgetCallbackFunc m_cancelCallback;
  bool m_confirmed = false;

  Maybe<EntityId> m_sourceEntityId;

  Maybe<RpcPromiseKeeper<Json>> m_resultPromise;
  AssetsConstPtr m_assets;
};

}
