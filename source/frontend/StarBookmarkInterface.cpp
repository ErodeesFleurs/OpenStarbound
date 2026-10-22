import std;
import star.json;
import star.strong_typedef;
import star.data_stream;
import star.game_types;
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
import star.format;

namespace Star {

EditBookmarkDialog::EditBookmarkDialog(GuiContext& guiContext, std::shared_ptr<PlayerUniverseMap> playerUniverseMap)
  : Pane(guiContext), m_playerUniverseMap(std::move(playerUniverseMap)), m_isNew(true) {}

std::expected<std::shared_ptr<EditBookmarkDialog>, StarException> EditBookmarkDialog::create(GuiContext& guiContext, std::shared_ptr<PlayerUniverseMap> playerUniverseMap) {
  auto pane = std::shared_ptr<EditBookmarkDialog>(new EditBookmarkDialog(guiContext, std::move(playerUniverseMap)));
  auto base = pane->initializePane();
  if (!base) return std::unexpected(std::move(base.error()));
  auto initialized = pane->initialize();
  if (!initialized) return std::unexpected(std::move(initialized.error()));
  return pane;
}

std::expected<void, StarException> EditBookmarkDialog::initialize() {
  GuiReader reader(*context());
  auto assets = context()->root().assets();
  if (!assets) return std::unexpected(std::move(assets.error()));
  reader.registerCallback("ok", [this](Widget*) { return ok(); });
  reader.registerCallback("remove", [this](Widget*) { return remove(); });
  reader.registerCallback("close", [this](Widget*) -> std::expected<void, StarException> { close(); return {}; });
  reader.registerCallback("name", [](Widget*) -> std::expected<void, StarException> { return {}; });
  auto layout = (*assets)->json("/interface/windowconfig/editbookmark.config:paneLayout");
  if (!layout) return std::unexpected(std::move(layout.error()));
  auto constructed = reader.construct(*layout, this);
  if (!constructed) return std::unexpected(std::move(constructed.error()));
  dismiss();
  return {};
}

std::expected<void, StarException> EditBookmarkDialog::setBookmark(TeleportBookmark bookmark) {
  auto bookmarks = m_playerUniverseMap->teleportBookmarks();
  if (!bookmarks) return std::unexpected(std::move(bookmarks.error()));
  m_bookmark = bookmark;
  m_isNew = true;
  for (auto const& existing : *bookmarks) {
    if (existing == bookmark) {
      m_bookmark.bookmarkName = existing.bookmarkName;
      m_isNew = false;
    }
  }
  return {};
}

std::expected<void, StarException> EditBookmarkDialog::show() {
  auto shown = Pane::show();
  if (!shown) return std::unexpected(std::move(shown.error()));

  auto titleWidget = fetchChild<LabelWidget>("lblTitle");
  if (!titleWidget) return std::unexpected(std::move(titleWidget.error()));
  auto title = (*titleWidget)->setText(m_isNew ? "NEW BOOKMARK" : "EDIT BOOKMARK");
  if (!title) return std::unexpected(std::move(title.error()));
  auto removeWidget = fetchChild<ButtonWidget>("remove");
  if (!removeWidget) return std::unexpected(std::move(removeWidget.error()));
  auto removeVisibility = (*removeWidget)->setVisibility(!m_isNew);
  if (!removeVisibility) return std::unexpected(std::move(removeVisibility.error()));
  auto iconWidget = fetchChild<ImageWidget>("imgIcon");
  if (!iconWidget) return std::unexpected(std::move(iconWidget.error()));
  auto icon = (*iconWidget)->setImage(strf("/interface/bookmarks/icons/{}.png", m_bookmark.icon));
  if (!icon) return std::unexpected(std::move(icon.error()));
  auto planetNameWidget = fetchChild<LabelWidget>("lblPlanetName");
  if (!planetNameWidget) return std::unexpected(std::move(planetNameWidget.error()));
  auto planetName = (*planetNameWidget)->setText(m_bookmark.targetName);
  if (!planetName) return std::unexpected(std::move(planetName.error()));
  auto nameWidget = fetchChild<TextBoxWidget>("name");
  if (!nameWidget) return std::unexpected(std::move(nameWidget.error()));
  auto name = (*nameWidget)->setText(m_bookmark.bookmarkName, false);
  if (!name) return std::unexpected(std::move(name.error()));
  return (*nameWidget)->focus();
}

std::expected<void, StarException> EditBookmarkDialog::ok() {
  auto name = fetchChild<TextBoxWidget>("name");
  if (!name) return std::unexpected(std::move(name.error()));
  m_bookmark.bookmarkName = (*name)->getText();
  if (m_bookmark.bookmarkName.empty())
    m_bookmark.bookmarkName = m_bookmark.targetName;
  if (!m_isNew) {
    auto removed = m_playerUniverseMap->removeTeleportBookmark(m_bookmark);
    if (!removed) return std::unexpected(std::move(removed.error()));
  }
  auto added = m_playerUniverseMap->addTeleportBookmark(m_bookmark);
  if (!added) return std::unexpected(std::move(added.error()));
  dismiss();
  return {};
}

std::expected<void, StarException> EditBookmarkDialog::remove() {
  auto removed = m_playerUniverseMap->removeTeleportBookmark(m_bookmark);
  if (!removed) return std::unexpected(std::move(removed.error()));
  dismiss();
  return {};
}

void EditBookmarkDialog::close() {
  dismiss();
}

std::expected<void, StarException> setupBookmarkEntry(std::shared_ptr<Widget> const& entry, TeleportBookmark const& bookmark) {
  auto nameWidget = entry->fetchChild<LabelWidget>("name");
  if (!nameWidget) return std::unexpected(std::move(nameWidget.error()));
  auto name = (*nameWidget)->setText(bookmark.bookmarkName);
  if (!name) return std::unexpected(std::move(name.error()));
  auto planetNameWidget = entry->fetchChild<LabelWidget>("planetName");
  if (!planetNameWidget) return std::unexpected(std::move(planetNameWidget.error()));
  auto planetName = (*planetNameWidget)->setText(bookmark.targetName);
  if (!planetName) return std::unexpected(std::move(planetName.error()));
  auto iconWidget = entry->fetchChild<ImageWidget>("icon");
  if (!iconWidget) return std::unexpected(std::move(iconWidget.error()));
  return (*iconWidget)->setImage(strf("/interface/bookmarks/icons/{}.png", bookmark.icon));
}

}
