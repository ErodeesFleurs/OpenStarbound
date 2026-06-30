#pragma once

#include "StarIAssets.hpp"
#include "StarPlayerUniverseMap.hpp"
#include "StarPane.hpp"

namespace Star {

class EditBookmarkDialog : public Pane {
public:
  struct Services {
    IAssetsConstPtr assets;
  };

  EditBookmarkDialog(PlayerUniverseMapPtr playerUniverseMap, Services services);

  virtual void show() override;

  void setBookmark(TeleportBookmark bookmark);

  void ok();
  void remove();
  void close();

private:
  PlayerUniverseMapPtr m_playerUniverseMap;
  IAssetsConstPtr m_assets;
  TeleportBookmark m_bookmark;

  bool m_isNew;
};

void setupBookmarkEntry(WidgetPtr const& entry, TeleportBookmark const& bookmark);
}
