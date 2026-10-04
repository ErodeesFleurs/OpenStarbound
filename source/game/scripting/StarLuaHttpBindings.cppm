module;

#include <functional>

namespace Star {
class LuaCallbacks;
class String;
}

export module star.lua_http_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeHttpCallbacks(bool enabled);

  using HttpTrustRequestCallback = std::function<void(String const& domain)>;

  void setHttpTrustRequestCallback(HttpTrustRequestCallback callback);
  void clearHttpTrustRequestCallback();
  void handleHttpTrustReply(String const& domain, bool allowed);
}
