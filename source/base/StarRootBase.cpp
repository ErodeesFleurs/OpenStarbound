#include "StarConfig.hpp"
import star.json;
import star.ordered_map;
import star.bi_map;
import star.thread;
import star.io_device;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.directives;
import star.asset_path;
import star.ref_ptr;

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