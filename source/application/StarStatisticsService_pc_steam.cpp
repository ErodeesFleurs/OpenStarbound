#include "StarConfig.hpp"
import star.maybe;
import star.format;
import star.thread;
import star.json;
import star.string;
import star.input_event;
import star.application;
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
import star.platform_services_pc;
import star.statistics_service_pc_steam;
import star.logging;

namespace Star {

SteamStatisticsService::SteamStatisticsService(PcPlatformServicesStatePtr state)
: m_state(std::move(state)),
  m_callbackUserStatsReceived(m_state->steamDispatch, this, &SteamStatisticsService::onUserStatsReceived),
  m_callbackUserStatsStored(m_state->steamDispatch, this, &SteamStatisticsService::onUserStatsStored),
  m_callbackAchievementStored(m_state->steamDispatch, this, &SteamStatisticsService::onAchievementStored) {
  m_appId = SteamAPI_ISteamUtils_GetAppID(SteamAPI_SteamUtils());
  refresh();
}

bool SteamStatisticsService::initialized() const {
  return m_initialized;
}

Maybe<String> SteamStatisticsService::error() const {
  return m_error;
}

bool SteamStatisticsService::setStat(String const& name, String const& type, Json const& value) {
  if (type == "int")
    return SteamAPI_ISteamUserStats_SetStatInt32(SteamAPI_SteamUserStats(), name.utf8Ptr(), (int32_t)value.toInt());

  if (type == "float")
    return SteamAPI_ISteamUserStats_SetStatFloat(SteamAPI_SteamUserStats(), name.utf8Ptr(), value.toFloat());

  return false;
}

Json SteamStatisticsService::getStat(String const& name, String const& type, Json def) const {
  if (type == "int") {
    int32_t intValue = 0;
    if (SteamAPI_ISteamUserStats_GetStatInt32(SteamAPI_SteamUserStats(), name.utf8Ptr(), &intValue))
      return Json(intValue);
  }

  if (type == "float") {
    float floatValue = 0.0f;
    if (SteamAPI_ISteamUserStats_GetStatFloat(SteamAPI_SteamUserStats(), name.utf8Ptr(), &floatValue))
      return Json(floatValue);
  }

  return def;
}

bool SteamStatisticsService::reportEvent(String const&, Json const&) {
  // Steam doesn't support events
  return false;
}

bool SteamStatisticsService::unlockAchievement(String const& name) {
  if (!SteamAPI_ISteamUserStats_SetAchievement(SteamAPI_SteamUserStats(), name.utf8Ptr())) {
    Logger::error("Cannot set Steam achievement {}", name);
    return false;
  }
  return true;
}

StringSet SteamStatisticsService::achievementsUnlocked() const {
  StringSet achievements;
  for (uint32_t i = 0; i < SteamAPI_ISteamUserStats_GetNumAchievements(SteamAPI_SteamUserStats()); ++i) {
    String achievement = SteamAPI_ISteamUserStats_GetAchievementName(SteamAPI_SteamUserStats(), i);

    bool unlocked = false;
    if (SteamAPI_ISteamUserStats_GetAchievement(SteamAPI_SteamUserStats(), achievement.utf8Ptr(), &unlocked) && unlocked) {
      achievements.add(achievement);
    }
  }
  return {};
}

void SteamStatisticsService::refresh() {
  if (!SteamAPI_ISteamUser_BLoggedOn(SteamAPI_SteamUser())) {
    m_error = {"Not logged in"};
    return;
  }

  SteamAPI_ISteamUserStats_RequestCurrentStats(SteamAPI_SteamUserStats());
}

void SteamStatisticsService::flush() {
  SteamAPI_ISteamUserStats_StoreStats(SteamAPI_SteamUserStats());
}

bool SteamStatisticsService::reset() {
  SteamAPI_ISteamUserStats_ResetAllStats(SteamAPI_SteamUserStats(), true);
  return true;
}

void SteamStatisticsService::onUserStatsReceived(UserStatsReceived_t* callback) {
  if (callback->m_nGameID != m_appId)
    return;

  if (callback->m_eResult != k_EResultOK) {
    m_error = {strf("Steam RequestCurrentStats failed with code {}", (int)callback->m_eResult)};
    return;
  }

  Logger::debug("Steam RequestCurrentStats successful");
  m_initialized = true;
}

void SteamStatisticsService::onUserStatsStored(UserStatsStored_t* callback) {
  if (callback->m_nGameID != m_appId)
    return;

  if (callback->m_eResult == k_EResultOK) {
    Logger::debug("Steam StoreStats successful");
    return;
  }

  if (callback->m_eResult == k_EResultInvalidParam) {
    // A stat we set broke a constraint and was reverted on the service.
    Logger::info("Steam StoreStats: Some stats failed validation");
    return;
  }

  m_error = {strf("Steam StoreStats failed with code {}", (int)callback->m_eResult)};
}

void SteamStatisticsService::onAchievementStored(UserAchievementStored_t* callback) {
  if (callback->m_nGameID != m_appId)
    return;

  Logger::debug("Steam achievement {} stored successfully", callback->m_rgchAchievementName);
}

}
