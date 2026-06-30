#include "StarHttpTrustDialog.hpp"
#include "StarAssets.hpp"
#include "StarConfiguration.hpp"
#include "StarException.hpp"
#include "StarGuiReader.hpp"
#include "StarLabelWidget.hpp"
#include "StarButtonWidget.hpp"

namespace Star {

HttpTrustDialog::HttpTrustDialog(Services services)
  : Pane(services.guiContext),
    m_assets(std::move(services.assets)),
    m_configuration(std::move(services.configuration)),
    m_confirmed(false) {
  if (!m_assets)
    throw StarException("HttpTrustDialog requires assets service");
  if (!m_configuration)
    throw StarException("HttpTrustDialog requires configuration service");
}

void HttpTrustDialog::displayRequest(String const& domain, function<void(HttpTrustReply, bool)> callback) {
  removeAllChildren();

  GuiReader reader(context());

  m_domain = domain;
  m_callback = std::move(callback);

  reader.registerCallback("yes", [this](Widget*) { reply(HttpTrustReply::Allow); });
  reader.registerCallback("no", [this](Widget*) { reply(HttpTrustReply::Deny); });
  // reader.registerCallback("rememberCheckbox", [this](Widget*) {
  //   // just to capture it
  // });

  m_confirmed = false;

  const Json config = m_assets->json("/interface/httpwarning/warning.config");

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
    JsonArray trustedSites;
    if (auto existing = m_configuration->getPath("safe.luaHttp.trustedSites").optArray())
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
      m_configuration->setPath("safe.luaHttp.trustedSites", trustedSites);
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
