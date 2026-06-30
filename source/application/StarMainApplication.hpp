#pragma once

#include "StarApplication.hpp"
#include "StarApplicationController.hpp"
#include "StarRenderer.hpp"

namespace Star {
  int runMainApplication(UniquePtr<Application> application, StringList cmdLineArgs);
}
