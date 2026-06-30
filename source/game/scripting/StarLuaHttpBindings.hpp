#pragma once

#include "StarLua.hpp"
#include "StarConfiguration.hpp"

namespace Star::LuaBindings {

// Creates the http callback table for Lua. All callbacks return RpcPromise values
// so scripts can wait on asynchronous-style results even though the requests
// are currently resolved synchronously. The callbacks are only usable when
// safe.luaHttp.eabled is truq
LuaCallbacks makeHttpCallbacks(bool enabled, ConfigurationPtr configuration);

using HttpTrustRequestCallback = std::function<void(String const& domain)>;

void setHttpTrustRequestCallback(HttpTrustRequestCallback callback);

void clearHttpTrustRequestCallback();

void handleHttpTrustReply(String const& domain, bool allowed);
}

