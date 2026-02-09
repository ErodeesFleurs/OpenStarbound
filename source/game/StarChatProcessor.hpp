#pragma once

#include "StarChatTypes.hpp"
#include "StarSet.hpp"
#include "StarThread.hpp"

import std;

namespace Star {

// Handles all chat routing and command parsing for client / server chat.
// Thread safe.
class ChatProcessor {
public:
  static char const* ServerNick;

  // CommandHandler is passed the origin connection, the command portion
  // excluding the '/' character, and the remaining command line in full.
  using CommandHandler = std::function<String(ConnectionId, String, String)>;

  auto connectClient(ConnectionId clientId, String nick = "") -> String;
  // Returns any pending messages.
  auto disconnectClient(ConnectionId clientId) -> List<ChatReceivedMessage>;

  auto clients() const -> List<ConnectionId>;
  auto hasClient(ConnectionId clientId) const -> bool;

  // Clears all clients and channels
  void reset();

  // Will return nothing if nick is not found.
  auto findNick(String const& nick) const -> std::optional<ConnectionId>;
  auto connectionNick(ConnectionId connectionId) const -> String;
  auto renick(ConnectionId clientId, String const& nick) -> String;

  // join / leave return true in the even that the client channel state was
  // actually changed.
  auto joinChannel(ConnectionId clientId, String const& channelName) -> bool;
  auto leaveChannel(ConnectionId clientId, String const& channelName) -> bool;

  auto clientChannels(ConnectionId clientId) const -> StringList;
  auto activeChannels() const -> StringList;

  void broadcast(ConnectionId sourceConnectionId, String const& text, JsonObject data = {});
  void message(ConnectionId sourceConnectionId, MessageContext::Mode context, String const& channelName, String const& text, JsonObject data = {});
  void whisper(ConnectionId sourceConnectionId, ConnectionId targetClientId, String const& text, JsonObject data = {});

  // Shorthand for passing ServerConnectionId as sourceConnectionId to
  // broadcast / message / whisper
  void adminBroadcast(String const& text);
  void adminMessage(MessageContext::Mode context, String const& channelName, String const& text);
  void adminWhisper(ConnectionId targetClientId, String const& text);

  auto pullPendingMessages(ConnectionId clientId) -> List<ChatReceivedMessage>;

  void setCommandHandler(CommandHandler commandHandler);
  void clearCommandHandler();

private:
  struct ClientInfo {
    ClientInfo(ConnectionId clientId, String const& nick);

    ConnectionId clientId;
    String nick;
    List<ChatReceivedMessage> pendingMessages;
  };

  auto makeNickUnique(String nick) -> String;

  // Returns true if message was handled completely and needs no further
  // processing.
  auto handleCommand(ChatReceivedMessage& message) -> bool;

  mutable RecursiveMutex m_mutex;

  HashMap<ConnectionId, ClientInfo> m_clients;
  StringMap<ConnectionId> m_nicks;
  StringMap<Set<ConnectionId>> m_channels;

  CommandHandler m_commandHandler;
};

}// namespace Star
