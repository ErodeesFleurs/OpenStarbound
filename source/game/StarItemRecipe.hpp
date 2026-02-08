#pragma once

#include "StarException.hpp"
#include "StarItemDescriptor.hpp"
#include "StarGameTypes.hpp"

import std;

namespace Star {

using RecipeException = ExceptionDerived<"RecipeException">;

struct ItemRecipe {
  auto toJson() const -> Json;

  auto isNull() const -> bool;

  auto operator==(ItemRecipe const& rhs) const -> bool;
  auto operator!=(ItemRecipe const& rhs) const -> bool;

  StringMap<std::uint64_t> currencyInputs;
  List<ItemDescriptor> inputs;
  ItemDescriptor output;
  float duration;
  StringSet groups;
  Rarity outputRarity;
  String guiFilterString;
  StringMap<String> collectables;
  bool matchInputParameters;
};

template <>
struct hash<ItemRecipe> {
  auto operator()(ItemRecipe const& v) const -> std::size_t;
};

auto operator<<(std::ostream& os, ItemRecipe const& recipe) -> std::ostream&;
}

template <> struct std::formatter<Star::ItemRecipe> : Star::ostream_formatter {};
