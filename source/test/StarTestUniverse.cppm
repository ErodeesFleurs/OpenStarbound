module;
import std;
import star.string;
import star.vector;
import star.list;
import star.drawable;
import star.warping;
import star.format;

namespace Star {

class UniverseServer;
class UniverseClient;
class Player;
class Root;

class TestUniverse {
public:
  // Publishes only a fully connected fixture; failed setup tears down its
  // partially created client, server, and temporary storage.
  [[nodiscard]] static std::expected<std::unique_ptr<TestUniverse>, StarException> create(Root& root, Vec2U clientWindowSize);
  ~TestUniverse();

  [[nodiscard]] std::expected<void, StarException> warpPlayer(WorldId worldId);
  WorldId currentPlayerWorld() const;

  [[nodiscard]] std::expected<void, StarException> update(unsigned times = 1);

  [[nodiscard]] std::expected<List<Drawable>, StarException> currentClientDrawables();
  // Call before releasing the fixture to assert teardown failures. Destruction
  // still completes cleanup, reporting otherwise unobserved errors urgently.
  [[nodiscard]] std::expected<void, StarException> close();

private:
  explicit TestUniverse(Vec2U clientWindowSize);
  Vec2U m_clientWindowSize;
  String m_storagePath;
  std::shared_ptr<UniverseServer> m_server;
  std::shared_ptr<UniverseClient> m_client;
  std::shared_ptr<Player> m_mainPlayer;
};

}


export module star.test_universe;

export namespace Star {
  using ::Star::TestUniverse;
}
