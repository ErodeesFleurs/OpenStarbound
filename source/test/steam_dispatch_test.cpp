#define STEAM_API_EXPORTS
#include "steam/steam_api.h"
#include "steam/steam_api_flat.h"
#include "../application/StarSteamDispatch.hpp"

import std;

#include "gtest/gtest.h"

namespace {
struct Packet {
  int callback;
  std::vector<unsigned char> payload;
};

struct ApiResult {
  Packet packet;
  bool failed;
};

struct ManualApi {
  std::deque<Packet> callbacks;
  std::map<SteamAPICall_t, ApiResult> results;
  bool outstanding = false;

  template <typename Event>
  static Packet packet(Event const& event) {
    Packet packet{Event::k_iCallback, std::vector<unsigned char>(sizeof(event))};
    std::memcpy(packet.payload.data(), &event, sizeof(event));
    return packet;
  }

  template <typename Event>
  void enqueue(Event const& event) {
    callbacks.push_back(packet(event));
  }

  void stats(uint64 game) {
    UserStatsReceived_t event{};
    event.m_nGameID = game;
    enqueue(event);
  }

  void result(SteamAPICall_t call, PublishedFileId_t file, bool failed) {
    CreateItemResult_t event{};
    event.m_eResult = failed ? k_EResultFail : k_EResultOK;
    event.m_nPublishedFileId = file;
    results.emplace(call, ApiResult{packet(event), failed});
  }

  void complete(SteamAPICall_t call, uint32 size = sizeof(CreateItemResult_t)) {
    SteamAPICallCompleted_t event{};
    event.m_hAsyncCall = call;
    event.m_iCallback = CreateItemResult_t::k_iCallback;
    event.m_cubParam = size;
    enqueue(event);
  }
};

thread_local ManualApi* api = nullptr;

struct StatsObserver {
  std::vector<uint64> received;
  std::function<void()> action;
  void receive(UserStatsReceived_t* event) {
    uint64 game = event->m_nGameID;
    received.push_back(game);
    if (action)
      action();
  }
};

struct ResultObserver {
  std::vector<std::pair<PublishedFileId_t, bool>> received;
  std::function<void(PublishedFileId_t)> action;
  void receive(CreateItemResult_t* event, bool failed) {
    PublishedFileId_t file = event->m_nPublishedFileId;
    received.emplace_back(file, failed);
    if (action)
      action(file);
  }
};

class SteamDispatchTest : public testing::Test {
protected:
  void SetUp() override {
    api = &runtime;
    dispatch.initialize();
  }
  void TearDown() override {
    EXPECT_FALSE(runtime.outstanding);
    EXPECT_TRUE(runtime.callbacks.empty());
    api = nullptr;
  }
  ManualApi runtime;
  Star::SteamDispatch dispatch;
};
}// namespace

// A stateful C-ABI fixture: callback storage remains outstanding until freed.
// It deliberately refuses a second fetch during delivery, as the SDK requires.
S_API void S_CALLTYPE SteamAPI_ManualDispatch_Init() {}
S_API HSteamPipe S_CALLTYPE SteamAPI_GetHSteamPipe() { return 1; }
S_API void S_CALLTYPE SteamAPI_ReleaseCurrentThreadMemory() {}
S_API void S_CALLTYPE SteamAPI_ManualDispatch_RunFrame(HSteamPipe) {}
S_API bool S_CALLTYPE SteamAPI_ManualDispatch_GetNextCallback(HSteamPipe, CallbackMsg_t* out) {
  EXPECT_FALSE(api->outstanding);
  if (api->outstanding || api->callbacks.empty())
    return false;
  auto& packet = api->callbacks.front();
  out->m_hSteamUser = 1;
  out->m_iCallback = packet.callback;
  out->m_pubParam = packet.payload.data();
  out->m_cubParam = static_cast<int>(packet.payload.size());
  api->outstanding = true;
  return true;
}
S_API void S_CALLTYPE SteamAPI_ManualDispatch_FreeLastCallback(HSteamPipe) {
  EXPECT_TRUE(api->outstanding);
  if (api->outstanding) {
    api->callbacks.pop_front();
    api->outstanding = false;
  }
}
S_API bool S_CALLTYPE SteamAPI_ManualDispatch_GetAPICallResult(HSteamPipe, SteamAPICall_t call,
                                                               void* out, int size, int callback, bool* failed) {
  auto found = api->results.find(call);
  if (found == api->results.end() || found->second.packet.callback != callback
      || found->second.packet.payload.size() != static_cast<std::size_t>(size))
    return false;
  std::memcpy(out, found->second.packet.payload.data(), size);
  *failed = found->second.failed;
  return true;
}

TEST_F(SteamDispatchTest, CancellationDuringDeliveryPreservesRemainingCallbacks) {
  using Callback = Star::SteamCallback<StatsObserver, UserStatsReceived_t>;
  StatsObserver survivor, victim, controller;
  auto surviving = std::make_unique<Callback>(dispatch, &survivor, &StatsObserver::receive);
  auto removed = std::make_unique<Callback>(dispatch, &victim, &StatsObserver::receive);
  auto current = std::make_unique<Callback>(dispatch, &controller, &StatsObserver::receive);
  controller.action = [&] {
    current.reset();
    removed.reset();
    dispatch.runCallbacks();
  };
  runtime.stats(11);
  runtime.stats(12);
  dispatch.runCallbacks();
  EXPECT_EQ(controller.received, (std::vector<uint64>{11}));
  EXPECT_EQ(victim.received, (std::vector<uint64>{}));
  EXPECT_EQ(survivor.received, (std::vector<uint64>{11, 12}));
}

TEST_F(SteamDispatchTest, ExceptionDoesNotPoisonNextPumpOrRetainCallbackStorage) {
  StatsObserver observer;
  Star::SteamCallback<StatsObserver, UserStatsReceived_t> callback(dispatch, &observer, &StatsObserver::receive);
  observer.action = [] { throw std::runtime_error("handler failure"); };
  runtime.stats(13);
  runtime.stats(14);
  EXPECT_THROW(dispatch.runCallbacks(), std::runtime_error);
  observer.action = {};
  dispatch.runCallbacks();
  EXPECT_EQ(observer.received, (std::vector<uint64>{13, 14}));
}

TEST_F(SteamDispatchTest, OneShotResultsCanRearmAndPropagateIoFailure) {
  using Result = Star::SteamCallResult<ResultObserver, CreateItemResult_t>;
  ResultObserver observer;
  auto current = std::make_unique<Result>(dispatch, 1, &observer, &ResultObserver::receive);
  observer.action = [&](PublishedFileId_t file) {
    if (file == 101)
      current = std::make_unique<Result>(dispatch, 2, &observer, &ResultObserver::receive);
  };
  auto abandonedObserver = std::make_unique<ResultObserver>();
  auto abandoned = std::make_unique<Result>(dispatch, 3, abandonedObserver.get(), &ResultObserver::receive);
  abandoned.reset();
  abandonedObserver.reset();
  runtime.result(1, 101, false);
  runtime.result(2, 202, true);
  runtime.result(3, 303, false);
  runtime.complete(1);
  runtime.complete(2);
  runtime.complete(1);
  runtime.complete(3);
  dispatch.runCallbacks();
  EXPECT_EQ(observer.received, (std::vector<std::pair<PublishedFileId_t, bool>>{{101, false}, {202, true}}));
}

TEST_F(SteamDispatchTest, IncorrectPayloadSizesAreIgnoredWithoutLosingValidResults) {
  StatsObserver stats;
  ResultObserver result;
  Star::SteamCallback<StatsObserver, UserStatsReceived_t> callback(dispatch, &stats, &StatsObserver::receive);
  Star::SteamCallResult<ResultObserver, CreateItemResult_t> call(dispatch, 4, &result, &ResultObserver::receive);
  runtime.stats(15);
  runtime.callbacks.back().payload.resize(sizeof(UserStatsReceived_t) - 1);
  runtime.stats(16);
  runtime.result(4, 404, false);
  runtime.complete(4, sizeof(CreateItemResult_t) - 1);
  runtime.complete(4);
  dispatch.runCallbacks();
  EXPECT_EQ(stats.received, (std::vector<uint64>{16}));
  EXPECT_EQ(result.received, (std::vector<std::pair<PublishedFileId_t, bool>>{{404, false}}));
}
