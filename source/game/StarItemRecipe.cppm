module;

#include "StarItemDescriptor.hpp"
#include "StarGameTypes.hpp"

namespace Star {

struct RecipeExceptionTag {
  static constexpr char const* name() { return "RecipeException"; }
};
using RecipeException = StarError<RecipeExceptionTag, StarException>;

struct ItemRecipe {
  friend std::ostream& operator<<(std::ostream& os, ItemRecipe const& recipe);
  Json toJson() const;

  bool isNull() const;

  bool operator==(ItemRecipe const& rhs) const;
  bool operator!=(ItemRecipe const& rhs) const;

  StringMap<uint64_t> currencyInputs;
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
  size_t operator()(ItemRecipe const& v) const;
};

std::ostream& operator<<(std::ostream& os, ItemRecipe const& recipe);
}

template <> struct fmt::formatter<Star::ItemRecipe> : ostream_formatter {};

export module star.item_recipe;

export namespace Star {
  using ::Star::RecipeExceptionTag;
  using ::Star::RecipeException;
  using ::Star::ItemRecipe;
}
