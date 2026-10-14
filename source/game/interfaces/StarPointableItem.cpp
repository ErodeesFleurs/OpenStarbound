#include "StarConfig.hpp"
import star.json;
import star.game_types;
import star.string;
import star.data_stream;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
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
