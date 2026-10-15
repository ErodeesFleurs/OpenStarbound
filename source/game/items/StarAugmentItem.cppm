module;
import std;
import star.json;
import star.set;
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
import star.bi_map;
import star.game_types;
import star.strong_typedef;

import star.drawable;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.item;

namespace Star {

class AugmentItem : public Item {
public:
  AugmentItem(Json const& config, String const& directory, Json const& parameters = JsonObject());
  AugmentItem(AugmentItem const& rhs);

  std::shared_ptr<Item> clone() const override;

  StringList augmentScripts() const;

  // Makes no change to the given item if the augment can't be applied.
  // Consumes itself and returns true if the augment is applied.
  // Has no effect if augmentation fails.
  std::shared_ptr<Item> applyTo(std::shared_ptr<Item> const item);
};

}

export module star.augment_item;

export namespace Star {
  using ::Star::AugmentItem;
}
