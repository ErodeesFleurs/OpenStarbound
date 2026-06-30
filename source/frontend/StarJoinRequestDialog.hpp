#pragma once

#include "StarIAssets.hpp"
#include "StarPane.hpp"
#include "StarRpcPromise.hpp"

namespace Star {

class JoinRequestDialog;
using JoinRequestDialogPtr = SharedPtr<JoinRequestDialog>;

class JoinRequestDialog : public Pane {
public:
  struct Services {
    IAssetsConstPtr assets;
  };

  JoinRequestDialog(Services services = {});

  virtual ~JoinRequestDialog() = default;

  void displayRequest(String const& userName, function<void(P2PJoinRequestReply)> callback);

  void dismissed() override;

private:
  void reply(P2PJoinRequestReply reply);

  IAssetsConstPtr m_assets;
  function<void(P2PJoinRequestReply)> m_callback;
  bool m_confirmed;
};

}
