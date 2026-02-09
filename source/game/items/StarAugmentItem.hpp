#pragma once

#include "StarConfig.hpp"
#include "StarItem.hpp"

namespace Star {

class AugmentItem : public Item {
public:
  AugmentItem(Json const& config, String const& directory, Json const& parameters = JsonObject());
  AugmentItem(AugmentItem const& rhs);

  [[nodiscard]] auto clone() const -> Ptr<Item> override;

  [[nodiscard]] auto augmentScripts() const -> StringList;

  // Makes no change to the given item if the augment can't be applied.
  // Consumes itself and returns true if the augment is applied.
  // Has no effect if augmentation fails.
  auto applyTo(Ptr<Item> const item) -> Ptr<Item>;
};

}// namespace Star
