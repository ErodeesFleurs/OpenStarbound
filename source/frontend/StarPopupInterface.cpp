#include "StarPopupInterface.hpp"
#include "StarGuiReader.hpp"
#include "StarException.hpp"
#include "StarLabelWidget.hpp"
#include "StarRandom.hpp"
#include "StarAssets.hpp"

namespace Star {

PopupInterface::PopupInterface(Services services)
  : m_assets(std::move(services.assets)) {
  if (!m_assets)
    throw StarException("PopupInterface requires assets service");

  GuiReader reader;

  reader.registerCallback("close", [=, this](Widget*) { dismiss(); });
  reader.registerCallback("ok", [=, this](Widget*) { dismiss(); });

  reader.construct(m_assets->json("/interface/windowconfig/popup.config:paneLayout"), this);
}

void PopupInterface::displayMessage(String const& message, String const& title, String const& subtitle, Maybe<String> const& onShowSound) {
  setTitleString(title, subtitle);
  fetchChild<LabelWidget>("message")->setText(message);
  show();
  auto sound = onShowSound.value(Random::randValueFrom(m_assets->json("/interface/windowconfig/popup.config:onShowSound").toArray(), "").toString());
  if (!sound.empty())
    context()->playAudio(sound);
}

}
