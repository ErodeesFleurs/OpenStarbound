#pragma once

#include "StarAiTypes.hpp"
#include "StarGameTimers.hpp"
#include "StarGameTypes.hpp"
#include "StarPlayerTypes.hpp"
#include "StarRadioMessageDatabase.hpp"
#include "StarRpcPromise.hpp"
#include "StarStatusTypes.hpp"

namespace Star {

class Player;
class StatusController;
using StatusControllerPtr = SharedPtr<StatusController>;
class PlayerLog;
using PlayerLogPtr = SharedPtr<PlayerLog>;

class PlayerNarrativeQueue {
public:
  explicit PlayerNarrativeQueue(Player* player);

  void init(List<PersistentStatusEffect> inCinematicStatusEffects);

  bool interruptRadioMessage();
  void requestInterrupt();
  Maybe<RadioMessage> pullPendingRadioMessage();
  void queueRadioMessage(Json const& messageConfig, float delay = 0);
  void queueRadioMessage(RadioMessage message);
  void tickDelayedRadio(float dt);

  Maybe<Json> pullPendingCinematic();
  void setPendingCinematic(Json const& cinematic, bool unique = false);
  void setInCinematic(bool inCinematic);

  Maybe<pair<Maybe<pair<StringList, int>>, float>> pullPendingAltMusic();
  void setPendingAltMusic(Maybe<pair<StringList, int>> tracks, float fadeTime);

  Maybe<PlayerWarpRequest> pullPendingWarp();
  void setPendingWarp(String const& action, Maybe<String> const& animation = {}, bool deploy = false);

  Maybe<pair<Json, RpcPromiseKeeper<Json>>> pullPendingConfirmation();
  void queueConfirmation(Json const& dialogConfig, RpcPromiseKeeper<Json> const& resultPromise);

private:
  Player* m_player;

  StringSet m_missionRadioMessages;
  bool m_interruptRadioMessage;
  List<pair<GameTimer, RadioMessage>> m_delayedRadioMessages;
  Deque<RadioMessage> m_pendingRadioMessages;
  Maybe<Json> m_pendingCinematic;
  Maybe<pair<Maybe<pair<StringList, int>>, float>> m_pendingAltMusic;
  Maybe<PlayerWarpRequest> m_pendingWarp;
  Deque<pair<Json, RpcPromiseKeeper<Json>>> m_pendingConfirmations;
  List<PersistentStatusEffect> m_inCinematicStatusEffects;
};

}

