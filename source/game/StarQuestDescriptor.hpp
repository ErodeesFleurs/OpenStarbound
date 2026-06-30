#pragma once

#include "StarCelestialCoordinate.hpp"
#include "StarGameTypes.hpp"
#include "StarItemDescriptor.hpp"
#include "StarJson.hpp"
#include "StarPoly.hpp"
#include "StarStrongTypedef.hpp"

namespace Star {

class ItemDatabase;
using ItemDatabaseConstPtr = SharedPtr<ItemDatabase const>;
class VersioningDatabase;
using VersioningDatabaseConstPtr = SharedPtr<VersioningDatabase const>;

// Item name - always one single item. QuestItem and QuestItemList are
// distinct due to how the surrounding text interacts with the parameter
// in the quest text. For a single item we might want to say "the <bandage>" or
// "any <bandage>", whereas the text for QuestItemList is always a list, e.g.
// "<1 bandage, 3 apple>."
struct QuestItem {
  bool operator==(QuestItem const& rhs) const;
  [[nodiscard]] ItemDescriptor descriptor() const;

  String itemName;
  Json parameters;
};

// An item itemTag, indicating a set of possible items
using QuestItemTag = StrongTypedef<String, struct QuestItemTagTag>;

// A collection of items
using QuestItemList = StrongTypedef<List<ItemDescriptor>, struct QuestItemListTag>;

// The uniqueId of a specific entity
struct QuestEntity {
  bool operator==(QuestEntity const& rhs) const;

  Maybe<String> uniqueId;
  Maybe<String> species;
  Maybe<Gender> gender;
};

// A location within the world, which could represent a spawn point or a dungeon
struct QuestLocation {
  bool operator==(QuestLocation const& rhs) const;

  Maybe<String> uniqueId;
  RectF region;
};

struct QuestMonsterType {
  bool operator==(QuestMonsterType const& rhs) const;

  String typeName;
  JsonObject parameters;
};

struct QuestNpcType {
  bool operator==(QuestNpcType const& rhs) const;

  String species;
  String typeName;
  JsonObject parameters;
  Maybe<uint64_t> seed;
};

struct QuestCoordinate {
  bool operator==(QuestCoordinate const& rhs) const;

  CelestialCoordinate coordinate;
};

using QuestJson = Json;

using QuestParamDetail = MVariant<QuestItem, QuestItemTag, QuestItemList, QuestEntity, QuestLocation, QuestMonsterType, QuestNpcType, QuestCoordinate, QuestJson>;

struct QuestParam {
  [[nodiscard]] static QuestParam fromJson(Json const& json);
  [[nodiscard]] static QuestParam diskLoad(Json const& json, VersioningDatabaseConstPtr versioningDatabase);

  [[nodiscard]] Json toJson() const;
  [[nodiscard]] Json diskStore(VersioningDatabaseConstPtr versioningDatabase) const;

  bool operator==(QuestParam const& rhs) const;

  QuestParamDetail detail;
  Maybe<String> name;
  Maybe<Json> portrait;
  Maybe<String> indicator;
};

struct QuestDescriptor {
  [[nodiscard]] static QuestDescriptor fromJson(Json const& json);
  [[nodiscard]] static QuestDescriptor diskLoad(Json const& json, VersioningDatabaseConstPtr versioningDatabase);

  [[nodiscard]] Json toJson() const;
  [[nodiscard]] Json diskStore(VersioningDatabaseConstPtr versioningDatabase) const;

  bool operator==(QuestDescriptor const& rhs) const;

  String questId;
  String templateId;
  StringMap<QuestParam> parameters;
  uint64_t seed;
};

struct QuestArcDescriptor {
  [[nodiscard]] static QuestArcDescriptor fromJson(Json const& json);
  [[nodiscard]] static QuestArcDescriptor diskLoad(Json const& json, VersioningDatabaseConstPtr versioningDatabase);

  [[nodiscard]] Json toJson() const;
  [[nodiscard]] Json diskStore(VersioningDatabaseConstPtr versioningDatabase) const;

  bool operator==(QuestArcDescriptor const& rhs) const;

  List<QuestDescriptor> quests;
  Maybe<String> stagehandUniqueId;
};

[[nodiscard]] String questParamText(QuestParam const& param, ItemDatabaseConstPtr itemDatabase);
[[nodiscard]] StringMap<String> questParamTags(StringMap<QuestParam> const& parameters, ItemDatabaseConstPtr itemDatabase);

[[nodiscard]] StringMap<QuestParam> questParamsFromJson(Json const& json);
[[nodiscard]] StringMap<QuestParam> questParamsDiskLoad(Json const& json, VersioningDatabaseConstPtr versioningDatabase);
[[nodiscard]] Json questParamsToJson(StringMap<QuestParam> const& parameters);
[[nodiscard]] Json questParamsDiskStore(StringMap<QuestParam> const& parameters, VersioningDatabaseConstPtr versioningDatabase);

DataStream& operator>>(DataStream& ds, QuestItem& param);
DataStream& operator<<(DataStream& ds, QuestItem const& param);
DataStream& operator>>(DataStream& ds, QuestEntity& param);
DataStream& operator<<(DataStream& ds, QuestEntity const& param);
DataStream& operator>>(DataStream& ds, QuestMonsterType& param);
DataStream& operator<<(DataStream& ds, QuestMonsterType const& param);
DataStream& operator>>(DataStream& ds, QuestNpcType& param);
DataStream& operator<<(DataStream& ds, QuestNpcType const& param);
DataStream& operator>>(DataStream& ds, QuestCoordinate& param);
DataStream& operator<<(DataStream& ds, QuestCoordinate const& param);
DataStream& operator>>(DataStream& ds, QuestParam& param);
DataStream& operator<<(DataStream& ds, QuestParam const& param);
DataStream& operator>>(DataStream& ds, QuestDescriptor& quest);
DataStream& operator<<(DataStream& ds, QuestDescriptor const& quest);
DataStream& operator>>(DataStream& ds, QuestArcDescriptor& questArc);
DataStream& operator<<(DataStream& ds, QuestArcDescriptor const& questArc);
}// namespace Star
