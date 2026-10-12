module;
#include "StarJson.hpp"

import star.input_event;
import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
import star.application_controller;
#include "StarVariant.hpp"
#include "StarBiMap.hpp"
import star.ref_ptr;
import star.renderer;

namespace Star {
  int runMainApplication(ApplicationUPtr application, StringList cmdLineArgs);
}

export module star.main_application;

export namespace Star {
  using ::Star::runMainApplication;
}
