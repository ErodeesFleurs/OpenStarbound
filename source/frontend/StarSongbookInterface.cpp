
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarApplicationController.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarRenderer.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarRoot.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
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
#include "StarPlayer.hpp"
#include "StarAssets.hpp"

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