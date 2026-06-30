#pragma once

#include "StarItem.hpp"
#include "StarAssets.hpp"

namespace Star {

class CurrencyItem;

class CurrencyItem : public Item {
public:
  CurrencyItem(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, Json const& config, String const& directory);

  ItemPtr clone() const override;

  String pickupSound() const override;

  String currencyType();

  // Value of a single instance of this currency
  uint64_t currencyValue();

  // Total value of all currencies (so currencyValue * count)
  uint64_t totalValue();

private:
  String m_currency;
  uint64_t m_value;
};

}
