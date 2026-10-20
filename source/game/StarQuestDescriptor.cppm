module;
import std;
import star.string;
import star.list;
import star.variant;
import star.maybe;
import star.json;

import star.game_types;
import star.strong_typedef;
import star.data_stream;
import star.bi_map;
import star.item_descriptor;
import star.celestial_coordinate;

import star.rect;
namespace Star {

class Root;

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
template <typename BaseType>
struct QuestItemTagWrapper : BaseType {
  using BaseType::BaseType;

  QuestItemTagWrapper() : BaseType() {}

  QuestItemTagWrapper(QuestItemTagWrapper const& nt) : BaseType(nt) {}

  QuestItemTagWrapper(QuestItemTagWrapper&& nt) : BaseType(std::move(nt)) {}

  explicit QuestItemTagWrapper(BaseType const& bt) : BaseType(bt) {}

  explicit QuestItemTagWrapper(BaseType&& bt) : BaseType(std::move(bt)) {}

  QuestItemTagWrapper& operator=(QuestItemTagWrapper const& rhs) {
    BaseType::operator=(rhs);
    return *this;
  }

  QuestItemTagWrapper& operator=(QuestItemTagWrapper&& rhs) {
    BaseType::operator=(std::move(rhs));
    return *this;
  }

  template <class Arg>
  QuestItemTagWrapper& operator=(Arg&& other) {
    static_assert(std::is_base_of<BaseType, typename std::decay<Arg>::type>::value == false
            || std::is_same<QuestItemTagWrapper, typename std::decay<Arg>::type>::value,
        "QuestItemTag can not implicitly be assigned from String-derived classes or strong String typedefs");

    BaseType::operator=(std::forward<Arg>(other));
    return *this;
  }
};
typedef QuestItemTagWrapper<String> QuestItemTag;

// A collection of items
template <typename BaseType>
struct QuestItemListWrapper : BaseType {
  using BaseType::BaseType;

  QuestItemListWrapper() : BaseType() {}

  QuestItemListWrapper(QuestItemListWrapper const& nt) : BaseType(nt) {}

  QuestItemListWrapper(QuestItemListWrapper&& nt) : BaseType(std::move(nt)) {}

  explicit QuestItemListWrapper(BaseType const& bt) : BaseType(bt) {}

  explicit QuestItemListWrapper(BaseType&& bt) : BaseType(std::move(bt)) {}

  QuestItemListWrapper& operator=(QuestItemListWrapper const& rhs) {
    BaseType::operator=(rhs);
    return *this;
  }

  QuestItemListWrapper& operator=(QuestItemListWrapper&& rhs) {
    BaseType::operator=(std::move(rhs));
    return *this;
  }

  template <class Arg>
  QuestItemListWrapper& operator=(Arg&& other) {
    static_assert(std::is_base_of<BaseType, typename std::decay<Arg>::type>::value == false
            || std::is_same<QuestItemListWrapper, typename std::decay<Arg>::type>::value,
        "QuestItemList can not implicitly be assigned from List<ItemDescriptor>-derived classes or strong List<ItemDescriptor> typedefs");

    BaseType::operator=(std::forward<Arg>(other));
    return *this;
  }
};
typedef QuestItemListWrapper<List<ItemDescriptor>> QuestItemList;

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
  Maybe<std::uint64_t> seed;
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
  static QuestParam diskLoad(Root& root, Json const& json);

  Json toJson() const;
  Json diskStore(Root& root) const;

  bool operator==(QuestParam const& rhs) const;

  QuestParamDetail detail;
  Maybe<String> name;
  Maybe<Json> portrait;
  Maybe<String> indicator;
};

struct QuestDescriptor {
  friend DataStream& operator>>(DataStream& ds, QuestDescriptor& quest);
  friend DataStream& operator<<(DataStream& ds, QuestDescriptor const& quest);

  static QuestDescriptor fromJson(Root& root, Json const& json);
  static QuestDescriptor diskLoad(Root& root, Json const& json);

  Json toJson() const;
  Json diskStore(Root& root) const;

  bool operator==(QuestDescriptor const& rhs) const;

  String questId;
  String templateId;
  StringMap<QuestParam> parameters;
  std::uint64_t seed;
};

struct QuestArcDescriptor {
  friend DataStream& operator>>(DataStream& ds, QuestArcDescriptor& questArc);
  friend DataStream& operator<<(DataStream& ds, QuestArcDescriptor const& questArc);

  static QuestArcDescriptor fromJson(Root& root, Json const& json);
  static QuestArcDescriptor diskLoad(Root& root, Json const& json);

  Json toJson() const;
  Json diskStore(Root& root) const;

  bool operator==(QuestArcDescriptor const& rhs) const;

  List<QuestDescriptor> quests;
  Maybe<String> stagehandUniqueId;
};

String questParamText(Root& root, QuestParam const& param);
StringMap<String> questParamTags(Root& root, StringMap<QuestParam> const& parameters);

StringMap<QuestParam> questParamsFromJson(Json const& json);
StringMap<QuestParam> questParamsDiskLoad(Root& root, Json const& json);
Json questParamsToJson(StringMap<QuestParam> const& parameters);
Json questParamsDiskStore(Root& root, StringMap<QuestParam> const& parameters);

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
