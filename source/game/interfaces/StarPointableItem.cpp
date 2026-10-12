#include "StarJson.hpp"
import star.game_types;
#include "StarString.hpp"
#include "StarDataStream.hpp"
import star.color;
#include "StarList.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
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
