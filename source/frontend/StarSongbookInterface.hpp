#pragma once

#include "StarSongbook.hpp"
#include "StarPane.hpp"
#include "StarListener.hpp"
#include "StarIAssets.hpp"

namespace Star {

class Player;
using PlayerPtr = SharedPtr<Player>;

class SongbookInterface;
using SongbookInterfacePtr = SharedPtr<SongbookInterface>;

struct SongbookInterfaceServices {
  IAssetsConstPtr assets;
  function<void(ListenerWeakPtr)> registerReloadListener;
};

class SongbookInterface : public Pane {
public:
  SongbookInterface(PlayerPtr player, SongbookInterfaceServices services);

  void update(float dt) override;

private:
  PlayerPtr m_player;
  IAssetsConstPtr m_assets;
  function<void(ListenerWeakPtr)> m_registerReloadListener;
  StringList m_files;
  String m_lastSearch;
  CallbackListenerPtr m_reloadListener;
  bool play();
  void refresh(bool reloadFiles = false);
};

}
