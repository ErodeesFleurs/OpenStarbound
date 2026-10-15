import star.json;
import star.json_extra;
import star.set;
import star.string;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.strong_typedef;
import star.bi_map;
import star.game_types;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.version;

import star.lua_components;
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

std::shared_ptr<Item> AugmentItem::clone() const {
  return std::make_shared<AugmentItem>(*this);
}

StringList AugmentItem::augmentScripts() const {
  return jsonToStringList(instanceValue("scripts")).transformed(std::bind(&AssetPath::relativeTo, directory(), std::placeholders::_1));
}

std::shared_ptr<Item> AugmentItem::applyTo(std::shared_ptr<Item> const item) {
  return Root::singleton().itemDatabase()->applyAugment(item, this);
}

}
