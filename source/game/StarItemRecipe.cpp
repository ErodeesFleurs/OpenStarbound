#include "StarJsonExtra.hpp"
#include "StarJson.hpp"
#include "StarSet.hpp"
#include "StarString.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarAssetPath.hpp"
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
#include "StarListener.hpp"
#include "StarVersion.hpp"

import star.drawable;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.item;
import star.item_descriptor;
import star.item_recipe;
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
