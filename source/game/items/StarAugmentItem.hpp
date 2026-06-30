#pragma once

#include "StarItem.hpp"
#include "StarAssets.hpp"

namespace Star {

class AugmentItem;
class ItemDatabase;

class AugmentItem : public Item {
public:
  AugmentItem(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, Json const& config, String const& directory, ItemDatabase const& itemDatabase, Json const& parameters = JsonObject());
  AugmentItem(AugmentItem const& rhs);

  [[nodiscard]] ItemPtr clone() const override;

  [[nodiscard]] StringList augmentScripts() const;

  // Makes no change to the given item if the augment can't be applied.
  // Consumes itself and returns true if the augment is applied.
  // Has no effect if augmentation fails.
  [[nodiscard]] ItemPtr applyTo(ItemPtr const item);

private:
  ItemDatabase const& m_itemDatabase;
};

}
