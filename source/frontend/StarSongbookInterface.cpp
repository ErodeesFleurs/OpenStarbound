#include "StarSongbookInterface.hpp"
#include "StarAlgorithm.hpp"
#include "StarException.hpp"
#include "StarGuiReader.hpp"
#include "StarListWidget.hpp"
#include "StarLabelWidget.hpp"
#include "StarTextBoxWidget.hpp"
#include "StarPlayer.hpp"
#include "StarPythonic.hpp"

namespace Star {

String const SongPathPrefix = "/songs/";

SongbookInterface::SongbookInterface(PlayerPtr player, SongbookInterfaceServices services)
  : Pane(services.guiContext),
    m_player(requireServiceValueAs<StarException>(std::move(player), "SongbookInterface", "player")),
    m_assets(requireServiceValueAs<StarException>(std::move(services.assets), "SongbookInterface", "assets")),
    m_registerReloadListener(requireServiceValueAs<StarException>(std::move(services.registerReloadListener), "SongbookInterface", "reload listener")) {
  GuiReader reader(context());

  reader.registerCallback("close", [=, this](Widget*) { dismiss(); });
  reader.registerCallback("btnPlay",
      [=, this](Widget*) {
        if (play())
          dismiss();
      });
  reader.registerCallback("group", [=](Widget*) {});
  reader.registerCallback("search", [=](Widget*) {});

  reader.construct(m_assets->json("/interface/windowconfig/songbook.config:paneLayout"), this);

  m_registerReloadListener(
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
  auto buffer = m_assets->bytes(songName);
  song["abc"] = String(buffer->ptr(), buffer->size());

  m_player->songbook()->play(song, group);
  return true;
}

void SongbookInterface::refresh(bool reloadFiles) {
  if (reloadFiles) {
    m_files = m_assets->scan(".abc");
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
      for (auto const& fileAndIndex : enumerateIterator(m_files)) {
        auto widget = songList->addItem();
        widget->setData(fileAndIndex.second);
        auto songName = widget->fetchChild<LabelWidget>("songName");
        String const& song = fileAndIndex.first;
        songName->setText(song.substr(SongPathPrefix.size(), song.size() - (SongPathPrefix.size() + 4)));
        widget->show();
      }
    } else {
      for (auto const& fileAndIndex : enumerateIterator(m_files)) {
        StringView song = fileAndIndex.first;
        song = song.substr(SongPathPrefix.size(), song.size() - (SongPathPrefix.size() + 4));
        auto find = song.find(search, 0, String::CaseInsensitive);
        if (find != NPos) {
          auto widget = songList->addItem();
          widget->setData(fileAndIndex.second);
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
