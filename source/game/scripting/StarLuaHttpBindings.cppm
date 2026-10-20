module;

import std;

namespace Star {
class LuaCallbacks;
class Root;
class String;
class LuaHttpSession;

namespace LuaBindings {
  using HttpTrustRequestCallback = std::function<void(String const& domain)>;
  LuaCallbacks makeHttpCallbacks(std::shared_ptr<LuaHttpSession> const& session, bool enabled);
}

// One script/client session owns its trust policy and blocking HTTP executor.
// The Root must outlive the session. Stop before destroying its GUI callback.
class LuaHttpSession {
public:
  explicit LuaHttpSession(Root& root);
  ~LuaHttpSession();

  LuaHttpSession(LuaHttpSession const&) = delete;
  LuaHttpSession& operator=(LuaHttpSession const&) = delete;
  LuaHttpSession(LuaHttpSession&&) = delete;
  LuaHttpSession& operator=(LuaHttpSession&&) = delete;

  void setHttpTrustRequestCallback(LuaBindings::HttpTrustRequestCallback callback);
  void clearHttpTrustRequestCallback();
  void handleHttpTrustReply(String const& domain, bool allowed);

  // Fail outstanding Lua promises, cancel queued work, and join active requests.
  // Late trust replies and userdata polling stay bound to this stopped session.
  void stop();
  void poll();

private:
  friend LuaCallbacks LuaBindings::makeHttpCallbacks(std::shared_ptr<LuaHttpSession> const& session, bool enabled);

  struct Impl;
  std::unique_ptr<Impl> m_impl;
};

}

export module star.lua_http_bindings;

export namespace Star {
  using ::Star::LuaHttpSession;
}

export namespace Star::LuaBindings {
  using ::Star::LuaBindings::HttpTrustRequestCallback;
  using ::Star::LuaBindings::makeHttpCallbacks;
}
