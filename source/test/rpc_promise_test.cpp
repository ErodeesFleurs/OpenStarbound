import std;
import star.rpc_promise;
import star.string;
import star.json;
import star.json_rpc;
import star.byte_array;
import star.data_stream_devices;
import star.list;
import star.logging;

#include "gtest/gtest.h"

using namespace Star;

namespace {
struct NonDefaultValue {
  explicit NonDefaultValue(int value) : value(value) {}
  int value;
};
}

TEST(RpcPromiseTest, SharedObservationAndSingleResolution) {
  auto [promise, keeper] = RpcPromise<int>::createPair();
  auto observer = promise;
  auto otherKeeper = keeper;
  EXPECT_FALSE(observer.finished());
  EXPECT_FALSE(observer.succeeded());
  EXPECT_FALSE(observer.failed());
  EXPECT_FALSE(observer.result());
  EXPECT_FALSE(observer.error());

  keeper.fulfill(42);
  EXPECT_TRUE(observer.finished());
  EXPECT_TRUE(observer.succeeded());
  EXPECT_FALSE(observer.failed());
  ASSERT_TRUE(observer.result());
  EXPECT_EQ(*observer.result(), 42);
  EXPECT_FALSE(observer.error());
  EXPECT_THROW(otherKeeper.fulfill(9), RpcPromiseException);
  EXPECT_THROW(otherKeeper.fail("late failure"), RpcPromiseException);
  EXPECT_THROW(otherKeeper.chain(RpcPromise<int>::createFulfilled(1)), RpcPromiseException);
  EXPECT_EQ(*promise.result(), 42);

  auto [failure, failureKeeper] = RpcPromise<int>::createPair();
  failureKeeper.fail("failed");
  EXPECT_TRUE(failure.finished());
  EXPECT_FALSE(failure.succeeded());
  EXPECT_TRUE(failure.failed());
  EXPECT_FALSE(failure.result());
  ASSERT_TRUE(failure.error());
  EXPECT_EQ(*failure.error(), "failed");
  EXPECT_THROW(failureKeeper.fulfill(1), RpcPromiseException);
  EXPECT_THROW(failureKeeper.fail("again"), RpcPromiseException);
}

TEST(RpcPromiseTest, DelegationIsResolutionEvenBeforeTargetFinishes) {
  auto [outer, outerKeeper] = RpcPromise<int>::createPair();
  auto [middle, middleKeeper] = RpcPromise<int>::createPair();
  auto [inner, innerKeeper] = RpcPromise<int>::createPair();
  outerKeeper.chain(middle);
  middleKeeper.chain(inner);
  EXPECT_FALSE(outer.finished());
  EXPECT_FALSE(outer.succeeded());
  EXPECT_FALSE(outer.failed());
  EXPECT_FALSE(outer.result());
  EXPECT_FALSE(outer.error());
  EXPECT_THROW(outerKeeper.fulfill(1), RpcPromiseException);
  EXPECT_THROW(outerKeeper.fail("late"), RpcPromiseException);
  EXPECT_THROW(outerKeeper.chain(inner), RpcPromiseException);
  innerKeeper.fulfill(73);
  EXPECT_TRUE(outer.finished());
  EXPECT_TRUE(outer.succeeded());
  EXPECT_EQ(*outer.result(), 73);

  auto [failedOuter, failedKeeper] = RpcPromise<int>::createPair();
  failedKeeper.chain(RpcPromise<int>::createFailed("delegated error"));
  EXPECT_TRUE(failedOuter.finished());
  EXPECT_TRUE(failedOuter.failed());
  EXPECT_FALSE(failedOuter.result());
  EXPECT_EQ(*failedOuter.error(), "delegated error");
}

TEST(RpcPromiseTest, LazyWrapperCachesAcrossAllObservers) {
  auto [source, keeper] = RpcPromise<int>::createPair();
  int calls = 0;
  auto wrapped = source.wrap([&calls](int value) {
    ++calls;
    return String(std::to_string(value));
  });
  auto observer = wrapped;
  EXPECT_EQ(calls, 0);
  EXPECT_FALSE(wrapped.finished());
  EXPECT_FALSE(wrapped.result());
  EXPECT_FALSE(wrapped.error());
  EXPECT_EQ(calls, 0);
  keeper.fulfill(21);
  EXPECT_EQ(calls, 0);
  EXPECT_TRUE(observer.finished());
  EXPECT_TRUE(wrapped.succeeded());
  EXPECT_FALSE(observer.failed());
  EXPECT_EQ(*wrapped.result(), "21");
  EXPECT_FALSE(observer.error());
  EXPECT_EQ(*observer.result(), "21");
  EXPECT_EQ(calls, 1);
}

TEST(RpcPromiseTest, ThrowingWrapperRemainsQueryable) {
  auto source = RpcPromise<int>::createFulfilled(8);
  int calls = 0;
  auto wrapped = source.wrap([&calls](int value) {
    if (++calls < 3)
      throw std::runtime_error("wrapper failed");
    return value * 2;
  });
  auto observer = wrapped;
  EXPECT_THROW(wrapped.finished(), std::runtime_error);
  EXPECT_THROW(observer.error(), std::runtime_error);
  EXPECT_EQ(calls, 2);
  EXPECT_EQ(*observer.result(), 16);
  EXPECT_TRUE(wrapped.succeeded());
  EXPECT_FALSE(wrapped.failed());
  EXPECT_EQ(calls, 3);
}

TEST(RpcPromiseTest, WrappedFailureAndWrappedDelegation) {
  int calls = 0;
  auto [source, keeper] = RpcPromise<int, int>::createPair();
  auto wrapped = source.wrap([&calls](int value) {
    ++calls;
    return value + 1;
  });
  auto [outer, outerKeeper] = RpcPromise<int, int>::createPair();
  outerKeeper.chain(wrapped);
  EXPECT_FALSE(outer.finished());
  keeper.fail(19);
  EXPECT_TRUE(outer.finished());
  EXPECT_TRUE(outer.failed());
  EXPECT_FALSE(outer.result());
  EXPECT_EQ(*outer.error(), 19);
  EXPECT_EQ(calls, 0);

  auto [success, successKeeper] = RpcPromise<int, int>::createPair();
  auto transformed = success.wrap([&calls](int value) {
    ++calls;
    return value + 1;
  });
  auto [delegated, delegatedKeeper] = RpcPromise<int, int>::createPair();
  delegatedKeeper.chain(transformed);
  successKeeper.fulfill(10);
  EXPECT_EQ(calls, 0);
  EXPECT_EQ(*delegated.result(), 11);
  EXPECT_TRUE(delegated.finished());
  EXPECT_EQ(calls, 1);
}

TEST(RpcPromiseTest, ArbitraryPayloadTypesDoNotNeedDefaultConstruction) {
  auto [promise, keeper] = RpcPromise<NonDefaultValue, NonDefaultValue>::createPair();
  keeper.fulfill(NonDefaultValue(7));
  ASSERT_TRUE(promise.result());
  EXPECT_EQ(promise.result()->value, 7);
  EXPECT_FALSE(promise.error());
  auto failed = RpcPromise<NonDefaultValue, NonDefaultValue>::createFailed(NonDefaultValue(12));
  EXPECT_TRUE(failed.failed());
  ASSERT_TRUE(failed.error());
  EXPECT_EQ(failed.error()->value, 12);

  auto [moveOnly, moveOnlyKeeper] = RpcPromise<std::unique_ptr<int>, std::unique_ptr<int>>::createPair();
  moveOnlyKeeper.fulfill(std::make_unique<int>(3));
  EXPECT_TRUE(moveOnly.succeeded());
  auto moveOnlyFailure = RpcPromise<std::unique_ptr<int>, std::unique_ptr<int>>::createFailed(std::make_unique<int>(4));
  EXPECT_TRUE(moveOnlyFailure.failed());
}

TEST(RpcPromiseTest, ConcurrentKeepersChooseExactlyOneOutcome) {
  auto [promise, keeper] = RpcPromise<int, int>::createPair();
  std::barrier start(3);
  std::atomic<int> rejected = 0;
  auto fulfillKeeper = keeper;
  auto failKeeper = keeper;
  std::thread fulfill([&] {
    start.arrive_and_wait();
    try {
      fulfillKeeper.fulfill(5);
    } catch (RpcPromiseException const&) {
      ++rejected;
    }
  });
  std::thread fail([&] {
    start.arrive_and_wait();
    try {
      failKeeper.fail(6);
    } catch (RpcPromiseException const&) {
      ++rejected;
    }
  });
  start.arrive_and_wait();
  fulfill.join();
  fail.join();
  EXPECT_EQ(rejected.load(), 1);
  EXPECT_TRUE(promise.finished());
  EXPECT_NE(promise.succeeded(), promise.failed());
  if (promise.succeeded())
    EXPECT_EQ(*promise.result(), 5);
  else
    EXPECT_EQ(*promise.error(), 6);
}

TEST(RpcPromiseTest, AutoFailOwnershipAndExplicitDecisions) {
  auto [abandoned, keeper] = RpcPromise<int>::createPair();
  auto automatic = std::make_shared<AutoFailRpcPromiseKeeper<int>>(keeper);
  auto alias = automatic;
  automatic.reset();
  EXPECT_FALSE(abandoned.finished());
  alias.reset();
  EXPECT_TRUE(abandoned.failed());
  EXPECT_EQ(*abandoned.error(), "Keeper destroyed");

  auto [fulfilled, fulfilledKeeper] = RpcPromise<int>::createPair();
  {
    AutoFailRpcPromiseKeeper<int> automaticKeeper(fulfilledKeeper);
    automaticKeeper.fulfill(10);
    EXPECT_THROW(automaticKeeper.fail("second decision"), RpcPromiseException);
  }
  EXPECT_EQ(*fulfilled.result(), 10);

  auto [failed, failedKeeper] = RpcPromise<int>::createPair();
  {
    AutoFailRpcPromiseKeeper<int> automaticKeeper(failedKeeper);
    automaticKeeper.fail("explicit failure");
  }
  EXPECT_EQ(*failed.error(), "explicit failure");

  auto [delegated, delegatedKeeper] = RpcPromise<int>::createPair();
  auto [target, targetKeeper] = RpcPromise<int>::createPair();
  {
    AutoFailRpcPromiseKeeper<int> automaticKeeper(delegatedKeeper);
    automaticKeeper.chain(target);
  }
  EXPECT_FALSE(delegated.finished());
  targetKeeper.fulfill(17);
  EXPECT_EQ(*delegated.result(), 17);
}

TEST(RpcPromiseTest, WrapperRetainsCaptureUntilLastObserverDies) {
  std::weak_ptr<int> capture;
  {
    auto resource = std::make_shared<int>(9);
    capture = resource;
    auto wrapped = RpcPromise<int>::createFulfilled(2).wrap([resource](int value) { return value + *resource; });
    resource.reset();
    {
      auto observer = wrapped;
      EXPECT_EQ(*observer.result(), 11);
    }
    EXPECT_FALSE(capture.expired());
  }
  EXPECT_TRUE(capture.expired());
}

TEST(RpcPromiseTest, EmptyHandlesRetainBadFunctionCallContract) {
  RpcPromise<int> promise;
  RpcPromiseKeeper<int> keeper;
  EXPECT_THROW(promise.finished(), std::bad_function_call);
  EXPECT_THROW(promise.result(), std::bad_function_call);
  EXPECT_THROW(keeper.fulfill(1), std::bad_function_call);
  EXPECT_THROW(keeper.fail("error"), std::bad_function_call);
}

TEST(JsonRpcTest, RemoteFailureStillFulfillsNull) {
  Logger logger;
  JsonRpc sender(logger);
  auto response = sender.invokeRemote("remote", JsonArray{1, 2});
  EXPECT_FALSE(response.finished());
  DataStreamBuffer requestBuffer(sender.send());
  List<Json> requests;
  requestBuffer.readContainer(requests);
  ASSERT_EQ(requests.size(), 1u);
  EXPECT_EQ(requests[0].getString("handler"), "remote");
  DataStreamBuffer failBuffer;
  failBuffer.writeContainer(List<Json>{JsonObject{{"command", "fail"}, {"id", requests[0].get("id")}}});
  sender.receive(failBuffer.takeData());
  EXPECT_TRUE(response.finished());
  EXPECT_TRUE(response.succeeded());
  EXPECT_FALSE(response.failed());
  ASSERT_TRUE(response.result());
  EXPECT_TRUE(response.result()->isNull());
  EXPECT_FALSE(response.error());
}

TEST(JsonRpcTest, SuccessfulTransportPreservesStructuredResult) {
  Logger logger;
  JsonRpc sender(logger);
  JsonRpc receiver(logger);
  receiver.registerHandler("echo", [](Json const& arguments) { return arguments; });
  Json arguments = JsonObject{{"message", "round trip"}, {"values", JsonArray{1, false, Json()}}};
  auto response = sender.invokeRemote("echo", arguments);
  receiver.receive(sender.send());
  EXPECT_FALSE(response.finished());
  sender.receive(receiver.send());
  EXPECT_TRUE(response.succeeded());
  EXPECT_EQ(*response.result(), arguments);
  EXPECT_FALSE(response.error());
}

TEST(RpcPromiseTest, ConcurrentObserversShareOneLazyEvaluation) {
  std::atomic<int> calls = 0;
  auto wrapped = RpcPromise<int>::createFulfilled(7).wrap([&calls](int value) {
    ++calls;
    return value * 3;
  });
  std::barrier start(5);
  std::atomic<int> observed = 0;
  std::vector<std::thread> observers;
  for (int i = 0; i != 4; ++i) {
    observers.emplace_back([observer = wrapped, &start, &observed] {
      start.arrive_and_wait();
      if (observer.result().value_or(0) == 21)
        ++observed;
    });
  }
  start.arrive_and_wait();
  for (auto& observer : observers)
    observer.join();
  EXPECT_EQ(observed.load(), 4);
  EXPECT_EQ(calls.load(), 1);
}

TEST(RpcPromiseTest, ReferenceReturningWrapperCachesOwnedValue) {
  int value = 13;
  auto wrapped = RpcPromise<int>::createFulfilled(1).wrap([&value](int) -> int const& {
    return value;
  });
  EXPECT_EQ(*wrapped.result(), 13);
  value = 24;
  EXPECT_EQ(*wrapped.result(), 13);
}
