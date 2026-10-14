module;
import std;
import star.json;
import star.game_types;
import star.vector;
#include "StarCasting.hpp"
import star.input_event;
import star.color;
import star.list;
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
import star.set;
import star.audio;
import star.map;
import star.ordered_map;

#include "StarLuaComponents.hpp"
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

import star.animation;

import star.interface_cursor;
import star.widget_parsing;
import star.gui_reader;
import star.list_widget;
import star.ambient;


namespace Star {
class UniverseClient;
using UniverseClientPtr = std::shared_ptr<UniverseClient>;
using UniverseClientConstPtr = std::shared_ptr<UniverseClient const>;
using UniverseClientWeakPtr = std::weak_ptr<UniverseClient>;
using UniverseClientConstWeakPtr = std::weak_ptr<UniverseClient const>;
using UniverseClientUPtr = std::unique_ptr<UniverseClient>;
using UniverseClientConstUPtr = std::unique_ptr<UniverseClient const>;
class Renderer;
using RendererPtr = std::shared_ptr<Renderer>;
using RendererConstPtr = std::shared_ptr<Renderer const>;
using RendererWeakPtr = std::weak_ptr<Renderer>;
using RendererConstWeakPtr = std::weak_ptr<Renderer const>;
using RendererUPtr = std::unique_ptr<Renderer>;
using RendererConstUPtr = std::unique_ptr<Renderer const>;
}

namespace Star {

class Player;
using PlayerPtr = std::shared_ptr<Player>;
using PlayerConstPtr = std::shared_ptr<Player const>;
using PlayerWeakPtr = std::weak_ptr<Player>;
using PlayerConstWeakPtr = std::weak_ptr<Player const>;
using PlayerUPtr = std::unique_ptr<Player>;
using PlayerConstUPtr = std::unique_ptr<Player const>;
class PlayerStorage;
using PlayerStoragePtr = std::shared_ptr<PlayerStorage>;
using PlayerStorageConstPtr = std::shared_ptr<PlayerStorage const>;
using PlayerStorageWeakPtr = std::weak_ptr<PlayerStorage>;
using PlayerStorageConstWeakPtr = std::weak_ptr<PlayerStorage const>;
using PlayerStorageUPtr = std::unique_ptr<PlayerStorage>;
using PlayerStorageConstUPtr = std::unique_ptr<PlayerStorage const>;
class CharCreationPane;
using CharCreationPanePtr = std::shared_ptr<CharCreationPane>;
using CharCreationPaneConstPtr = std::shared_ptr<CharCreationPane const>;
using CharCreationPaneWeakPtr = std::weak_ptr<CharCreationPane>;
using CharCreationPaneConstWeakPtr = std::weak_ptr<CharCreationPane const>;
using CharCreationPaneUPtr = std::unique_ptr<CharCreationPane>;
using CharCreationPaneConstUPtr = std::unique_ptr<CharCreationPane const>;
class CharSelectionPane;
using CharSelectionPanePtr = std::shared_ptr<CharSelectionPane>;
using CharSelectionPaneConstPtr = std::shared_ptr<CharSelectionPane const>;
using CharSelectionPaneWeakPtr = std::weak_ptr<CharSelectionPane>;
using CharSelectionPaneConstWeakPtr = std::weak_ptr<CharSelectionPane const>;
using CharSelectionPaneUPtr = std::unique_ptr<CharSelectionPane>;
using CharSelectionPaneConstUPtr = std::unique_ptr<CharSelectionPane const>;
class OptionsMenu;
using OptionsMenuPtr = std::shared_ptr<OptionsMenu>;
using OptionsMenuConstPtr = std::shared_ptr<OptionsMenu const>;
using OptionsMenuWeakPtr = std::weak_ptr<OptionsMenu>;
using OptionsMenuConstWeakPtr = std::weak_ptr<OptionsMenu const>;
using OptionsMenuUPtr = std::unique_ptr<OptionsMenu>;
using OptionsMenuConstUPtr = std::unique_ptr<OptionsMenu const>;
class ModsMenu;
using ModsMenuPtr = std::shared_ptr<ModsMenu>;
using ModsMenuConstPtr = std::shared_ptr<ModsMenu const>;
using ModsMenuWeakPtr = std::weak_ptr<ModsMenu>;
using ModsMenuConstWeakPtr = std::weak_ptr<ModsMenu const>;
using ModsMenuUPtr = std::unique_ptr<ModsMenu>;
using ModsMenuConstUPtr = std::unique_ptr<ModsMenu const>;
class GuiContext;
using GuiContextPtr = std::shared_ptr<GuiContext>;
using GuiContextConstPtr = std::shared_ptr<GuiContext const>;
using GuiContextWeakPtr = std::weak_ptr<GuiContext>;
using GuiContextConstWeakPtr = std::weak_ptr<GuiContext const>;
using GuiContextUPtr = std::unique_ptr<GuiContext>;
using GuiContextConstUPtr = std::unique_ptr<GuiContext const>;
class Pane;
using PanePtr = std::shared_ptr<Pane>;
using PaneConstPtr = std::shared_ptr<Pane const>;
using PaneWeakPtr = std::weak_ptr<Pane>;
using PaneConstWeakPtr = std::weak_ptr<Pane const>;
using PaneUPtr = std::unique_ptr<Pane>;
using PaneConstUPtr = std::unique_ptr<Pane const>;
class PaneManager;
using PaneManagerPtr = std::shared_ptr<PaneManager>;
using PaneManagerConstPtr = std::shared_ptr<PaneManager const>;
using PaneManagerWeakPtr = std::weak_ptr<PaneManager>;
using PaneManagerConstWeakPtr = std::weak_ptr<PaneManager const>;
using PaneManagerUPtr = std::unique_ptr<PaneManager>;
using PaneManagerConstUPtr = std::unique_ptr<PaneManager const>;
class Mixer;
using MixerPtr = std::shared_ptr<Mixer>;
using MixerConstPtr = std::shared_ptr<Mixer const>;
using MixerWeakPtr = std::weak_ptr<Mixer>;
using MixerConstWeakPtr = std::weak_ptr<Mixer const>;
using MixerUPtr = std::unique_ptr<Mixer>;
using MixerConstUPtr = std::unique_ptr<Mixer const>;
class EnvironmentPainter;
using EnvironmentPainterPtr = std::shared_ptr<EnvironmentPainter>;
using EnvironmentPainterConstPtr = std::shared_ptr<EnvironmentPainter const>;
using EnvironmentPainterWeakPtr = std::weak_ptr<EnvironmentPainter>;
using EnvironmentPainterConstWeakPtr = std::weak_ptr<EnvironmentPainter const>;
using EnvironmentPainterUPtr = std::unique_ptr<EnvironmentPainter>;
using EnvironmentPainterConstUPtr = std::unique_ptr<EnvironmentPainter const>;
class CelestialMasterDatabase;
using CelestialMasterDatabasePtr = std::shared_ptr<CelestialMasterDatabase>;
using CelestialMasterDatabaseConstPtr = std::shared_ptr<CelestialMasterDatabase const>;
using CelestialMasterDatabaseWeakPtr = std::weak_ptr<CelestialMasterDatabase>;
using CelestialMasterDatabaseConstWeakPtr = std::weak_ptr<CelestialMasterDatabase const>;
using CelestialMasterDatabaseUPtr = std::unique_ptr<CelestialMasterDatabase>;
using CelestialMasterDatabaseConstUPtr = std::unique_ptr<CelestialMasterDatabase const>;
class ButtonWidget;
using ButtonWidgetPtr = std::shared_ptr<ButtonWidget>;
using ButtonWidgetConstPtr = std::shared_ptr<ButtonWidget const>;
using ButtonWidgetWeakPtr = std::weak_ptr<ButtonWidget>;
using ButtonWidgetConstWeakPtr = std::weak_ptr<ButtonWidget const>;
using ButtonWidgetUPtr = std::unique_ptr<ButtonWidget>;
using ButtonWidgetConstUPtr = std::unique_ptr<ButtonWidget const>;
class Sky;
using SkyPtr = std::shared_ptr<Sky>;
using SkyConstPtr = std::shared_ptr<Sky const>;
using SkyWeakPtr = std::weak_ptr<Sky>;
using SkyConstWeakPtr = std::weak_ptr<Sky const>;
using SkyUPtr = std::unique_ptr<Sky>;
using SkyConstUPtr = std::unique_ptr<Sky const>;

class TitleScreen;
using TitleScreenPtr = std::shared_ptr<TitleScreen>;
using TitleScreenConstPtr = std::shared_ptr<TitleScreen const>;
using TitleScreenWeakPtr = std::weak_ptr<TitleScreen>;
using TitleScreenConstWeakPtr = std::weak_ptr<TitleScreen const>;
using TitleScreenUPtr = std::unique_ptr<TitleScreen>;
using TitleScreenConstUPtr = std::unique_ptr<TitleScreen const>;

enum class TitleState {
  Main,
  Options,
  Mods,
  SinglePlayerSelectCharacter,
  SinglePlayerCreateCharacter,
  MultiPlayerSelectCharacter,
  MultiPlayerCreateCharacter,
  MultiPlayerConnect,
  StartSinglePlayer,
  StartMultiPlayer,
  Quit
};

class TitleScreen {
public:
  TitleScreen(PlayerStoragePtr playerStorage, MixerPtr mixer, UniverseClientPtr client);

  void renderInit(RendererPtr renderer);

  void render();

  bool handleInputEvent(InputEvent const& event);
  void update(float dt);

  bool textInputActive() const;

  typedef RegisteredPaneManager<String> TitlePaneManager;
  TitlePaneManager* paneManager();

  TitleState currentState() const;
  // TitleState is StartSinglePlayer, StartMultiPlayer, or Quit
  bool finishedState() const;
  void resetState();
  // Switches to multi player select character screen immediately, skipping the
  // connection screen if 'skipConnection' is true.  If the player backs out of
  // the multiplayer menu, the skip connection is forgotten.
  void goToMultiPlayerSelectCharacter(bool skipConnection);

  void stopMusic();

  PlayerPtr currentlySelectedPlayer() const;

  String multiPlayerAddress() const;
  void setMultiPlayerAddress(String address);

  String multiPlayerPort() const;
  void setMultiPlayerPort(String port);

  String multiPlayerAccount() const;
  void setMultiPlayerAccount(String account);

  String multiPlayerPassword() const;
  void setMultiPlayerPassword(String password);

  bool multiPlayerForceLegacy() const;
  void setMultiPlayerForceLegacy(bool const& forceLegacy);

private:
  void initMainMenu();
  void initCharSelectionMenu();
  void initCharCreationMenu();
  void initMultiPlayerMenu();
  void initOptionsMenu(UniverseClientPtr client);
  void initModsMenu();

  void renderCursor();

  void switchState(TitleState titleState);
  void back();

  void populateServerList(ListWidget* list);

  float interfaceScale() const;
  unsigned windowHeight() const;
  unsigned windowWidth() const;

  typedef LuaUpdatableComponent<LuaBaseComponent> ScriptComponent;
  std::shared_ptr<ScriptComponent> m_scriptComponent;

  GuiContext* m_guiContext;

  RendererPtr m_renderer;
  EnvironmentPainterPtr m_environmentPainter;

  PanePtr m_multiPlayerMenu;
  PanePtr m_serverSelectPane;
  Json m_serverList;

  TitlePaneManager m_paneManager;

  Vec2I m_cursorScreenPos;
  InterfaceCursor m_cursor;
  TitleState m_titleState;

  PanePtr m_mainMenu;
  PanePtr m_backgroundMenu;
  List<std::pair<ButtonWidgetPtr, Vec2I>> m_rightAnchoredButtons;

  PlayerPtr m_mainAppPlayer;
  PlayerStoragePtr m_playerStorage;

  bool m_skipMultiPlayerConnection;
  String m_connectionAddress;
  String m_connectionPort;
  String m_account;
  String m_password;
  bool m_forceLegacy;

  CelestialMasterDatabasePtr m_celestialDatabase;

  MixerPtr m_mixer;

  SkyPtr m_skyBackdrop;

  AmbientNoisesDescriptionPtr m_musicTrack;
  AudioInstancePtr m_currentMusicTrack;
  AmbientManager m_musicTrackManager;
};

}

export module star.title_screen;

export namespace Star {
  using ::Star::Player;
  using ::Star::PlayerPtr;
  using ::Star::PlayerConstPtr;
  using ::Star::PlayerWeakPtr;
  using ::Star::PlayerConstWeakPtr;
  using ::Star::PlayerUPtr;
  using ::Star::PlayerConstUPtr;
  using ::Star::PlayerStorage;
  using ::Star::PlayerStoragePtr;
  using ::Star::PlayerStorageConstPtr;
  using ::Star::PlayerStorageWeakPtr;
  using ::Star::PlayerStorageConstWeakPtr;
  using ::Star::PlayerStorageUPtr;
  using ::Star::PlayerStorageConstUPtr;
  using ::Star::CharCreationPane;
  using ::Star::CharCreationPanePtr;
  using ::Star::CharCreationPaneConstPtr;
  using ::Star::CharCreationPaneWeakPtr;
  using ::Star::CharCreationPaneConstWeakPtr;
  using ::Star::CharCreationPaneUPtr;
  using ::Star::CharCreationPaneConstUPtr;
  using ::Star::CharSelectionPane;
  using ::Star::CharSelectionPanePtr;
  using ::Star::CharSelectionPaneConstPtr;
  using ::Star::CharSelectionPaneWeakPtr;
  using ::Star::CharSelectionPaneConstWeakPtr;
  using ::Star::CharSelectionPaneUPtr;
  using ::Star::CharSelectionPaneConstUPtr;
  using ::Star::OptionsMenu;
  using ::Star::OptionsMenuPtr;
  using ::Star::OptionsMenuConstPtr;
  using ::Star::OptionsMenuWeakPtr;
  using ::Star::OptionsMenuConstWeakPtr;
  using ::Star::OptionsMenuUPtr;
  using ::Star::OptionsMenuConstUPtr;
  using ::Star::ModsMenu;
  using ::Star::ModsMenuPtr;
  using ::Star::ModsMenuConstPtr;
  using ::Star::ModsMenuWeakPtr;
  using ::Star::ModsMenuConstWeakPtr;
  using ::Star::ModsMenuUPtr;
  using ::Star::ModsMenuConstUPtr;
  using ::Star::GuiContext;
  using ::Star::GuiContextPtr;
  using ::Star::GuiContextConstPtr;
  using ::Star::GuiContextWeakPtr;
  using ::Star::GuiContextConstWeakPtr;
  using ::Star::GuiContextUPtr;
  using ::Star::GuiContextConstUPtr;
  using ::Star::Pane;
  using ::Star::PanePtr;
  using ::Star::PaneConstPtr;
  using ::Star::PaneWeakPtr;
  using ::Star::PaneConstWeakPtr;
  using ::Star::PaneUPtr;
  using ::Star::PaneConstUPtr;
  using ::Star::PaneManager;
  using ::Star::PaneManagerPtr;
  using ::Star::PaneManagerConstPtr;
  using ::Star::PaneManagerWeakPtr;
  using ::Star::PaneManagerConstWeakPtr;
  using ::Star::PaneManagerUPtr;
  using ::Star::PaneManagerConstUPtr;
  using ::Star::Mixer;
  using ::Star::MixerPtr;
  using ::Star::MixerConstPtr;
  using ::Star::MixerWeakPtr;
  using ::Star::MixerConstWeakPtr;
  using ::Star::MixerUPtr;
  using ::Star::MixerConstUPtr;
  using ::Star::EnvironmentPainter;
  using ::Star::EnvironmentPainterPtr;
  using ::Star::EnvironmentPainterConstPtr;
  using ::Star::EnvironmentPainterWeakPtr;
  using ::Star::EnvironmentPainterConstWeakPtr;
  using ::Star::EnvironmentPainterUPtr;
  using ::Star::EnvironmentPainterConstUPtr;
  using ::Star::CelestialMasterDatabase;
  using ::Star::CelestialMasterDatabasePtr;
  using ::Star::CelestialMasterDatabaseConstPtr;
  using ::Star::CelestialMasterDatabaseWeakPtr;
  using ::Star::CelestialMasterDatabaseConstWeakPtr;
  using ::Star::CelestialMasterDatabaseUPtr;
  using ::Star::CelestialMasterDatabaseConstUPtr;
  using ::Star::ButtonWidget;
  using ::Star::ButtonWidgetPtr;
  using ::Star::ButtonWidgetConstPtr;
  using ::Star::ButtonWidgetWeakPtr;
  using ::Star::ButtonWidgetConstWeakPtr;
  using ::Star::ButtonWidgetUPtr;
  using ::Star::ButtonWidgetConstUPtr;
  using ::Star::Sky;
  using ::Star::SkyPtr;
  using ::Star::SkyConstPtr;
  using ::Star::SkyWeakPtr;
  using ::Star::SkyConstWeakPtr;
  using ::Star::SkyUPtr;
  using ::Star::SkyConstUPtr;
  using ::Star::TitleScreen;
  using ::Star::TitleScreenPtr;
  using ::Star::TitleScreenConstPtr;
  using ::Star::TitleScreenWeakPtr;
  using ::Star::TitleScreenConstWeakPtr;
  using ::Star::TitleScreenUPtr;
  using ::Star::TitleScreenConstUPtr;
  using ::Star::TitleState;
}
