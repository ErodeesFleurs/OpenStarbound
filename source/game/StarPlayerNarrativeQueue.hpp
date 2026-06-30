#pragma once

#include "StarAiTypes.hpp"
#include "StarGameTimers.hpp"
#include "StarGameTypes.hpp"
#include "StarPlayerTypes.hpp"
#include "StarRadioMessageDatabase.hpp"
#include "StarRpcPromise.hpp"
#include "StarStatusTypes.hpp"

#include "StarConfiguration.hpp"

namespace Star {

class Player;
class AiDatabase;
using AiDatabaseConstPtr = SharedPtr<AiDatabase const>;
class RadioMessageDatabase;
using RadioMessageDatabaseConstPtr = SharedPtr<RadioMessageDatabase const>;
class StatusController;
using StatusControllerPtr = SharedPtr<StatusController>;
class PlayerLog;
using PlayerLogPtr = SharedPtr<PlayerLog>;

class PlayerNarrativeQueue {
public:
  PlayerNarrativeQueue(Player& player, RadioMessageDatabaseConstPtr radioMessageDatabase, ConfigurationPtr configuration, AiDatabaseConstPtr aiDatabase);

  void init(List<PersistentStatusEffect> inCinematicStatusEffects);

  [[nodiscard]] bool interruptRadioMessage();
  void requestInterrupt();
  [[nodiscard]] Maybe<RadioMessage> pullPendingRadioMessage();
  void queueRadioMessage(Json const& messageConfig, float delay = 0);
  void queueRadioMessage(RadioMessage message);
  void tickDelayedRadio(float dt);

  [[nodiscard]] Maybe<Json> pullPendingCinematic();
  void setPendingCinematic(Json const& cinematic, bool unique = false);
  void setInCinematic(bool inCinematic);

  [[nodiscard]] Maybe<pair<Maybe<pair<StringList, int>>, float>> pullPendingAltMusic();
  void setPendingAltMusic(Maybe<pair<StringList, int>> tracks, float fadeTime);

  [[nodiscard]] Maybe<PlayerWarpRequest> pullPendingWarp();
  void setPendingWarp(String const& action, Maybe<String> const& animation = {}, bool deploy = false);

  [[nodiscard]] Maybe<pair<Json, RpcPromiseKeeper<Json>>> pullPendingConfirmation();
  void queueConfirmation(Json const& dialogConfig, RpcPromiseKeeper<Json> const& resultPromise);

private:
  Player& m_player;
  RadioMessageDatabaseConstPtr m_radioMessageDatabase;
  ConfigurationPtr m_configuration;
  AiDatabaseConstPtr m_aiDatabase;

  StringSet m_missionRadioMessages;
  bool m_interruptRadioMessage;

  struct DelayedRadioMessage {
    GameTimer timer;
    RadioMessage message;
  };
  struct PendingAltMusic {
    Maybe<pair<StringList, int>> tracks;
    float fadeTime;
  };
  struct PendingConfirmation {
    Json dialogConfig;
    RpcPromiseKeeper<Json> resultPromise;
  };
  List<DelayedRadioMessage> m_delayedRadioMessages;
  Deque<RadioMessage> m_pendingRadioMessages;
  Maybe<Json> m_pendingCinematic;
  Maybe<PendingAltMusic> m_pendingAltMusic;
  Maybe<PlayerWarpRequest> m_pendingWarp;
  Deque<PendingConfirmation> m_pendingConfirmations;
  List<PersistentStatusEffect> m_inCinematicStatusEffects;
};

}
