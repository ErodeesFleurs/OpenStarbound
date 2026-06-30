#include "StarPlayerNarrativeQueue.hpp"
#include "StarPlayer.hpp"
#include "StarPlayerLog.hpp"
#include "StarRoot.hpp"
#include "StarStatusController.hpp"
#include "StarRadioMessageDatabase.hpp"
#include "StarAssets.hpp"
#include "StarAiDatabase.hpp"
#include "StarLogging.hpp"

namespace Star {

PlayerNarrativeQueue::PlayerNarrativeQueue(Player* player)
  : m_player(player), m_interruptRadioMessage(false) {}

void PlayerNarrativeQueue::init(List<PersistentStatusEffect> inCinematicStatusEffects) {
  m_interruptRadioMessage = false;
  m_inCinematicStatusEffects = std::move(inCinematicStatusEffects);
}

bool PlayerNarrativeQueue::interruptRadioMessage() {
  if (m_interruptRadioMessage) {
    m_interruptRadioMessage = false;
    return true;
  }
  return false;
}

void PlayerNarrativeQueue::requestInterrupt() {
  m_interruptRadioMessage = true;
}

Maybe<RadioMessage> PlayerNarrativeQueue::pullPendingRadioMessage() {
  if (m_pendingRadioMessages.count()) {
    if (m_pendingRadioMessages.at(0).unique)
      m_player->log()->addRadioMessage(m_pendingRadioMessages.at(0).messageId);
    return m_pendingRadioMessages.takeFirst();
  }
  return {};
}

void PlayerNarrativeQueue::queueRadioMessage(Json const& messageConfig, float delay) {
  RadioMessage message;
  try {
    message = Root::singleton().radioMessageDatabase()->createRadioMessage(messageConfig);

    while (message.speciesAiMessage.contains(m_player->shipSpecies()) || message.speciesMessage.contains(m_player->species()))
      message = message.speciesAiMessage.value(m_player->shipSpecies(), message.speciesMessage.value(m_player->species()));

    if (message.type == RadioMessageType::Tutorial && !Root::singleton().configuration()->get("tutorialMessages").toBool())
      return;

    if (!message.portraitImage.empty() && message.portraitImage[0] != '/')
      message.portraitImage = Root::singleton().aiDatabase()->portraitImage(m_player->shipSpecies(), message.portraitImage);
  } catch (RadioMessageDatabaseException const& e) {
    Logger::error("Couldn't queue radio message '{}': {}", messageConfig, e.what());
    return;
  }

  if (m_player->log()->radioMessages().contains(message.messageId)) {
    return;
  } else {
    if (message.type == RadioMessageType::Mission) {
      if (m_missionRadioMessages.contains(message.messageId))
        return;
      else
        m_missionRadioMessages.add(message.messageId);
    }

    for (RadioMessage const& pendingMessage : m_pendingRadioMessages) {
      if (pendingMessage.messageId == message.messageId)
        return;
    }
    for (auto& delayedMessagePair : m_delayedRadioMessages) {
      if (delayedMessagePair.second.messageId == message.messageId) {
        if (delay == 0)
          delayedMessagePair.first.setDone();
        return;
      }
    }
  }

  if (delay > 0) {
    m_delayedRadioMessages.append(pair<GameTimer, RadioMessage>{GameTimer(delay), message});
  } else {
    queueRadioMessage(message);
  }
}

void PlayerNarrativeQueue::queueRadioMessage(RadioMessage message) {
  if (message.important) {
    m_interruptRadioMessage = true;
    m_pendingRadioMessages.prepend(message);
  } else {
    m_pendingRadioMessages.append(message);
  }
}

void PlayerNarrativeQueue::tickDelayedRadio(float dt) {
  for (auto& pair : m_delayedRadioMessages) {
    if (pair.first.tick(dt))
      queueRadioMessage(pair.second);
  }
  m_delayedRadioMessages.filter([](pair<GameTimer, RadioMessage>& pair) { return !pair.first.ready(); });
}

Maybe<Json> PlayerNarrativeQueue::pullPendingCinematic() {
  if (m_pendingCinematic && m_pendingCinematic->isType(Json::Type::String))
    m_player->log()->addCinematic(m_pendingCinematic->toString());
  return take(m_pendingCinematic);
}

void PlayerNarrativeQueue::setPendingCinematic(Json const& cinematic, bool unique) {
  if (unique && cinematic.isType(Json::Type::String) && m_player->log()->cinematics().contains(cinematic.toString()))
    return;
  m_pendingCinematic = cinematic;
}

void PlayerNarrativeQueue::setInCinematic(bool inCinematic) {
  if (inCinematic)
    m_player->statusController()->setPersistentEffects("cinematic", m_inCinematicStatusEffects);
  else
    m_player->statusController()->setPersistentEffects("cinematic", {});
}

Maybe<pair<Maybe<pair<StringList, int>>, float>> PlayerNarrativeQueue::pullPendingAltMusic() {
  if (m_pendingAltMusic)
    return m_pendingAltMusic.take();
  return {};
}

void PlayerNarrativeQueue::setPendingAltMusic(Maybe<pair<StringList, int>> tracks, float fadeTime) {
  m_pendingAltMusic = pair<Maybe<pair<StringList, int>>, float>(std::move(tracks), fadeTime);
}

Maybe<PlayerWarpRequest> PlayerNarrativeQueue::pullPendingWarp() {
  if (m_pendingWarp)
    return m_pendingWarp.take();
  return {};
}

void PlayerNarrativeQueue::setPendingWarp(String const& action, Maybe<String> const& animation, bool deploy) {
  m_pendingWarp = PlayerWarpRequest{action, animation, deploy};
}

Maybe<pair<Json, RpcPromiseKeeper<Json>>> PlayerNarrativeQueue::pullPendingConfirmation() {
  if (m_pendingConfirmations.count() > 0)
    return m_pendingConfirmations.takeFirst();
  return {};
}

void PlayerNarrativeQueue::queueConfirmation(Json const& dialogConfig, RpcPromiseKeeper<Json> const& resultPromise) {
  m_pendingConfirmations.append(make_pair(dialogConfig, resultPromise));
}

}
