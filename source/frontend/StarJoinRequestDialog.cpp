#include "StarJoinRequestDialog.hpp"
#include "StarAssets.hpp"
#include "StarGuiReader.hpp"
#include "StarRoot.hpp"
#include "StarLabelWidget.hpp"
#include "StarButtonWidget.hpp"
#include "StarImageWidget.hpp"
#include "StarRandom.hpp"

namespace Star {

JoinRequestDialog::JoinRequestDialog(Services services)
  : m_assets(services.assets ? std::move(services.assets) : Root::singleton().assets()), m_confirmed(false) {}

void JoinRequestDialog::displayRequest(String const& userName, function<void(P2PJoinRequestReply)> callback) {
  removeAllChildren();

  GuiReader reader;

  m_callback = std::move(callback);

  reader.registerCallback("yes", [this](Widget*){ reply(P2PJoinRequestReply::Yes); });
  reader.registerCallback("no", [this](Widget*){ reply(P2PJoinRequestReply::No); });
  reader.registerCallback("ignore", [this](Widget*){ reply(P2PJoinRequestReply::Ignore); });

  m_confirmed = false;

  Json config = m_assets->json("/interface/windowconfig/joinrequest.config");

  reader.construct(config.get("paneLayout"), this);

  String message = config.getString("joinMessage").replaceTags(StringMap<String>{{"username", userName}});
  fetchChild<LabelWidget>("message")->setText(message);

  show();
}

void JoinRequestDialog::reply(P2PJoinRequestReply reply) {
  m_confirmed = true;
  m_callback(reply);
  dismiss();
}

void JoinRequestDialog::dismissed() {
  if (!m_confirmed)
    m_callback(P2PJoinRequestReply::No);

  Pane::dismissed();
}

}
