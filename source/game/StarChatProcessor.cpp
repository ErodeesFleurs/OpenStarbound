import std;
import star.string;
import star.list;
import star.format;
import star.json;
import star.data_stream;
import star.rpc_promise;
import star.game_types;
import star.chat_types;
import star.set;
import star.thread;
import star.uuid;
import star.random;

import star.chat_processor;

import star.algorithm;
namespace Star {

ChatProcessor::ChatProcessor(RandomService& random) : m_random(random) {}

char const* const ChatProcessor::ServerNick = "server";

std::expected<String, StarException> ChatProcessor::connectClient(ConnectionId clientId, String nick) {
  RecursiveMutexLocker locker(m_mutex);

  if (nick.empty())
    nick = strf("Player_{}", clientId);

  nick = makeNickUnique(nick);

  for (auto& pair : m_clients) {
    pair.second.pendingMessages.append({
        {MessageContext::Broadcast},
        ServerConnectionId,
        ServerNick,
        strf("Player '{}' connected", nick)
      });
  }

  auto added = m_clients.add(clientId, ClientInfo(clientId, nick));
  if (!added) return std::unexpected(std::move(added.error()));
  m_nicks[nick] = clientId;
  return nick;
}

std::expected<List<ChatReceivedMessage>, StarException> ChatProcessor::disconnectClient(ConnectionId clientId) {
  RecursiveMutexLocker locker(m_mutex);

  for (auto channel : clientChannels(clientId))
    leaveChannel(clientId, channel);

  auto clientInfoResult = m_clients.take(clientId);
  if (!clientInfoResult) return std::unexpected(std::move(clientInfoResult.error()));
  auto clientInfo = std::move(*clientInfoResult);

  m_nicks.remove(clientInfo.nick);

  for (auto& pair : m_clients) {
    pair.second.pendingMessages.append({
        {MessageContext::Broadcast},
        ServerConnectionId,
        ServerNick,
        strf("Player '{}' disconnected", clientInfo.nick)
      });
  }

  return clientInfo.pendingMessages;
}

List<ConnectionId> ChatProcessor::clients() const {
  RecursiveMutexLocker locker(m_mutex);
  return m_clients.keys();
}

bool ChatProcessor::hasClient(ConnectionId clientId) const {
  RecursiveMutexLocker locker(m_mutex);
  return m_clients.contains(clientId);
}

std::optional<ConnectionId> ChatProcessor::findNick(String const& nick) const {
  RecursiveMutexLocker locker(m_mutex);
  if (auto m = m_nicks.maybe(nick))
    return m;
  if (nick == ServerNick)
    return ServerConnectionId;
  return {};
}

std::expected<String, StarException> ChatProcessor::connectionNick(ConnectionId clientId) const {
  RecursiveMutexLocker locker(m_mutex);
  if (clientId == ServerConnectionId)
    return ServerNick;
  auto client = m_clients.get(clientId);
  if (!client) return std::unexpected(std::move(client.error()));
  return (*client)->nick;
}

std::expected<String, StarException> ChatProcessor::renick(ConnectionId clientId, String const& nick) {
  RecursiveMutexLocker locker(m_mutex);

  auto clientInfoResult = m_clients.get(clientId);
  if (!clientInfoResult) return std::unexpected(std::move(clientInfoResult.error()));
  auto& clientInfo = **clientInfoResult;
  m_nicks.remove(clientInfo.nick);

  clientInfo.nick = makeNickUnique(nick);
  clientInfo.nick = nick;
  m_nicks[nick] = clientId;
  return nick;
}

bool ChatProcessor::joinChannel(ConnectionId clientId, String const& channelName) {
  RecursiveMutexLocker locker(m_mutex);

  // Right now channels are simply created on join if they don't exist.
  return m_channels[channelName].add(clientId);
}

bool ChatProcessor::leaveChannel(ConnectionId clientId, String const& channelName) {
  RecursiveMutexLocker locker(m_mutex);
  return m_channels[channelName].remove(clientId);
}

StringList ChatProcessor::clientChannels(ConnectionId clientId) const {
  RecursiveMutexLocker locker(m_mutex);

  StringList channels;
  for (auto const& pair : m_channels) {
    if (pair.second.contains(clientId))
      channels.append(pair.first);
  }
  return channels;
}

StringList ChatProcessor::activeChannels() const {
  RecursiveMutexLocker locker(m_mutex);

  StringList channels;
  for (auto const& pair : m_channels) {
    if (!pair.second.empty())
      channels.append(pair.first);
  }
  return channels;
}

std::expected<void, StarException> ChatProcessor::broadcast(ConnectionId sourceConnectionId, String const& text, JsonObject data) {
  RecursiveMutexLocker locker(m_mutex);

  auto nick = connectionNick(sourceConnectionId);
  if (!nick) return std::unexpected(std::move(nick.error()));
  ChatReceivedMessage message = {
    {MessageContext::Broadcast},
    sourceConnectionId,
    *nick,
    text
  };

  message.data = std::move(data);

  auto handled = handleCommand(message);
  if (!handled) return std::unexpected(std::move(handled.error()));
  if (*handled) return {};

  for (auto& pair : m_clients)
    pair.second.pendingMessages.append(message);
  return {};
}

std::expected<void, StarException> ChatProcessor::message(ConnectionId sourceConnectionId, MessageContext::Mode mode, String const& channelName, String const& text, JsonObject data) {
  RecursiveMutexLocker locker(m_mutex);

  auto nick = connectionNick(sourceConnectionId);
  if (!nick) return std::unexpected(std::move(nick.error()));
  ChatReceivedMessage message = {
    {mode, channelName},
    sourceConnectionId,
    *nick,
    text
  };

  message.data = std::move(data);

  auto handled = handleCommand(message);
  if (!handled) return std::unexpected(std::move(handled.error()));
  if (*handled) return {};

  for (auto clientId : m_channels[channelName]) {
    auto clientInfoResult = m_clients.get(clientId);
    if (!clientInfoResult) return std::unexpected(std::move(clientInfoResult.error()));
    auto& clientInfo = **clientInfoResult;
    clientInfo.pendingMessages.append(message);
  }
  return {};
}

std::expected<void, StarException> ChatProcessor::whisper(ConnectionId sourceConnectionId, ConnectionId targetClientId, String const& text, JsonObject data) {
  RecursiveMutexLocker locker(m_mutex);

  auto nick = connectionNick(sourceConnectionId);
  if (!nick) return std::unexpected(std::move(nick.error()));
  ChatReceivedMessage message = {
    {MessageContext::Whisper},
    sourceConnectionId,
    *nick,
    text
  };

  message.data = std::move(data);

  auto handled = handleCommand(message);
  if (!handled) return std::unexpected(std::move(handled.error()));
  if (*handled) return {};

  if (sourceConnectionId != ServerConnectionId) {
    auto source = m_clients.get(sourceConnectionId);
    if (!source) return std::unexpected(std::move(source.error()));
    (*source)->pendingMessages.append(message);
  }
  auto target = m_clients.get(targetClientId);
  if (!target) return std::unexpected(std::move(target.error()));
  (*target)->pendingMessages.append(message);
  return {};
}

std::expected<void, StarException> ChatProcessor::adminBroadcast(String const& text) {
  RecursiveMutexLocker locker(m_mutex);
  return broadcast(ServerConnectionId, text, {});
}

std::expected<void, StarException> ChatProcessor::adminMessage(MessageContext::Mode context, String const& channelName, String const& text) {
  RecursiveMutexLocker locker(m_mutex);
  return ChatProcessor::message(ServerConnectionId, context, channelName, text, {});
}

std::expected<void, StarException> ChatProcessor::adminWhisper(ConnectionId targetClientId, String const& text) {
  RecursiveMutexLocker locker(m_mutex);
  return whisper(ServerConnectionId, targetClientId, text, {});
}

std::expected<List<ChatReceivedMessage>, StarException> ChatProcessor::pullPendingMessages(ConnectionId clientId) {
  RecursiveMutexLocker locker(m_mutex);
  if (auto client = m_clients.ptr(clientId))
    return take(client->pendingMessages);
  return List<ChatReceivedMessage>{};
}

void ChatProcessor::setCommandHandler(CommandHandler commandHandler) {
  RecursiveMutexLocker locker(m_mutex);
  m_commandHandler = commandHandler;
}

void ChatProcessor::clearCommandHandler() {
  RecursiveMutexLocker locker(m_mutex);
  m_commandHandler = {};
}

ChatProcessor::ClientInfo::ClientInfo(ConnectionId clientId, String const& nick) : clientId(clientId), nick(nick) {}

String ChatProcessor::makeNickUnique(String nick) {
  while (m_nicks.contains(nick) || nick == ServerNick)
    nick.append("_");

  return nick;
}

std::expected<bool, StarException> ChatProcessor::handleCommand(ChatReceivedMessage& message) {
  auto isCommand = message.text.beginsWith("/");
  if (!isCommand) return std::unexpected(std::move(isCommand.error()));
  if (!*isCommand) return false;
  auto escaped = message.text.beginsWith("//");
  if (!escaped) return std::unexpected(std::move(escaped.error()));
  auto line = message.text.substr(1);
  if (!line) return std::unexpected(std::move(line.error()));
  if (*escaped) {
    message.text = std::move(*line);
    return false;
  }
  String commandLine = std::move(*line);
  auto command = commandLine.extract();
  if (!command) return std::unexpected(std::move(command.error()));
  String response;
  if (*command == "nick") {
    auto trimmed = commandLine.trim();
    if (!trimmed) return std::unexpected(std::move(trimmed.error()));
    auto newNick = renick(message.fromConnection, *trimmed);
    if (!newNick) return std::unexpected(std::move(newNick.error()));
    response = strf("Nick changed to {}", *newNick);
  } else if (*command == "w") {
    auto target = commandLine.extract();
    if (!target) return std::unexpected(std::move(target.error()));
    if (auto id = m_nicks.ptr(*target)) {
      auto trimmed = commandLine.trim();
      if (!trimmed) return std::unexpected(std::move(trimmed.error()));
      auto sent = whisper(message.fromConnection, *id, *trimmed, {});
      if (!sent) return std::unexpected(std::move(sent.error()));
    } else {
      response = strf("No such nick {}", *target);
    }
  } else if (m_commandHandler) {
    auto res = m_commandHandler(message.fromConnection, *command, commandLine);
    if (!res) return std::unexpected(std::move(res.error()));
    if (auto text = std::get_if<String>(&*res))
      response = std::move(*text);
    else {
      Uuid uuid(m_random);
      m_commandPromises[uuid] = std::make_pair(message.fromConnection, std::move(std::get<RpcPromise<String>>(*res)));
    }
  } else {
    response = strf("No such command {}", *command);
  }
  if (!response.empty()) {
    auto client = m_clients.get(message.fromConnection);
    if (!client) return std::unexpected(std::move(client.error()));
    (*client)->pendingMessages.append({MessageContext(MessageContext::CommandResult), ServerConnectionId, ServerNick, response});
  }
  return true;
}

std::expected<void, StarException> ChatProcessor::updatePromises() {
  RecursiveMutexLocker locker(m_mutex);
  for (auto const& uuid : m_commandPromises.keys()) {
    auto& entry = m_commandPromises[uuid];
    auto finished = entry.second.finished();
    if (!finished) return std::unexpected(std::move(finished.error()));
    if (*finished) {
      String response;
      auto succeeded = entry.second.succeeded();
      if (!succeeded) return std::unexpected(std::move(succeeded.error()));
      if (*succeeded) {
        auto value = entry.second.result();
        if (!value) return std::unexpected(std::move(value.error()));
        if (!*value) return std::unexpected(RpcPromiseException("Completed command has no result"));
        response = **value;
      } else {
        auto error = entry.second.error();
        if (!error) return std::unexpected(std::move(error.error()));
        if (!*error) return std::unexpected(RpcPromiseException("Failed command has no error"));
        response = strf("Error: {}", **error);
      }
      if (!response.empty()) {
        auto client = m_clients.get(entry.first);
        if (!client) return std::unexpected(std::move(client.error()));
        (*client)->pendingMessages.append({MessageContext(MessageContext::CommandResult), ServerConnectionId, ServerNick, response});
      }
      m_commandPromises.remove(uuid);
    }
  }
  return {};
}

}
