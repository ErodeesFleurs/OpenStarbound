#include "StarTestUniverse.hpp"
#include "StarFile.hpp"
#include "StarQuests.hpp"
#include "StarPlayerFactory.hpp"
#include "StarPlayerStorage.hpp"
#include "StarStatistics.hpp"
#include "StarStatisticsService.hpp"
#include "StarPlayer.hpp"
#include "StarAssets.hpp"
#include "StarWorldClient.hpp"
#include "StarLua.hpp"

namespace Star {

TestUniverse::TestUniverse(Vec2U clientWindowSize) {
  auto& root = Root::singleton();

  m_clientWindowSize = clientWindowSize;

  m_storagePath = File::temporaryDirectory();
  auto playerStorage = make_shared<PlayerStorage>(File::relativeTo(m_storagePath, "player"));
  auto statistics = make_shared<Statistics>(File::relativeTo(m_storagePath, "statistics"));
  m_server = make_shared<UniverseServer>(File::relativeTo(m_storagePath, "universe"));
  m_client = make_shared<UniverseClient>(playerStorage, statistics, File::relativeTo(m_storagePath, "universeclient"));

  m_server->start();

  m_mainPlayer = root.playerFactory()->create();
  m_mainPlayer->finalizeCreation();
  // A player only carries a ship species once it has been saved and loaded
  // (Player::diskStore falls back to the identity species, Player's save/load
  // ctor reads it back), which is what the real client always does
  // (StarClientApplication::loadPlayer).  The connect packet sends the ship
  // species and the server builds the ship world from it, so a player that was
  // only created in memory cannot connect: the server fails to create its ship
  // world and drops the connection.
  m_mainPlayer->setShipSpecies(m_mainPlayer->species());
  m_mainPlayer->setAdmin(true);
  m_mainPlayer->setModeType(PlayerMode::Survival);
  m_client->setMainPlayer(m_mainPlayer);
  // The real client provides the 'input' callback table to player scripts
  // (StarClientApplication::setLuaCallbacks -> LuaBindings::makeInputCallbacks).
  // That binds to the global Input, which a headless harness never initializes,
  // so provide a stub where nothing is ever bound.  OpenStarbound's player
  // scripts (e.g. /scripts/opensb/player/copy_paste.lua) call input.bindDown
  // from update() and would otherwise log a Lua error every frame.
  LuaCallbacks inputCallbacks;
  auto bindState = [](String const&, String const&) -> Maybe<unsigned> { return {}; };
  auto noKey = [](String const&) -> Maybe<unsigned> { return {}; };
  auto noButton = [](String const&) -> Maybe<List<Vec2F>> { return {}; };
  inputCallbacks.registerCallback("bindDown", bindState);
  inputCallbacks.registerCallback("bindUp", bindState);
  inputCallbacks.registerCallback("bindHeld", [](String const&, String const&) -> bool { return false; });
  inputCallbacks.registerCallback("bind", [](String const&, String const&) -> bool { return false; });
  inputCallbacks.registerCallback("keyDown", noKey);
  inputCallbacks.registerCallback("keyUp", noKey);
  inputCallbacks.registerCallback("keyHeld", [](String const&) -> bool { return false; });
  inputCallbacks.registerCallback("key", [](String const&) -> bool { return false; });
  inputCallbacks.registerCallback("mouseDown", noButton);
  inputCallbacks.registerCallback("mouseUp", noButton);
  inputCallbacks.registerCallback("mouseHeld", [](String const&) -> bool { return false; });
  inputCallbacks.registerCallback("mouse", [](String const&) -> bool { return false; });
  inputCallbacks.registerCallback("mousePosition", []() -> Vec2F { return {}; });
  inputCallbacks.registerCallback("getTag", [](String const&) -> unsigned { return 0; });
  inputCallbacks.registerCallback("events", []() -> Json { return JsonArray(); });
  inputCallbacks.registerCallback("resetBinds", [](String const&, String const&) {});
  inputCallbacks.registerCallback("setBinds", [](String const&, String const&, Json const&) {});
  inputCallbacks.registerCallback("getDefaultBinds", [](String const&, String const&) -> Json { return JsonObject(); });
  inputCallbacks.registerCallback("getBinds", [](String const&, String const&) -> Json { return JsonObject(); });
  m_client->setLuaCallbacks("input", inputCallbacks);
  m_client->connect(m_server->addLocalClient(), "test", "");
}

TestUniverse::~TestUniverse() {
  m_client = {};
  m_server = {};
  m_mainPlayer = {};
  File::removeDirectoryRecursive(m_storagePath);
}

void TestUniverse::warpPlayer(WorldId worldId) {
  // The client context only exists once the server has answered the connect
  // handshake, and both the warp call and the caller's expectations read it, so
  // let the client settle first.
  int64_t connectStart = Time::monotonicMilliseconds();
  while (!m_client->clientContext() && Time::monotonicMilliseconds() - connectStart < 60000) {
    m_client->update(0.016f);
    Thread::sleep(16);
  }

  m_client->warpPlayer(WarpToWorld(worldId), true);
  // Bounded by wall clock: if the world cannot be loaded the player stays
  // teleporting forever, so give up after five minutes and let the caller's
  // expectations report the failure instead of hanging the test.  A ship flight
  // to a celestial world legitimately takes tens of seconds, and a sanitizer
  // build several times that, so the runner's per-case timeout
  // (OPENSTARBOUND_TEST_TIMEOUT) is the practical limit.
  int64_t startTime = Time::monotonicMilliseconds();
  while (m_mainPlayer->isTeleporting() || m_client->playerWorld().empty()) {
    m_client->update(0.016f);
    Thread::sleep(16);
    if (Time::monotonicMilliseconds() - startTime > 300000)
      break;
  }
}

WorldId TestUniverse::currentPlayerWorld() const {
  if (auto clientContext = m_client->clientContext())
    return clientContext->playerWorldId();

  return {};
}

void TestUniverse::update(unsigned times) {
  for (unsigned i = 0; i < times; ++i) {
    m_client->update(0.016f);
    Thread::sleep(16);
  }
}

List<Drawable> TestUniverse::currentClientDrawables() {
  WorldRenderData renderData;
  auto worldClient = m_client->worldClient();
  worldClient->centerClientWindowOnPlayer(m_clientWindowSize);
  worldClient->render(renderData, 0);

  List<Drawable> drawables;
  for (auto& ed : renderData.entityDrawables) {
    for (auto& p : ed.layers)
      drawables.appendAll(std::move(p.second));
  }

  return drawables;
}

}
