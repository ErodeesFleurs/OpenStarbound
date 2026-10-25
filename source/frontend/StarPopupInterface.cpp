import std;
import star.format;
import star.json;
import star.casting;
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
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;
import star.random;


import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
import star.application_controller;
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


import star.popup_interface;
import star.widget_parsing;
import star.gui_reader;
import star.label_widget;

namespace Star {

PopupInterface::PopupInterface(GuiContext& guiContext) : Pane(guiContext) {}

std::expected<std::shared_ptr<PopupInterface>, StarException> PopupInterface::create(GuiContext& guiContext) {
  auto pane = std::shared_ptr<PopupInterface>(new PopupInterface(guiContext));
  auto initialized = pane->initializePane();
  if (!initialized)
    return std::unexpected(std::move(initialized.error()));
  auto assets = guiContext.root().assets();
  if (!assets)
    return std::unexpected(std::move(assets.error()));
  GuiReader reader(guiContext);
  auto close = [paneRef = std::ref(*pane)](Widget&) -> std::expected<void, StarException> { paneRef.get().dismiss(); return {}; };
  reader.registerCallback("close", close);
  reader.registerCallback("ok", close);
  auto layout = (*assets)->json("/interface/windowconfig/popup.config:paneLayout");
  if (!layout)
    return std::unexpected(std::move(layout.error()));
  auto constructed = reader.construct(*layout, *pane);
  if (!constructed)
    return std::unexpected(std::move(constructed.error()));
  return pane;
}

std::expected<void, StarException> PopupInterface::displayMessage(String const& message, String const& title, String const& subtitle, std::optional<String> const& onShowSound) {
  setTitleString(title, subtitle);
  auto label = fetchChild<LabelWidget>("message");
  if (!label)
    return std::unexpected(std::move(label.error()));
  if (!*label)
    return std::unexpected(GUIBuilderException("Missing popup message label"));
  auto text = (*label)->setText(message);
  if (!text)
    return std::unexpected(std::move(text.error()));
  auto shown = show();
  if (!shown)
    return std::unexpected(std::move(shown.error()));
  auto assets = context().root().assets();
  if (!assets)
    return std::unexpected(std::move(assets.error()));
  auto soundsJson = (*assets)->json("/interface/windowconfig/popup.config:onShowSound");
  if (!soundsJson)
    return std::unexpected(std::move(soundsJson.error()));
  auto sounds = soundsJson->toArray();
  if (!sounds)
    return std::unexpected(std::move(sounds.error()));
  auto chosen = context().root().random().randValueFrom(*sounds, "").toString();
  if (!chosen)
    return std::unexpected(std::move(chosen.error()));
  auto sound = onShowSound.value_or(std::move(*chosen));
  if (!sound.empty()) {
    auto played = context().playAudio(sound);
    if (!played)
      return std::unexpected(std::move(played.error()));
  }
  return {};
}

}
