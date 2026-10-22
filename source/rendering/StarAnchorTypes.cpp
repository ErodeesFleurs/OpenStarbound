import std;
import star.format;
import star.bi_map;
import star.anchor_types;

namespace Star {

extern std::expected<EnumMap<HorizontalAnchor>, StarException> const HorizontalAnchorNames = EnumMap<HorizontalAnchor>::create({
    {HorizontalAnchor::LeftAnchor, "left"},
    {HorizontalAnchor::HMidAnchor, "mid"},
    {HorizontalAnchor::RightAnchor, "right"},
});

extern std::expected<EnumMap<VerticalAnchor>, StarException> const VerticalAnchorNames = EnumMap<VerticalAnchor>::create({
    {VerticalAnchor::BottomAnchor, "bottom"}, {VerticalAnchor::VMidAnchor, "mid"}, {VerticalAnchor::TopAnchor, "top"},
});
}
