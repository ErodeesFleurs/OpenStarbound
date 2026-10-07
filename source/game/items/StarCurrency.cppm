module;
#include "StarJson.hpp"
#include "StarJsonExtra.hpp"
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
#include "StarRandom.hpp"


import star.drawable;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.item;


export module star.currency_item;

export namespace Star {

STAR_CLASS(CurrencyItem);

class CurrencyItem : public Item {
public:
  CurrencyItem(Json const& config, String const& directory);

  virtual ItemPtr clone() const override;

  virtual String pickupSound() const override;

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

namespace Star {

CurrencyItem::CurrencyItem(Json const& config, String const& directory) : Item(config, directory) {
  m_currency = config.getString("currency");
  m_value = config.getUInt("value");
}

ItemPtr CurrencyItem::clone() const {
  return make_shared<CurrencyItem>(*this);
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

uint64_t CurrencyItem::currencyValue() {
  return m_value;
}

uint64_t CurrencyItem::totalValue() {
  return m_value * count();
}

}
