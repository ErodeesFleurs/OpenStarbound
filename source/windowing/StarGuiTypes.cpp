import std;
import star.format;
import star.string;
import star.bi_map;
import star.game_types;
import star.gui_types;

namespace Star {

bool ImageStretchSet::fullyPopulated() const {
  return inner.size() && begin.size() && end.size();
}

GuiDirection otherDirection(GuiDirection direction) {
  switch (direction) {
    case GuiDirection::Horizontal:
      return GuiDirection::Vertical;
    case GuiDirection::Vertical:
      return GuiDirection::Horizontal;
    default:
      if constexpr (Star::DebugEnabled) {
        Star::starAssertFailed((false));
      }
      return (GuiDirection)-1;
  }
}

extern EnumMap<GuiDirection> const GuiDirectionNames{
    {GuiDirection::Horizontal, "horizontal"}, {GuiDirection::Vertical, "vertical"},
};

String rarityBorder(String rarity) {
  return (rarity.beginsWith("/") ? rarity : strf("/interface/inventory/itemborder{}.png", rarity)).toLower();
}

}
