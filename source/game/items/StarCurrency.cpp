#include "StarCurrency.hpp"

#include "StarConfig.hpp"
#include "StarJsonExtra.hpp"
#include "StarRandom.hpp"

import std;

namespace Star {

CurrencyItem::CurrencyItem(Json const& config, String const& directory) : Item(config, directory) {
  m_currency = config.getString("currency");
  m_value = config.getUInt("value");
}

auto CurrencyItem::clone() const -> Ptr<Item> {
  return std::make_shared<CurrencyItem>(*this);
}

auto CurrencyItem::pickupSound() const -> String {
  if (count() <= instanceValue("smallStackLimit", 100).toUInt()) {
    if (!instanceValue("pickupSoundsSmall", {}).isNull())
      return Random::randFrom(jsonToStringSet(instanceValue("pickupSoundsSmall")));
  } else if (count() <= instanceValue("mediumStackLimit", 10000).toUInt()) {
    if (!instanceValue("pickupSoundsMedium", {}).isNull())
      return Random::randFrom(jsonToStringSet(instanceValue("pickupSoundsMedium")));
  } else {
    if (!instanceValue("pickupSoundsLarge", {}).isNull())
      return Random::randFrom(jsonToStringSet(instanceValue("pickupSoundsLarge")));
  }
  return Item::pickupSound();
}

auto CurrencyItem::currencyType() -> String {
  return m_currency;
}

auto CurrencyItem::currencyValue() -> std::uint64_t {
  return m_value;
}

auto CurrencyItem::totalValue() -> std::uint64_t {
  return m_value * count();
}

}// namespace Star
