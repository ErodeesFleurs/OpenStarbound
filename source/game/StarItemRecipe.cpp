#include "StarItemRecipe.hpp"
#include "StarRoot.hpp"
#include "StarItemDatabase.hpp"
#include "StarJsonExtra.hpp"

namespace Star {

[[nodiscard]] Json ItemRecipe::toJson() const {
  JsonArray inputList;
  inputList.reserve(inputs.size());
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

[[nodiscard]] bool ItemRecipe::isNull() const {
  return currencyInputs.empty() && inputs.empty() && output.isNull();
}

[[nodiscard]] bool ItemRecipe::operator==(ItemRecipe const& rhs) const {
  return std::tie(currencyInputs, inputs, output) == std::tie(rhs.currencyInputs, rhs.inputs, rhs.output);
}

[[nodiscard]] bool ItemRecipe::operator!=(ItemRecipe const& rhs) const {
  return std::tie(currencyInputs, inputs, output) != std::tie(rhs.currencyInputs, rhs.inputs, rhs.output);
}

std::ostream& operator<<(std::ostream& os, ItemRecipe const& recipe) {
  os << "CurrencyInputs: " << recipe.currencyInputs << "Inputs: " << recipe.inputs << "\nOutput: " << recipe.output
      << "\nDuration: " << recipe.duration << "\nGroups: " << recipe.groups;
  return os;
}

[[nodiscard]] size_t hash<ItemRecipe>::operator()(ItemRecipe const& v) const {
  return hashOf(v.currencyInputs.keys(), v.currencyInputs.values(), v.inputs, v.output);
}

}
