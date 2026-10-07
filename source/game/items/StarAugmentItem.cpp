#include "StarJsonExtra.hpp"
#include "StarJson.hpp"
#include "StarSet.hpp"
#include "StarString.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarStrongTypedef.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarBiMap.hpp"
#include "StarGameTypes.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarThread.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
import star.listener;
#include "StarConfig.hpp"
import star.version;

#include "StarLuaComponents.hpp"
import star.drawable;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.item;
import star.item_descriptor;
import star.item_recipe;
import star.augment_item;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

import star.config_lua_bindings;
import star.item_database;


namespace Star {

AugmentItem::AugmentItem(Json const& config, String const& directory, Json const& parameters)
  : Item(config, directory, parameters) {}

AugmentItem::AugmentItem(AugmentItem const& rhs) : AugmentItem(rhs.config(), rhs.directory(), rhs.parameters()) {}

ItemPtr AugmentItem::clone() const {
  return make_shared<AugmentItem>(*this);
}

StringList AugmentItem::augmentScripts() const {
  return jsonToStringList(instanceValue("scripts")).transformed(bind(&AssetPath::relativeTo, directory(), _1));
}

ItemPtr AugmentItem::applyTo(ItemPtr const item) {
  return Root::singleton().itemDatabase()->applyAugment(item, this);
}

}
