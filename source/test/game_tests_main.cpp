import star.json;
import star.logging;
import star.file;
import star.string;
import star.variant;
import star.ordered_map;
import star.ordered_set;
import std;
import star.version;
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
  std::unique_ptr<Root> root;
  Root::Settings settings;

  TestEnvironment(Root::Settings settings)
    : settings(std::move(settings)) {}

  virtual void SetUp() {
    Logger::addSink(std::make_shared<ErrorLogSink>());
    root = std::make_unique<Root>(settings);
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
