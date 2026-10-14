module;
import std;
import star.json;

import star.vector;
#include "StarCasting.hpp"
import star.input_event;
import star.color;
import star.list;
import star.rect;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.bi_map;
import star.string;
import star.text;
import star.asset_path;
import star.maybe;
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;

import star.ordered_map;




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
import star.game_timers;
import star.pane_manager;
import star.registered_pane_manager;
import star.main_interface_types;

namespace Star {

class Player;
using PlayerPtr = std::shared_ptr<Player>;
using PlayerConstPtr = std::shared_ptr<Player const>;
using PlayerWeakPtr = std::weak_ptr<Player>;
using PlayerConstWeakPtr = std::weak_ptr<Player const>;
using PlayerUPtr = std::unique_ptr<Player>;
using PlayerConstUPtr = std::unique_ptr<Player const>;
class UniverseClient;
using UniverseClientPtr = std::shared_ptr<UniverseClient>;
using UniverseClientConstPtr = std::shared_ptr<UniverseClient const>;
using UniverseClientWeakPtr = std::weak_ptr<UniverseClient>;
using UniverseClientConstWeakPtr = std::weak_ptr<UniverseClient const>;
using UniverseClientUPtr = std::unique_ptr<UniverseClient>;
using UniverseClientConstUPtr = std::unique_ptr<UniverseClient const>;
class StatusPane;
using StatusPanePtr = std::shared_ptr<StatusPane>;
using StatusPaneConstPtr = std::shared_ptr<StatusPane const>;
using StatusPaneWeakPtr = std::weak_ptr<StatusPane>;
using StatusPaneConstWeakPtr = std::weak_ptr<StatusPane const>;
using StatusPaneUPtr = std::unique_ptr<StatusPane>;
using StatusPaneConstUPtr = std::unique_ptr<StatusPane const>;

class StatusPane : public Pane {
public:
  StatusPane(MainInterfacePaneManager* paneManager, UniverseClientPtr client);

  virtual PanePtr createTooltip(Vec2I const& screenPosition) override;

protected:
  virtual void renderImpl() override;
  virtual void update(float dt) override;

private:
  struct StatusEffectIndicator {
    String icon;
    Maybe<float> durationPercentage;
    String label;
    RectF screenRect;
  };

  MainInterfacePaneManager* m_paneManager;
  UniverseClientPtr m_client;
  PlayerPtr m_player;

  GuiContext* m_guiContext;
  List<StatusEffectIndicator> m_statusIndicators;
};

}

export module star.status_pane;

export namespace Star {
  using ::Star::Player;
  using ::Star::PlayerPtr;
  using ::Star::PlayerConstPtr;
  using ::Star::PlayerWeakPtr;
  using ::Star::PlayerConstWeakPtr;
  using ::Star::PlayerUPtr;
  using ::Star::PlayerConstUPtr;
  using ::Star::UniverseClient;
  using ::Star::UniverseClientPtr;
  using ::Star::UniverseClientConstPtr;
  using ::Star::UniverseClientWeakPtr;
  using ::Star::UniverseClientConstWeakPtr;
  using ::Star::UniverseClientUPtr;
  using ::Star::UniverseClientConstUPtr;
  using ::Star::StatusPane;
  using ::Star::StatusPanePtr;
  using ::Star::StatusPaneConstPtr;
  using ::Star::StatusPaneWeakPtr;
  using ::Star::StatusPaneConstWeakPtr;
  using ::Star::StatusPaneUPtr;
  using ::Star::StatusPaneConstUPtr;
}
