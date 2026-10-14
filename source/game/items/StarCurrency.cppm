module;
import std;
import star.json;
#include "StarJsonExtra.hpp"
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
import star.random;


import star.drawable;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.item;


export module star.currency_item;

export namespace Star {

class CurrencyItem;
using CurrencyItemPtr = std::shared_ptr<CurrencyItem>;
using CurrencyItemConstPtr = std::shared_ptr<const CurrencyItem>;
using CurrencyItemWeakPtr = std::weak_ptr<CurrencyItem>;
using CurrencyItemConstWeakPtr = std::weak_ptr<const CurrencyItem>;
using CurrencyItemUPtr = std::unique_ptr<CurrencyItem>;
using CurrencyItemConstUPtr = std::unique_ptr<const CurrencyItem>;

class CurrencyItem : public Item {
public:
  CurrencyItem(Json const& config, String const& directory);

  virtual ItemPtr clone() const override;

  virtual String pickupSound() const override;

  String currencyType();

  // Value of a single instance of this currency
  std::uint64_t currencyValue();

  // Total value of all currencies (so currencyValue * count)
  std::uint64_t totalValue();

private:
  String m_currency;
  std::uint64_t m_value;
};

}

namespace Star {

CurrencyItem::CurrencyItem(Json const& config, String const& directory) : Item(config, directory) {
  m_currency = config.getString("currency");
  m_value = config.getUInt("value");
}

ItemPtr CurrencyItem::clone() const {
  return std::make_shared<CurrencyItem>(*this);
}

String CurrencyItem::pickupSound() const {
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

String CurrencyItem::currencyType() {
  return m_currency;
}

std::uint64_t CurrencyItem::currencyValue() {
  return m_value;
}

std::uint64_t CurrencyItem::totalValue() {
  return m_value * count();
}

}
