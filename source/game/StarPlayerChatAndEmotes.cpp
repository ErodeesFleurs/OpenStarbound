#include "StarPlayerChatAndEmotes.hpp"
#include "StarPlayer.hpp"
#include "StarRoot.hpp"
#include "StarEmoteProcessor.hpp"
#include "StarDanceDatabase.hpp"
#include "StarActorMovementController.hpp"

namespace Star {

PlayerChatAndEmotes::PlayerChatAndEmotes(Player* player)
  : m_player(player),
    m_emoteState(HumanoidEmote::Idle),
    m_chatMessageChanged(false),
    m_chatMessageUpdated(false) {}

void PlayerChatAndEmotes::init(float emoteCooldown, Vec2F blinkInterval) {
  m_emoteCooldown = emoteCooldown;
  m_blinkInterval = blinkInterval;
  m_emoteCooldownTimer = GameTimer(m_emoteCooldown);
  m_blinkCooldownTimer = GameTimer(0);
  m_chatMessageChanged = false;
  m_chatMessageUpdated = false;
}

void PlayerChatAndEmotes::addChatMessage(String const& message, Json const& config) {
  starAssert(!m_player->isSlave());
  m_chatMessage = message;
  m_chatMessageUpdated = true;
  m_chatMessageChanged = true;
  m_pendingChatActions.append(SayChatAction{m_player->entityId(), message, m_player->mouthPosition(), config});
}

void PlayerChatAndEmotes::addEmote(HumanoidEmote const& emote, Maybe<float> emoteCooldown) {
  starAssert(!m_player->isSlave());
  m_emoteState = emote;
  m_emoteCooldownTimer = GameTimer(emoteCooldown.value(m_emoteCooldown));
}

void PlayerChatAndEmotes::setDance(Maybe<String> const& danceName) {
  starAssert(!m_player->isSlave());
  m_dance = danceName;

  if (danceName.isValid()) {
    auto danceDatabase = Root::singleton().danceDatabase();
    DancePtr dance = danceDatabase->getDance(*danceName);
    m_danceCooldownTimer = GameTimer(dance->duration);
  }
}

pair<HumanoidEmote, float> PlayerChatAndEmotes::currentEmote() const {
  return make_pair(m_emoteState, m_emoteCooldownTimer.timer);
}

List<ChatAction> PlayerChatAndEmotes::pullPendingChatActions() {
  return take(m_pendingChatActions);
}

void PlayerChatAndEmotes::playEmote(HumanoidEmote emote) {
  addEmote(emote);
}

void PlayerChatAndEmotes::requestEmote(String const& emote) {
  auto state = HumanoidEmoteNames.getLeft(emote);
  if (state != HumanoidEmote::Idle
      && (m_emoteState == state || m_emoteState == HumanoidEmote::Idle || m_emoteState == HumanoidEmote::Blink))
    addEmote(state);
}

void PlayerChatAndEmotes::tickChatAndEmotes(float dt) {
  if (m_emoteCooldownTimer.tick(dt))
    m_emoteState = HumanoidEmote::Idle;
  if (m_danceCooldownTimer.tick(dt))
    m_dance = {};

  if (m_chatMessageUpdated) {
    auto state = Root::singleton().emoteProcessor()->detectEmotes(m_chatMessage);
    if (state != HumanoidEmote::Idle)
      addEmote(state);
    m_chatMessageUpdated = false;
  }
}

void PlayerChatAndEmotes::tickBlink(float dt) {
  if (m_blinkCooldownTimer.tick(dt)) {
    m_blinkCooldownTimer = GameTimer(Random::randf(m_blinkInterval[0], m_blinkInterval[1]));
    auto loungeAnchor = as<LoungeAnchor>(m_player->movementController()->entityAnchor());
    if (m_emoteState == HumanoidEmote::Idle && (!loungeAnchor || !loungeAnchor->emote))
      addEmote(HumanoidEmote::Blink);
  }
}

HumanoidEmote PlayerChatAndEmotes::detectEmotes(String const& chatter) {
  return Root::singleton().emoteProcessor()->detectEmotes(chatter);
}

HumanoidEmote PlayerChatAndEmotes::emoteState() const {
  return m_emoteState;
}

void PlayerChatAndEmotes::setEmoteState(HumanoidEmote emote) {
  m_emoteState = emote;
}

Maybe<String> const& PlayerChatAndEmotes::dance() const {
  return m_dance;
}

String const& PlayerChatAndEmotes::chatMessage() const {
  return m_chatMessage;
}

void PlayerChatAndEmotes::setChatMessage(String const& message) {
  m_chatMessage = message;
  m_chatMessageUpdated = true;
}

bool PlayerChatAndEmotes::chatMessageChanged() const {
  return m_chatMessageChanged;
}

void PlayerChatAndEmotes::clearChatMessageChanged() {
  m_chatMessageChanged = false;
}

}

