#pragma once

#include "StarException.hpp"
#include "StarString.hpp"
#include "StarThread.hpp"

import std;

// A thread-safe version of RpcPromise.
// This is just a copy-paste with a Mutex tacked on. I don't like that, but it's 11 PM.

namespace Star {

using RpcThreadPromiseException = ExceptionDerived<"RpcThreadPromiseException">;

template <typename Result, typename Error = String>
class RpcThreadPromiseKeeper {
public:
  void fulfill(Result result);
  void fail(Error error);

private:
  template <typename ResultT, typename ErrorT>
  friend class RpcThreadPromise;

  std::function<void(Result)> m_fulfill;
  std::function<void(Error)> m_fail;
};

template <typename Result, typename Error = String>
class RpcThreadPromise {
public:
  static auto createPair() -> std::pair<RpcThreadPromise, RpcThreadPromiseKeeper<Result, Error>>;
  static auto createFulfilled(Result result) -> RpcThreadPromise;
  static auto createFailed(Error error) -> RpcThreadPromise;

  // Has the respoonse either failed or succeeded?
  [[nodiscard]] auto finished() const -> bool;
  // Has the response finished with success?
  [[nodiscard]] auto succeeded() const -> bool;
  // Has the response finished with failure?
  [[nodiscard]] auto failed() const -> bool;

  // Returns the result of the rpc call on success, nothing on failure or when
  // not yet finished.
  auto result() const -> std::optional<Result>;

  // Returns the error of a failed rpc call.  Returns nothing if the call is
  // successful or not yet finished.
  auto error() const -> std::optional<Error>;

private:
  template <typename ResultT, typename ErrorT>
  friend class RpcThreadPromise;

  struct Value {
    Mutex mutex;

    std::optional<Result> result;
    std::optional<Error> error;
  };

  RpcThreadPromise() = default;

  std::function<Value*()> m_getValue;
};

template <typename Result, typename Error>
void RpcThreadPromiseKeeper<Result, Error>::fulfill(Result result) {
  m_fulfill(std::move(result));
}

template <typename Result, typename Error>
void RpcThreadPromiseKeeper<Result, Error>::fail(Error error) {
  m_fail(std::move(error));
}

template <typename Result, typename Error>
auto RpcThreadPromise<Result, Error>::createPair() -> std::pair<RpcThreadPromise<Result, Error>, RpcThreadPromiseKeeper<Result, Error>> {
  auto valuePtr = make_shared<Value>();

  RpcThreadPromise promise;
  promise.m_getValue = [valuePtr]() -> auto {
    return valuePtr.get();
  };

  RpcThreadPromiseKeeper<Result, Error> keeper;
  keeper.m_fulfill = [valuePtr](Result result) -> auto {
    MutexLocker lock(valuePtr->mutex);
    if (valuePtr->result || valuePtr->error)
      throw RpcThreadPromiseException("fulfill called on already finished RpcThreadPromise");
    valuePtr->result = std::move(result);
  };
  keeper.m_fail = [valuePtr](Error error) -> auto {
    MutexLocker lock(valuePtr->mutex);
    if (valuePtr->result || valuePtr->error)
      throw RpcThreadPromiseException("fail called on already finished RpcThreadPromise");
    valuePtr->error = std::move(error);
  };

  return {std::move(promise), std::move(keeper)};
}

template <typename Result, typename Error>
auto RpcThreadPromise<Result, Error>::createFulfilled(Result result) -> RpcThreadPromise<Result, Error> {
  auto valuePtr = make_shared<Value>();
  valuePtr->result = std::move(result);

  RpcThreadPromise<Result, Error> promise;
  promise.m_getValue = [valuePtr]() -> auto {
    return valuePtr.get();
  };
  return promise;
}

template <typename Result, typename Error>
auto RpcThreadPromise<Result, Error>::createFailed(Error error) -> RpcThreadPromise<Result, Error> {
  auto valuePtr = make_shared<Value>();
  valuePtr->error = std::move(error);

  RpcThreadPromise<Result, Error> promise;
  promise.m_getValue = [valuePtr]() -> auto {
    return valuePtr.get();
  };
  return promise;
}

template <typename Result, typename Error>
auto RpcThreadPromise<Result, Error>::finished() const -> bool {
  auto val = m_getValue();
  MutexLocker lock(val->mutex);
  return val->result || val->error;
}

template <typename Result, typename Error>
auto RpcThreadPromise<Result, Error>::succeeded() const -> bool {
  auto val = m_getValue();
  MutexLocker lock(val->mutex);
  return val->result.isValid();
}

template <typename Result, typename Error>
auto RpcThreadPromise<Result, Error>::failed() const -> bool {
  auto val = m_getValue();
  MutexLocker lock(val->mutex);
  return val->error.isValid();
}

template <typename Result, typename Error>
auto RpcThreadPromise<Result, Error>::result() const -> std::optional<Result> {
  auto val = m_getValue();
  MutexLocker lock(val->mutex);
  return val->result;
}

template <typename Result, typename Error>
auto RpcThreadPromise<Result, Error>::error() const -> std::optional<Error> {
  auto val = m_getValue();
  MutexLocker lock(val->mutex);
  return val->error;
}

}// namespace Star
