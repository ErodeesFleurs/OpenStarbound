module;

import std;
import star.format;
import star.bi_map;

namespace Star {

enum class HorizontalAnchor {
  LeftAnchor,
  HMidAnchor,
  RightAnchor
};
extern std::expected<EnumMap<HorizontalAnchor>, StarException> const HorizontalAnchorNames;

enum class VerticalAnchor {
  BottomAnchor,
  VMidAnchor,
  TopAnchor
};
extern std::expected<EnumMap<VerticalAnchor>, StarException> const VerticalAnchorNames;

}

export module star.anchor_types;

export namespace Star {
  using ::Star::HorizontalAnchor;
  using ::Star::HorizontalAnchorNames;
  using ::Star::VerticalAnchor;
  using ::Star::VerticalAnchorNames;
}
