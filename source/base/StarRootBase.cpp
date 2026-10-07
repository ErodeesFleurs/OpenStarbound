#include "StarJson.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarBiMap.hpp"
#include "StarThread.hpp"
#include "StarIODevice.hpp"
#include "StarList.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
#include "StarRefPtr.hpp"

import star.asset_source;
import star.assets;
import star.root_base;

namespace Star {
  atomic<RootBase*> RootBase::s_singleton;

  RootBase* RootBase::singletonPtr() {
    return s_singleton.load();
  }

  RootBase& RootBase::singleton() {
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
}