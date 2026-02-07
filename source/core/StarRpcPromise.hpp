#pragma once

#include "StarException.hpp"
#include "StarString.hpp"

import std;

namespace Star {

using RpcPromiseException = ExceptionDerived<"RpcPromiseException">;

// The other side of an RpcPromise, can be used to either fulfill or fail a
// paired promise.  Call either fulfill or fail function exactly once, any
// further invocations will result in an exception.
template <typename Result, typename Error = String>
class RpcPromiseKeeper {
public:
  void fulfill(Result result);
  void fail(Error error);

private:
  template <typename ResultT, typename ErrorT>
  friend class RpcPromise;

  std::function<void(Result)> m_fulfill;
  std::function<void(Error)> m_fail;
};

// A generic promise for the result of a remote procedure call.  It has
// reference semantics and is implicitly shared, but is not thread safe.
template <typename Result, typename Error = String>
class RpcPromise {
public:
  static auto createPair() -> std::pair<RpcPromise, RpcPromiseKeeper<Result, Error>>;
  static auto createFulfilled(Result result) -> RpcPromise;
  static auto createFailed(Error error) -> RpcPromise;

  // Has the respoonse either failed or succeeded?
  [[nodiscard]] auto finished() const -> bool;
  // Has the response finished with success?
  [[nodiscard]] auto succeeded() const -> bool;
  // Has the response finished with failure?
  [[nodiscard]] auto failed() const -> bool;

  // Returns the result of the rpc call on success, nothing on failure or when
  // not yet finished.
  auto result() const -> std::optional<Result> const&;

  // Returns the error of a failed rpc call.  Returns nothing if the call is
  // successful or not yet finished.
  auto error() const -> std::optional<Error> const&;

  // Wrap this RpcPromise into another promise which returns instead the result
  // of this function when fulfilled
  template <typename Function>
  auto wrap(Function function) -> decltype(auto);

private:
  template <typename ResultT, typename ErrorT>
  friend class RpcPromise;

  struct Value {
    std::optional<Result> result;
    std::optional<Error> error;
  };

  RpcPromise() = default;

  std::function<Value const*()> m_getValue;
};

template <typename Result, typename Error>
void RpcPromiseKeeper<Result, Error>::fulfill(Result result) {
  m_fulfill(std::move(result));
}

template <typename Result, typename Error>
void RpcPromiseKeeper<Result, Error>::fail(Error error) {
  m_fail(std::move(error));
}

template <typename Result, typename Error>
auto RpcPromise<Result, Error>::createPair() -> std::pair<RpcPromise<Result, Error>, RpcPromiseKeeper<Result, Error>> {
  auto valuePtr = std::make_shared<Value>();

  RpcPromise promise;
  promise.m_getValue = [valuePtr]() -> auto {
    return valuePtr.get();
  };

  RpcPromiseKeeper<Result, Error> keeper;
  keeper.m_fulfill = [valuePtr](Result result) -> auto {
    if (valuePtr->result || valuePtr->error)
      throw RpcPromiseException("fulfill called on already finished RpcPromise");
    valuePtr->result = std::move(result);
  };
  keeper.m_fail = [valuePtr](Error error) -> auto {
    if (valuePtr->result || valuePtr->error)
      throw RpcPromiseException("fail called on already finished RpcPromise");
    valuePtr->error = std::move(error);
  };

  return {std::move(promise), std::move(keeper)};
}

template <typename Result, typename Error>
auto RpcPromise<Result, Error>::createFulfilled(Result result) -> RpcPromise<Result, Error> {
  auto valuePtr = std::make_shared<Value>();
  valuePtr->result = std::move(result);

  RpcPromise<Result, Error> promise;
  promise.m_getValue = [valuePtr]() -> auto {
    return valuePtr.get();
  };
  return promise;
}

template <typename Result, typename Error>
auto RpcPromise<Result, Error>::createFailed(Error error) -> RpcPromise<Result, Error> {
  auto valuePtr = std::make_shared<Value>();
  valuePtr->error = std::move(error);

  RpcPromise<Result, Error> promise;
  promise.m_getValue = [valuePtr]() -> auto {
    return valuePtr.get();
  };
  return promise;
}

template <typename Result, typename Error>
auto RpcPromise<Result, Error>::finished() const -> bool {
  auto val = m_getValue();
  return val->result || val->error;
}

template <typename Result, typename Error>
auto RpcPromise<Result, Error>::succeeded() const -> bool {
  return m_getValue()->result.has_value();
}

template <typename Result, typename Error>
auto RpcPromise<Result, Error>::failed() const -> bool {
  return m_getValue()->error.has_value();
}

template <typename Result, typename Error>
auto RpcPromise<Result, Error>::result() const -> std::optional<Result> const& {
  return m_getValue()->result;
}

template <typename Result, typename Error>
auto RpcPromise<Result, Error>::error() const -> std::optional<Error> const& {
  return m_getValue()->error;
}

template <typename Result, typename Error>
template <typename Function>
auto RpcPromise<Result, Error>::wrap(Function function) -> decltype(auto) {
  using WrappedPromise = RpcPromise< std::decay_t<decltype(function(std::declval<Result>()))>, Error>;
  WrappedPromise wrappedPromise;
  wrappedPromise.m_getValue = [wrapper = std::move(function), valuePtr = std::make_shared<typename WrappedPromise::Value>(), otherGetValue = m_getValue]() -> auto {
    if (!valuePtr->result && !valuePtr->error) {
      auto otherValue = otherGetValue();
      if (otherValue->result)
        valuePtr->result = wrapper(*otherValue->result);
      else if (otherValue->error)
        valuePtr->error = *otherValue->error;
    }
    return valuePtr.get();
  };
  return wrappedPromise;
}

}
