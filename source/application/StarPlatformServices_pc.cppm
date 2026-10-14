module;

#include "StarConfig.hpp"
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

#ifdef STAR_ENABLE_DISCORD_INTEGRATION
#include "discord/discord.h"
#endif

namespace Star {

STAR_CLASS(PcPlatformServices);
STAR_STRUCT(PcPlatformServicesState);

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

  unique_ptr<discord::Core> discordCore;
  
  Maybe<discord::User> discordCurrentUser;
  ThreadFunction<void> discordEventThread;
  atomic<bool> discordEventShutdown;
#endif

  bool overlayActive = false;
};


class PcPlatformServices {
public:
  // Any command line arguments that start with '+platform' will be stripped
  // out and passed here
  static PcPlatformServicesUPtr create(String const& path, StringList platformArguments);

  StatisticsServicePtr statisticsService() const;
  P2PNetworkingServicePtr p2pNetworkingService() const;
  UserGeneratedContentServicePtr userGeneratedContentService() const;
  DesktopServicePtr desktopService() const;

  // Will return true if there is an in-game overlay active.  This is important
  // because the cursor must be visible when such an overlay is active,
  // regardless of the ApplicationController setting.
  bool overlayActive() const;

  void update();

private:
  PcPlatformServices() = default;

  PcPlatformServicesStatePtr m_state;

  StatisticsServicePtr m_statisticsService;
  P2PNetworkingServicePtr m_p2pNetworkingService;
  UserGeneratedContentServicePtr m_userGeneratedContentService;
  DesktopServicePtr m_desktopService;
};

}

export module star.platform_services_pc;

export namespace Star {
  using ::Star::PcPlatformServices;
  using ::Star::PcPlatformServicesPtr;
  using ::Star::PcPlatformServicesConstPtr;
  using ::Star::PcPlatformServicesUPtr;
  using ::Star::PcPlatformServicesConstUPtr;
  using ::Star::PcPlatformServicesWeakPtr;
  using ::Star::PcPlatformServicesConstWeakPtr;
  using ::Star::PcPlatformServicesState;
  using ::Star::PcPlatformServicesStatePtr;
  using ::Star::PcPlatformServicesStateConstPtr;
  using ::Star::PcPlatformServicesStateUPtr;
  using ::Star::PcPlatformServicesStateConstUPtr;
  using ::Star::PcPlatformServicesStateWeakPtr;
  using ::Star::PcPlatformServicesStateConstWeakPtr;
}
