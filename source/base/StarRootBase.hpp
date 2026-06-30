#pragma once

#include "StarAssets.hpp"

namespace Star {

STAR_CLASS(Configuration);

struct RootExceptionTag { static constexpr char const* typeName = "RootException"; };
using RootException = TypedException<StarException, RootExceptionTag>;

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