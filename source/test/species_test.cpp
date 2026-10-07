#include "StarJson.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarBiMap.hpp"
#include "StarThread.hpp"
#include "StarIODevice.hpp"
#include "StarAssetPath.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarListener.hpp"
#include "StarVersion.hpp"

import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

#include "gtest/gtest.h"

import star.species_database;

using namespace Star;

TEST(SpeciesTest, NameGenAndOuchNoisesMustCoverEveryGender) {
  auto assets = Root::singleton().assets();
  auto speciesFiles = assets->scanExtension("species");
  ASSERT_FALSE(speciesFiles.empty());

  Json config = assets->json(*speciesFiles.begin());
  EXPECT_NO_THROW((void)SpeciesDefinition(config));

  // Both lists are indexed by Gender, so a species has to provide an entry for
  // every gender.  A shorter list used to be read with operator[], which is out
  // of bounds (upstream #599: an empty ouchNoises array was an access
  // violation).
  for (auto key : {String("nameGen"), String("ouchNoises")}) {
    EXPECT_THROW((void)SpeciesDefinition(config.set(key, JsonArray())), StarException);
    EXPECT_THROW((void)SpeciesDefinition(config.set(key, JsonArray{config.get(key).get(0)})), StarException);
  }
}
