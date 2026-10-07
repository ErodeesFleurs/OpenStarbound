module;
#include "StarJson.hpp"
#include "StarSet.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarBiMap.hpp"
#include "StarGameTypes.hpp"
#include "StarStrongTypedef.hpp"
#include "StarVector.hpp"

import star.drawable;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.item;

namespace Star {

STAR_CLASS(AugmentItem);

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
