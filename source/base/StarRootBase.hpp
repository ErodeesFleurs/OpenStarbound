#pragma once

#include "StarAssets.hpp"
#include "StarConfig.hpp"
#include "StarConfiguration.hpp"
#include "StarException.hpp"

import std;

namespace Star {

using RootException = ExceptionDerived<"RootException">;

class RootBase {
public:
  static auto singletonPtr() -> RootBase*;
  static auto singleton() -> RootBase&;

  virtual auto assets() -> ConstPtr<Assets> = 0;
  virtual auto configuration() -> Ptr<Configuration> = 0;

protected:
  RootBase();

  static std::atomic<RootBase*> s_singleton;
};

}// namespace Star
