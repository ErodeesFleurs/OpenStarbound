#include "StarJson.hpp"
#include "StarLogging.hpp"
#include "StarFile.hpp"
#include "StarString.hpp"
#include "StarVariant.hpp"
#include "StarOrderedMap.hpp"
#include "StarOrderedSet.hpp"
#include "StarConfig.hpp"
import star.version;
#include "StarRect.hpp"
#include "StarBiMap.hpp"
#include "StarThread.hpp"
#include "StarIODevice.hpp"
#include "StarList.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
#include "StarRefPtr.hpp"
import star.listener;

import star.option_parser;
import star.version_option_parser;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.root_loader;

#include "gtest/gtest.h"

using namespace Star;

struct ErrorLogSink : public LogSink {
  ErrorLogSink() {
    setLevel(LogLevel::Error);
  }

  void log(char const* msg, LogLevel) override {
    ADD_FAILURE() << "Error was logged: " << msg;
  }
};

class TestEnvironment : public testing::Environment {
public:
  unique_ptr<Root> root;
  Root::Settings settings;

  TestEnvironment(Root::Settings settings)
    : settings(std::move(settings)) {}

  virtual void SetUp() {
    Logger::addSink(make_shared<ErrorLogSink>());
    root = make_unique<Root>(settings);
    root->configuration()->set("clearUniverseFiles", true);
    root->configuration()->set("clearPlayerFiles", true);
  }

  virtual void TearDown() {
    root.reset();
  }
};

GTEST_API_ int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  testing::AddGlobalTestEnvironment(new TestEnvironment(RootLoader({{}, {}, {}, LogLevel::Error, true, {}}).commandParseOrDie(argc, argv).first));
  return RUN_ALL_TESTS();
}
