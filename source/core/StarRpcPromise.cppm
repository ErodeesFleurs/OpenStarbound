module;
import std;
import star.format;
import star.string;
import star.thread;

namespace Star {

struct RpcPromiseExceptionTag {
  static constexpr char const* name() { return "RpcPromiseException"; }
};
using RpcPromiseException = StarError<RpcPromiseExceptionTag, StarException>;

template <typename Result, typename Error>
class RpcPromise;

// The other side of an RpcPromise, can be used to either fulfill or fail a
// paired promise.  Call either fulfill or fail function exactly once, any
// further invocations will result in an exception.
template <typename Result, typename Error = String>
class RpcPromiseKeeper {
public:
  void chain(RpcPromise<Result,Error> promise);
  void fulfill(Result result);
  void fail(Error error);

private:
  template <typename ResultT, typename ErrorT>
  friend class RpcPromise;

  std::shared_ptr<typename RpcPromise<Result, Error>::State> m_state;
};

// Wraps an RpcPromiseKeeper, best used as a shared_ptr. Fails the promise on destruction.
template <typename Result, typename Error = String>
class AutoFailRpcPromiseKeeper {
public:
  void chain(RpcPromise<Result,Error> promise);
  void fulfill(Result result);
  void fail(Error error);

  AutoFailRpcPromiseKeeper(RpcPromiseKeeper<Result,Error> keeper);
  ~AutoFailRpcPromiseKeeper();
private:
  RpcPromiseKeeper<Result,Error> m_keeper;
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

  // Has the response either failed or succeeded?
  bool finished() const;
  // Has the response finished with success?
  bool succeeded() const;
  // Has the response finished with failure?
  bool failed() const;

  // Returns the result of the rpc call on success, nothing on failure or when
  // not yet finished.
  std::optional<Result> result() const;

  // Returns the error of a failed rpc call.  Returns nothing if the call is
  // successful or not yet finished.
  std::optional<Error> error() const;

  // Wrap this RpcPromise into another promise which returns instead the result
  // of this function when fulfilled. Evaluation is lazy and its successful
  // result is cached for every observer. A throwing function leaves it unevaluated.
  template <typename Function>
  decltype(auto) wrap(Function function);

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
    virtual void poll() {}

    bool pending() const {
      return std::holds_alternative<Pending>(outcome) || outcome.valueless_by_exception();
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

    void poll() override {
      if (!this->pending())
        return;
      // Do not commit an outcome until the wrapper returns successfully.
      // A throwing wrapper remains lazy and is called by the next query.
      if (other.succeeded())
        this->outcome.template emplace<Fulfilled>(function(*other.result()));
      else if (other.failed())
        this->fail(std::move(*other.error()));
    }

    Function const function;
    RpcPromise<SourceResult, Error> other;
  };

  State& state() const {
    if (!m_state)
      throw std::bad_function_call();
    return *m_state;
  }

  std::shared_ptr<State> m_state;
};

template <typename Result, typename Error>
void RpcPromiseKeeper<Result, Error>::chain(RpcPromise<Result,Error> promise) {
  if (!m_state)
    throw std::bad_function_call();
  MutexLocker lock(m_state->mutex);
  if (!m_state->pending())
    throw RpcPromiseException("fulfill called on already finished RpcPromise");
  m_state->outcome.template emplace<typename RpcPromise<Result, Error>::Chained>(std::move(promise));
}

template <typename Result, typename Error>
void RpcPromiseKeeper<Result, Error>::fulfill(Result result) {
  if (!m_state)
    throw std::bad_function_call();
  MutexLocker lock(m_state->mutex);
  if (!m_state->pending())
    throw RpcPromiseException("fulfill called on already finished RpcPromise");
  m_state->fulfill(std::move(result));
}

template <typename Result, typename Error>
void RpcPromiseKeeper<Result, Error>::fail(Error error) {
  if (!m_state)
    throw std::bad_function_call();
  MutexLocker lock(m_state->mutex);
  if (!m_state->pending())
    throw RpcPromiseException("fail called on already finished RpcPromise");
  m_state->fail(std::move(error));
}

template <typename Result, typename Error>
void AutoFailRpcPromiseKeeper<Result, Error>::chain(RpcPromise<Result,Error> promise) {
  m_done = true;
  m_keeper.chain(std::move(promise));
}

template <typename Result, typename Error>
void AutoFailRpcPromiseKeeper<Result, Error>::fulfill(Result result) {
  m_done = true;
  m_keeper.fulfill(std::move(result));
}

template <typename Result, typename Error>
void AutoFailRpcPromiseKeeper<Result, Error>::fail(Error error) {
  m_done = true;
  m_keeper.fail(std::move(error));
}

template <typename Result, typename Error>
AutoFailRpcPromiseKeeper<Result, Error>::AutoFailRpcPromiseKeeper(RpcPromiseKeeper<Result,Error> keeper) : m_keeper(keeper) {}

template <typename Result, typename Error>
AutoFailRpcPromiseKeeper<Result, Error>::~AutoFailRpcPromiseKeeper() {
  if (!m_done) {
    m_keeper.fail("Keeper destroyed");
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
bool RpcPromise<Result, Error>::finished() const {
  auto& val = state();
  MutexLocker lock(val.mutex);
  val.poll();
  if (auto chained = std::get_if<Chained>(&val.outcome))
    return chained->promise.finished();
  return std::holds_alternative<Fulfilled>(val.outcome) || std::holds_alternative<Failed>(val.outcome);
}

template <typename Result, typename Error>
bool RpcPromise<Result, Error>::succeeded() const {
  auto& val = state();
  MutexLocker lock(val.mutex);
  val.poll();
  if (auto chained = std::get_if<Chained>(&val.outcome))
    return chained->promise.succeeded();
  return std::holds_alternative<Fulfilled>(val.outcome);
}

template <typename Result, typename Error>
bool RpcPromise<Result, Error>::failed() const {
  auto& val = state();
  MutexLocker lock(val.mutex);
  val.poll();
  if (auto chained = std::get_if<Chained>(&val.outcome))
    return chained->promise.failed();
  return std::holds_alternative<Failed>(val.outcome);
}

template <typename Result, typename Error>
std::optional<Result> RpcPromise<Result, Error>::result() const {
  auto& val = state();
  MutexLocker lock(val.mutex);
  val.poll();
  if (auto chained = std::get_if<Chained>(&val.outcome))
    return chained->promise.result();
  if (auto fulfilled = std::get_if<Fulfilled>(&val.outcome))
    return std::optional<Result>(std::in_place, fulfilled->value);
  return {};
}

template <typename Result, typename Error>
std::optional<Error> RpcPromise<Result, Error>::error() const {
  auto& val = state();
  MutexLocker lock(val.mutex);
  val.poll();
  if (auto chained = std::get_if<Chained>(&val.outcome))
    return chained->promise.error();
  if (auto failed = std::get_if<Failed>(&val.outcome))
    return std::optional<Error>(std::in_place, failed->value);
  return {};
}

template <typename Result, typename Error>
template <typename Function>
decltype(auto) RpcPromise<Result, Error>::wrap(Function function) {
  using WrappedPromise = RpcPromise<std::decay_t<decltype(function(std::declval<Result>()))>, Error>;
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
