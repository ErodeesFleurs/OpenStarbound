module;
import std;
import star.string;
import star.list;
import star.map;
import star.format;
import star.json;
import star.byte_array;
import star.rpc_promise;

namespace Star {

class JsonRpcInterface;
using JsonRpcInterfacePtr = std::shared_ptr<JsonRpcInterface>;
using JsonRpcInterfaceConstPtr = std::shared_ptr<const JsonRpcInterface>;
using JsonRpcInterfaceWeakPtr = std::weak_ptr<JsonRpcInterface>;
using JsonRpcInterfaceConstWeakPtr = std::weak_ptr<const JsonRpcInterface>;
using JsonRpcInterfaceUPtr = std::unique_ptr<JsonRpcInterface>;
using JsonRpcInterfaceConstUPtr = std::unique_ptr<const JsonRpcInterface>;
class JsonRpc;
using JsonRpcPtr = std::shared_ptr<JsonRpc>;
using JsonRpcConstPtr = std::shared_ptr<const JsonRpc>;
using JsonRpcWeakPtr = std::weak_ptr<JsonRpc>;
using JsonRpcConstWeakPtr = std::weak_ptr<const JsonRpc>;
using JsonRpcUPtr = std::unique_ptr<JsonRpc>;
using JsonRpcConstUPtr = std::unique_ptr<const JsonRpc>;

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
