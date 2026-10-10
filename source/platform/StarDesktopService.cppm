module;

#include "StarString.hpp"

namespace Star {

STAR_CLASS(DesktopService);

class DesktopService {
public:
  ~DesktopService() = default;

  virtual void openUrl(String const& url) = 0;
};

}

export module star.desktop_service;

export namespace Star {
  using ::Star::DesktopService;
  using ::Star::DesktopServicePtr;
  using ::Star::DesktopServiceConstPtr;
  using ::Star::DesktopServiceWeakPtr;
  using ::Star::DesktopServiceConstWeakPtr;
  using ::Star::DesktopServiceUPtr;
  using ::Star::DesktopServiceConstUPtr;
}
