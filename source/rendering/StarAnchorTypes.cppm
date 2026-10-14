module;

import std;
import star.bi_map;

namespace Star {

enum class HorizontalAnchor {
  LeftAnchor,
  HMidAnchor,
  RightAnchor
};
extern EnumMap<HorizontalAnchor> const HorizontalAnchorNames;

enum class VerticalAnchor {
  BottomAnchor,
  VMidAnchor,
  TopAnchor
};
extern EnumMap<VerticalAnchor> const VerticalAnchorNames;

}

export module star.anchor_types;

export namespace Star {
  using ::Star::HorizontalAnchor;
  using ::Star::HorizontalAnchorNames;
  using ::Star::VerticalAnchor;
  using ::Star::VerticalAnchorNames;
}
