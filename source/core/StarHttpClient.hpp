#pragma once

#include "StarString.hpp"
#include "StarWorkerPool.hpp"

import std;

namespace Star {

struct HttpRequest {
  String method;
  String url;
  StringMap<String> headers;
  String body;

  int timeout = 30; // 0 - no timeout.
};

struct HttpResponse {
  int statusCode = 0;
  StringMap<String> headers;
  String body;
  String error;
};

class HttpClient {
public:
  struct Task {
    struct promise_type {
      HttpResponse response;
      std::exception_ptr exception;
      std::coroutine_handle<> continuation;

      auto get_return_object() { return Task{std::coroutine_handle<promise_type>::from_promise(*this)}; }
      auto initial_suspend() noexcept { return std::suspend_always{}; }
      auto final_suspend() noexcept {
        struct awaiter {
          auto await_ready() noexcept -> bool { return false; }
          auto await_suspend(std::coroutine_handle<promise_type> h) noexcept {
            return h.promise().continuation ? h.promise().continuation : std::noop_coroutine();
          }
          void await_resume() noexcept {}
        };
        return awaiter{};
      }
      void return_value(HttpResponse res) { response = std::move(res); }
      void unhandled_exception() { exception = std::current_exception(); }
    };

    std::coroutine_handle<promise_type> handle;

    Task(std::coroutine_handle<promise_type> h) : handle(h) {}
    Task(Task&& other) noexcept : handle(std::exchange(other.handle, nullptr)) {}
    ~Task() { if (handle) handle.destroy(); }

    [[nodiscard]] auto await_ready() const noexcept -> bool { return false; }
    void await_suspend(std::coroutine_handle<> h);
    auto await_resume() -> HttpResponse;
  };

  HttpClient();
  ~HttpClient();

  static auto request(HttpRequest request) -> Task;

  static auto get(String const& url, StringMap<String> const& headers = {}) -> Task;
  static auto post(String const& url, String const& body, StringMap<String> const& headers = {}) -> Task;
  static auto put(String const& url, String const& body, StringMap<String> const& headers = {}) -> Task;
  static auto delete_(String const& url, StringMap<String> const& headers = {}) -> Task;
  static auto patch(String const& url, String const& body, StringMap<String> const& headers = {}) -> Task;

  static auto requestAsync(HttpRequest const& request) -> WorkerPoolPromise<HttpResponse>;
  static auto getAsync(String const& url, StringMap<String> const& headers = {}) -> WorkerPoolPromise<HttpResponse>;
  static auto postAsync(String const& url, String const& body, StringMap<String> const& headers = {}) -> WorkerPoolPromise<HttpResponse>;
  static auto putAsync(String const& url, String const& body, StringMap<String> const& headers = {}) -> WorkerPoolPromise<HttpResponse>;
  static auto deleteAsync(String const& url, StringMap<String> const& headers = {}) -> WorkerPoolPromise<HttpResponse>;
  static auto patchAsync(String const& url, String const& body, StringMap<String> const& headers = {}) -> WorkerPoolPromise<HttpResponse>;

private:
  static auto workerPool() -> WorkerPool&;
};

}
