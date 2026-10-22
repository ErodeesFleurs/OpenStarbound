import std;
import star.memory;
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
import star.format;
import star.version;
import star.string;
import star.text;
import star.asset_path;
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;
import star.id_map;
import star.rpc_promise;
import star.strong_typedef;
import star.a_star;
import star.ordered_set;
import star.either;
import star.random;
import star.periodic_function;


import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
import star.application_controller;
import star.renderer;
import star.lua_root;
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


import star.songbook_interface;
import star.widget_parsing;
import star.gui_reader;
import star.list_widget;
import star.label_widget;
import star.text_box_widget;
import star.uuid;
import star.item_descriptor;
import star.animation;
import star.particle;
import star.animated_part_set;
import star.light_source;
import star.networked_animator;
import star.humanoid;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.entity;
import star.interaction_types;
import star.tile_damage;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
import star.force_regions;
import star.world;
import star.physics_entity;
import star.anchorable_entity;
import star.mobile_entity;
import star.actor_entity;
import star.tool_user_entity;
import star.lounging_entities;
import star.chat_action;
import star.chatty_entity;
import star.emote_entity;
import star.portrait_entity;
import star.damage_bar_entity;
import star.nametag_entity;
import star.inspectable_entity;
import star.inventory_types;
import star.movement_controller;
import star.platformer_astar_types;
import star.game_timers;
import star.actor_movement_controller;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.ai_types;
import star.entity_rendering_types;
import star.entity_rendering;
import star.player_types;
import star.lua_components;
import star.lua_actor_movement_component;
import star.lua_animation_component;
import star.radio_message_database;
import star.player;

import star.songbook;

import star.algorithm;
namespace Star {

String const SongPathPrefix = "/songs/";

SongbookInterface::SongbookInterface(GuiContext& guiContext, std::shared_ptr<Player> player)
  : Pane(guiContext), m_player(std::move(player)) {}

std::expected<std::shared_ptr<SongbookInterface>, StarException> SongbookInterface::create(GuiContext& guiContext, std::shared_ptr<Player> player) {
  auto pane = std::shared_ptr<SongbookInterface>(new SongbookInterface(guiContext, std::move(player)));
  auto base = pane->initializePane();
  if (!base) return std::unexpected(std::move(base.error()));
  auto initialized = pane->initialize();
  if (!initialized) return std::unexpected(std::move(initialized.error()));
  return pane;
}

std::expected<void, StarException> SongbookInterface::initialize() {
  auto assets = context()->root().assets();
  if (!assets) return std::unexpected(std::move(assets.error()));
  GuiReader reader(*context());
  reader.registerCallback("close", [this](Widget*) -> std::expected<void, StarException> { dismiss(); return {}; });
  reader.registerCallback("btnPlay", [this](Widget*) -> std::expected<void, StarException> {
    auto played = play();
    if (!played) return std::unexpected(std::move(played.error()));
    if (*played) dismiss();
    return {};
  });
  reader.registerCallback("group", [](Widget*) -> std::expected<void, StarException> { return {}; });
  reader.registerCallback("search", [](Widget*) -> std::expected<void, StarException> { return {}; });
  auto layout = (*assets)->json("/interface/windowconfig/songbook.config:paneLayout");
  if (!layout) return std::unexpected(std::move(layout.error()));
  auto constructed = reader.construct(*layout, this);
  if (!constructed) return std::unexpected(std::move(constructed.error()));
  m_reloadSubscription = context()->root().registerReloadListener(
    m_reloadListener = std::make_shared<CallbackListener>([this]() { return refresh(true); })
  );
  return refresh(true);
}

SongbookInterface::~SongbookInterface() {
  m_reloadSubscription.disconnect();
}

std::expected<void, StarException> SongbookInterface::update(float dt) {
  auto updated = Pane::update(dt);
  if (!updated) return std::unexpected(std::move(updated.error()));
  return refresh();
}

std::expected<bool, StarException> SongbookInterface::play() {
  auto songList = fetchChild<ListWidget>("songs.list");
  if (!songList) return std::unexpected(std::move(songList.error()));
  auto songWidget = (*songList)->selectedWidget();
  if (!songWidget) return false;
  auto index = songWidget->data().toUInt();
  if (!index) return std::unexpected(std::move(index.error()));
  auto songName = m_files.at(*index);
  if (!songName) return std::unexpected(std::move(songName.error()));
  auto group = fetchChild<TextBoxWidget>("group");
  if (!group) return std::unexpected(std::move(group.error()));
  JsonObject song;
  song["resource"] = **songName;
  auto assets = context()->root().assets();
  if (!assets) return std::unexpected(std::move(assets.error()));
  auto buffer = (*assets)->bytes(**songName);
  if (!buffer) return std::unexpected(std::move(buffer.error()));
  song["abc"] = String((*buffer)->ptr(), (*buffer)->size());
  m_player->songbook()->play(song, (*group)->getText());
  return true;
}

std::expected<void, StarException> SongbookInterface::refresh(bool reloadFiles) {
  if (reloadFiles) {
    auto assets = context()->root().assets();
    if (!assets) return std::unexpected(std::move(assets.error()));
    auto scanned = (*assets)->scanExtension(".abc");
    if (!scanned) return std::unexpected(std::move(scanned.error()));
    m_files.clear();
    for (auto const& path : **scanned) {
      String song = path.string();
      auto inSongs = song.beginsWith(SongPathPrefix, String::CaseInsensitive);
      if (!inSongs) return std::unexpected(std::move(inSongs.error()));
      if (!*inSongs) {
        auto logged = context()->root().logger().warn("Song '{}' isn't in {}, ignoring", song, SongPathPrefix);
        if (!logged) return std::unexpected(std::move(logged.error()));
      } else {
        m_files.append(std::move(song));
      }
    }
    auto sorted = sortByComputedValue(m_files, [](String const& song) { return song.toLower(); });
    if (!sorted) return std::unexpected(std::move(sorted.error()));
  }
  auto searchWidget = fetchChild<TextBoxWidget>("search");
  if (!searchWidget) return std::unexpected(std::move(searchWidget.error()));
  auto const& search = (*searchWidget)->getText();
  if (m_lastSearch == search && !reloadFiles) return {};
  m_lastSearch = search;
  auto songList = fetchChild<ListWidget>("songs.list");
  if (!songList) return std::unexpected(std::move(songList.error()));
  auto cleared = (*songList)->clear();
  if (!cleared) return std::unexpected(std::move(cleared.error()));
  auto prefixSize = SongPathPrefix.size();
  if (!prefixSize) return std::unexpected(std::move(prefixSize.error()));
  auto searchSize = search.size();
  if (!searchSize) return std::unexpected(std::move(searchSize.error()));
  for (std::size_t i = 0; i != m_files.size(); ++i) {
    StringView path = m_files[i];
    auto pathSize = path.size();
    if (!pathSize) return std::unexpected(std::move(pathSize.error()));
    auto song = path.substr(*prefixSize, *pathSize - (*prefixSize + 4));
    if (!song) return std::unexpected(std::move(song.error()));
    String text;
    if (search.empty()) {
      text = String(*song);
    } else {
      auto find = song->find(search, 0, String::CaseInsensitive);
      if (!find) return std::unexpected(std::move(find.error()));
      if (*find == NPos) continue;
      std::size_t last = 0;
      do {
        auto before = song->substr(last, *find - last);
        if (!before) return std::unexpected(std::move(before.error()));
        auto matched = song->substr(*find, *searchSize);
        if (!matched) return std::unexpected(std::move(matched.error()));
        text += strf("^#bbb;{}^#7f7;{}", *before, *matched);
        last = *find + *searchSize;
        find = song->find(search, last, String::CaseInsensitive);
        if (!find) return std::unexpected(std::move(find.error()));
      } while (*find != NPos);
      auto rest = song->substr(last);
      if (!rest) return std::unexpected(std::move(rest.error()));
      text += strf("^#bbb;{}", *rest);
    }
    auto widget = (*songList)->addItem();
    if (!widget) return std::unexpected(std::move(widget.error()));
    (*widget)->setData(i);
    auto songName = (*widget)->fetchChild<LabelWidget>("songName");
    if (!songName) return std::unexpected(std::move(songName.error()));
    auto labeled = (*songName)->setText(text);
    if (!labeled) return std::unexpected(std::move(labeled.error()));
    auto shown = (*widget)->show();
    if (!shown) return std::unexpected(std::move(shown.error()));
  }
  return {};
}

}