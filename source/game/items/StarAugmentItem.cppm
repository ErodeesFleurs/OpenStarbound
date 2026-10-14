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
#include "StarStrongTypedef.hpp"

import star.drawable;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.item;

namespace Star {

class AugmentItem;
using AugmentItemPtr = std::shared_ptr<AugmentItem>;
using AugmentItemConstPtr = std::shared_ptr<const AugmentItem>;
using AugmentItemWeakPtr = std::weak_ptr<AugmentItem>;
using AugmentItemConstWeakPtr = std::weak_ptr<const AugmentItem>;
using AugmentItemUPtr = std::unique_ptr<AugmentItem>;
using AugmentItemConstUPtr = std::unique_ptr<const AugmentItem>;

class AugmentItem : public Item {
public:
  AugmentItem(Json const& config, String const& directory, Json const& parameters = JsonObject());
  AugmentItem(AugmentItem const& rhs);

  ItemPtr clone() const override;

  StringList augmentScripts() const;

  // Makes no change to the given item if the augment can't be applied.
  // Consumes itself and returns true if the augment is applied.
  // Has no effect if augmentation fails.
  ItemPtr applyTo(ItemPtr const item);
};

}

export module star.augment_item;

export namespace Star {
  using ::Star::AugmentItem;
  using ::Star::AugmentItemPtr;
  using ::Star::AugmentItemConstPtr;
  using ::Star::AugmentItemWeakPtr;
  using ::Star::AugmentItemConstWeakPtr;
  using ::Star::AugmentItemUPtr;
  using ::Star::AugmentItemConstUPtr;
}
