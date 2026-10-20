#include "gtest/gtest.h"
import std;
import star.game_test;
import star.root;
import star.configuration;
import star.logging;

namespace Star {
namespace {

class ErrorLogSink : public LogSink {
public:
  ErrorLogSink() { setLevel(LogLevel::Error); }
  void log(char const* message, LogLevel) override {
    ADD_FAILURE() << "Error was logged: " << message;
  }
};

std::vector<GameTestRegistration const*>& gameTestRegistrations() {
  static std::vector<GameTestRegistration const*> registrations;
  return registrations;
}

}

GameTestEnvironment::GameTestEnvironment(Root::Settings settings)
  : m_settings(std::move(settings)) {}

void GameTestEnvironment::SetUp() {
  m_root = std::make_unique<Root>(m_settings);
  m_root->logger().addSink(std::make_shared<ErrorLogSink>());
  m_root->configuration()->set("clearUniverseFiles", true);
  m_root->configuration()->set("clearPlayerFiles", true);
}

void GameTestEnvironment::TearDown() {
  m_root.reset();
}

GameTest::GameTest(GameTestEnvironment& environment, Body body)
  : m_environment(environment), m_body(body) {}

Root& GameTest::root() const {
  return *m_environment.m_root;
}

Root::Settings const& GameTest::rootSettings() const {
  return m_environment.m_settings;
}

void GameTest::TestBody() {
  m_body(*this);
}

GameTestRegistration::GameTestRegistration(char const* suite, char const* name,
    GameTest::Body body, std::source_location location)
  : m_suite(suite), m_name(name), m_body(body), m_location(location) {
  gameTestRegistrations().push_back(this);
}

void GameTestRegistration::bind(GameTestEnvironment& environment) const {
  testing::RegisterTest(m_suite, m_name, nullptr, nullptr, m_location.file_name(),
      static_cast<int>(m_location.line()), [&environment, body = m_body]() {
        return new GameTest(environment, body);
      });
}

void registerGameTests(GameTestEnvironment& environment) {
  for (auto const* registration : gameTestRegistrations())
    registration->bind(environment);
}

}
