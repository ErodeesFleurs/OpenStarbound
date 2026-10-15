module;
import std;
import star.json;
import star.string;
import star.list;
import star.maybe;
import star.variant;
import star.thread;
import star.vector;
import star.byte_array;
import star.application;
import star.input_event;
import star.host_address;
import star.p2p_networking_service;
import star.world_render_data;
import star.key_bindings;
import star.root;

namespace Star {

class Input;
class Voice;
class CallbackListener;
class MainMixer;
class GuiContext;
class Cinematic;
class ErrorScreen;
class PlayerStorage;
class Statistics;
class UniverseClient;
class TitleScreen;
class Player;
class WorldPainter;
class MainInterface;
class UniverseServer;

class ClientApplication : public Application {
public:
  void setPostProcessLayerPasses(String const& layer, unsigned const& passes);
  void setPostProcessGroupEnabled(String const& group, bool const& enabled, Maybe<bool> const& save);
  bool postProcessGroupEnabled(String const& group);
  Json postProcessGroups();
  virtual unsigned framesSkipped() const override;

protected:
  virtual void startup(StringList const& cmdLineArgs) override;
  virtual void shutdown() override;

  virtual void applicationInit(std::shared_ptr<ApplicationController> appController) override;
  virtual void renderInit(std::shared_ptr<Renderer> renderer) override;

  virtual void windowChanged(WindowMode windowMode, Vec2U screenSize) override;

  virtual void processInput(InputEvent const& event) override;

  virtual void update() override;
  virtual void render() override;

  virtual void getAudioData(std::int16_t* stream, std::size_t len) override;

private:
  enum class MainAppState {
    Quit,
    Startup,
    SteamFlatpakWarning,
    Mods,
    ModsWarning,
    Splash,
    Error,
    Title,
    SinglePlayer,
    MultiPlayer
  };

  struct PendingMultiPlayerConnection {
    Variant<P2PNetworkingPeerId, HostAddressWithPort> server;
    String account;
    String password;
    bool forceLegacy;
  };
  
  struct PostProcessGroup {
    bool enabled;
  };
  
  struct PostProcessLayer {
    List<String> effects;
    unsigned passes;
    PostProcessGroup* group;
  };

  void renderReload();

  void changeState(MainAppState newState);
  void setError(String const& error);
  void setError(String const& error, std::exception const& e);

  void loadMods();
  void updateSteamFlatpakWarning(float dt);
  void updateMods(float dt);
  void updateModsWarning(float dt);
  void updateSplash(float dt);
  void updateError(float dt);
  void updateTitle(float dt);
  void updateRunning(float dt);

  bool isActionTaken(InterfaceAction action) const;
  bool isActionTakenEdge(InterfaceAction action) const;

  void updateCamera(float dt);

  std::unique_ptr<Root> m_root;
  ThreadFunction<void> m_rootLoader;
  std::shared_ptr<CallbackListener> m_reloadListener;

  MainAppState m_state = MainAppState::Startup;

  // Valid after applicationInit is called
  std::shared_ptr<MainMixer> m_mainMixer;
  std::shared_ptr<GuiContext> m_guiContext;
  std::shared_ptr<Input> m_input;
  std::shared_ptr<Voice> m_voice;

  // Valid after renderInit is called the first time
  std::shared_ptr<Cinematic> m_cinematicOverlay;
  std::shared_ptr<ErrorScreen> m_errorScreen;

  // Valid if main app state >= Title
  std::shared_ptr<PlayerStorage> m_playerStorage;
  std::shared_ptr<Statistics> m_statistics;
  std::shared_ptr<UniverseClient> m_universeClient;
  std::shared_ptr<TitleScreen> m_titleScreen;

  // Valid if main app state > Title
  std::shared_ptr<Player> m_player;
  std::shared_ptr<WorldPainter> m_worldPainter;
  WorldRenderData m_renderData;
  std::shared_ptr<MainInterface> m_mainInterface;
  
  StringMap<PostProcessGroup> m_postProcessGroups;
  List<PostProcessLayer> m_postProcessLayers;
  StringMap<std::size_t> m_labelledPostProcessLayers;

  // Valid if main app state == SinglePlayer
  std::shared_ptr<UniverseServer> m_universeServer;

  float m_cameraXOffset = 0.0f;
  float m_cameraYOffset = 0.0f;
  bool m_snapBackCameraOffset = false;
  float m_cameraOffsetDownTime = 0.f;
  Vec2F m_cameraPositionSmoother;
  Vec2F m_cameraSmoothDelta;
  int m_cameraZoomDirection = 0;

  unsigned m_framesSkipped = 0;
  float m_minInterfaceScale = 2;
  float m_maxInterfaceScale = 3;
  Vec2F m_crossoverRes;

  bool m_controllerInput;
  Vec2F m_controllerLeftStick;
  Vec2F m_controllerRightStick;
  List<KeyDownEvent> m_heldKeyEvents;
  List<KeyDownEvent> m_edgeKeyEvents;

  Maybe<PendingMultiPlayerConnection> m_pendingMultiPlayerConnection;
  Maybe<HostAddressWithPort> m_currentRemoteJoin;
  std::int64_t m_timeSinceJoin = 0;

  ByteArray m_immediateFont;

  bool m_loggedUGCCheck;
};

}


export module star.client_application;

export namespace Star {
  using ::Star::ClientApplication;
}
