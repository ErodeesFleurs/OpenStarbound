#pragma once

#include "StarAssets.hpp"

namespace Star {

class Configuration;
using ConfigurationPtr = SharedPtr<Configuration>;

struct RootExceptionTag { static constexpr char const* typeName = "RootException"; };
using RootException = TypedException<StarException, RootExceptionTag>;

class RootBase {
public:
  [[nodiscard]] virtual AssetsConstPtr assets() = 0;
  [[nodiscard]] virtual ConfigurationPtr configuration() = 0;

protected:
  RootBase();
  ~RootBase();

  static atomic<RootBase*> s_activeRoot;
};

}
