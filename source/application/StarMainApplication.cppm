module;
import std;
import star.json;
import star.string;

import star.input_event;
import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
import star.application_controller;
import star.variant;
import star.bi_map;
import star.ref_ptr;
import star.renderer;

namespace Star {
  int runMainApplication(std::unique_ptr<Application> application, StringList cmdLineArgs);
}

export module star.main_application;

export namespace Star {
  using ::Star::runMainApplication;
}
