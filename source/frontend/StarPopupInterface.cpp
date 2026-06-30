#include "StarPopupInterface.hpp"
#include "StarAlgorithm.hpp"
#include "StarGuiReader.hpp"
#include "StarLabelWidget.hpp"
#include "StarRandom.hpp"
#include "StarAssets.hpp"

namespace Star {

PopupInterface::PopupInterface(Services services)
  : Pane(services.guiContext),
    m_assets(requireServiceValueAs<StarException>(std::move(services.assets), "PopupInterface", "assets")) {
  GuiReader reader(context());

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
    context().playAudio(sound);
}

}
