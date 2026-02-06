#include "StarHttpClient.hpp"
#include "StarFormat.hpp"
#include "StarWorkerPool.hpp"
#include "StarThread.hpp"

#include <curl/curl.h>

import std;

namespace Star {

// Global curl initialization
struct CurlGlobal {
  CurlGlobal() { curl_global_init(CURL_GLOBAL_ALL); }
  ~CurlGlobal() { curl_global_cleanup(); }
} s_curlGlobal;

class CurlMultiManager : public Thread {
public:
  static auto instance() -> CurlMultiManager& {
    static CurlMultiManager manager;
    return manager;
  }

  void addRequest(CURL* easy, std::function<void(CURLcode, CURL*)> callback) {
    MutexLocker locker(m_mutex);
    m_pendingAdditions.push_back({easy, std::move(callback)});
    m_condition.broadcast();
    if (!isRunning())
      start();
  }

  CurlMultiManager() : Thread("CurlMultiManager") {
    m_multi = curl_multi_init();
    m_stop = false;
  }

  ~CurlMultiManager() override {
    m_stop = true;
    m_condition.broadcast();
    join();
    if (m_multi)
      curl_multi_cleanup(m_multi);
  }

protected:
  void run() override {
    while (!m_stop) {
      {
        MutexLocker locker(m_mutex);
        for (auto& p : m_pendingAdditions) {
          curl_multi_add_handle(m_multi, p.easy);
          m_callbacks[p.easy] = std::move(p.callback);
        }
        m_pendingAdditions.clear();
      }

      int running;
      CURLMcode mc = curl_multi_perform(m_multi, &running);
      if (mc != CURLM_OK) {
        // Handle error?
      }

      int msgs_in_queue;
      CURLMsg* msg;
      while ((msg = curl_multi_info_read(m_multi, &msgs_in_queue))) {
        if (msg->msg == CURLMSG_DONE) {
          CURL* easy = msg->easy_handle;
          CURLcode res = msg->data.result;
          auto it = m_callbacks.find(easy);
          if (it != m_callbacks.end()) {
            auto cb = std::move(it->second);
            m_callbacks.erase(it);
            curl_multi_remove_handle(m_multi, easy);
            cb(res, easy);
          }
        }
      }

      if (running == 0) {
        MutexLocker locker(m_mutex);
        if (m_pendingAdditions.empty()) {
          m_condition.wait(m_mutex, 100);
          continue;
        }
      }

      curl_multi_wait(m_multi, nullptr, 0, 10, nullptr);
    }
  }

private:
  CURLM* m_multi = nullptr;
  std::atomic<bool> m_stop;
  Mutex m_mutex;
  ConditionVariable m_condition;
  struct Pending { CURL* easy; std::function<void(CURLcode, CURL*)> callback; };
  List<Pending> m_pendingAdditions;
  std::unordered_map<CURL*, std::function<void(CURLcode, CURL*)>> m_callbacks;
};

static auto curlWriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
  auto* response = static_cast<HttpResponse*>(userdata);
  response->body.append(ptr, size * nmemb);
  return size * nmemb;
}

static auto curlHeaderCallback(char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
  auto* response = static_cast<HttpResponse*>(userdata);
  String line(ptr, size * nmemb);
  size_t colon = line.find(":");
  if (colon != std::numeric_limits<size_t>::max()) {
    String name = line.substr(0, colon).trim();
    String value = line.substr(colon + 1).trim();
    response->headers[name] = value;
  }
  return size * nmemb;
}

static auto setupEasyHandle(CURL* easy, HttpRequest const& req, HttpResponse* response, curl_slist** headers) {
  curl_easy_setopt(easy, CURLOPT_URL, Star::String(req.url).utf8Ptr());
  curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, curlWriteCallback);
  curl_easy_setopt(easy, CURLOPT_WRITEDATA, response);
  curl_easy_setopt(easy, CURLOPT_HEADERFUNCTION, curlHeaderCallback);
  curl_easy_setopt(easy, CURLOPT_HEADERDATA, response);
  curl_easy_setopt(easy, CURLOPT_TIMEOUT, (long)req.timeout);
  curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, 1L);

  for (auto const& [name, value] : req.headers) {
    *headers = curl_slist_append(*headers, Star::String(strf("{}: {}", name, value)).utf8Ptr());
  }
  if (*headers)
    curl_easy_setopt(easy, CURLOPT_HTTPHEADER, *headers);

  if (req.method == "POST") {
    curl_easy_setopt(easy, CURLOPT_POST, 1L);
    curl_easy_setopt(easy, CURLOPT_POSTFIELDS, req.body.utf8Ptr());
    curl_easy_setopt(easy, CURLOPT_POSTFIELDSIZE, (long)req.body.utf8Size());
  } else if (req.method == "PUT") {
    curl_easy_setopt(easy, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(easy, CURLOPT_POSTFIELDS, Star::String(req.body).utf8Ptr());
    curl_easy_setopt(easy, CURLOPT_POSTFIELDSIZE, (long)Star::String(req.body).utf8Size());
  } else if (req.method == "DELETE") {
    curl_easy_setopt(easy, CURLOPT_CUSTOMREQUEST, "DELETE");
  } else if (req.method == "PATCH") {
    curl_easy_setopt(easy, CURLOPT_CUSTOMREQUEST, "PATCH");
    curl_easy_setopt(easy, CURLOPT_POSTFIELDS, Star::String(req.body).utf8Ptr());
    curl_easy_setopt(easy, CURLOPT_POSTFIELDSIZE, (long)Star::String(req.body).utf8Size());
  } else if (req.method != "GET") {
    curl_easy_setopt(easy, CURLOPT_CUSTOMREQUEST, Star::String(req.method).utf8Ptr());
  }
}

void HttpClient::Task::await_suspend(std::coroutine_handle<> h) {
  handle.promise().continuation = h;
  handle.resume();
}

auto HttpClient::Task::await_resume() -> HttpResponse {
  if (handle.promise().exception)
    std::rethrow_exception(handle.promise().exception);
  return std::move(handle.promise().response);
}

HttpClient::HttpClient() = default;
HttpClient::~HttpClient() = default;

auto HttpClient::workerPool() -> WorkerPool& {
  static WorkerPool pool("HttpClient", 4);
  return pool;
}

auto HttpClient::request(HttpRequest req) -> Task {
  struct Awaiter {
    explicit Awaiter(HttpRequest request) : request(std::move(request)) {}
    HttpRequest request;
    HttpResponse response = {};
    curl_slist* headers = nullptr;

    [[nodiscard]] auto await_ready() const noexcept -> bool { return false; }
    void await_suspend(std::coroutine_handle<> h) {
      CURL* easy = curl_easy_init();
      setupEasyHandle(easy, request, &response, &headers);
      CurlMultiManager::instance().addRequest(easy, [this, h, easy](CURLcode res, CURL*) -> void {
        if (res != CURLE_OK) {
          response.error = strf("CURL error: {}", curl_easy_strerror(res));
        } else {
          long code;
          curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &code);
          response.statusCode = (int)code;
        }
        if (headers) curl_slist_free_all(headers);
        curl_easy_cleanup(easy);
        h.resume();
      });
    }
    auto await_resume() -> HttpResponse { return std::move(response); }
  };

  co_return co_await Awaiter{std::move(req)};
}

auto HttpClient::get(String const& url, StringMap<String> const& headers) -> Task {
  return request({.method="GET", .url=url, .headers=headers, .body=""});
}

auto HttpClient::post(String const& url, String const& body, StringMap<String> const& headers) -> Task {
  return request({.method="POST", .url=url, .headers=headers, .body=body});
}

auto HttpClient::put(String const& url, String const& body, StringMap<String> const& headers) -> Task {
  return request({.method="PUT", .url=url, .headers=headers, .body=body});
}

auto HttpClient::delete_(String const& url, StringMap<String> const& headers) -> Task {
  return request({.method="DELETE", .url=url, .headers=headers, .body=""});
}

auto HttpClient::patch(String const& url, String const& body, StringMap<String> const& headers) -> Task {
  return request({.method="PATCH", .url=url, .headers=headers, .body=body});
}

static auto performRequestSync(HttpRequest const& req) -> HttpResponse {
  HttpResponse response;
  CURL* easy = curl_easy_init();
  curl_slist* headers = nullptr;
  setupEasyHandle(easy, req, &response, &headers);

  CURLcode res = curl_easy_perform(easy);
  if (res != CURLE_OK) {
    response.error = strf("CURL error: {}", curl_easy_strerror(res));
  } else {
    long code;
    curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &code);
    response.statusCode = (int)code;
  }

  if (headers) curl_slist_free_all(headers);
  curl_easy_cleanup(easy);
  return response;
}

auto HttpClient::requestAsync(HttpRequest const& request) -> WorkerPoolPromise<HttpResponse> {
  return workerPool().addProducer<HttpResponse>([request]() -> HttpResponse {
    return performRequestSync(request);
  });
}

auto HttpClient::getAsync(String const& url, StringMap<String> const& headers) -> WorkerPoolPromise<HttpResponse> {
  return requestAsync({.method="GET", .url=url, .headers=headers, .body=""});
}

auto HttpClient::postAsync(String const& url, String const& body, StringMap<String> const& headers) -> WorkerPoolPromise<HttpResponse> {
  return requestAsync({.method="POST", .url=url, .headers=headers, .body=body});
}

auto HttpClient::putAsync(String const& url, String const& body, StringMap<String> const& headers) -> WorkerPoolPromise<HttpResponse> {
  return requestAsync({.method="PUT", .url=url, .headers=headers, .body=body});
}

auto HttpClient::deleteAsync(String const& url, StringMap<String> const& headers) -> WorkerPoolPromise<HttpResponse> {
  return requestAsync({.method="DELETE", .url=url, .headers=headers, .body=""});
}

auto HttpClient::patchAsync(String const& url, String const& body, StringMap<String> const& headers) -> WorkerPoolPromise<HttpResponse> {
  return requestAsync({.method="PATCH", .url=url, .headers=headers, .body=body});
}

}
