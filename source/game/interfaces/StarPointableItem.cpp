#include "StarGameTypes.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
import star.drawable;
import star.pointable_item;

namespace Star {

float PointableItem::getAngleDir(float angle, Direction) {
  return getAngle(angle);
}

float PointableItem::getAngle(float angle) {
  return angle;
}

}
