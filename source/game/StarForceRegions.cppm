module;

#include "StarPoly.hpp"
#include "StarVariant.hpp"
#include "StarJson.hpp"

namespace Star {

struct PhysicsCategoryFilter {
  friend DataStream& operator>>(DataStream& ds, PhysicsCategoryFilter& region);
  friend DataStream& operator<<(DataStream& ds, PhysicsCategoryFilter const& region);
  enum Type { Whitelist, Blacklist };

  static PhysicsCategoryFilter whitelist(StringSet categories);
  static PhysicsCategoryFilter blacklist(StringSet categories);

  PhysicsCategoryFilter(Type type = Blacklist, StringSet categories = {});

  bool check(StringSet const& categories) const;

  bool operator==(PhysicsCategoryFilter const& rhs) const;

  Type type;
  StringSet categories;
};

DataStream& operator>>(DataStream& ds, PhysicsCategoryFilter& rfr);
DataStream& operator<<(DataStream& ds, PhysicsCategoryFilter const& rfr);

PhysicsCategoryFilter jsonToPhysicsCategoryFilter(Json const& json);

struct DirectionalForceRegion {
  friend DataStream& operator>>(DataStream& ds, DirectionalForceRegion& region);
  friend DataStream& operator<<(DataStream& ds, DirectionalForceRegion const& region);
  static DirectionalForceRegion fromJson(Json const& json);

  RectF boundBox() const;

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
  friend DataStream& operator>>(DataStream& ds, RadialForceRegion& region);
  friend DataStream& operator<<(DataStream& ds, RadialForceRegion const& region);
  static RadialForceRegion fromJson(Json const& json);

  RectF boundBox() const;

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
  friend DataStream& operator>>(DataStream& ds, GradientForceRegion& region);
  friend DataStream& operator<<(DataStream& ds, GradientForceRegion const& region);
  static GradientForceRegion fromJson(Json const& json);

  RectF boundBox() const;

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

typedef Variant<DirectionalForceRegion, RadialForceRegion, GradientForceRegion> PhysicsForceRegion;

PhysicsForceRegion jsonToPhysicsForceRegion(Json const& json);

}

export module star.force_regions;

export namespace Star {
  using ::Star::PhysicsCategoryFilter;
  using ::Star::DirectionalForceRegion;
  using ::Star::RadialForceRegion;
  using ::Star::GradientForceRegion;
  using ::Star::PhysicsForceRegion;
  using ::Star::jsonToPhysicsCategoryFilter;
  using ::Star::jsonToPhysicsForceRegion;
}
