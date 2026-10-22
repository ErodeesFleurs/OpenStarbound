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
  [[nodiscard]] virtual std::expected<std::shared_ptr<Assets const>, StarException> assets() = 0;
  [[nodiscard]] virtual std::expected<std::shared_ptr<Configuration>, StarException> configuration() = 0;
  virtual ~RootBase() = default;
protected:
  RootBase() = default;
};

}
export module star.root_base;

export namespace Star {
  using ::Star::RootExceptionTag;
  using ::Star::RootException;
  using ::Star::RootBase;
  using ::Star::Configuration;
}
