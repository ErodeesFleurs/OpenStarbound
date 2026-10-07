module;
#include "StarGameTypes.hpp"
#include "StarJson.hpp"
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarOrderedMap.hpp"

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
STAR_CLASS(UniverseClient);
STAR_CLASS(Renderer);
}

namespace Star {

STAR_CLASS(Player);
STAR_CLASS(PlayerStorage);
STAR_CLASS(CharCreationPane);
STAR_CLASS(CharSelectionPane);
STAR_CLASS(OptionsMenu);
STAR_CLASS(ModsMenu);
STAR_CLASS(GuiContext);
STAR_CLASS(Pane);
STAR_CLASS(PaneManager);
STAR_CLASS(Mixer);
STAR_CLASS(EnvironmentPainter);
STAR_CLASS(CelestialMasterDatabase);
STAR_CLASS(ButtonWidget);
STAR_CLASS(Sky);

STAR_CLASS(TitleScreen);

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
  shared_ptr<ScriptComponent> m_scriptComponent;

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
  List<pair<ButtonWidgetPtr, Vec2I>> m_rightAnchoredButtons;

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
