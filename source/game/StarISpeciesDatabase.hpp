#pragma once

#include "StarGameTypes.hpp"
#include "StarHumanoid.hpp"
#include "StarString.hpp"

namespace Star {

class SpeciesDefinition;
using SpeciesDefinitionPtr = SharedPtr<SpeciesDefinition>;

class ISpeciesDatabase {
public:
  virtual ~ISpeciesDatabase() = default;

  virtual bool hasSpecies(String const& kind) const = 0;
  virtual SpeciesDefinitionPtr species(String const& kind) const = 0;

  virtual StringList speciesNames() const = 0;
};

using ISpeciesDatabasePtr = SharedPtr<ISpeciesDatabase>;
using ISpeciesDatabaseConstPtr = SharedPtr<ISpeciesDatabase const>;

}
