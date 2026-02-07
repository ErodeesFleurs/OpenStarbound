#pragma once

#include "StarCelestialCoordinate.hpp"
#include "StarGameTypes.hpp"
#include "StarItemDescriptor.hpp"
#include "StarJson.hpp"
#include "StarRect.hpp"
#include "StarString.hpp"
#include "StarStrongTypedef.hpp"

import std;

namespace Star {

// Item name - always one single item. QuestItem and QuestItemList are
// distinct due to how the surrounding text interacts with the parameter
// in the quest text. For a single item we might want to say "the <bandage>" or
// "any <bandage>", whereas the text for QuestItemList is always a list, e.g.
// "<1 bandage, 3 apple>."
struct QuestItem {
  QuestItem(String itemName, Json parameters) : itemName(std::move(itemName)), parameters(std::move(parameters)) {}
  auto operator==(QuestItem const& rhs) const -> bool;
  [[nodiscard]] auto descriptor() const -> ItemDescriptor;

  String itemName;
  Json parameters;
};

// An item itemTag, indicating a set of possible items
using QuestItemTag = StrongTypedef<String>;

// A collection of items
using QuestItemList = StrongTypedef<List<ItemDescriptor>>;

// The uniqueId of a specific entity
struct QuestEntity {
  auto operator==(QuestEntity const& rhs) const -> bool;

  std::optional<String> uniqueId;
  std::optional<String> species;
  std::optional<Gender> gender;
};

// A location within the world, which could represent a spawn point or a dungeon
struct QuestLocation {
  auto operator==(QuestLocation const& rhs) const -> bool;

  std::optional<String> uniqueId;
  RectF region;
};

struct QuestMonsterType {
  auto operator==(QuestMonsterType const& rhs) const -> bool;

  String typeName;
  JsonObject parameters;
};

struct QuestNpcType {
  auto operator==(QuestNpcType const& rhs) const -> bool;

  String species;
  String typeName;
  JsonObject parameters;
  std::optional<std::uint64_t> seed;
};

struct QuestCoordinate {
  auto operator==(QuestCoordinate const& rhs) const -> bool;

  CelestialCoordinate coordinate;
};

using QuestJson = Json;

using QuestParamDetail = MVariant<QuestItem, QuestItemTag, QuestItemList, QuestEntity, QuestLocation, QuestMonsterType, QuestNpcType, QuestCoordinate, QuestJson>;

struct QuestParam {
  static auto fromJson(Json const& json) -> QuestParam;
  static auto diskLoad(Json const& json) -> QuestParam;

  [[nodiscard]] auto toJson() const -> Json;
  [[nodiscard]] auto diskStore() const -> Json;

  auto operator==(QuestParam const& rhs) const -> bool;

  QuestParamDetail detail;
  std::optional<String> name;
  std::optional<Json> portrait;
  std::optional<String> indicator;
};

struct QuestDescriptor {
  static auto fromJson(Json const& json) -> QuestDescriptor;
  static auto diskLoad(Json const& json) -> QuestDescriptor;

  [[nodiscard]] auto toJson() const -> Json;
  [[nodiscard]] auto diskStore() const -> Json;

  auto operator==(QuestDescriptor const& rhs) const -> bool;

  String questId;
  String templateId;
  StringMap<QuestParam> parameters;
  std::uint64_t seed;
};

struct QuestArcDescriptor {
  static auto fromJson(Json const& json) -> QuestArcDescriptor;
  static auto diskLoad(Json const& json) -> QuestArcDescriptor;

  [[nodiscard]] auto toJson() const -> Json;
  [[nodiscard]] auto diskStore() const -> Json;

  auto operator==(QuestArcDescriptor const& rhs) const -> bool;

  List<QuestDescriptor> quests;
  std::optional<String> stagehandUniqueId;
};

auto questParamText(QuestParam const& param) -> String;
auto questParamTags(StringMap<QuestParam> const& parameters) -> StringMap<String>;

auto questParamsFromJson(Json const& json) -> StringMap<QuestParam>;
auto questParamsDiskLoad(Json const& json) -> StringMap<QuestParam>;
auto questParamsToJson(StringMap<QuestParam> const& parameters) -> Json;
auto questParamsDiskStore(StringMap<QuestParam> const& parameters) -> Json;

auto operator>>(DataStream& ds, QuestItem& param) -> DataStream&;
auto operator<<(DataStream& ds, QuestItem const& param) -> DataStream&;
auto operator>>(DataStream& ds, QuestEntity& param) -> DataStream&;
auto operator<<(DataStream& ds, QuestEntity const& param) -> DataStream&;
auto operator>>(DataStream& ds, QuestMonsterType& param) -> DataStream&;
auto operator<<(DataStream& ds, QuestMonsterType const& param) -> DataStream&;
auto operator>>(DataStream& ds, QuestNpcType& param) -> DataStream&;
auto operator<<(DataStream& ds, QuestNpcType const& param) -> DataStream&;
auto operator>>(DataStream& ds, QuestCoordinate& param) -> DataStream&;
auto operator<<(DataStream& ds, QuestCoordinate const& param) -> DataStream&;
auto operator>>(DataStream& ds, QuestParam& param) -> DataStream&;
auto operator<<(DataStream& ds, QuestParam const& param) -> DataStream&;
auto operator>>(DataStream& ds, QuestDescriptor& quest) -> DataStream&;
auto operator<<(DataStream& ds, QuestDescriptor const& quest) -> DataStream&;
auto operator>>(DataStream& ds, QuestArcDescriptor& questArc) -> DataStream&;
auto operator<<(DataStream& ds, QuestArcDescriptor const& questArc) -> DataStream&;
}// namespace Star
