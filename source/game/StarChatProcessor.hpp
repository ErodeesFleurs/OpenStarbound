#pragma once

#include "StarChatTypes.hpp"
#include "StarSet.hpp"
#include "StarThread.hpp"

namespace Star {

class ChatProcessor;
using ChatProcessorPtr = SharedPtr<ChatProcessor>;

// Handles all chat routing and command parsing for client / server chat.
// Thread safe.
class ChatProcessor {
public:
  static char const* ServerNick;

  // CommandHandler is passed the origin connection, the command portion
  // excluding the '/' character, and the remaining command line in full.
  using CommandHandler = function<String(ConnectionId, String, String)>;

  [[nodiscard]] String connectClient(ConnectionId clientId, String nick = "");
  // Returns any pending messages.
  [[nodiscard]] List<ChatReceivedMessage> disconnectClient(ConnectionId clientId);

  [[nodiscard]] List<ConnectionId> clients() const;
  [[nodiscard]] bool hasClient(ConnectionId clientId) const;

  // Clears all clients and channels
  void reset();

  // Will return nothing if nick is not found.
  [[nodiscard]] Maybe<ConnectionId> findNick(String const& nick) const;
  [[nodiscard]] String connectionNick(ConnectionId connectionId) const;
  [[nodiscard]] String renick(ConnectionId clientId, String const& nick);

  // join / leave return true in the even that the client channel state was
  // actually changed.
  [[nodiscard]] bool joinChannel(ConnectionId clientId, String const& channelName);
  [[nodiscard]] bool leaveChannel(ConnectionId clientId, String const& channelName);

  [[nodiscard]] StringList clientChannels(ConnectionId clientId) const;
  [[nodiscard]] StringList activeChannels() const;

  void broadcast(ConnectionId sourceConnectionId, String const& text, JsonObject data = {});
  void message(ConnectionId sourceConnectionId, MessageContext::Mode context, String const& channelName, String const& text, JsonObject data = {});
  void whisper(ConnectionId sourceConnectionId, ConnectionId targetClientId, String const& text, JsonObject data = {});

  // Shorthand for passing ServerConnectionId as sourceConnectionId to
  // broadcast / message / whisper
  void adminBroadcast(String const& text);
  void adminMessage(MessageContext::Mode context, String const& channelName, String const& text);
  void adminWhisper(ConnectionId targetClientId, String const& text);

  [[nodiscard]] List<ChatReceivedMessage> pullPendingMessages(ConnectionId clientId);

  void setCommandHandler(CommandHandler commandHandler);
  void clearCommandHandler();

private:
  struct ClientInfo {
    ClientInfo(ConnectionId clientId, String const& nick);

    ConnectionId clientId;
    String nick;
    List<ChatReceivedMessage> pendingMessages;
  };

  [[nodiscard]] String makeNickUnique(String nick);

  // Returns true if message was handled completely and needs no further
  // processing.
  [[nodiscard]] bool handleCommand(ChatReceivedMessage& message);

  mutable RecursiveMutex m_mutex;

  HashMap<ConnectionId, ClientInfo> m_clients;
  StringMap<ConnectionId> m_nicks;
  StringMap<Set<ConnectionId>> m_channels;

  CommandHandler m_commandHandler;
};

}
