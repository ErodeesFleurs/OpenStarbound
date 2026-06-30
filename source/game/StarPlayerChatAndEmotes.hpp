#pragma once

#include "StarChatAction.hpp"
#include "StarChatTypes.hpp"
#include "StarGameTimers.hpp"
#include "StarGameTypes.hpp"
#include "StarHumanoid.hpp"
#include "StarJson.hpp"
#include "StarRpcPromise.hpp"

namespace Star {

class Player;
class DanceDatabase;
using DanceDatabaseConstPtr = SharedPtr<DanceDatabase const>;
class EmoteProcessor;
using EmoteProcessorConstPtr = SharedPtr<EmoteProcessor const>;

class PlayerChatAndEmotes {
public:
  PlayerChatAndEmotes(Player& player, DanceDatabaseConstPtr danceDatabase, EmoteProcessorConstPtr emoteProcessor);

  void init(float emoteCooldown, Vec2F blinkInterval);

  void addChatMessage(String const& message, Json const& config = {});
  void addEmote(HumanoidEmote const& emote, Maybe<float> emoteCooldown = {});
  void setDance(Maybe<String> const& danceName);
  [[nodiscard]] pair<HumanoidEmote, float> currentEmote() const;
  [[nodiscard]] List<ChatAction> pullPendingChatActions();
  void playEmote(HumanoidEmote emote);
  void requestEmote(String const& emote);
  void tickChatAndEmotes(float dt);
  void tickBlink(float dt);

  [[nodiscard]] HumanoidEmote emoteState() const;
  void setEmoteState(HumanoidEmote emote);
  [[nodiscard]] Maybe<String> const& dance() const;
  [[nodiscard]] String const& chatMessage() const;
  void setChatMessage(String const& message);
  [[nodiscard]] bool chatMessageChanged() const;
  void clearChatMessageChanged();

private:
  [[nodiscard]] HumanoidEmote detectEmotes(String const& chatter);

  Player& m_player;
  DanceDatabaseConstPtr m_danceDatabase;
  EmoteProcessorConstPtr m_emoteProcessor;

  HumanoidEmote m_emoteState = HumanoidEmote::Idle;
  Maybe<String> m_dance;
  GameTimer m_danceCooldownTimer;
  GameTimer m_emoteCooldownTimer;
  GameTimer m_blinkCooldownTimer;
  float m_emoteCooldown;
  Vec2F m_blinkInterval;

  String m_chatMessage;
  bool m_chatMessageChanged = false;
  bool m_chatMessageUpdated = false;

  List<ChatAction> m_pendingChatActions;
};

}
