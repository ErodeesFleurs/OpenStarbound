#pragma once

#include "StarConfig.hpp"
#include "StarItem.hpp"

import std;

namespace Star {

class CurrencyItem : public Item {
public:
  CurrencyItem(Json const& config, String const& directory);

  [[nodiscard]] auto clone() const -> Ptr<Item> override;

  [[nodiscard]] auto pickupSound() const -> String override;

  auto currencyType() -> String;

  // Value of a single instance of this currency
  auto currencyValue() -> std::uint64_t;

  // Total value of all currencies (so currencyValue * count)
  auto totalValue() -> std::uint64_t;

private:
  String m_currency;
  std::uint64_t m_value;
};

}// namespace Star
