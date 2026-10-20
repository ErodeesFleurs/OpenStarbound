#ifdef STAR_ENABLE_STEAM_INTEGRATION
#include "steam/steam_api.h"
#include "steam/steam_api_flat.h"
#endif
#ifdef STAR_ENABLE_DISCORD_INTEGRATION
#include "discord/discord.h"
#endif
import std;
import star.logging;
import star.thread;
import star.string;
import star.file;
import star.input_event;
import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;

#ifdef STAR_ENABLE_STEAM_INTEGRATION
import star.steam_dispatch;
#endif

import star.platform_services_pc;

import star.strong_typedef;
import star.rpc_promise;
import star.p2p_networking_service_pc;

#ifdef STAR_ENABLE_STEAM_INTEGRATION
import star.statistics_service_pc_steam;

import star.user_generated_content_service_pc_steam;

import star.desktop_service_pc_steam;
#endif

namespace Star {

#ifdef STAR_ENABLE_DISCORD_INTEGRATION
std::uint64_t const DiscordClientId = 467102538278109224;
#endif

PcPlatformServicesState::PcPlatformServicesState(Logger& logger)
  : logger(logger)
#ifdef STAR_ENABLE_STEAM_INTEGRATION
  , callbackGameOverlayActivated(steamDispatch, this, &PcPlatformServicesState::onGameOverlayActivated)
#endif
{

#ifdef STAR_ENABLE_STEAM_INTEGRATION
  bool shouldLoadSteam = true;

  #ifdef STAR_SYSTEM_LINUX
  // Flatpak doesn't create ~/.steam by default, which prevents
  // SteamAPI from initializing...
  //
  // Flatpak also causes crashes should the user rectify this by
  // symlinking.
  //
  // We'll determine if the user is using Flatpak for Steam here,
  // so we can warn them about such issues.
  shouldLoadSteam = true;

  bool steamFlatpakInstalled = false;

  if (File::isDirectory("/var/lib/flatpak/app/com.valvesoftware.Steam")) {
    logger.info("Steam flatpak is installed");
    steamFlatpakInstalled = true;
  }

  String steamRootPath = String::joinWith("", std::getenv("HOME"), "/.steam");
  if (!File::isDirectory(steamRootPath)) {
    if (steamFlatpakInstalled) {
      steamIsFlatpak = true;
    } else {
      logger.info("Could not find ~/.steam; Steam platform services will be unavailable!");
      shouldLoadSteam = false;
    }
  } else {
    // Account for the user having Steam installed as both a flatpak
    // and normal.
    if (steamFlatpakInstalled) {
      // Check if ~/.steam points to a flatpak installation
      String steamAbsRootPath = File::fullPath(steamRootPath);
      if (steamAbsRootPath.contains("com.valvesoftware.Steam")) {
        logger.info("~/.steam points towards the installed flatpak");
        steamIsFlatpak = true;
        shouldLoadSteam = false;
      } else {
        logger.info("~/.steam does not point towards the installed flatpak");
      }
    }
  }
  #endif

  if (shouldLoadSteam) {
    SteamErrMsg errMsg;
    if (SteamAPI_InitEx(&errMsg) == k_ESteamAPIInitResult_OK) {
      steamAvailable = true;
      steamDispatch.initialize();
      logger.info("Initialized Steam platform services");
    } else {
      logger.info("Failed to initialize Steam platform services: {}", errMsg);
    }
  } else {
    logger.info("Skipping Steam platform initialization");
  }
#endif

#ifdef STAR_ENABLE_DISCORD_INTEGRATION
  static std::int64_t const DiscordEventSleep = 3;

  discord::Core* discordCorePtr = nullptr;
  discord::Result res = discord::Core::Create(DiscordClientId, DiscordCreateFlags_NoRequireDiscord, &discordCorePtr);
  if (res == discord::Result::Ok && discordCorePtr) {
    discordCore.reset(discordCorePtr);
    discordAvailable = true;

    discordCore->UserManager().OnCurrentUserUpdate.Connect([this](){
        discord::User user;
        auto res = discordCore->UserManager().GetCurrentUser(&user);
        if (res != discord::Result::Ok)
          this->logger.error("Could not get current Discord user. (err {})", (int)res);
        else
          discordCurrentUser = user;
      });

  } else {
    logger.error("Failed to instantiate Discord core (err {})", (int)res);
  }

  if (discordAvailable) {
    MutexLocker locker(discordMutex);
    discordCore->SetLogHook(discord::LogLevel::Info, [this](discord::LogLevel level, char const* msg) {
      if (level == discord::LogLevel::Debug)
        this->logger.debug("[Discord]: {}", msg);
      else if (level == discord::LogLevel::Error)
        this->logger.debug("[Discord]: {}", msg);
      else if (level == discord::LogLevel::Info)
        this->logger.info("[Discord]: {}", msg);
      else if (level == discord::LogLevel::Warn)
        this->logger.warn("[Discord]: {}", msg);
    });
    discordEventShutdown = false;
    discordEventThread = Thread::invoke("PcPlatformServices::discordEventThread", [this]() {
        while (!discordEventShutdown) {
          {
            MutexLocker locker(discordMutex);
            discordCore->RunCallbacks();
            discordCore->LobbyManager().FlushNetwork();
          }
          Thread::sleep(DiscordEventSleep);
        }
      });

    logger.info("Initialized Discord platform services");
  } else {
    logger.info("Was not able to authenticate with Discord and create all components, Discord services will be unavailable");
  }
#endif
}

PcPlatformServicesState::~PcPlatformServicesState() {
#ifdef STAR_ENABLE_DISCORD_INTEGRATION
  if (discordAvailable) {
    discordEventShutdown = true;
    discordEventThread.finish();
  }
#endif
#ifdef STAR_ENABLE_STEAM_INTEGRATION
  if (steamAvailable)
    SteamAPI_Shutdown();
#endif
}

#ifdef STAR_ENABLE_STEAM_INTEGRATION
void PcPlatformServicesState::onGameOverlayActivated(GameOverlayActivated_t* callback) {
  overlayActive = callback->m_bActive;
}
#endif

std::unique_ptr<PcPlatformServices> PcPlatformServices::create(Logger& logger, [[maybe_unused]] String const& path, StringList platformArguments) {
  auto services = std::unique_ptr<PcPlatformServices>(new PcPlatformServices);

  services->m_state = std::make_shared<PcPlatformServicesState>(logger);

  bool provideP2PNetworking = false;

#ifdef STAR_ENABLE_STEAM_INTEGRATION
  provideP2PNetworking |= services->m_state->steamAvailable;
#endif

#ifdef STAR_ENABLE_DISCORD_INTEGRATION
  provideP2PNetworking |= services->m_state->discordAvailable;
#endif

  if (provideP2PNetworking) {
    auto p2pNetworkingService = std::make_shared<PcP2PNetworkingService>(services->m_state);
    for (auto& argument : platformArguments) {
      if (argument.beginsWith("+platform:connect:")) {
        logger.info("PC platform services joining from command line argument '{}'", argument);
        p2pNetworkingService->addPendingJoin(std::move(argument));
      } else {
        throw ApplicationException::format("Unrecognized PC platform services command line argument '{}'", argument);
      }
    }

    services->m_p2pNetworkingService = p2pNetworkingService;
  }

#ifdef STAR_ENABLE_STEAM_INTEGRATION
  if (services->m_state->steamAvailable) {
    services->m_statisticsService = std::make_shared<SteamStatisticsService>(services->m_state);
    services->m_userGeneratedContentService = std::make_shared<SteamUserGeneratedContentService>(services->m_state);
    services->m_desktopService = std::make_shared<SteamDesktopService>(services->m_state);
  }
#endif

#ifdef STAR_ENABLE_DISCORD_INTEGRATION
  MutexLocker discordLocker(services->m_state->discordMutex);
  if (services->m_state->discordAvailable) {
    logger.debug("Registering Starbound to Discord at path: {}", path);
    services->m_state->discordCore->ActivityManager().RegisterCommand(path.utf8Ptr());
  }
#endif

  return services;
}

std::shared_ptr<StatisticsService> PcPlatformServices::statisticsService() const {
  return m_statisticsService;
}

std::shared_ptr<P2PNetworkingService> PcPlatformServices::p2pNetworkingService() const {
  return m_p2pNetworkingService;
}

std::shared_ptr<UserGeneratedContentService> PcPlatformServices::userGeneratedContentService() const {
  return m_userGeneratedContentService;
}

std::shared_ptr<DesktopService> PcPlatformServices::desktopService() const {
  return m_desktopService;
}

bool PcPlatformServices::steamIsFlatpak() const {
  return m_state->steamIsFlatpak;
}

bool PcPlatformServices::overlayActive() const {
  return m_state->overlayActive;
}

void PcPlatformServices::update() {
#ifdef STAR_ENABLE_STEAM_INTEGRATION
  if (m_state->steamAvailable)
    m_state->steamDispatch.runCallbacks();
#endif
}

}
