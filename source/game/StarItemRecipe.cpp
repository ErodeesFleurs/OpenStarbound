#include "StarItemRecipe.hpp"

#include "StarJsonExtra.hpp"

import std;

namespace Star {

auto ItemRecipe::toJson() const -> Json {
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

auto ItemRecipe::isNull() const -> bool {
  return currencyInputs.size() == 0 && inputs.size() == 0 && output.isNull();
}

auto ItemRecipe::operator==(ItemRecipe const& rhs) const -> bool {
  return std::tie(currencyInputs, inputs, output) == std::tie(rhs.currencyInputs, rhs.inputs, rhs.output);
}

auto ItemRecipe::operator!=(ItemRecipe const& rhs) const -> bool {
  return std::tie(currencyInputs, inputs, output) != std::tie(rhs.currencyInputs, rhs.inputs, rhs.output);
}

auto operator<<(std::ostream& os, ItemRecipe const& recipe) -> std::ostream& {
  os << "CurrencyInputs: " << recipe.currencyInputs << "Inputs: " << recipe.inputs << "\nOutput: " << recipe.output
      << "\nDuration: " << recipe.duration << "\nGroups: " << recipe.groups;
  return os;
}

auto hash<ItemRecipe>::operator()(ItemRecipe const& v) const -> std::size_t {
  return hashOf(v.currencyInputs.keys(), v.currencyInputs.values(), v.inputs, v.output);
}

}
