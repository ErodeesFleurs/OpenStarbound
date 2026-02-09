#pragma once

#include "StarJson.hpp"
#include "StarPoly.hpp"
#include "StarVariant.hpp"

import std;

namespace Star {

struct PhysicsCategoryFilter {
  enum Type { Whitelist,
              Blacklist };

  static auto whitelist(StringSet categories) -> PhysicsCategoryFilter;
  static auto blacklist(StringSet categories) -> PhysicsCategoryFilter;

  PhysicsCategoryFilter(Type type = Blacklist, StringSet categories = {});

  [[nodiscard]] auto check(StringSet const& categories) const -> bool;

  auto operator==(PhysicsCategoryFilter const& rhs) const -> bool;

  Type type;
  StringSet categories;
};

auto operator>>(DataStream& ds, PhysicsCategoryFilter& rfr) -> DataStream&;
auto operator<<(DataStream& ds, PhysicsCategoryFilter const& rfr) -> DataStream&;

auto jsonToPhysicsCategoryFilter(Json const& json) -> PhysicsCategoryFilter;

struct DirectionalForceRegion {
  static auto fromJson(Json const& json) -> DirectionalForceRegion;

  [[nodiscard]] auto boundBox() const -> RectF;

  void translate(Vec2F const& pos);

  auto operator==(DirectionalForceRegion const& rhs) const -> bool;

  PolyF region;
  std::optional<float> xTargetVelocity;
  std::optional<float> yTargetVelocity;
  float controlForce;
  PhysicsCategoryFilter categoryFilter;
};

auto operator>>(DataStream& ds, DirectionalForceRegion& rfr) -> DataStream&;
auto operator<<(DataStream& ds, DirectionalForceRegion const& rfr) -> DataStream&;

struct RadialForceRegion {
  static auto fromJson(Json const& json) -> RadialForceRegion;

  [[nodiscard]] auto boundBox() const -> RectF;

  void translate(Vec2F const& pos);

  auto operator==(RadialForceRegion const& rhs) const -> bool;

  Vec2F center;
  float outerRadius;
  float innerRadius;
  float targetRadialVelocity;
  float controlForce;
  PhysicsCategoryFilter categoryFilter;
};

auto operator>>(DataStream& ds, RadialForceRegion& rfr) -> DataStream&;
auto operator<<(DataStream& ds, RadialForceRegion const& rfr) -> DataStream&;

struct GradientForceRegion {
  static auto fromJson(Json const& json) -> GradientForceRegion;

  [[nodiscard]] auto boundBox() const -> RectF;

  void translate(Vec2F const& pos);

  auto operator==(GradientForceRegion const& rhs) const -> bool;

  PolyF region;
  Line2F gradient;
  float baseTargetVelocity;
  float baseControlForce;
  PhysicsCategoryFilter categoryFilter;
};

auto operator>>(DataStream& ds, GradientForceRegion& rfr) -> DataStream&;
auto operator<<(DataStream& ds, GradientForceRegion const& rfr) -> DataStream&;

using PhysicsForceRegion = Variant<DirectionalForceRegion, RadialForceRegion, GradientForceRegion>;

auto jsonToPhysicsForceRegion(Json const& json) -> PhysicsForceRegion;

}// namespace Star
