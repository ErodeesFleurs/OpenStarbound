#include "StarRootBase.hpp"

import std;

namespace Star {
std::atomic<RootBase*> RootBase::s_singleton;

auto RootBase::singletonPtr() -> RootBase* {
  return s_singleton.load();
}

auto RootBase::singleton() -> RootBase& {
  auto ptr = s_singleton.load();
  if (!ptr)
    throw RootException("RootBase::singleton() called with no Root instance available");
  else
    return *ptr;
}

RootBase::RootBase() {
  RootBase* oldRoot = nullptr;
  if (!s_singleton.compare_exchange_strong(oldRoot, this))
    throw RootException("Singleton Root has been constructed twice");
}
}// namespace Star
