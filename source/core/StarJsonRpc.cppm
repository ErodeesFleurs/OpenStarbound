module;
import std;
import star.string;
import star.list;
import star.map;
import star.format;
import star.json;
import star.byte_array;
import star.rpc_promise;
import star.logging;

namespace Star {

struct JsonRpcExceptionTag {
  static constexpr char const* name() { return "JsonRpcException"; }
};
using JsonRpcException = StarError<JsonRpcExceptionTag, StarException>;

typedef std::function<Json(Json const&)> JsonRpcRemoteFunction;

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
  explicit JsonRpc(Logger& logger);

  void registerHandler(String const& handler, JsonRpcRemoteFunction func);
  void registerHandlers(JsonRpcHandlers const& handlers);

  void removeHandler(String const& handler);
  void clearHandlers();

  RpcPromise<Json> invokeRemote(String const& handler, Json const& arguments) override;

  bool sendPending() const;
  ByteArray send();
  void receive(ByteArray const& inbuffer);

private:
  Logger& m_logger;
  JsonRpcHandlers m_handlers;
  Map<std::uint64_t, RpcPromiseKeeper<Json>> m_pendingResponse;
  List<Json> m_pending;
  std::uint64_t m_requestId;
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
}
