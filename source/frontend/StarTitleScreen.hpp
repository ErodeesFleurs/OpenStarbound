#pragma once

#include "StarSky.hpp"
#include "StarAmbient.hpp"
#include "StarRegisteredPaneManager.hpp"
#include "StarInterfaceCursor.hpp"
#include "StarUniverseClient.hpp"
#include "StarListWidget.hpp"

namespace Star {

class Player;
using PlayerPtr = SharedPtr<Player>;
class PlayerStorage;
using PlayerStoragePtr = SharedPtr<PlayerStorage>;
class GuiContext;
using GuiContextPtr = SharedPtr<GuiContext>;
class Pane;
using PanePtr = SharedPtr<Pane>;
class Mixer;
using MixerPtr = SharedPtr<Mixer>;
class EnvironmentPainter;
using EnvironmentPainterPtr = SharedPtr<EnvironmentPainter>;
class CelestialMasterDatabase;
using CelestialMasterDatabasePtr = SharedPtr<CelestialMasterDatabase>;
class ButtonWidget;
using ButtonWidgetPtr = SharedPtr<ButtonWidget>;

class TitleScreen;
using TitleScreenPtr = SharedPtr<TitleScreen>;

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

  using TitlePaneManager = RegisteredPaneManager<String>;
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

  void populateServerList(ListWidgetPtr list);

  float interfaceScale() const;
  unsigned windowHeight() const;
  unsigned windowWidth() const;

  using ScriptComponent = LuaUpdatableComponent<LuaBaseComponent>;
  SharedPtr<ScriptComponent> m_scriptComponent;

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
