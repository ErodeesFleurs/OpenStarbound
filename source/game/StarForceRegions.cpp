#include "StarForceRegions.hpp"
#include "StarDataStreamExtra.hpp"
#include "StarJsonExtra.hpp"

import std;

namespace Star {

auto PhysicsCategoryFilter::whitelist(StringSet categories) -> PhysicsCategoryFilter {
  return {Whitelist, std::move(categories)};
}

auto PhysicsCategoryFilter::blacklist(StringSet categories) -> PhysicsCategoryFilter {
  return {Blacklist, std::move(categories)};
}

PhysicsCategoryFilter::PhysicsCategoryFilter(Type type, StringSet categories)
    : type(type), categories(std::move(categories)) {}

auto PhysicsCategoryFilter::check(StringSet const& otherCategories) const -> bool {
  bool intersection = categories.hasIntersection(otherCategories);
  if (type == Whitelist)
    return intersection;
  else
    return !intersection;
}

auto PhysicsCategoryFilter::operator==(PhysicsCategoryFilter const& rhs) const -> bool {
  return tie(type, categories) == tie(rhs.type, rhs.categories);
}

auto operator>>(DataStream& ds, PhysicsCategoryFilter& pcf) -> DataStream& {
  ds >> pcf.type;
  ds >> pcf.categories;
  return ds;
}

auto operator<<(DataStream& ds, PhysicsCategoryFilter const& pcf) -> DataStream& {
  ds << pcf.type;
  ds << pcf.categories;
  return ds;
}

auto jsonToPhysicsCategoryFilter(Json const& json) -> PhysicsCategoryFilter {
  auto whitelist = json.opt("categoryWhitelist");
  auto blacklist = json.opt("categoryBlacklist");
  if (whitelist && blacklist)
    throw JsonException::format("Cannot specify both a physics category whitelist and blacklist");
  if (whitelist)
    return PhysicsCategoryFilter::whitelist(jsonToStringSet(*whitelist));
  if (blacklist)
    return PhysicsCategoryFilter::blacklist(jsonToStringSet(*blacklist));
  return {};
}

auto DirectionalForceRegion::fromJson(Json const& json) -> DirectionalForceRegion {
  DirectionalForceRegion dfr;
  if (json.contains("polyRegion"))
    dfr.region = jsonToPolyF(json.get("polyRegion"));
  else
    dfr.region = PolyF(jsonToRectF(json.get("rectRegion")));
  dfr.xTargetVelocity = json.optFloat("xTargetVelocity");
  dfr.yTargetVelocity = json.optFloat("yTargetVelocity");
  dfr.controlForce = json.getFloat("controlForce");
  dfr.categoryFilter = jsonToPhysicsCategoryFilter(json);
  return dfr;
}

auto DirectionalForceRegion::boundBox() const -> RectF {
  return region.boundBox();
}

void DirectionalForceRegion::translate(Vec2F const& pos) {
  region.translate(pos);
}

auto DirectionalForceRegion::operator==(DirectionalForceRegion const& rhs) const -> bool {
  return tie(region, xTargetVelocity, yTargetVelocity, controlForce, categoryFilter)
    == tie(rhs.region, rhs.xTargetVelocity, rhs.yTargetVelocity, rhs.controlForce, rhs.categoryFilter);
}

auto operator>>(DataStream& ds, DirectionalForceRegion& dfr) -> DataStream& {
  ds >> dfr.region;
  ds >> dfr.xTargetVelocity;
  ds >> dfr.yTargetVelocity;
  ds >> dfr.controlForce;
  ds >> dfr.categoryFilter;
  return ds;
}

auto operator<<(DataStream& ds, DirectionalForceRegion const& dfr) -> DataStream& {
  ds << dfr.region;
  ds << dfr.xTargetVelocity;
  ds << dfr.yTargetVelocity;
  ds << dfr.controlForce;
  ds << dfr.categoryFilter;
  return ds;
}

auto RadialForceRegion::fromJson(Json const& json) -> RadialForceRegion {
  RadialForceRegion rfr;
  rfr.center = json.opt("center").transform(jsonToVec2F).value();
  rfr.outerRadius = json.getFloat("outerRadius");
  rfr.innerRadius = json.getFloat("innerRadius");
  rfr.targetRadialVelocity = json.getFloat("targetRadialVelocity");
  rfr.controlForce = json.getFloat("controlForce");
  rfr.categoryFilter = jsonToPhysicsCategoryFilter(json);
  return rfr;
}

auto RadialForceRegion::boundBox() const -> RectF {
  return RectF::withCenter(center, Vec2F::filled(outerRadius));
}

void RadialForceRegion::translate(Vec2F const& pos) {
  center += pos;
}

auto RadialForceRegion::operator==(RadialForceRegion const& rhs) const -> bool {
  return tie(center, outerRadius, innerRadius, targetRadialVelocity, controlForce, categoryFilter)
    == tie(rhs.center, rhs.outerRadius, rhs.innerRadius, rhs.targetRadialVelocity, rhs.controlForce, rhs.categoryFilter);
}

auto operator>>(DataStream& ds, RadialForceRegion& rfr) -> DataStream& {
  ds >> rfr.center;
  ds >> rfr.outerRadius;
  ds >> rfr.innerRadius;
  ds >> rfr.targetRadialVelocity;
  ds >> rfr.controlForce;
  ds >> rfr.categoryFilter;
  return ds;
}

auto operator<<(DataStream& ds, RadialForceRegion const& gfr) -> DataStream& {
  ds << gfr.center;
  ds << gfr.outerRadius;
  ds << gfr.innerRadius;
  ds << gfr.targetRadialVelocity;
  ds << gfr.controlForce;
  ds << gfr.categoryFilter;
  return ds;
}

auto GradientForceRegion::fromJson(Json const& json) -> GradientForceRegion {
  GradientForceRegion gfr;
  if (json.contains("polyRegion"))
    gfr.region = jsonToPolyF(json.get("polyRegion"));
  else
    gfr.region = PolyF(jsonToRectF(json.get("rectRegion")));
  gfr.gradient = jsonToLine2F(json.get("gradient"));
  gfr.baseTargetVelocity = json.getFloat("baseTargetVelocity");
  gfr.baseControlForce = json.getFloat("baseControlForce");
  gfr.categoryFilter = jsonToPhysicsCategoryFilter(json);
  return gfr;
}

auto GradientForceRegion::boundBox() const -> RectF {
  return region.boundBox();
}

void GradientForceRegion::translate(Vec2F const& pos) {
  region.translate(pos);
  gradient.translate(pos);
}

auto GradientForceRegion::operator==(GradientForceRegion const& rhs) const -> bool {
  return std::tie(region, gradient, baseTargetVelocity, baseControlForce, categoryFilter)
    == std::tie(rhs.region, rhs.gradient, rhs.baseTargetVelocity, rhs.baseControlForce, rhs.categoryFilter);
}

auto operator>>(DataStream& ds, GradientForceRegion& gfr) -> DataStream& {
  ds >> gfr.region;
  ds >> gfr.gradient;
  ds >> gfr.baseTargetVelocity;
  ds >> gfr.baseControlForce;
  ds >> gfr.categoryFilter;
  return ds;
}

auto operator<<(DataStream& ds, GradientForceRegion const& gfr) -> DataStream& {
  ds << gfr.region;
  ds << gfr.gradient;
  ds << gfr.baseTargetVelocity;
  ds << gfr.baseControlForce;
  ds << gfr.categoryFilter;
  return ds;
}

auto jsonToPhysicsForceRegion(Json const& json) -> PhysicsForceRegion {
  String type = json.getString("type");
  if (type.equalsIgnoreCase("DirectionalForceRegion"))
    return DirectionalForceRegion::fromJson(json);
  else if (type.equalsIgnoreCase("RadialForceRegion"))
    return RadialForceRegion::fromJson(json);
  else if (type.equalsIgnoreCase("GradientForceRegion"))
    return GradientForceRegion::fromJson(json);
  else
    throw JsonException::format("No such physics force region type '{}'", type);
}

}// namespace Star
