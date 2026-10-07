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
#include "StarIdMap.hpp"
#include "StarArray.hpp"
#include "StarNetElementSystem.hpp"
#include "StarVariant.hpp"
#include "StarRpcPromise.hpp"
#include "StarStrongTypedef.hpp"
#include "StarAStar.hpp"
#include "StarOrderedSet.hpp"
#include "StarEither.hpp"
#include "StarInterpolation.hpp"
#include "StarRandom.hpp"
import star.periodic_function;
#include "StarMatrix3.hpp"
#include "StarNetElement.hpp"


import star.application;
#include "StarStatisticsService.hpp"
#include "StarP2PNetworkingService.hpp"
#include "StarUserGeneratedContentService.hpp"
#include "StarDesktopService.hpp"
#include "StarImage.hpp"
import star.application_controller;
import star.renderer;
#include "StarLuaRoot.hpp"
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
#include "StarLuaComponents.hpp"
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.radio_message_database;
import star.player;

import star.songbook;

namespace Star {

String const SongPathPrefix = "/songs/";

SongbookInterface::SongbookInterface(PlayerPtr player) {
  m_player = std::move(player);

  auto assets = Root::singleton().assets();

  GuiReader reader;

  reader.registerCallback("close", [this](Widget*) { dismiss(); });
  reader.registerCallback("btnPlay",
      [this](Widget*) {
        if (play())
          dismiss();
      });
  reader.registerCallback("group", [=](Widget*) {});
  reader.registerCallback("search", [=](Widget*) {});

  reader.construct(assets->json("/interface/windowconfig/songbook.config:paneLayout"), this);

  Root::singleton().registerReloadListener(
    m_reloadListener = make_shared<CallbackListener>([this]() {
      refresh(true);
    })
  );

  refresh(true);
}

void SongbookInterface::update(float dt) {
  Pane::update(dt);
  refresh();
}

bool SongbookInterface::play() {
  auto songList = fetchChild<ListWidget>("songs.list");
  auto songWidget = songList->selectedWidget();
  if (!songWidget) return false;
  auto& songName = m_files.at(songWidget->data().toUInt());
  auto group = fetchChild<TextBoxWidget>("group")->getText();

  JsonObject song;
  song["resource"] = songName;
  auto buffer = Root::singleton().assets()->bytes(songName);
  song["abc"] = String(buffer->ptr(), buffer->size());

  m_player->songbook()->play(song, group);
  return true;
}

void SongbookInterface::refresh(bool reloadFiles) {
  if (reloadFiles) {
    m_files = Root::singleton().assets()->scanExtension(".abc").values();
    eraseWhere(m_files, [](String& song) {
      if (!song.beginsWith(SongPathPrefix, String::CaseInsensitive)) {
        Logger::warn("Song '{}' isn't in {}, ignoring", song, SongPathPrefix);
        return true;
      }
      return false;
    });
    sort(m_files, [](String const& a, String const& b) -> bool { return b.compare(a, String::CaseInsensitive) > 0; });
  }
  auto& search = fetchChild<TextBoxWidget>("search")->getText();
  if (m_lastSearch != search || reloadFiles) {
    m_lastSearch = search;
    auto songList = fetchChild<ListWidget>("songs.list");
    songList->clear();
    if (search.empty()) {
      for (size_t i = 0; i != m_files.size(); ++i) {
        auto widget = songList->addItem();
        widget->setData(i);
        auto songName = widget->fetchChild<LabelWidget>("songName");
        String const& song = m_files[i];
        songName->setText(song.substr(SongPathPrefix.size(), song.size() - (SongPathPrefix.size() + 4)));
        widget->show();
      }
    } else {
      for (size_t i = 0; i != m_files.size(); ++i) {
        StringView song = m_files[i];
        song = song.substr(SongPathPrefix.size(), song.size() - (SongPathPrefix.size() + 4));
        auto find = song.find(search, 0, String::CaseInsensitive);
        if (find != NPos) {
          auto widget = songList->addItem();
          widget->setData(i);
          String text = "";
          size_t last = 0;
          do {
            text += strf("^#bbb;{}^#7f7;{}", song.substr(last, find - last), song.substr(find, search.size()));
            last = find + search.size();
            find = song.find(search, last, String::CaseInsensitive);
          } while (find != NPos);
          auto songName = widget->fetchChild<LabelWidget>("songName");
          songName->setText(text + strf("^#bbb;{}", song.substr(last)));
          widget->show();
        }
      }
    }
  }
}

}