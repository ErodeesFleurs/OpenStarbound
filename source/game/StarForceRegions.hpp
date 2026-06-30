#pragma once

#include "StarPoly.hpp"
#include "StarVariant.hpp"
#include "StarJson.hpp"

namespace Star {

struct PhysicsCategoryFilter {
  enum Type { Whitelist, Blacklist };

  [[nodiscard]] static PhysicsCategoryFilter whitelist(StringSet categories);
  [[nodiscard]] static PhysicsCategoryFilter blacklist(StringSet categories);

  PhysicsCategoryFilter(Type type = Blacklist, StringSet categories = {});

  [[nodiscard]] bool check(StringSet const& categories) const;

  bool operator==(PhysicsCategoryFilter const& rhs) const;

  Type type;
  StringSet categories;
};

DataStream& operator>>(DataStream& ds, PhysicsCategoryFilter& rfr);
DataStream& operator<<(DataStream& ds, PhysicsCategoryFilter const& rfr);

[[nodiscard]] PhysicsCategoryFilter jsonToPhysicsCategoryFilter(Json const& json);

struct DirectionalForceRegion {
  [[nodiscard]] static DirectionalForceRegion fromJson(Json const& json);

  [[nodiscard]] RectF boundBox() const;

  void translate(Vec2F const& pos);

  bool operator==(DirectionalForceRegion const& rhs) const;

  PolyF region;
  Maybe<float> xTargetVelocity;
  Maybe<float> yTargetVelocity;
  float controlForce;
  PhysicsCategoryFilter categoryFilter;
};

DataStream& operator>>(DataStream& ds, DirectionalForceRegion& rfr);
DataStream& operator<<(DataStream& ds, DirectionalForceRegion const& rfr);

struct RadialForceRegion {
  [[nodiscard]] static RadialForceRegion fromJson(Json const& json);

  [[nodiscard]] RectF boundBox() const;

  void translate(Vec2F const& pos);

  bool operator==(RadialForceRegion const& rhs) const;

  Vec2F center;
  float outerRadius;
  float innerRadius;
  float targetRadialVelocity;
  float controlForce;
  PhysicsCategoryFilter categoryFilter;
};

DataStream& operator>>(DataStream& ds, RadialForceRegion& rfr);
DataStream& operator<<(DataStream& ds, RadialForceRegion const& rfr);

struct GradientForceRegion {
  [[nodiscard]] static GradientForceRegion fromJson(Json const& json);

  [[nodiscard]] RectF boundBox() const;

  void translate(Vec2F const& pos);

  bool operator==(GradientForceRegion const& rhs) const;

  PolyF region;
  Line2F gradient;
  float baseTargetVelocity;
  float baseControlForce;
  PhysicsCategoryFilter categoryFilter;
};

DataStream& operator>>(DataStream& ds, GradientForceRegion& rfr);
DataStream& operator<<(DataStream& ds, GradientForceRegion const& rfr);

using PhysicsForceRegion = Variant<DirectionalForceRegion, RadialForceRegion, GradientForceRegion>;

[[nodiscard]] PhysicsForceRegion jsonToPhysicsForceRegion(Json const& json);

}
