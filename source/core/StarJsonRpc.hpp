#pragma once

#include "StarByteArray.hpp"
#include "StarException.hpp"
#include "StarJson.hpp"
#include "StarRpcPromise.hpp"

import std;

namespace Star {

using JsonRpcException = ExceptionDerived<"JsonRpcException">;

using JsonRpcRemoteFunction = std::function<Json(Json const&)>;

using JsonRpcHandlers = StringMap<JsonRpcRemoteFunction>;

// Simple interface to just the method invocation part of JsonRpc.
class JsonRpcInterface {
public:
  virtual ~JsonRpcInterface();
  virtual auto invokeRemote(String const& handler, Json const& arguments) -> RpcPromise<Json> = 0;
};

// Simple class to handle remote methods based on Json types.  Does not
// handle any of the network details, simply turns rpc calls into ByteArray
// messages to be sent and received.
class JsonRpc : public JsonRpcInterface {
public:
  JsonRpc();

  void registerHandler(String const& handler, JsonRpcRemoteFunction func);
  void registerHandlers(JsonRpcHandlers const& handlers);

  void removeHandler(String const& handler);
  void clearHandlers();

  auto invokeRemote(String const& handler, Json const& arguments) -> RpcPromise<Json> override;

  [[nodiscard]] auto sendPending() const -> bool;
  auto send() -> ByteArray;
  void receive(ByteArray const& inbuffer);

private:
  JsonRpcHandlers m_handlers;
  Map<std::uint64_t, RpcPromiseKeeper<Json>> m_pendingResponse;
  List<Json> m_pending;
  std::uint64_t m_requestId;
};

}// namespace Star
