#include "StarPlayerNarrativeQueue.hpp"
#include "StarAlgorithm.hpp"
#include "StarPlayer.hpp"
#include "StarPlayerLog.hpp"
#include "StarStatusController.hpp"
#include "StarRadioMessageDatabase.hpp"
#include "StarAssets.hpp"
#include "StarAiDatabase.hpp"
#include "StarConfiguration.hpp"
#include "StarLogging.hpp"

namespace Star {

PlayerNarrativeQueue::PlayerNarrativeQueue(Player& player, RadioMessageDatabaseConstPtr radioMessageDatabase, ConfigurationPtr configuration, AiDatabaseConstPtr aiDatabase)
  : m_player(player),
    m_radioMessageDatabase(requireServiceValueAs<StarException>(std::move(radioMessageDatabase), "PlayerNarrativeQueue", "radio message database")),
    m_configuration(requireServiceValueAs<StarException>(std::move(configuration), "PlayerNarrativeQueue", "configuration")),
    m_aiDatabase(requireServiceValueAs<StarException>(std::move(aiDatabase), "PlayerNarrativeQueue", "ai database")),
    m_interruptRadioMessage(false) {
}

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
      m_player.log()->addRadioMessage(m_pendingRadioMessages.at(0).messageId);
    return m_pendingRadioMessages.takeFirst();
  }
  return {};
}

void PlayerNarrativeQueue::queueRadioMessage(Json const& messageConfig, float delay) {
  RadioMessage message;
  try {
    message = m_radioMessageDatabase->createRadioMessage(messageConfig);

    while (message.speciesAiMessage.contains(m_player.shipSpecies()) || message.speciesMessage.contains(m_player.species()))
      message = message.speciesAiMessage.value(m_player.shipSpecies(), message.speciesMessage.value(m_player.species()));

    if (message.type == RadioMessageType::Tutorial && !m_configuration->get("tutorialMessages").toBool())
      return;

    if (!message.portraitImage.empty() && message.portraitImage[0] != '/')
      message.portraitImage = m_aiDatabase->portraitImage(m_player.shipSpecies(), message.portraitImage);
  } catch (RadioMessageDatabaseException const& e) {
    Logger::error("Couldn't queue radio message '{}': {}", messageConfig, e.what());
    return;
  }

  if (m_player.log()->radioMessages().contains(message.messageId)) {
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
    for (auto& delayedMessage : m_delayedRadioMessages) {
      if (delayedMessage.message.messageId == message.messageId) {
        if (delay == 0)
          delayedMessage.timer.setDone();
        return;
      }
    }
  }

  if (delay > 0) {
    m_delayedRadioMessages.append(DelayedRadioMessage{GameTimer(delay), message});
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
  for (auto& delayedMessage : m_delayedRadioMessages) {
    if (delayedMessage.timer.tick(dt))
      queueRadioMessage(delayedMessage.message);
  }
  m_delayedRadioMessages.filter([](DelayedRadioMessage& delayedMessage) {
      return !delayedMessage.timer.ready();
    });
}

Maybe<Json> PlayerNarrativeQueue::pullPendingCinematic() {
  if (m_pendingCinematic && m_pendingCinematic->isType(Json::Type::String))
    m_player.log()->addCinematic(m_pendingCinematic->toString());
  return take(m_pendingCinematic);
}

void PlayerNarrativeQueue::setPendingCinematic(Json const& cinematic, bool unique) {
  if (unique && cinematic.isType(Json::Type::String) && m_player.log()->cinematics().contains(cinematic.toString()))
    return;
  m_pendingCinematic = cinematic;
}

void PlayerNarrativeQueue::setInCinematic(bool inCinematic) {
  if (inCinematic)
    m_player.statusController()->setPersistentEffects("cinematic", m_inCinematicStatusEffects);
  else
    m_player.statusController()->setPersistentEffects("cinematic", {});
}

Maybe<pair<Maybe<pair<StringList, int>>, float>> PlayerNarrativeQueue::pullPendingAltMusic() {
  if (m_pendingAltMusic) {
    auto pendingAltMusic = m_pendingAltMusic.take();
    return pair<Maybe<pair<StringList, int>>, float>(std::move(pendingAltMusic.tracks), pendingAltMusic.fadeTime);
  }
  return {};
}

void PlayerNarrativeQueue::setPendingAltMusic(Maybe<pair<StringList, int>> tracks, float fadeTime) {
  m_pendingAltMusic = PendingAltMusic{std::move(tracks), fadeTime};
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
  if (m_pendingConfirmations.count() > 0) {
    auto pendingConfirmation = m_pendingConfirmations.takeFirst();
    return pair<Json, RpcPromiseKeeper<Json>>(std::move(pendingConfirmation.dialogConfig), std::move(pendingConfirmation.resultPromise));
  }
  return {};
}

void PlayerNarrativeQueue::queueConfirmation(Json const& dialogConfig, RpcPromiseKeeper<Json> const& resultPromise) {
  m_pendingConfirmations.append(PendingConfirmation{dialogConfig, resultPromise});
}

}
