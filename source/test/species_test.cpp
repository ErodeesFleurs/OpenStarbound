#include "StarAssets.hpp"
#include "StarRoot.hpp"
#include "StarSpeciesDatabase.hpp"

#include "gtest/gtest.h"

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
