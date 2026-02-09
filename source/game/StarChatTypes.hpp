#pragma once

#include "StarDataStream.hpp"
#include "StarGameTypes.hpp"
#include "StarJson.hpp"

import std;

namespace Star {

enum class ChatSendMode : std::uint8_t {
  Broadcast,
  Local,
  Party
};

extern EnumMap<ChatSendMode> const ChatSendModeNames;

struct MessageContext {
  enum Mode : std::uint8_t {
    Local,
    Party,
    Broadcast,
    Whisper,
    CommandResult,
    RadioMessage,
    World
  };

  MessageContext();
  MessageContext(Mode mode);
  MessageContext(Mode mode, String const& channelName);

  Mode mode;

  // only for Local and Party modes
  String channelName;
};

extern EnumMap<MessageContext::Mode> const MessageContextModeNames;

auto operator>>(DataStream& ds, MessageContext& messageContext) -> DataStream&;
auto operator<<(DataStream& ds, MessageContext const& messageContext) -> DataStream&;

struct ChatReceivedMessage {
  ChatReceivedMessage();
  ChatReceivedMessage(MessageContext context, ConnectionId fromConnection, String const& fromNick, String const& text);
  ChatReceivedMessage(MessageContext context, ConnectionId fromConnection, String const& fromNick, String const& text, String const& portrait);
  ChatReceivedMessage(Json const& json);

  [[nodiscard]] auto toJson() const -> Json;

  MessageContext context;

  ConnectionId fromConnection;
  String fromNick;
  String portrait;

  String text;

  JsonObject data;
};

auto operator>>(DataStream& ds, ChatReceivedMessage& receivedMessage) -> DataStream&;
auto operator<<(DataStream& ds, ChatReceivedMessage const& receivedMessage) -> DataStream&;

};// namespace Star
