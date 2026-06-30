#pragma once

#include "StarItem.hpp"
#include "StarAssets.hpp"

namespace Star {

class CurrencyItem;

class CurrencyItem : public Item {
public:
  CurrencyItem(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, Json const& config, String const& directory);

  [[nodiscard]] ItemPtr clone() const override;

  [[nodiscard]] String pickupSound() const override;

  [[nodiscard]] String currencyType();

  // Value of a single instance of this currency
  [[nodiscard]] uint64_t currencyValue();

  // Total value of all currencies (so currencyValue * count)
  [[nodiscard]] uint64_t totalValue();

private:
  String m_currency;
  uint64_t m_value;
};

}
