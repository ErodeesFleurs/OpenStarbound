module;

import std;
import star.string;
import star.set;
import star.json;

namespace Star {

class PlayerLog;
using PlayerLogPtr = std::shared_ptr<PlayerLog>;
using PlayerLogConstPtr = std::shared_ptr<const PlayerLog>;
using PlayerLogWeakPtr = std::weak_ptr<PlayerLog>;
using PlayerLogConstWeakPtr = std::weak_ptr<const PlayerLog>;
using PlayerLogUPtr = std::unique_ptr<PlayerLog>;
using PlayerLogConstUPtr = std::unique_ptr<const PlayerLog>;

class PlayerLog {
public:
  PlayerLog();
  PlayerLog(Json const& json);

  Json toJson() const;

  int deathCount() const;
  void addDeathCount(int deaths);

  double playTime() const;
  void addPlayTime(double elapsedTime);

  bool introComplete() const;
  void setIntroComplete(bool complete);

  StringSet scannedObjects() const;
  bool addScannedObject(String const& objectName);
  void removeScannedObject(String const& objectName);
  void clearScannedObjects();

  StringSet radioMessages() const;
  bool addRadioMessage(String const& messageName);
  void clearRadioMessages();

  StringSet cinematics() const;
  bool addCinematic(String const& cinematic);
  void clearCinematics();

  StringList collections() const;
  StringSet collectables(String const& collection) const;
  bool addCollectable(String const& collection, String const& collectable);
  void clearCollectables(String const& collection);

private:
  int m_deathCount;
  double m_playTime;
  bool m_introComplete;
  StringSet m_scannedObjects;
  StringSet m_radioMessages;
  StringSet m_cinematics;
  StringMap<StringSet> m_collections;
};

}

export module star.player_log;

export namespace Star {
using ::Star::PlayerLog;
using ::Star::PlayerLogPtr;
using ::Star::PlayerLogConstPtr;
using ::Star::PlayerLogWeakPtr;
using ::Star::PlayerLogConstWeakPtr;
using ::Star::PlayerLogUPtr;
using ::Star::PlayerLogConstUPtr;
}
