module;

import std;
import star.format;
import star.string;
import star.vector;
import star.bi_map;
import star.game_types;

namespace Star {

struct ImageStretchSet {
  // how does inner fill up space?
  enum class ImageStretchType {
    Stretch,
    Repeat
  };

  String begin;
  String inner;
  String end;
  ImageStretchType type;

  bool fullyPopulated() const;
};

enum class GuiDirection {
  Horizontal,
  Vertical
};
extern std::expected<EnumMap<GuiDirection>, StarException> const GuiDirectionNames;

GuiDirection otherDirection(GuiDirection direction);

template <typename T>
T directionalValueFromVector(GuiDirection direction, Vector<T, 2> const& vec);

[[nodiscard]] std::expected<String, StarException> rarityBorder(String border);

template <typename T>
T& directionalValueFromVector(GuiDirection direction, Vector<T, 2> const& vec) {
  switch (direction) {
    case GuiDirection::Horizontal:
      return vec[0];
    case GuiDirection::Vertical:
      return vec[1];
  }
}

}

export module star.gui_types;

export namespace Star {
  using ::Star::ImageStretchSet;
  using ::Star::GuiDirection;
  using ::Star::GuiDirectionNames;
  using ::Star::otherDirection;
  using ::Star::directionalValueFromVector;
  using ::Star::rarityBorder;
}
