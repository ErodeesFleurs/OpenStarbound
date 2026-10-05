#pragma once

// Parse foundations before importing global bookmark value types on GCC.
#include "StarJson.hpp"
#include "StarWarping.hpp"
#include "StarCelestialCoordinate.hpp"
#include "StarSystemWorld.hpp"
#include "StarPane.hpp"
import star.player_universe_map;

namespace Star {

STAR_CLASS(EditBookmarkDialog);

class EditBookmarkDialog : public Pane {
public:
  EditBookmarkDialog(PlayerUniverseMapPtr playerUniverseMap);

  virtual void show() override;

  void setBookmark(TeleportBookmark bookmark);

  void ok();
  void remove();
  void close();

private:
  PlayerUniverseMapPtr m_playerUniverseMap;
  TeleportBookmark m_bookmark;

  bool m_isNew;
};

void setupBookmarkEntry(WidgetPtr const& entry, TeleportBookmark const& bookmark);
}
