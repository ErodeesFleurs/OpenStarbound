module;
#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarThread.hpp"
#include "StarVersion.hpp"


#include "StarApplicationController.hpp"
#include "StarRenderer.hpp"
import star.asset_source;
import star.assets;
import star.root_base;
import star.root;
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
import star.label_widget;
import star.button_group;
import star.button_widget;
import star.configuration;

module star.http_trust_dialog;

namespace Star {

HttpTrustDialog::HttpTrustDialog() : m_confirmed(false) {}

HttpTrustDialog::~HttpTrustDialog() = default;

void HttpTrustDialog::displayRequest(String const& domain, function<void(HttpTrustReply, bool)> callback) {
  const auto assets = Root::singleton().assets();

  removeAllChildren();

  GuiReader reader;

  m_domain = domain;
  m_callback = std::move(callback);

  reader.registerCallback("yes", [this](Widget*) { reply(HttpTrustReply::Allow); });
  reader.registerCallback("no", [this](Widget*) { reply(HttpTrustReply::Deny); });
  // reader.registerCallback("rememberCheckbox", [this](Widget*) {
  //   // just to capture it
  // });

  m_confirmed = false;

  const Json config = assets->json("/interface/httpwarning/warning.config");

  reader.construct(config.get("paneLayout"), this);

  // Update message with domain
  const String message = strf("^green;{}^reset;", domain);
  fetchChild<LabelWidget>("domain")->setText(message);
  // fetchChild<ButtonWidget>("yes")->setText("Allow");
  // fetchChild<ButtonWidget>("no")->setText("Deny"); // I did it cuz of: if some smart guy will swap yes/no buttons texts in the config file
  fetchChild<ButtonWidget>("yes")->setText("✅");
  fetchChild<ButtonWidget>("no")->setText("❌"); // Emoji buttons dont need to be translated

  show();
}

void HttpTrustDialog::reply(const HttpTrustReply replyType) {
  m_confirmed = true;

  // Check if "remember" checkbox is checked
  const bool remember = fetchChild<ButtonWidget>("rememberCheckbox")->isChecked();

  // If allowing and remember is checked, add to trusted list
  if (replyType == HttpTrustReply::Allow && remember) {
    auto& root = Root::singleton();
    const auto config = root.configuration();

    JsonArray trustedSites;
    if (auto existing = config->getPath("safe.luaHttp.trustedSites").optArray())
      trustedSites = *existing;

    // Check if already exists
    bool exists = false;
    for (auto const& site : trustedSites) {
      if (site.toString() == m_domain) {
        exists = true;
        break;
      }
    }

    if (!exists) {
      trustedSites.append(m_domain);
      config->setPath("safe.luaHttp.trustedSites", trustedSites);
    }
  }

  m_callback(replyType, remember);
  dismiss();
}

void HttpTrustDialog::dismissed() {
  if (!m_confirmed)
    m_callback(HttpTrustReply::Deny, false);

  Pane::dismissed();
}

}
