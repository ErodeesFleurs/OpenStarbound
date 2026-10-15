module;

#ifdef STAR_ENABLE_STEAM_INTEGRATION
#include "steam/steam_api.h"
#include "steam/steam_api_flat.h"
#endif
#ifdef STAR_ENABLE_DISCORD_INTEGRATION
#include "discord/discord.h"
#endif
import std;
import star.maybe;
import star.thread;
import star.input_event;
import star.application;
import star.string;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;

#ifdef STAR_ENABLE_STEAM_INTEGRATION
#include "StarSteamDispatch.hpp"
#endif


namespace Star {

class PcPlatformServices;

struct PcPlatformServicesState {
  PcPlatformServicesState();
  ~PcPlatformServicesState();

#ifdef STAR_ENABLE_STEAM_INTEGRATION
  void onGameOverlayActivated(GameOverlayActivated_t* callback);
  SteamDispatch steamDispatch;
  SteamCallback<PcPlatformServicesState, GameOverlayActivated_t> callbackGameOverlayActivated;

  bool steamAvailable = false;
#endif

#ifdef STAR_ENABLE_DISCORD_INTEGRATION
  bool discordAvailable = false;

  // Must lock discordMutex before accessing any of the managers when not inside
  // a discord callback.
  Mutex discordMutex;

  std::unique_ptr<discord::Core> discordCore;
  
  Maybe<discord::User> discordCurrentUser;
  ThreadFunction<void> discordEventThread;
  std::atomic<bool> discordEventShutdown;
#endif

  bool overlayActive = false;
};


class PcPlatformServices {
public:
  // Any command line arguments that start with '+platform' will be stripped
  // out and passed here
  static std::unique_ptr<PcPlatformServices> create(String const& path, StringList platformArguments);

  std::shared_ptr<StatisticsService> statisticsService() const;
  std::shared_ptr<P2PNetworkingService> p2pNetworkingService() const;
  std::shared_ptr<UserGeneratedContentService> userGeneratedContentService() const;
  std::shared_ptr<DesktopService> desktopService() const;

  // Will return true if there is an in-game overlay active.  This is important
  // because the cursor must be visible when such an overlay is active,
  // regardless of the ApplicationController setting.
  bool overlayActive() const;

  void update();

private:
  PcPlatformServices() = default;

  std::shared_ptr<PcPlatformServicesState> m_state;

  std::shared_ptr<StatisticsService> m_statisticsService;
  std::shared_ptr<P2PNetworkingService> m_p2pNetworkingService;
  std::shared_ptr<UserGeneratedContentService> m_userGeneratedContentService;
  std::shared_ptr<DesktopService> m_desktopService;
};

}

export module star.platform_services_pc;

export namespace Star {
  using ::Star::PcPlatformServices;
  using ::Star::PcPlatformServicesState;
}
