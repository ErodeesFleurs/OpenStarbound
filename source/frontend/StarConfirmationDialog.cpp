import std;
import star.json;
#include "StarCasting.hpp"
import star.input_event;
import star.color;
import star.font;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.bi_map;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.version;
import star.string;
import star.text;
import star.asset_path;
import star.maybe;
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;
import star.rpc_promise;
import star.random;


import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
import star.application_controller;
import star.variant;
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

import star.algorithm;
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

  reader.registerCallback("close", std::bind(&ConfirmationDialog::dismiss, this));
  reader.registerCallback("cancel", std::bind(&ConfirmationDialog::dismiss, this));
  reader.registerCallback("ok", std::bind(&ConfirmationDialog::ok, this));

  m_confirmed = false;

  String paneLayoutPath =
      config.optString("paneLayout").value("/interface/windowconfig/confirmation.config:paneLayout");
  reader.construct(assets->json(paneLayoutPath), this);

  ImageWidgetPtr titleIcon = {};
  if (config.contains("icon"))
    titleIcon = std::make_shared<ImageWidget>(config.getString("icon"));

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
