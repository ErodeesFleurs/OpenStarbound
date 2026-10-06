module;

#include "StarJson.hpp"
#include "StarPoly.hpp"
#include "StarGameTypes.hpp"
#include "StarStrongTypedef.hpp"
#include "StarItemDescriptor.hpp"
#include "StarJson.hpp"
#include "StarVector.hpp"
import star.celestial_coordinate;

namespace Star {

// Item name - always one single item. QuestItem and QuestItemList are
// distinct due to how the surrounding text interacts with the parameter
// in the quest text. For a single item we might want to say "the <bandage>" or
// "any <bandage>", whereas the text for QuestItemList is always a list, e.g.
// "<1 bandage, 3 apple>."
struct QuestItem {
  friend DataStream& operator>>(DataStream& ds, QuestItem& param);
  friend DataStream& operator<<(DataStream& ds, QuestItem const& param);

  bool operator==(QuestItem const& rhs) const;
  ItemDescriptor descriptor() const;

  String itemName;
  Json parameters;
};

// An item itemTag, indicating a set of possible items
strong_typedef(String, QuestItemTag);

// A collection of items
strong_typedef(List<ItemDescriptor>, QuestItemList);

// The uniqueId of a specific entity
struct QuestEntity {
  friend DataStream& operator>>(DataStream& ds, QuestEntity& param);
  friend DataStream& operator<<(DataStream& ds, QuestEntity const& param);

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
  friend DataStream& operator>>(DataStream& ds, QuestMonsterType& param);
  friend DataStream& operator<<(DataStream& ds, QuestMonsterType const& param);

  bool operator==(QuestMonsterType const& rhs) const;

  String typeName;
  JsonObject parameters;
};

struct QuestNpcType {
  friend DataStream& operator>>(DataStream& ds, QuestNpcType& param);
  friend DataStream& operator<<(DataStream& ds, QuestNpcType const& param);

  bool operator==(QuestNpcType const& rhs) const;

  String species;
  String typeName;
  JsonObject parameters;
  Maybe<uint64_t> seed;
};

struct QuestCoordinate {
  friend DataStream& operator>>(DataStream& ds, QuestCoordinate& param);
  friend DataStream& operator<<(DataStream& ds, QuestCoordinate const& param);

  bool operator==(QuestCoordinate const& rhs) const;

  CelestialCoordinate coordinate;
};

typedef Json QuestJson;

typedef MVariant<QuestItem, QuestItemTag, QuestItemList, QuestEntity, QuestLocation, QuestMonsterType, QuestNpcType, QuestCoordinate, QuestJson> QuestParamDetail;

struct QuestParam {
  friend DataStream& operator>>(DataStream& ds, QuestParam& param);
  friend DataStream& operator<<(DataStream& ds, QuestParam const& param);

  static QuestParam fromJson(Json const& json);
  static QuestParam diskLoad(Json const& json);

  Json toJson() const;
  Json diskStore() const;

  bool operator==(QuestParam const& rhs) const;

  QuestParamDetail detail;
  Maybe<String> name;
  Maybe<Json> portrait;
  Maybe<String> indicator;
};

struct QuestDescriptor {
  friend DataStream& operator>>(DataStream& ds, QuestDescriptor& quest);
  friend DataStream& operator<<(DataStream& ds, QuestDescriptor const& quest);

  static QuestDescriptor fromJson(Json const& json);
  static QuestDescriptor diskLoad(Json const& json);

  Json toJson() const;
  Json diskStore() const;

  bool operator==(QuestDescriptor const& rhs) const;

  String questId;
  String templateId;
  StringMap<QuestParam> parameters;
  uint64_t seed;
};

struct QuestArcDescriptor {
  friend DataStream& operator>>(DataStream& ds, QuestArcDescriptor& questArc);
  friend DataStream& operator<<(DataStream& ds, QuestArcDescriptor const& questArc);

  static QuestArcDescriptor fromJson(Json const& json);
  static QuestArcDescriptor diskLoad(Json const& json);

  Json toJson() const;
  Json diskStore() const;

  bool operator==(QuestArcDescriptor const& rhs) const;

  List<QuestDescriptor> quests;
  Maybe<String> stagehandUniqueId;
};

String questParamText(QuestParam const& param);
StringMap<String> questParamTags(StringMap<QuestParam> const& parameters);

StringMap<QuestParam> questParamsFromJson(Json const& json);
StringMap<QuestParam> questParamsDiskLoad(Json const& json);
Json questParamsToJson(StringMap<QuestParam> const& parameters);
Json questParamsDiskStore(StringMap<QuestParam> const& parameters);

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
}

export module star.quest_descriptor;

export namespace Star {
  using ::Star::QuestItem;
  using ::Star::QuestItemTagWrapper;
  using ::Star::QuestItemTag;
  using ::Star::QuestItemListWrapper;
  using ::Star::QuestItemList;
  using ::Star::QuestEntity;
  using ::Star::QuestLocation;
  using ::Star::QuestMonsterType;
  using ::Star::QuestNpcType;
  using ::Star::QuestCoordinate;
  using ::Star::QuestJson;
  using ::Star::QuestParamDetail;
  using ::Star::QuestParam;
  using ::Star::QuestDescriptor;
  using ::Star::QuestArcDescriptor;
  using ::Star::questParamText;
  using ::Star::questParamTags;
  using ::Star::questParamsFromJson;
  using ::Star::questParamsDiskLoad;
  using ::Star::questParamsToJson;
  using ::Star::questParamsDiskStore;
}
