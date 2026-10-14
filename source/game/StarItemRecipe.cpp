import star.hash;
#include "StarJsonExtra.hpp"
import star.json;
import star.item_recipe;
import star.set;
import star.string;
import star.color;
import star.list;
import star.image_processing;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
#include "StarStrongTypedef.hpp"
import star.bi_map;
import star.game_types;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
#include "StarConfig.hpp"
import star.version;

import star.drawable;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.item;
import star.item_descriptor;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;


import star.item_database;

namespace Star {

Json ItemRecipe::toJson() const {
  JsonArray inputList;
  inputList.reserve(inputList.size());
  for (auto& input : inputs)
    inputList.append(input.toJson());

  return JsonObject{
      {"currencyInputs", jsonFromMap(currencyInputs)},
      {"input", inputList},
      {"output", output.toJson()},
      {"duration", duration},
      {"groups", jsonFromStringSet(groups)},
      {"collectables", jsonFromMap(collectables)},
      {"matchInputParameters", matchInputParameters}
    };
}

bool ItemRecipe::isNull() const {
  return currencyInputs.size() == 0 && inputs.size() == 0 && output.isNull();
}

bool ItemRecipe::operator==(ItemRecipe const& rhs) const {
  return std::tie(currencyInputs, inputs, output) == std::tie(rhs.currencyInputs, rhs.inputs, rhs.output);
}

bool ItemRecipe::operator!=(ItemRecipe const& rhs) const {
  return std::tie(currencyInputs, inputs, output) != std::tie(rhs.currencyInputs, rhs.inputs, rhs.output);
}

std::ostream& operator<<(std::ostream& os, ItemRecipe const& recipe) {
  os << "CurrencyInputs: " << recipe.currencyInputs << "Inputs: " << recipe.inputs << "\nOutput: " << recipe.output
      << "\nDuration: " << recipe.duration << "\nGroups: " << recipe.groups;
  return os;
}

size_t hash<ItemRecipe>::operator()(ItemRecipe const& v) const {
  return hashOf(v.currencyInputs.keys(), v.currencyInputs.values(), v.inputs, v.output);
}

}
