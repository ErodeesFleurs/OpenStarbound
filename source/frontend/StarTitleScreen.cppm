module;
import std;
import star.json;
import star.game_types;
import star.vector;
import star.casting;
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

import star.lua_components;
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
class Renderer;
}

namespace Star {

class Player;
class PlayerStorage;
class CharCreationPane;
class CharSelectionPane;
class OptionsMenu;
class ModsMenu;
class GuiContext;
class Pane;
class PaneManager;
class Mixer;
class EnvironmentPainter;
class CelestialMasterDatabase;
class ButtonWidget;
class Sky;

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
  TitleScreen(GuiContext& context, std::shared_ptr<PlayerStorage> playerStorage, std::shared_ptr<Mixer> mixer, std::shared_ptr<UniverseClient> client);

  void renderInit(std::shared_ptr<Renderer> renderer);

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

  std::shared_ptr<Player> currentlySelectedPlayer() const;

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
  void initOptionsMenu(std::shared_ptr<UniverseClient> client);
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

  std::shared_ptr<Renderer> m_renderer;
  std::shared_ptr<EnvironmentPainter> m_environmentPainter;

  std::shared_ptr<Pane> m_multiPlayerMenu;
  std::shared_ptr<Pane> m_serverSelectPane;
  Json m_serverList;

  TitlePaneManager m_paneManager;

  Vec2I m_cursorScreenPos;
  InterfaceCursor m_cursor;
  TitleState m_titleState;

  std::shared_ptr<Pane> m_mainMenu;
  std::shared_ptr<Pane> m_backgroundMenu;
  List<std::pair<std::shared_ptr<ButtonWidget>, Vec2I>> m_rightAnchoredButtons;

  std::shared_ptr<Player> m_mainAppPlayer;
  std::shared_ptr<PlayerStorage> m_playerStorage;

  bool m_skipMultiPlayerConnection;
  String m_connectionAddress;
  String m_connectionPort;
  String m_account;
  String m_password;
  bool m_forceLegacy;

  std::shared_ptr<CelestialMasterDatabase> m_celestialDatabase;

  std::shared_ptr<Mixer> m_mixer;

  std::shared_ptr<Sky> m_skyBackdrop;

  std::shared_ptr<AmbientNoisesDescription> m_musicTrack;
  std::shared_ptr<AudioInstance> m_currentMusicTrack;
  AmbientManager m_musicTrackManager;
};

}

export module star.title_screen;

export namespace Star {
  using ::Star::Player;
  using ::Star::PlayerStorage;
  using ::Star::CharCreationPane;
  using ::Star::CharSelectionPane;
  using ::Star::OptionsMenu;
  using ::Star::ModsMenu;
  using ::Star::GuiContext;
  using ::Star::Pane;
  using ::Star::PaneManager;
  using ::Star::Mixer;
  using ::Star::EnvironmentPainter;
  using ::Star::CelestialMasterDatabase;
  using ::Star::ButtonWidget;
  using ::Star::Sky;
  using ::Star::TitleScreen;
  using ::Star::TitleState;
}
