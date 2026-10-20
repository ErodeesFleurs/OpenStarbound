import std;
import star.json;
import star.lua;
import star.thread;
import star.memory;
import star.string;
import star.map;
import star.format;
import star.list;
import star.logging;
import star.rpc_promise;
import star.configuration;
import star.worker_pool;
import star.lua_game_converters;
import star.root;
import star.http_client;

import star.lua_http_bindings;

namespace Star {

struct LuaHttpResponse {
  int statusCode = 0;
  String body;
};

// Userdata polls only its originating session. The promise survives its owner,
// and stop() settles it before that session's Root can be destroyed.
struct LuaHttpRequest {
  std::weak_ptr<LuaHttpSession> session;
  RpcPromise<LuaHttpResponse> promise;

  void poll() const {
    if (auto owner = session.lock())
      owner->poll();
  }
};

namespace {

String extractDomain(String const& url) {
  const std::size_t end = url.find("://");
  if (end == NPos)
    return url;

  const std::size_t domainStart = end + 3;
  const std::size_t pathStart = url.find('/', domainStart);
  const std::size_t portStart = url.find(':', domainStart);

  std::size_t domainEnd = NPos;
  if (pathStart != NPos && portStart != NPos)
    domainEnd = std::min(pathStart, portStart);
  else if (pathStart != NPos)
    domainEnd = pathStart;
  else if (portStart != NPos)
    domainEnd = portStart;

  if (domainEnd == NPos)
    return url.substr(domainStart);
  return url.substr(domainStart, domainEnd - domainStart);
}

}

struct LuaHttpSession::Impl {
  struct PendingHttpRequest {
    HttpRequest httpRequest;
    RpcPromiseKeeper<LuaHttpResponse> rpcKeeper;
    String domain;
  };

  struct AsyncHttpRequest {
    WorkerPoolPromise<HttpResponse> workerPromise;
    RpcPromiseKeeper<LuaHttpResponse> rpcKeeper;
  };

  explicit Impl(Root& root) : root(root) {}

  // Called with mutex held, so stop cannot race a new submission or settlement.
  bool isTrustedDomain(String const& domain) const {
    if (stopped)
      return false;
    auto config = root.configuration();
    if (auto trustedSites = config->getPath("safe.luaHttp.trustedSites").optArray()) {
      for (auto const& site : *trustedSites) {
        if (site.toString() == domain)
          return true;
      }
    }
    return false;
  }

  void executeHttpRequest(HttpRequest const& request, RpcPromiseKeeper<LuaHttpResponse> keeper) {
    try {
      auto workerPromise = httpClient.requestAsync(request);
      asyncRequests.add(nextRequestId++, {std::move(workerPromise), keeper});
    } catch (std::exception const& e) {
      keeper.fail(strf("HTTP request failed: {}", e.what()));
    } catch (...) {
      keeper.fail("HTTP request failed: unknown exception");
    }
  }

  RpcPromise<LuaHttpResponse> request(HttpRequest request) {
    auto [promise, keeper] = RpcPromise<LuaHttpResponse>::createPair();
    auto domain = extractDomain(request.url);

    // Serialize callback removal with notification, but release the state lock
    // before calling UI code: a handler may deliver its trust reply immediately.
    RecursiveMutexLocker callbackLock(callbackMutex);
    MutexLocker lock(mutex);
    if (stopped) {
      keeper.fail("HTTP session stopped");
    } else if (isTrustedDomain(domain)) {
      executeHttpRequest(request, keeper);
    } else {
      pendingRequests.append({std::move(request), keeper, domain});
      auto callback = trustRequestCallback;
      if (callback) {
        lock.unlock();
        callback(domain);
      } else {
        root.logger().warn("HTTP request to untrusted domain '{}' but no trust dialog handler available", domain);
      }
    }
    return promise;
  }

  Root& root;
  HttpClient httpClient;
  RecursiveMutex callbackMutex;
  LuaBindings::HttpTrustRequestCallback trustRequestCallback;
  Mutex mutex;
  List<PendingHttpRequest> pendingRequests;
  HashMap<std::uint64_t, AsyncHttpRequest> asyncRequests;
  std::uint64_t nextRequestId = 1;
  bool stopped = false;
};

LuaHttpSession::LuaHttpSession(Root& root) : m_impl(std::make_unique<Impl>(root)) {}

LuaHttpSession::~LuaHttpSession() {
  stop();
}

void LuaHttpSession::setHttpTrustRequestCallback(LuaBindings::HttpTrustRequestCallback callback) {
  RecursiveMutexLocker callbackLock(m_impl->callbackMutex);
  MutexLocker lock(m_impl->mutex);
  if (!m_impl->stopped)
    m_impl->trustRequestCallback = std::move(callback);
}

void LuaHttpSession::clearHttpTrustRequestCallback() {
  RecursiveMutexLocker callbackLock(m_impl->callbackMutex);
  m_impl->trustRequestCallback = {};
}

void LuaHttpSession::handleHttpTrustReply(String const& domain, bool allowed) {
  MutexLocker lock(m_impl->mutex);
  if (m_impl->stopped)
    return;

  auto& pending = m_impl->pendingRequests;
  auto firstRemoved = std::remove_if(pending.begin(), pending.end(), [&](auto& request) {
    if (request.domain != domain)
      return false;
    if (allowed)
      m_impl->executeHttpRequest(request.httpRequest, request.rpcKeeper);
    else
      request.rpcKeeper.fail(strf("HTTP request to domain '{}' denied by user", domain));
    return true;
  });
  pending.erase(firstRemoved, pending.end());
}

void LuaHttpSession::poll() {
  MutexLocker lock(m_impl->mutex);
  auto& requests = m_impl->asyncRequests;
  for (auto it = requests.begin(); it != requests.end();) {
    auto& request = it->second;
    if (!request.workerPromise.done()) {
      ++it;
      continue;
    }

    try {
      auto& response = request.workerPromise.get();
      if (!response.error.empty())
        request.rpcKeeper.fail(strf("HTTP request failed: {}", response.error));
      else
        request.rpcKeeper.fulfill({response.statusCode, std::move(response.body)});
    } catch (std::exception const& e) {
      request.rpcKeeper.fail(strf("HTTP request exception: {}", e.what()));
    } catch (...) {
      request.rpcKeeper.fail("HTTP request exception: unknown exception");
    }
    it = requests.erase(it);
  }
}

void LuaHttpSession::stop() {
  // Waiting for this lock also waits for any in-flight trust notification, so
  // the caller may safely destroy the UI after stop returns.
  RecursiveMutexLocker callbackLock(m_impl->callbackMutex);
  {
    MutexLocker lock(m_impl->mutex);
    if (!m_impl->stopped) {
      m_impl->stopped = true;
      m_impl->trustRequestCallback = {};
      for (auto& request : m_impl->pendingRequests)
        request.rpcKeeper.fail("HTTP session stopped");
      m_impl->pendingRequests.clear();
      for (auto& [id, request] : m_impl->asyncRequests)
        request.rpcKeeper.fail("HTTP session stopped");
      m_impl->asyncRequests.clear();
    }
  }
  m_impl->httpClient.stop();
}

template <>
struct LuaConverter<LuaHttpResponse> {
  static LuaValue from(LuaEngine& engine, LuaHttpResponse const& response) {
    auto table = engine.createTable();
    table.set("status", response.statusCode);
    table.set("body", response.body);
    return table;
  }

  static std::optional<LuaHttpResponse> to(LuaEngine& /*engine*/, LuaValue const& v) {
    if (auto table = v.ptr<LuaTable>()) {
      LuaHttpResponse response;
      response.statusCode = table->get<std::optional<int>>("status").value_or(0);
      if (table->contains("body"))
        response.body = table->get<String>("body");
      return response;
    }
    return {};
  }
};

template <>
struct LuaUserDataMethods<LuaHttpRequest> {
  static LuaMethods<LuaHttpRequest> make() {
    LuaMethods<LuaHttpRequest> methods;
    methods.registerMethod("finished", [](LuaHttpRequest const& request) {
      request.poll();
      return request.promise.finished();
    });
    methods.registerMethod("succeeded", [](LuaHttpRequest const& request) {
      request.poll();
      return request.promise.succeeded();
    });
    methods.registerMethod("failed", [](LuaHttpRequest const& request) {
      request.poll();
      return request.promise.failed();
    });
    methods.registerMethod("result", [](LuaHttpRequest const& request) {
      request.poll();
      return request.promise.result();
    });
    methods.registerMethod("error", [](LuaHttpRequest const& request) {
      request.poll();
      return request.promise.error();
    });
    return methods;
  }
};

template <>
struct LuaConverter<LuaHttpRequest> : LuaUserDataConverter<LuaHttpRequest> {};

LuaCallbacks LuaBindings::makeHttpCallbacks(std::shared_ptr<LuaHttpSession> const& session, bool enabled) {
  if (!session)
    throw std::invalid_argument("HTTP callbacks require an owning LuaHttpSession");

  LuaCallbacks callbacks;
  std::weak_ptr<LuaHttpSession> weakSession = session;
  callbacks.registerCallback("available", [enabled]() { return enabled; });

  auto requestForMethod = [weakSession, enabled](LuaEngine& engine, String const& method, String const& url, std::optional<LuaTable> const& options) -> LuaHttpRequest {
    if (!enabled)
      return {weakSession, RpcPromise<LuaHttpResponse>::createFailed("luaHttp disabled by configuration")};
    auto owner = weakSession.lock();
    if (!owner)
      return {weakSession, RpcPromise<LuaHttpResponse>::createFailed("HTTP session stopped")};

    try {
      HttpRequest request;
      request.method = method;
      request.url = url;
      if (options) {
        auto const& optTable = *options;
        if (optTable.contains("headers")) {
          auto headersValue = optTable.get("headers");
          if (auto headersTable = headersValue.ptr<LuaTable>()) {
            headersTable->iterate([&](LuaValue key, LuaValue value) {
              if (auto keyStr = key.ptr<LuaString>()) {
                if (auto valStr = value.ptr<LuaString>())
                  request.headers[keyStr->toString()] = valStr->toString();
                else
                  request.headers[keyStr->toString()] = engine.luaTo<String>(value);
              }
            });
          }
        }
        if (optTable.contains("body")) {
          auto bodyValue = optTable.get("body");
          if (auto strValue = bodyValue.ptr<LuaString>())
            request.body = strValue->toString();
          else
            request.body = engine.luaTo<Json>(bodyValue).repr();
        }
        if (optTable.contains("timeout"))
          request.timeout = optTable.get<int>("timeout");
      }
      return {weakSession, owner->m_impl->request(std::move(request))};
    } catch (std::exception const& e) {
      return {weakSession, RpcPromise<LuaHttpResponse>::createFailed(strf("HTTP request failed: {}", e.what()))};
    } catch (...) {
      return {weakSession, RpcPromise<LuaHttpResponse>::createFailed("HTTP request failed: unknown exception")};
    }
  };

  callbacks.registerCallback("createRequest", [requestForMethod](LuaEngine& engine, String const& method, String const& url, std::optional<LuaTable> const& options) {
    return requestForMethod(engine, method, url, options);
  });
  callbacks.registerCallback("get", [requestForMethod](LuaEngine& engine, String const& url, std::optional<LuaTable> const& options) {
    return requestForMethod(engine, "GET", url, options);
  });
  callbacks.registerCallback("post", [requestForMethod](LuaEngine& engine, String const& url, std::optional<LuaTable> const& options) {
    return requestForMethod(engine, "POST", url, options);
  });
  callbacks.registerCallback("put", [requestForMethod](LuaEngine& engine, String const& url, std::optional<LuaTable> const& options) {
    return requestForMethod(engine, "PUT", url, options);
  });
  callbacks.registerCallback("delete", [requestForMethod](LuaEngine& engine, String const& url, std::optional<LuaTable> const& options) {
    return requestForMethod(engine, "DELETE", url, options);
  });
  callbacks.registerCallback("patch", [requestForMethod](LuaEngine& engine, String const& url, std::optional<LuaTable> const& options) {
    return requestForMethod(engine, "PATCH", url, options);
  });
  callbacks.registerCallback("isTrusted", [weakSession](String const& domain) {
    if (auto owner = weakSession.lock()) {
      MutexLocker lock(owner->m_impl->mutex);
      return owner->m_impl->isTrustedDomain(domain);
    }
    return false;
  });
  return callbacks;
}

}
