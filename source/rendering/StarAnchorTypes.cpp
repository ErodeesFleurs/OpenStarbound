import std;
import star.bi_map;
import star.anchor_types;

namespace Star {

extern EnumMap<HorizontalAnchor> const HorizontalAnchorNames{
    {HorizontalAnchor::LeftAnchor, "left"},
    {HorizontalAnchor::HMidAnchor, "mid"},
    {HorizontalAnchor::RightAnchor, "right"},
};

extern EnumMap<VerticalAnchor> const VerticalAnchorNames{
    {VerticalAnchor::BottomAnchor, "bottom"}, {VerticalAnchor::VMidAnchor, "mid"}, {VerticalAnchor::TopAnchor, "top"},
};
}
