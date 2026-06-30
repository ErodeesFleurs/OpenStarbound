#include "StarAugmentItem.hpp"
#include "StarAssets.hpp"
#include "StarItemDatabase.hpp"
#include "StarLuaComponents.hpp"
#include "StarItemLuaBindings.hpp"
#include "StarConfigLuaBindings.hpp"
#include "StarJsonExtra.hpp"

namespace Star {

AugmentItem::AugmentItem(IAssetsConstPtr assets, Json const& config, String const& directory, ItemDatabase const* itemDatabase, Json const& parameters)
  : Item(std::move(assets), config, directory, parameters), m_itemDatabase(itemDatabase) {}

AugmentItem::AugmentItem(AugmentItem const& rhs) = default;

ItemPtr AugmentItem::clone() const {
  return make_shared<AugmentItem>(*this);
}

StringList AugmentItem::augmentScripts() const {
  return jsonToStringList(instanceValue("scripts")).transformed([dir = directory()](String const& s) { return AssetPath::relativeTo(dir, s); });
}

ItemPtr AugmentItem::applyTo(ItemPtr const item) {
  return m_itemDatabase->applyAugment(item, this);
}

}
