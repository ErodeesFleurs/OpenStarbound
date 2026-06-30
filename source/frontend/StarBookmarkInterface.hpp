#pragma once

#include "StarAssets.hpp"
#include "StarPlayerUniverseMap.hpp"
#include "StarPane.hpp"

namespace Star {

class EditBookmarkDialog : public Pane {
public:
  struct Services {
    AssetsConstPtr assets;
    GuiContext& guiContext;
  };

  EditBookmarkDialog(PlayerUniverseMapPtr playerUniverseMap, Services services);

  void show() override;

  void setBookmark(TeleportBookmark bookmark);

  void ok();
  void remove();
  void close();

private:
  PlayerUniverseMapPtr m_playerUniverseMap;
  AssetsConstPtr m_assets;
  TeleportBookmark m_bookmark;

  bool m_isNew = false;
};

}
