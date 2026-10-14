module;

import std;
import star.memory;
import star.format;

namespace Star {

class Assets;
using AssetsPtr = std::shared_ptr<Assets>;
using AssetsConstPtr = std::shared_ptr<Assets const>;
using AssetsWeakPtr = std::weak_ptr<Assets>;
using AssetsConstWeakPtr = std::weak_ptr<Assets const>;
using AssetsUPtr = std::unique_ptr<Assets>;
using AssetsConstUPtr = std::unique_ptr<Assets const>;

class Configuration;
using ConfigurationPtr = std::shared_ptr<Configuration>;
using ConfigurationConstPtr = std::shared_ptr<Configuration const>;
using ConfigurationWeakPtr = std::weak_ptr<Configuration>;
using ConfigurationConstWeakPtr = std::weak_ptr<Configuration const>;
using ConfigurationUPtr = std::unique_ptr<Configuration>;
using ConfigurationConstUPtr = std::unique_ptr<Configuration const>;

struct RootExceptionTag {
  static constexpr char const* name() { return "RootException"; }
};
using RootException = StarError<RootExceptionTag, StarException>;

class RootBase {
public:
  static RootBase* singletonPtr();
  static RootBase& singleton();

  virtual AssetsConstPtr assets() = 0;
  virtual ConfigurationPtr configuration() = 0;
protected:
  RootBase();

  static std::atomic<RootBase*> s_singleton;
};

}
export module star.root_base;

export namespace Star {
  using ::Star::RootExceptionTag;
  using ::Star::RootException;
  using ::Star::RootBase;
  using ::Star::Configuration;
  using ::Star::ConfigurationPtr;
  using ::Star::ConfigurationConstPtr;
  using ::Star::ConfigurationWeakPtr;
  using ::Star::ConfigurationConstWeakPtr;
  using ::Star::ConfigurationUPtr;
  using ::Star::ConfigurationConstUPtr;
}
