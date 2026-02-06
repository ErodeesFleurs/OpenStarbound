#pragma once

#include <optional>

#include "StarPane.hpp"
#include "StarRpcPromise.hpp"

namespace Star {

STAR_CLASS(ConfirmationDialog);

class ConfirmationDialog : public Pane {
public:
  ConfirmationDialog();

  virtual ~ConfirmationDialog() {}

  void displayConfirmation(Json const& dialogConfig, RpcPromiseKeeper<Json> resultPromise);
  void displayConfirmation(Json const& dialogConfig, WidgetCallbackFunc okCallback, WidgetCallbackFunc cancelCallback);
  
  std::optional<EntityId> sourceEntityId();

  void dismissed() override;

private:
  void ok();

  WidgetCallbackFunc m_okCallback;
  WidgetCallbackFunc m_cancelCallback;
  bool m_confirmed;

  std::optional<EntityId> m_sourceEntityId;

  std::optional<RpcPromiseKeeper<Json>> m_resultPromise;
};

}
