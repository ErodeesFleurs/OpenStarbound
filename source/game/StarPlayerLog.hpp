#pragma once

#include "StarSet.hpp"
#include "StarJson.hpp"

namespace Star {

class PlayerLog;
using PlayerLogPtr = SharedPtr<PlayerLog>;

class PlayerLog {
public:
  PlayerLog() = default;
  PlayerLog(Json const& json);

  [[nodiscard]] Json toJson() const;

  [[nodiscard]] int deathCount() const;
  void addDeathCount(int deaths);

  [[nodiscard]] double playTime() const;
  void addPlayTime(double elapsedTime);

  [[nodiscard]] bool introComplete() const;
  void setIntroComplete(bool complete);

  [[nodiscard]] StringSet scannedObjects() const;
  [[nodiscard]] bool addScannedObject(String const& objectName);
  void removeScannedObject(String const& objectName);
  void clearScannedObjects();

  [[nodiscard]] StringSet radioMessages() const;
  [[nodiscard]] bool addRadioMessage(String const& messageName);
  void clearRadioMessages();

  [[nodiscard]] StringSet cinematics() const;
  [[nodiscard]] bool addCinematic(String const& cinematic);
  void clearCinematics();

  [[nodiscard]] StringList collections() const;
  [[nodiscard]] StringSet collectables(String const& collection) const;
  [[nodiscard]] bool addCollectable(String const& collection, String const& collectable);
  void clearCollectables(String const& collection);

private:
  int m_deathCount = 0;
  double m_playTime = 0.0;
  bool m_introComplete = false;
  StringSet m_scannedObjects;
  StringSet m_radioMessages;
  StringSet m_cinematics;
  StringMap<StringSet> m_collections;
};

}
