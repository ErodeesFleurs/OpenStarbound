module;

import std;
import star.string;

namespace Star {

class DesktopService {
public:
  ~DesktopService() = default;

  virtual void openUrl(String const& url) = 0;
};

}

export module star.desktop_service;

export namespace Star {
  using ::Star::DesktopService;
}
