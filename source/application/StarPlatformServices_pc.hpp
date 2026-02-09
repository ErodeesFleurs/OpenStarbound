#pragma once

#include "StarConfig.hpp"
#include "StarDesktopService.hpp"
#include "StarP2PNetworkingService.hpp"
#include "StarStatisticsService.hpp"
#include "StarUserGeneratedContentService.hpp"

#ifdef STAR_ENABLE_STEAM_INTEGRATION
#include "steam/steam_api.h"
#endif

#ifdef STAR_ENABLE_DISCORD_INTEGRATION
#include "discord/discord.h"
#endif

namespace Star {

struct PcPlatformServicesState {
  PcPlatformServicesState();
  ~PcPlatformServicesState();

#ifdef STAR_ENABLE_STEAM_INTEGRATION
  STEAM_CALLBACK(PcPlatformServicesState, onGameOverlayActivated, GameOverlayActivated_t, callbackGameOverlayActivated);

  bool steamAvailable = false;
#endif

#ifdef STAR_ENABLE_DISCORD_INTEGRATION
  bool discordAvailable = false;

  // Must lock discordMutex before accessing any of the managers when not inside
  // a discord callback.
  Mutex discordMutex;

  unique_ptr<discord::Core> discordCore;

  std::optional<discord::User> discordCurrentUser;
  ThreadFunction<void> discordEventThread;
  atomic<bool> discordEventShutdown;
#endif

  bool overlayActive = false;
};

class PcPlatformServices {
public:
  // Any command line arguments that start with '+platform' will be stripped
  // out and passed here
  static auto create(String const& path, StringList platformArguments) -> UPtr<PcPlatformServices>;

  [[nodiscard]] auto statisticsService() const -> Ptr<StatisticsService>;
  [[nodiscard]] auto p2pNetworkingService() const -> Ptr<P2PNetworkingService>;
  [[nodiscard]] auto userGeneratedContentService() const -> Ptr<UserGeneratedContentService>;
  [[nodiscard]] auto desktopService() const -> Ptr<DesktopService>;

  // Will return true if there is an in-game overlay active.  This is important
  // because the cursor must be visible when such an overlay is active,
  // regardless of the ApplicationController setting.
  [[nodiscard]] auto overlayActive() const -> bool;

  void update();

private:
  PcPlatformServices() = default;

  Ptr<PcPlatformServicesState> m_state;

  Ptr<StatisticsService> m_statisticsService;
  Ptr<P2PNetworkingService> m_p2pNetworkingService;
  Ptr<UserGeneratedContentService> m_userGeneratedContentService;
  Ptr<DesktopService> m_desktopService;
};

}// namespace Star
