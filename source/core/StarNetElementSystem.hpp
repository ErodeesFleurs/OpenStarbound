#pragma once

#include "StarNetElementSyncGroup.hpp"
#include "StarNetElementTop.hpp"

namespace Star {

// Makes a good default top-level NetElement group.
using NetElementTopGroup = NetElementTop<NetElementCallbackGroup>;

}
