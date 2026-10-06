module;

#include "StarMemory.hpp"
#include "StarException.hpp"

namespace Star {

STAR_CLASS(Assets);

STAR_CLASS(Configuration);

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

  static atomic<RootBase*> s_singleton;
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
