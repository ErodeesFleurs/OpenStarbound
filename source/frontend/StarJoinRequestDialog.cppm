module;

#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
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

enum class P2PJoinRequestReply;

STAR_CLASS(JoinRequestDialog);

class JoinRequestDialog : public Pane {
public:
  JoinRequestDialog();

  virtual ~JoinRequestDialog() {}

  void displayRequest(String const& userName, function<void(P2PJoinRequestReply)> callback);

  void dismissed() override;

private:
  void reply(P2PJoinRequestReply reply);

  function<void(P2PJoinRequestReply)> m_callback;
  bool m_confirmed;
};

}

export module star.join_request_dialog;

export namespace Star {
  using ::Star::JoinRequestDialog;
  using ::Star::JoinRequestDialogPtr;
  using ::Star::JoinRequestDialogConstPtr;
  using ::Star::JoinRequestDialogWeakPtr;
  using ::Star::JoinRequestDialogConstWeakPtr;
  using ::Star::JoinRequestDialogUPtr;
  using ::Star::JoinRequestDialogConstUPtr;
}
