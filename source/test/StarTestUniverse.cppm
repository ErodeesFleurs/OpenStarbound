module;
import std;
import star.string;
import star.vector;
import star.list;
import star.drawable;
import star.warping;

namespace Star {

class UniverseServer;
class UniverseClient;
class Player;
class Root;

class TestUniverse {
public:
  TestUniverse(Root& root, Vec2U clientWindowSize);
  ~TestUniverse();

  void warpPlayer(WorldId worldId);
  WorldId currentPlayerWorld() const;

  void update(unsigned times = 1);

  List<Drawable> currentClientDrawables();

private:
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
