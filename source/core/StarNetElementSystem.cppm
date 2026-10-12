module;

import star.net_element_top;
import star.net_element_sync_group;

namespace Star {

// Makes a good default top-level NetElement group.
typedef NetElementTop<NetElementCallbackGroup> NetElementTopGroup;

}

export module star.net_element_system;

export namespace Star {
  using ::Star::NetElementTopGroup;
}
