#pragma once

#include "StarEither.hpp"
#include "StarString.hpp"
#include "StarThread.hpp"

namespace Star {

struct RpcPromiseExceptionTag { static constexpr char const* typeName = "RpcPromiseException"; };
using RpcPromiseException = TypedException<StarException, RpcPromiseExceptionTag>;

// The other side of an RpcPromise, can be used to either fulfill or fail a
// paired promise.  Call either fulfill or fail function exactly once, any
// further invocations will result in an exception.
template <typename Result, typename Error = String, bool ThreadSafe = false>
class RpcPromiseKeeper {
public:
  void fulfill(Result result);
  void fail(Error error);

private:
  template <typename ResultT, typename ErrorT, bool ThreadSafeT>
  friend class RpcPromise;

  function<void(Result)> m_fulfill;
  function<void(Error)> m_fail;
};

// A generic promise for the result of a remote procedure call.  It has
// reference semantics and is implicitly shared, and can optionally be made
// thread safe with the ThreadSafe template parameter.
template <typename Result, typename Error = String, bool ThreadSafe = false>
class RpcPromise {
public:
  [[nodiscard]] static pair<RpcPromise, RpcPromiseKeeper<Result, Error, ThreadSafe>> createPair();
  [[nodiscard]] static RpcPromise createFulfilled(Result result);
  [[nodiscard]] static RpcPromise createFailed(Error error);

  // Has the response either failed or succeeded?
  [[nodiscard]] bool finished() const;
  // Has the response finished with success?
  [[nodiscard]] bool succeeded() const;
  // Has the response finished with failure?
  [[nodiscard]] bool failed() const;

  // Returns the result of the rpc call on success, nothing on failure or when
  // not yet finished.  When ThreadSafe, returns by value; otherwise by const ref.
  [[nodiscard]] auto result() const -> std::conditional_t<ThreadSafe, Maybe<Result>, Maybe<Result> const&>;

  // Returns the error of a failed rpc call.  Returns nothing if the call is
  // successful or not yet finished.  When ThreadSafe, returns by value; otherwise by const ref.
  [[nodiscard]] auto error() const -> std::conditional_t<ThreadSafe, Maybe<Error>, Maybe<Error> const&>;

  // Wrap this RpcPromise into another promise which returns instead the result
  // of this function when fulfilled.  Only available when ThreadSafe is false.
  template <typename Function> requires (!ThreadSafe)
  [[nodiscard]] decltype(auto) wrap(Function function);

private:
  template <typename ResultT, typename ErrorT, bool ThreadSafeT>
  friend class RpcPromise;

  struct Value {
    Mutex mutex;
    Maybe<Result> result;
    Maybe<Error> error;
  };

  RpcPromise() = default;

  function<Value*()> m_getValue;
};

template <typename Result, typename Error, bool ThreadSafe>
void RpcPromiseKeeper<Result, Error, ThreadSafe>::fulfill(Result result) {
  m_fulfill(std::move(result));
}

template <typename Result, typename Error, bool ThreadSafe>
void RpcPromiseKeeper<Result, Error, ThreadSafe>::fail(Error error) {
  m_fail(std::move(error));
}

template <typename Result, typename Error, bool ThreadSafe>
[[nodiscard]] pair<RpcPromise<Result, Error, ThreadSafe>, RpcPromiseKeeper<Result, Error, ThreadSafe>> RpcPromise<Result, Error, ThreadSafe>::createPair() {
  auto valuePtr = std::make_shared<Value>();

  RpcPromise promise;
  promise.m_getValue = [valuePtr]() -> Value* {
    return valuePtr.get();
  };

  RpcPromiseKeeper<Result, Error, ThreadSafe> keeper;
  keeper.m_fulfill = [valuePtr](Result result) {
    if (valuePtr->result || valuePtr->error)
      throw RpcPromiseException("fulfill called on already finished RpcPromise");
    valuePtr->result = std::move(result);
  };
  keeper.m_fail = [valuePtr](Error error) {
    if (valuePtr->result || valuePtr->error)
      throw RpcPromiseException("fail called on already finished RpcPromise");
    valuePtr->error = std::move(error);
  };

  return {std::move(promise), std::move(keeper)};
}

template <typename Result, typename Error, bool ThreadSafe>
[[nodiscard]] RpcPromise<Result, Error, ThreadSafe> RpcPromise<Result, Error, ThreadSafe>::createFulfilled(Result result) {
  auto valuePtr = std::make_shared<Value>();
  valuePtr->result = std::move(result);

  RpcPromise promise;
  promise.m_getValue = [valuePtr]() -> Value* {
    return valuePtr.get();
  };
  return promise;
}

template <typename Result, typename Error, bool ThreadSafe>
[[nodiscard]] RpcPromise<Result, Error, ThreadSafe> RpcPromise<Result, Error, ThreadSafe>::createFailed(Error error) {
  auto valuePtr = std::make_shared<Value>();
  valuePtr->error = std::move(error);

  RpcPromise promise;
  promise.m_getValue = [valuePtr]() -> Value* {
    return valuePtr.get();
  };
  return promise;
}

template <typename Result, typename Error, bool ThreadSafe>
[[nodiscard]] bool RpcPromise<Result, Error, ThreadSafe>::finished() const {
  auto val = m_getValue();
  if constexpr (ThreadSafe) {
    MutexLocker lock(val->mutex);
    return val->result || val->error;
  } else {
    return val->result || val->error;
  }
}

template <typename Result, typename Error, bool ThreadSafe>
[[nodiscard]] bool RpcPromise<Result, Error, ThreadSafe>::succeeded() const {
  auto val = m_getValue();
  if constexpr (ThreadSafe) {
    MutexLocker lock(val->mutex);
    return val->result.isValid();
  } else {
    return val->result.isValid();
  }
}

template <typename Result, typename Error, bool ThreadSafe>
[[nodiscard]] bool RpcPromise<Result, Error, ThreadSafe>::failed() const {
  auto val = m_getValue();
  if constexpr (ThreadSafe) {
    MutexLocker lock(val->mutex);
    return val->error.isValid();
  } else {
    return val->error.isValid();
  }
}

template <typename Result, typename Error, bool ThreadSafe>
[[nodiscard]] auto RpcPromise<Result, Error, ThreadSafe>::result() const -> std::conditional_t<ThreadSafe, Maybe<Result>, Maybe<Result> const&> {
  auto val = m_getValue();
  if constexpr (ThreadSafe) {
    MutexLocker lock(val->mutex);
    return val->result;
  } else {
    return val->result;
  }
}

template <typename Result, typename Error, bool ThreadSafe>
[[nodiscard]] auto RpcPromise<Result, Error, ThreadSafe>::error() const -> std::conditional_t<ThreadSafe, Maybe<Error>, Maybe<Error> const&> {
  auto val = m_getValue();
  if constexpr (ThreadSafe) {
    MutexLocker lock(val->mutex);
    return val->error;
  } else {
    return val->error;
  }
}

template <typename Result, typename Error, bool ThreadSafe>
template <typename Function> requires (!ThreadSafe)
[[nodiscard]] decltype(auto) RpcPromise<Result, Error, ThreadSafe>::wrap(Function function) {
  using WrappedPromise = RpcPromise<std::decay_t<decltype(function(std::declval<Result>()))>, Error>;
  WrappedPromise wrappedPromise;
  wrappedPromise.m_getValue = [wrapper = std::move(function), valuePtr = std::make_shared<typename WrappedPromise::Value>(), otherGetValue = m_getValue]() -> typename WrappedPromise::Value* {
    if (!valuePtr->result && !valuePtr->error) {
      auto otherValue = otherGetValue();
      if (otherValue->result)
        valuePtr->result.set(wrapper(*otherValue->result));
      else if (otherValue->error)
        valuePtr->error.set(*otherValue->error);
    }
    return valuePtr.get();
  };
  return wrappedPromise;
}

}
