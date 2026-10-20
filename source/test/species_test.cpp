import star.json;
import star.string;
import star.ordered_map;
import star.bi_map;
import star.thread;
import star.io_device;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.directives;
import star.asset_path;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.format;
import star.version;

import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

#include "gtest/gtest.h"

import star.species_database;
import star.game_test;

using namespace Star;

const GameTestRegistration speciesGenderCoverage("SpeciesTest", "NameGenAndOuchNoisesMustCoverEveryGender", [](GameTest& fixture) {
  auto assets = fixture.root().assets();
  auto speciesFiles = assets->scanExtension("species");
  ASSERT_FALSE(speciesFiles.empty());

  Json config = assets->json(*speciesFiles.begin());
  EXPECT_NO_THROW((void)SpeciesDefinition(fixture.root(), config));

  // Both lists are indexed by Gender, so a species has to provide an entry for
  // every gender.  A shorter list used to be read with operator[], which is out
  // of bounds (upstream #599: an empty ouchNoises array was an access
  // violation).
  for (auto key : {String("nameGen"), String("ouchNoises")}) {
    EXPECT_THROW((void)SpeciesDefinition(fixture.root(), config.set(key, JsonArray())), StarException);
    EXPECT_THROW((void)SpeciesDefinition(fixture.root(), config.set(key, JsonArray{config.get(key).get(0)})), StarException);
  }
});
