module;

#include "StarInputEvent.hpp"
import star.application;
#include "StarInputEvent.hpp"
import star.application;
#include "StarStatisticsService.hpp"
#include "StarP2PNetworkingService.hpp"
#include "StarUserGeneratedContentService.hpp"
#include "StarDesktopService.hpp"
#include "StarImage.hpp"
#include "StarRect.hpp"
import star.application_controller;
#include "StarVariant.hpp"
#include "StarImage.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
#include "StarBiMap.hpp"
#include "StarRefPtr.hpp"
import star.renderer;

namespace Star {
  int runMainApplication(ApplicationUPtr application, StringList cmdLineArgs);
}

export module star.main_application;

export namespace Star {
  using ::Star::runMainApplication;
}
