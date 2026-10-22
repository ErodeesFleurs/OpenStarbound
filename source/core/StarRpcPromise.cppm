module;
import std;
import star.format;
import star.output_proxy;
import star.string;
import star.thread;
import star.logging;

namespace Star {

struct RpcPromiseExceptionTag {
  static constexpr char const* name() { return "RpcPromiseException"; }
};
using RpcPromiseException = StarError<RpcPromiseExceptionTag, StarException>;

template <typename Result, typename Error>
class RpcPromise;

// The other side of an RpcPromise. Completion is allowed exactly once; a
// missing keeper or a second completion returns a diagnostic.
template <typename Result, typename Error = String>
class RpcPromiseKeeper {
public:
  [[nodiscard]] std::expected<void, StarException> chain(RpcPromise<Result, Error> promise);
  [[nodiscard]] std::expected<void, StarException> fulfill(Result result);
  [[nodiscard]] std::expected<void, StarException> fail(Error error);

private:
  template <typename ResultT, typename ErrorT>
  friend class RpcPromise;

  std::shared_ptr<typename RpcPromise<Result, Error>::State> m_state;
};

// Wraps an RpcPromiseKeeper, best used as a shared_ptr. Fails the promise on destruction.
template <typename Result, typename Error = String>
class AutoFailRpcPromiseKeeper {
public:
  [[nodiscard]] std::expected<void, StarException> chain(RpcPromise<Result, Error> promise);
  [[nodiscard]] std::expected<void, StarException> fulfill(Result result);
  [[nodiscard]] std::expected<void, StarException> fail(Error error);

  AutoFailRpcPromiseKeeper(RpcPromiseKeeper<Result, Error> keeper);
  ~AutoFailRpcPromiseKeeper();
private:
  RpcPromiseKeeper<Result, Error> m_keeper;
  bool m_done = false;
};

// A generic promise for the result of a remote procedure call.  It has
// reference semantics and is implicitly shared. Thread safe via mutexes. Can be chained.
template <typename Result, typename Error = String>
class RpcPromise {
public:
  static std::pair<RpcPromise, RpcPromiseKeeper<Result, Error>> createPair();
  static RpcPromise createFulfilled(Result result);
  static RpcPromise createFailed(Error error);

  // Query diagnostics are distinct from the remote call's Error outcome.
  [[nodiscard]] std::expected<bool, StarException> finished() const;
  [[nodiscard]] std::expected<bool, StarException> succeeded() const;
  [[nodiscard]] std::expected<bool, StarException> failed() const;

  // A successful query contains a result only when the RPC succeeded.
  [[nodiscard]] std::expected<std::optional<Result>, StarException> result() const;

  // A successful query contains an Error only when the RPC failed.
  [[nodiscard]] std::expected<std::optional<Error>, StarException> error() const;

  // Evaluation is lazy and successful values are cached for every observer.
  // The callback returns expected<T, StarException>. A diagnostic leaves the
  // wrapper unevaluated, so a later query retries it.
  template <typename Function>
  [[nodiscard]] auto wrap(Function function)
      -> std::expected<RpcPromise<typename std::invoke_result_t<Function const&, Result>::value_type, Error>, StarException>;

private:
  template <typename ResultT, typename ErrorT>
  friend class RpcPromise;

  friend class RpcPromiseKeeper<Result, Error>;

  // Tagged outcomes keep pending, fulfilled, failed and delegated promises
  // mutually exclusive, including when Result and Error are the same type.
  struct Pending {};
  struct Fulfilled { Result value; };
  struct Failed { Error value; };
  struct Chained { RpcPromise promise; };

  struct State {
    virtual ~State() = default;
    [[nodiscard]] virtual std::expected<void, StarException> poll() { return {}; }

    bool pending() const {
      return std::holds_alternative<Pending>(outcome);
    }

    void fulfill(Result&& result) {
      outcome.template emplace<Fulfilled>(std::move(result));
    }

    void fail(Error&& error) {
      outcome.template emplace<Failed>(std::move(error));
    }

    Mutex mutex;
    std::variant<Pending, Fulfilled, Failed, Chained> outcome;
  };

  template <typename SourceResult, typename Function>
  struct WrappedState : State {
    WrappedState(Function function, RpcPromise<SourceResult, Error> other)
      : function(std::move(function)), other(std::move(other)) {}

    [[nodiscard]] std::expected<void, StarException> poll() override {
      if (!this->pending())
        return {};
      auto sourceResult = other.result();
      if (!sourceResult)
        return std::unexpected(std::move(sourceResult.error()));
      if (*sourceResult) {
        auto wrappedResult = function(std::move(**sourceResult));
        if (!wrappedResult)
          return std::unexpected(std::move(wrappedResult.error()));
        // Commit only after successful evaluation; diagnostics remain retryable.
        this->fulfill(std::move(*wrappedResult));
        return {};
      }
      auto sourceError = other.error();
      if (!sourceError)
        return std::unexpected(std::move(sourceError.error()));
      if (*sourceError)
        this->fail(std::move(**sourceError));
      return {};
    }

    Function const function;
    RpcPromise<SourceResult, Error> other;
  };

  [[nodiscard]] std::expected<State*, StarException> state() const {
    if (!m_state)
      return std::unexpected(RpcPromiseException("Cannot query an empty RpcPromise"));
    return m_state.get();
  }

  std::shared_ptr<State> m_state;
};

template <typename Result, typename Error>
[[nodiscard]] std::expected<void, StarException> RpcPromiseKeeper<Result, Error>::chain(RpcPromise<Result, Error> promise) {
  if (!m_state)
    return std::unexpected(RpcPromiseException("Cannot chain an empty RpcPromiseKeeper"));
  if (!promise.m_state)
    return std::unexpected(RpcPromiseException("Cannot chain an empty RpcPromise"));
  MutexLocker lock(m_state->mutex);
  if (!m_state->pending())
    return std::unexpected(RpcPromiseException("chain called on already finished RpcPromise"));
  m_state->outcome.template emplace<typename RpcPromise<Result, Error>::Chained>(std::move(promise));
  return {};
}

template <typename Result, typename Error>
[[nodiscard]] std::expected<void, StarException> RpcPromiseKeeper<Result, Error>::fulfill(Result result) {
  if (!m_state)
    return std::unexpected(RpcPromiseException("Cannot fulfill an empty RpcPromiseKeeper"));
  MutexLocker lock(m_state->mutex);
  if (!m_state->pending())
    return std::unexpected(RpcPromiseException("fulfill called on already finished RpcPromise"));
  m_state->fulfill(std::move(result));
  return {};
}

template <typename Result, typename Error>
[[nodiscard]] std::expected<void, StarException> RpcPromiseKeeper<Result, Error>::fail(Error error) {
  if (!m_state)
    return std::unexpected(RpcPromiseException("Cannot fail an empty RpcPromiseKeeper"));
  MutexLocker lock(m_state->mutex);
  if (!m_state->pending())
    return std::unexpected(RpcPromiseException("fail called on already finished RpcPromise"));
  m_state->fail(std::move(error));
  return {};
}

template <typename Result, typename Error>
[[nodiscard]] std::expected<void, StarException> AutoFailRpcPromiseKeeper<Result, Error>::chain(RpcPromise<Result, Error> promise) {
  auto completed = m_keeper.chain(std::move(promise));
  if (!completed)
    return std::unexpected(std::move(completed.error()));
  m_done = true;
  return {};
}

template <typename Result, typename Error>
[[nodiscard]] std::expected<void, StarException> AutoFailRpcPromiseKeeper<Result, Error>::fulfill(Result result) {
  auto completed = m_keeper.fulfill(std::move(result));
  if (!completed)
    return std::unexpected(std::move(completed.error()));
  m_done = true;
  return {};
}

template <typename Result, typename Error>
[[nodiscard]] std::expected<void, StarException> AutoFailRpcPromiseKeeper<Result, Error>::fail(Error error) {
  auto completed = m_keeper.fail(std::move(error));
  if (!completed)
    return std::unexpected(std::move(completed.error()));
  m_done = true;
  return {};
}

template <typename Result, typename Error>
AutoFailRpcPromiseKeeper<Result, Error>::AutoFailRpcPromiseKeeper(RpcPromiseKeeper<Result, Error> keeper)
  : m_keeper(std::move(keeper)) {}

template <typename Result, typename Error>
AutoFailRpcPromiseKeeper<Result, Error>::~AutoFailRpcPromiseKeeper() {
  if (!m_done) {
    auto completed = fail("Keeper destroyed");
    if (!completed)
      EmergencyLog::error("Failed to auto-fail RpcPromise keeper: {}", outputException(ExceptionDiagnostic(completed.error()), true));
  }
}

template <typename Result, typename Error>
std::pair<RpcPromise<Result, Error>, RpcPromiseKeeper<Result, Error>> RpcPromise<Result, Error>::createPair() {
  auto state = std::make_shared<State>();
  RpcPromise promise;
  promise.m_state = state;
  RpcPromiseKeeper<Result, Error> keeper;
  keeper.m_state = std::move(state);

  return {std::move(promise), std::move(keeper)};
}

template <typename Result, typename Error>
RpcPromise<Result, Error> RpcPromise<Result, Error>::createFulfilled(Result result) {
  RpcPromise promise;
  promise.m_state = std::make_shared<State>();
  promise.m_state->fulfill(std::move(result));
  return promise;
}

template <typename Result, typename Error>
RpcPromise<Result, Error> RpcPromise<Result, Error>::createFailed(Error error) {
  RpcPromise promise;
  promise.m_state = std::make_shared<State>();
  promise.m_state->fail(std::move(error));
  return promise;
}

template <typename Result, typename Error>
[[nodiscard]] std::expected<bool, StarException> RpcPromise<Result, Error>::finished() const {
  auto currentState = state();
  if (!currentState)
    return std::unexpected(std::move(currentState.error()));
  auto& val = **currentState;
  MutexLocker lock(val.mutex);
  auto polled = val.poll();
  if (!polled)
    return std::unexpected(std::move(polled.error()));
  if (auto chained = std::get_if<Chained>(&val.outcome))
    return chained->promise.finished();
  return std::holds_alternative<Fulfilled>(val.outcome) || std::holds_alternative<Failed>(val.outcome);
}

template <typename Result, typename Error>
[[nodiscard]] std::expected<bool, StarException> RpcPromise<Result, Error>::succeeded() const {
  auto currentState = state();
  if (!currentState)
    return std::unexpected(std::move(currentState.error()));
  auto& val = **currentState;
  MutexLocker lock(val.mutex);
  auto polled = val.poll();
  if (!polled)
    return std::unexpected(std::move(polled.error()));
  if (auto chained = std::get_if<Chained>(&val.outcome))
    return chained->promise.succeeded();
  return std::holds_alternative<Fulfilled>(val.outcome);
}

template <typename Result, typename Error>
[[nodiscard]] std::expected<bool, StarException> RpcPromise<Result, Error>::failed() const {
  auto currentState = state();
  if (!currentState)
    return std::unexpected(std::move(currentState.error()));
  auto& val = **currentState;
  MutexLocker lock(val.mutex);
  auto polled = val.poll();
  if (!polled)
    return std::unexpected(std::move(polled.error()));
  if (auto chained = std::get_if<Chained>(&val.outcome))
    return chained->promise.failed();
  return std::holds_alternative<Failed>(val.outcome);
}

template <typename Result, typename Error>
[[nodiscard]] std::expected<std::optional<Result>, StarException> RpcPromise<Result, Error>::result() const {
  auto currentState = state();
  if (!currentState)
    return std::unexpected(std::move(currentState.error()));
  auto& val = **currentState;
  MutexLocker lock(val.mutex);
  auto polled = val.poll();
  if (!polled)
    return std::unexpected(std::move(polled.error()));
  if (auto chained = std::get_if<Chained>(&val.outcome))
    return chained->promise.result();
  if (auto fulfilled = std::get_if<Fulfilled>(&val.outcome))
    return std::optional<Result>(std::in_place, fulfilled->value);
  return std::optional<Result>{};
}

template <typename Result, typename Error>
[[nodiscard]] std::expected<std::optional<Error>, StarException> RpcPromise<Result, Error>::error() const {
  auto currentState = state();
  if (!currentState)
    return std::unexpected(std::move(currentState.error()));
  auto& val = **currentState;
  MutexLocker lock(val.mutex);
  auto polled = val.poll();
  if (!polled)
    return std::unexpected(std::move(polled.error()));
  if (auto chained = std::get_if<Chained>(&val.outcome))
    return chained->promise.error();
  if (auto failed = std::get_if<Failed>(&val.outcome))
    return std::optional<Error>(std::in_place, failed->value);
  return std::optional<Error>{};
}

template <typename Result, typename Error>
template <typename Function>
[[nodiscard]] auto RpcPromise<Result, Error>::wrap(Function function)
    -> std::expected<RpcPromise<typename std::invoke_result_t<Function const&, Result>::value_type, Error>, StarException> {
  using WrappedResult = typename std::invoke_result_t<Function const&, Result>::value_type;
  static_assert(std::is_same_v<std::invoke_result_t<Function const&, Result>, std::expected<WrappedResult, StarException>>,
      "RpcPromise wrappers must return std::expected<T, StarException>");
  if (!m_state)
    return std::unexpected(RpcPromiseException("Cannot wrap an empty RpcPromise"));
  using WrappedPromise = RpcPromise<WrappedResult, Error>;
  WrappedPromise wrappedPromise;
  wrappedPromise.m_state = std::make_shared<typename WrappedPromise::template WrappedState<Result, Function>>(
      std::move(function), *this);
  return wrappedPromise;
}

}

export module star.rpc_promise;

export namespace Star {
using ::Star::RpcPromiseExceptionTag;
using ::Star::RpcPromiseException;
using ::Star::RpcPromiseKeeper;
using ::Star::AutoFailRpcPromiseKeeper;
using ::Star::RpcPromise;
}
