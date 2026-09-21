module;

#include "StarString.hpp"
#include "StarMap.hpp"
#include "StarMaybe.hpp"
#include "StarWorkerPool.hpp"

#include "StarLogging.hpp"
#include "StarFormat.hpp"
#include <cpr/cpr.h>

export module star.http_client;

export namespace Star {

struct HttpRequest {
  String method;
  String url;
  StringMap<String> headers;
  String body;

  int timeout = 30; // 0 - no timeout. NOT RECOMENDED YOU HEAR ME??
};

struct HttpResponse {
  int statusCode = 0;
  StringMap<String> headers;
  String body;
  String error;
};

class HttpClient {
public:
  HttpClient();
  ~HttpClient();

  static WorkerPoolPromise<HttpResponse> requestAsync(HttpRequest const& request);

  static WorkerPoolPromise<HttpResponse> getAsync(String const& url, StringMap<String> const& headers = {});
  static WorkerPoolPromise<HttpResponse> postAsync(String const& url, String const& body, StringMap<String> const& headers = {});
  static WorkerPoolPromise<HttpResponse> putAsync(String const& url, String const& body, StringMap<String> const& headers = {});
  static WorkerPoolPromise<HttpResponse> deleteAsync(String const& url, StringMap<String> const& headers = {});
  static WorkerPoolPromise<HttpResponse> patchAsync(String const& url, String const& body, StringMap<String> const& headers = {});

private:
  static WorkerPool& workerPool();
};

}

namespace Star {

WorkerPool& HttpClient::workerPool() {
  static WorkerPool pool("HttpClient", 4);
  return pool;
}

HttpClient::HttpClient() = default;

HttpClient::~HttpClient() = default;

static HttpResponse performRequest(HttpRequest const& req) {
  HttpResponse response;

  try {
    // CPR has in own header object
    cpr::Header cprHeaders;
    for (const auto& [fst, snd] : req.headers) {
      cprHeaders[fst.utf8()] = snd.utf8();
    }

    cpr::Timeout timeout{req.timeout * 1000}; // ms

    cpr::Response r;

    if (req.method == "GET") {
      r = cpr::Get(
        cpr::Url{req.url.utf8()},
        cprHeaders,
        timeout
      );
    } else if (req.method == "POST") {
      r = cpr::Post(
        cpr::Url{req.url.utf8()},
        cprHeaders,
        cpr::Body{req.body.utf8()},
        timeout
      );
    } else if (req.method == "PUT") {
      r = cpr::Put(
        cpr::Url{req.url.utf8()},
        cprHeaders,
        cpr::Body{req.body.utf8()},
        timeout
      );
    } else if (req.method == "DELETE") {
      r = cpr::Delete(
        cpr::Url{req.url.utf8()},
        cprHeaders,
        timeout
      );
    } else if (req.method == "PATCH") {
      r = cpr::Patch(
        cpr::Url{req.url.utf8()},
        cprHeaders,
        cpr::Body{req.body.utf8()},
        timeout
      );
    } else {
      response.error = strf("Unsupported HTTP method: {}", req.method);
      return response;
    }

    if (r.error.code != cpr::ErrorCode::OK) {
      response.error = strf("HTTP error: {}", r.error.message);
      return response;
    }

    response.statusCode = static_cast<int>(r.status_code);
    response.body = String(r.text);

    for (auto const& pair : r.header) {
      response.headers[String(pair.first)] = String(pair.second);
    }

  } catch (std::exception const& e) {
    response.error = strf("HTTP exception: {}", e.what());
  }

  return response;
}

WorkerPoolPromise<HttpResponse> HttpClient::requestAsync(HttpRequest const& request) {
  return workerPool().addProducer<HttpResponse>([request]() {
    return performRequest(request);
  });
}

}
