module;

#include "StarInputEvent.hpp"
#include "StarJson.hpp"
import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
#include "StarImage.hpp"
#include "StarRect.hpp"
import star.application_controller;
#include "StarVariant.hpp"
#include "StarPoly.hpp"
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
