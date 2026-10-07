#include "StarJson.hpp"
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
#include "StarBiMap.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarConfig.hpp"
import star.version;
#include "StarStringView.hpp"
#include "StarString.hpp"
import star.text;
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarRpcPromise.hpp"
#include "StarRandom.hpp"


import star.application;
#include "StarStatisticsService.hpp"
#include "StarP2PNetworkingService.hpp"
#include "StarUserGeneratedContentService.hpp"
#include "StarDesktopService.hpp"
#include "StarImage.hpp"
import star.application_controller;
#include "StarVariant.hpp"
import star.renderer;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
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


import star.confirmation_dialog;
import star.widget_parsing;
import star.gui_reader;
import star.label_widget;
import star.button_group;
import star.button_widget;
import star.image_widget;

namespace Star {

ConfirmationDialog::ConfirmationDialog() {}

void ConfirmationDialog::displayConfirmation(Json const& dialogConfig, RpcPromiseKeeper<Json> resultPromise) {
  m_resultPromise = resultPromise;
  displayConfirmation(dialogConfig, [this] (Widget*) { m_resultPromise->fulfill(true); }, [this] (Widget*) { m_resultPromise->fulfill(false); } );
}

void ConfirmationDialog::displayConfirmation(Json const& dialogConfig, WidgetCallbackFunc okCallback, WidgetCallbackFunc cancelCallback) {
  Json config;
  if (dialogConfig.isType(Json::Type::String))
    config = Root::singleton().assets()->json(dialogConfig.toString());
  else
    config = dialogConfig;

  auto assets = Root::singleton().assets();

  removeAllChildren();

  GuiReader reader;

  m_okCallback = std::move(okCallback);
  m_cancelCallback = std::move(cancelCallback);

  reader.registerCallback("close", bind(&ConfirmationDialog::dismiss, this));
  reader.registerCallback("cancel", bind(&ConfirmationDialog::dismiss, this));
  reader.registerCallback("ok", bind(&ConfirmationDialog::ok, this));

  m_confirmed = false;

  String paneLayoutPath =
      config.optString("paneLayout").value("/interface/windowconfig/confirmation.config:paneLayout");
  reader.construct(assets->json(paneLayoutPath), this);

  ImageWidgetPtr titleIcon = {};
  if (config.contains("icon"))
    titleIcon = make_shared<ImageWidget>(config.getString("icon"));

  setTitle(titleIcon, config.getString("title", ""), config.getString("subtitle", ""));
  fetchChild<LabelWidget>("message")->setText(config.getString("message"));

  if (config.contains("okCaption"))
    fetchChild<ButtonWidget>("ok")->setText(config.getString("okCaption"));
  if (config.contains("cancelCaption"))
    fetchChild<ButtonWidget>("cancel")->setText(config.getString("cancelCaption"));

  m_sourceEntityId = config.optInt("sourceEntityId");

  for (auto image : config.optObject("images").value({})) {
    auto widget = fetchChild<ImageWidget>(image.first);
    if (image.second.isType(Json::Type::String))
      widget->setImage(image.second.toString());
    else
      widget->setDrawables(image.second.toArray().transformed(construct<Drawable>()));
  }

  for (auto label : config.optObject("labels").value({})) {
    auto widget = fetchChild<LabelWidget>(label.first);
    widget->setText(label.second.toString());
  }

  show();
  auto sound = Random::randValueFrom(Root::singleton().assets()->json("/interface/windowconfig/confirmation.config:onShowSound").toArray(), "").toString();

  if (!sound.empty())
    context()->playAudio(sound);
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
