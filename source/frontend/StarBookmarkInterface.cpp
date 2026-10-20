import std;
import star.json;
import star.strong_typedef;
import star.data_stream;
import star.game_types;
import star.variant;
import star.random;
import star.weighted_pool;
import star.either;
import star.casting;
import star.input_event;
import star.color;
import star.font;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.bi_map;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.format;
import star.version;
import star.string;
import star.text;
import star.asset_path;
import star.maybe;
import star.listener;
import star.set;
import star.audio;
import star.map;


// Parse foundations before importing global bookmark value types on GCC.
import star.uuid;
import star.celestial_coordinate;
import star.warping;
import star.sky_types;
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.sky_parameters;
import star.system_world;
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
import star.player_universe_map;


import star.bookmark_interface;
import star.widget_parsing;
import star.gui_reader;
import star.button_group;
import star.button_widget;
import star.image_widget;
import star.text_box_widget;
import star.label_widget;

namespace Star {

EditBookmarkDialog::EditBookmarkDialog(GuiContext& guiContext, std::shared_ptr<PlayerUniverseMap> playerUniverseMap) : Pane(guiContext) {
  m_playerUniverseMap = playerUniverseMap;

  GuiReader reader(*context());
  auto assets = context()->root().assets();
  reader.registerCallback("ok", [this](Widget*) { ok(); });
  reader.registerCallback("remove", [this](Widget*) { remove(); });
  reader.registerCallback("close", [this](Widget*) { close(); });
  reader.registerCallback("name", [](Widget*) {});
  reader.construct(assets->json("/interface/windowconfig/editbookmark.config:paneLayout"), this);
  dismiss();
}

void EditBookmarkDialog::setBookmark(TeleportBookmark bookmark) {
  m_bookmark = bookmark;

  m_isNew = true;
  for (auto& existing : m_playerUniverseMap->teleportBookmarks()) {
    if (existing == bookmark) {
      m_bookmark.bookmarkName = existing.bookmarkName;
      m_isNew = false;
    }
  }
}

void EditBookmarkDialog::show() {
  Pane::show();

  if (m_isNew) {
    fetchChild<LabelWidget>("lblTitle")->setText("NEW BOOKMARK");
    fetchChild<ButtonWidget>("remove")->hide();
  } else {
    fetchChild<LabelWidget>("lblTitle")->setText("EDIT BOOKMARK");
    fetchChild<ButtonWidget>("remove")->show();
  }

  auto assets = context()->root().assets();
  fetchChild<ImageWidget>("imgIcon")->setImage(strf("/interface/bookmarks/icons/{}.png", m_bookmark.icon));

  fetchChild<LabelWidget>("lblPlanetName")->setText(m_bookmark.targetName);
  fetchChild<TextBoxWidget>("name")->setText(m_bookmark.bookmarkName, false);
  fetchChild<TextBoxWidget>("name")->focus();
}

void EditBookmarkDialog::ok() {
  m_bookmark.bookmarkName = fetchChild<TextBoxWidget>("name")->getText();
  if (m_bookmark.bookmarkName.empty())
    m_bookmark.bookmarkName = m_bookmark.targetName;
  if (!m_isNew)
    m_playerUniverseMap->removeTeleportBookmark(m_bookmark);
  m_playerUniverseMap->addTeleportBookmark(m_bookmark);
  dismiss();
}

void EditBookmarkDialog::remove() {
  m_playerUniverseMap->removeTeleportBookmark(m_bookmark);
  dismiss();
}

void EditBookmarkDialog::close() {
  dismiss();
}

void setupBookmarkEntry(std::shared_ptr<Widget> const& entry, TeleportBookmark const& bookmark) {
  entry->fetchChild<LabelWidget>("name")->setText(bookmark.bookmarkName);
  entry->fetchChild<LabelWidget>("planetName")->setText(bookmark.targetName);
  entry->fetchChild<ImageWidget>("icon")->setImage(strf("/interface/bookmarks/icons/{}.png", bookmark.icon));
}

}
