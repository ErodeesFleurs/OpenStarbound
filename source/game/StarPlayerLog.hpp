#pragma once

#include "StarJson.hpp"

namespace Star {

class PlayerLog {
public:
  PlayerLog();
  PlayerLog(Json const& json);

  [[nodiscard]] auto toJson() const -> Json;

  [[nodiscard]] auto deathCount() const -> int;
  void addDeathCount(int deaths);

  [[nodiscard]] auto playTime() const -> double;
  void addPlayTime(double elapsedTime);

  [[nodiscard]] auto introComplete() const -> bool;
  void setIntroComplete(bool complete);

  [[nodiscard]] auto scannedObjects() const -> StringSet;
  auto addScannedObject(String const& objectName) -> bool;
  void removeScannedObject(String const& objectName);
  void clearScannedObjects();

  [[nodiscard]] auto radioMessages() const -> StringSet;
  auto addRadioMessage(String const& messageName) -> bool;
  void clearRadioMessages();

  [[nodiscard]] auto cinematics() const -> StringSet;
  auto addCinematic(String const& cinematic) -> bool;
  void clearCinematics();

  [[nodiscard]] auto collections() const -> StringList;
  [[nodiscard]] auto collectables(String const& collection) const -> StringSet;
  auto addCollectable(String const& collection, String const& collectable) -> bool;
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

}// namespace Star
