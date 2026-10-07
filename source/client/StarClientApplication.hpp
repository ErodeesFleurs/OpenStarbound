#pragma once
#include "StarXXHash.hpp"
#include "StarJson.hpp"
#include "StarIdMap.hpp"
#include "StarInterpolation.hpp"
#include "StarImage.hpp"
#include "StarMultiArray.hpp"
#include "StarMathCommon.hpp"
#include "StarNetElementSystem.hpp"
#include "StarConfig.hpp"
import star.version;
#include "StarEither.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
#include "StarRandom.hpp"
import star.weighted_pool;
#include "StarCasting.hpp"
#include "StarStrongTypedef.hpp"
#include "StarRect.hpp"
#include "StarOrderedMap.hpp"
#include "StarBlockAllocator.hpp"
import star.lru_cache;
import star.time;
import star.ttl_cache;
import star.listener;
#include "StarBiMap.hpp"
import star.perlin;
#include "StarRandomPoint.hpp"
#include "StarFont.hpp"
#include "StarStringView.hpp"
#include "StarString.hpp"
#include "StarVector.hpp"
import star.text;
#include "StarGameTypes.hpp"
#include "StarRpcPromise.hpp"
#include "StarOrderedMap.hpp"
#include "StarArray.hpp"
#include "StarAudio.hpp"
#include "StarThread.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarSet.hpp"
#include "StarVector.hpp"
#include "StarMaybe.hpp"
#include "StarBiMap.hpp"
#include "StarString.hpp"
#include "StarInputEvent.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
import star.asset_path;
#include "StarConfig.hpp"
import star.lock_file;
#include "StarIODevice.hpp"
#include "StarByteArray.hpp"
#include "StarDataStreamDevices.hpp"

typedef struct ZSTD_CCtx_s ZSTD_CCtx;
typedef struct ZSTD_DCtx_s ZSTD_DCtx;
typedef ZSTD_DCtx ZSTD_DStream;
typedef ZSTD_CCtx ZSTD_CStream;
import star.zstd_compression;
#include "StarNetCompatibility.hpp"
#include "StarBTree.hpp"
import star.btree_database;
#include "StarOrderedSet.hpp"
import star.worker_pool;

#include "thread"
import star.sector_array_2d;
#include "StarVariant.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarRandom.hpp"
#include "StarBlockAllocator.hpp"
#include <atomic>
#include <memory>
#include <functional>
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"


import star.universe;
import star.worker_pool;
import star.host_address;
import star.tile_sector_array;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.world_layout;
import star.inspectable_entity;
import star.plant;
import star.biome_placement;
#include "StarLuaRoot.hpp"
import star.versioning_database;
import star.world_storage;
import star.tile_modification;
import star.world;
import star.celestial_types;
import star.chat_types;
import star.wiring;
import star.player_types;
import star.system_world;
import star.damage_manager;
import star.net_packets;
import star.cellular_liquid;
import star.world_structure;
#include "StarLuaComponents.hpp"
import star.world_client_state;
import star.interpolation_tracker;
import star.spawn_type_database;
import star.spawner;
import star.world_server;
import star.world_server_thread;
import star.world_template;
import star.system_world_server;
import star.system_world_server_thread;
import star.socket;
import star.tcp;
#include "StarP2PNetworkingService.hpp"
import star.net_packet_socket;
import star.universe_connection;
import star.universe_settings;
import star.universe_server;
import star.ai_types;
import star.chat_action;
import star.entity_rendering;
import star.weather;
import star.world_client;
import star.world_client_thread;
// TODO: make this more thread safe
import star.universe_client;
import star.world_geometry;
import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.collision_generator;
import star.world_tiles;
import star.entity_rendering_types;
import star.sky_types;
import star.celestial_coordinate;
import star.sky_parameters;
import star.sky_render_data;
import star.plant_database;
import star.parallax;
import star.particle;
import star.weather_types;
import star.damage_types;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.cellular_light_array;
import star.cellular_lighting;
import star.world_render_data;
import star.material_render_profile;
import star.renderer;
import star.tile_drawer;
import star.tile_painter;
import star.asset_texture_group;
import star.environment_painter;
import star.font_texture_group;
import star.anchor_types;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.text_painter;
import star.drawable_painter;
import star.world_camera;
import star.world_painter;
import star.application;
#include "StarStatisticsService.hpp"
#include "StarUserGeneratedContentService.hpp"
#include "StarDesktopService.hpp"
import star.application_controller;
import star.gui_types;
import star.gui_context;
import star.widget;
import star.pane;
import star.inventory_types;
import star.item_descriptor;
import star.interaction_types;
import star.quest_descriptor;
import star.interactive_entity;
import star.tile_entity;
import star.container_entity;
import star.container_interactor;
import star.game_timers;
import star.inventory;
import star.pane_manager;
import star.registered_pane_manager;
import star.main_interface_types;
import star.uuid;
import star.warping;
import star.main_interface;

import star.mixer;


import star.main_mixer;
import star.widget_parsing;
import star.gui_reader;
import star.list_widget;
import star.ambient;
import star.title_screen;


import star.drawable;

import star.animation;


import star.interface_cursor;


import star.error_screen;
import star.cinematic;
import star.key_bindings;
import star.main_application;

namespace Star {

STAR_CLASS(Input);
STAR_CLASS(Voice);

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

  virtual void applicationInit(ApplicationControllerPtr appController) override;
  virtual void renderInit(RendererPtr renderer) override;

  virtual void windowChanged(WindowMode windowMode, Vec2U screenSize) override;

  virtual void processInput(InputEvent const& event) override;

  virtual void update() override;
  virtual void render() override;

  virtual void getAudioData(int16_t* stream, size_t len) override;

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

  RootUPtr m_root;
  ThreadFunction<void> m_rootLoader;
  CallbackListenerPtr m_reloadListener;

  MainAppState m_state = MainAppState::Startup;

  // Valid after applicationInit is called
  MainMixerPtr m_mainMixer;
  GuiContextPtr m_guiContext;
  InputPtr m_input;
  VoicePtr m_voice;

  // Valid after renderInit is called the first time
  CinematicPtr m_cinematicOverlay;
  ErrorScreenPtr m_errorScreen;

  // Valid if main app state >= Title
  PlayerStoragePtr m_playerStorage;
  StatisticsPtr m_statistics;
  UniverseClientPtr m_universeClient;
  TitleScreenPtr m_titleScreen;

  // Valid if main app state > Title
  PlayerPtr m_player;
  WorldPainterPtr m_worldPainter;
  WorldRenderData m_renderData;
  MainInterfacePtr m_mainInterface;
  
  StringMap<PostProcessGroup> m_postProcessGroups;
  List<PostProcessLayer> m_postProcessLayers;
  StringMap<size_t> m_labelledPostProcessLayers;

  // Valid if main app state == SinglePlayer
  UniverseServerPtr m_universeServer;

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
  int64_t m_timeSinceJoin = 0;

  ByteArray m_immediateFont;

  bool m_loggedUGCCheck;
};

}
