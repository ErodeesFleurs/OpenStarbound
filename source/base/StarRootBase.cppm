module;

import std;
import star.memory;
import star.format;

namespace Star {

class Assets;

class Configuration;

struct RootExceptionTag {
  static constexpr char const* name() { return "RootException"; }
};
using RootException = StarError<RootExceptionTag, StarException>;

class RootBase {
public:
  static RootBase* singletonPtr();
  static RootBase& singleton();

  virtual std::shared_ptr<Assets const> assets() = 0;
  virtual std::shared_ptr<Configuration> configuration() = 0;
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
}
