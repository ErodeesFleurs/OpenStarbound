#include "StarAugmentItem.hpp"

#include "StarConfig.hpp"
#include "StarItemDatabase.hpp"
#include "StarItemLuaBindings.hpp"
#include "StarJsonExtra.hpp"
#include "StarRoot.hpp"

import std;

namespace Star {

AugmentItem::AugmentItem(Json const& config, String const& directory, Json const& parameters)
    : Item(config, directory, parameters) {}

AugmentItem::AugmentItem(AugmentItem const& rhs) : AugmentItem(rhs.config(), rhs.directory(), rhs.parameters()) {}

auto AugmentItem::clone() const -> Ptr<Item> {
  return std::make_shared<AugmentItem>(*this);
}

auto AugmentItem::augmentScripts() const -> StringList {
  return jsonToStringList(instanceValue("scripts")).transformed([capture0 = directory()](auto&& PH1) -> auto { return AssetPath::relativeTo(capture0, std::forward<decltype(PH1)>(PH1)); });
}

auto AugmentItem::applyTo(Ptr<Item> const item) -> Ptr<Item> {
  return Root::singleton().itemDatabase()->applyAugment(item, this);
}

}// namespace Star
