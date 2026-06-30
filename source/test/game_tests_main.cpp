#include "StarLogging.hpp"
#include "StarFile.hpp"
#include "StarRootLoader.hpp"
#include "StarSignalHandler.hpp"
#include "StarTestRoot.hpp"

#include "gtest/gtest.h"

using namespace Star;

namespace Star {

namespace {
Root* g_testRoot = nullptr;
}

Root& testRoot() {
  if (!g_testRoot)
    throw RootException("testRoot() called with no test Root instance available");
  return *g_testRoot;
}

}

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
    g_testRoot = root.get();
    root->configuration()->set("clearUniverseFiles", true);
    root->configuration()->set("clearPlayerFiles", true);
  }

  virtual void TearDown() {
    g_testRoot = nullptr;
    root.reset();
  }
};

GTEST_API_ int main(int argc, char** argv) {
  SignalHandler signalHandler;
  signalHandler.setHandleFatal(true);

  testing::InitGoogleTest(&argc, argv);
  testing::AddGlobalTestEnvironment(new TestEnvironment(RootLoader({{}, {}, {}, LogLevel::Error, true, {}}).commandParseOrDie(argc, argv).first));
  return RUN_ALL_TESTS();
}
