module;

import std;
import star.string;

namespace Star {

class DesktopService;
using DesktopServicePtr = std::shared_ptr<DesktopService>;
using DesktopServiceConstPtr = std::shared_ptr<DesktopService const>;
using DesktopServiceWeakPtr = std::weak_ptr<DesktopService>;
using DesktopServiceConstWeakPtr = std::weak_ptr<DesktopService const>;
using DesktopServiceUPtr = std::unique_ptr<DesktopService>;
using DesktopServiceConstUPtr = std::unique_ptr<DesktopService const>;

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
