module;

#include "StarJson.hpp"
import star.byte_array;
import star.rpc_promise;

namespace Star {

STAR_CLASS(JsonRpcInterface);
STAR_CLASS(JsonRpc);

struct JsonRpcExceptionTag {
  static constexpr char const* name() { return "JsonRpcException"; }
};
using JsonRpcException = StarError<JsonRpcExceptionTag, StarException>;

typedef function<Json(Json const&)> JsonRpcRemoteFunction;

typedef StringMap<JsonRpcRemoteFunction> JsonRpcHandlers;

// Simple interface to just the method invocation part of JsonRpc.
class JsonRpcInterface {
public:
  virtual ~JsonRpcInterface();
  virtual RpcPromise<Json> invokeRemote(String const& handler, Json const& arguments) = 0;
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

  RpcPromise<Json> invokeRemote(String const& handler, Json const& arguments) override;

  bool sendPending() const;
  ByteArray send();
  void receive(ByteArray const& inbuffer);

private:
  JsonRpcHandlers m_handlers;
  Map<uint64_t, RpcPromiseKeeper<Json>> m_pendingResponse;
  List<Json> m_pending;
  uint64_t m_requestId;
};

}

export module star.json_rpc;

export namespace Star {
using ::Star::JsonRpcInterface;
using ::Star::JsonRpc;
using ::Star::JsonRpcExceptionTag;
using ::Star::JsonRpcException;
using ::Star::JsonRpcRemoteFunction;
using ::Star::JsonRpcHandlers;
using ::Star::JsonRpcInterfacePtr;
using ::Star::JsonRpcInterfaceConstPtr;
using ::Star::JsonRpcInterfaceWeakPtr;
using ::Star::JsonRpcInterfaceConstWeakPtr;
using ::Star::JsonRpcInterfaceUPtr;
using ::Star::JsonRpcInterfaceConstUPtr;
using ::Star::JsonRpcPtr;
using ::Star::JsonRpcConstPtr;
using ::Star::JsonRpcWeakPtr;
using ::Star::JsonRpcConstWeakPtr;
using ::Star::JsonRpcUPtr;
using ::Star::JsonRpcConstUPtr;
}
