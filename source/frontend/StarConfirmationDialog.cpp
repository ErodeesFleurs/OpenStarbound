#include "StarConfirmationDialog.hpp"
#include "StarAlgorithm.hpp"
#include "StarGuiReader.hpp"
#include "StarLabelWidget.hpp"
#include "StarButtonWidget.hpp"
#include "StarImageWidget.hpp"
#include "StarRandom.hpp"
#include "StarAssets.hpp"

namespace Star {

ConfirmationDialog::ConfirmationDialog(Services services)
  : Pane(services.guiContext),
    m_assets(requireServiceValueAs<StarException>(std::move(services.assets), "ConfirmationDialog", "assets")) {
}

void ConfirmationDialog::displayConfirmation(Json const& dialogConfig, RpcPromiseKeeper<Json> resultPromise) {
  m_resultPromise = resultPromise;
  displayConfirmation(dialogConfig, [this] (Widget*) { m_resultPromise->fulfill(true); }, [this] (Widget*) { m_resultPromise->fulfill(false); } );
}

void ConfirmationDialog::displayConfirmation(Json const& dialogConfig, WidgetCallbackFunc okCallback, WidgetCallbackFunc cancelCallback) {
  Json config;
  if (dialogConfig.isType(Json::Type::String))
    config = m_assets->json(dialogConfig.toString());
  else
    config = dialogConfig;

  removeAllChildren();

  GuiReader reader(context());

  m_okCallback = requireServiceValueAs<StarException>(std::move(okCallback), "ConfirmationDialog", "ok callback");
  m_cancelCallback = requireServiceValueAs<StarException>(std::move(cancelCallback), "ConfirmationDialog", "cancel callback");

  reader.registerCallback("close", [this](Widget*) { dismiss(); });
  reader.registerCallback("cancel", [this](Widget*) { dismiss(); });
  reader.registerCallback("ok", [this](Widget*) { ok(); });

  m_confirmed = false;

  String paneLayoutPath =
      config.optString("paneLayout").value("/interface/windowconfig/confirmation.config:paneLayout");
  reader.construct(m_assets->json(paneLayoutPath), this);

  UniquePtr<Widget> titleIcon;
  if (config.contains("icon"))
    titleIcon = make_unique<ImageWidget>(context(), config.getString("icon"));

  setTitle(std::move(titleIcon), config.getString("title", ""), config.getString("subtitle", ""));
  fetchChild<LabelWidget>("message")->setText(config.getString("message"));

  if (config.contains("okCaption"))
    fetchChild<ButtonWidget>("ok")->setText(config.getString("okCaption"));
  if (config.contains("cancelCaption"))
    fetchChild<ButtonWidget>("cancel")->setText(config.getString("cancelCaption"));

  m_sourceEntityId = config.optInt("sourceEntityId");

  for (auto const& [widgetName, imageConfig] : config.optObject("images").value({})) {
    auto widget = fetchChild<ImageWidget>(widgetName);
    if (imageConfig.isType(Json::Type::String))
      widget->setImage(imageConfig.toString());
    else
      widget->setDrawables(imageConfig.toArray().transformed(construct<Drawable>()));
  }

  for (auto const& [widgetName, labelConfig] : config.optObject("labels").value({})) {
    auto widget = fetchChild<LabelWidget>(widgetName);
    widget->setText(labelConfig.toString());
  }

  show();
  auto sound = Random::randValueFrom(m_assets->json("/interface/windowconfig/confirmation.config:onShowSound").toArray(), "").toString();

  if (!sound.empty())
    context().playAudio(sound);
}

Maybe<EntityId> ConfirmationDialog::sourceEntityId() {
  return m_sourceEntityId;
}

void ConfirmationDialog::dismissed() {
  if (!m_confirmed)
    m_cancelCallback(this);

  Pane::dismissed();
}

void ConfirmationDialog::ok() {
  m_okCallback(this);
  m_confirmed = true;
  dismiss();
}

}
