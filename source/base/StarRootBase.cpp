#include "StarRootBase.hpp"

namespace Star {
  atomic<RootBase*> RootBase::s_activeRoot;

  RootBase::RootBase() {
    RootBase* oldRoot = nullptr;
    if (!s_activeRoot.compare_exchange_strong(oldRoot, this))
      throw RootException("Root has been constructed twice");
  }

  RootBase::~RootBase() {
    RootBase* activeRoot = this;
    s_activeRoot.compare_exchange_strong(activeRoot, nullptr);
  }
}
