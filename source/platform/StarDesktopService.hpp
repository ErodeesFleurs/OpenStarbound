#pragma once

namespace Star {

class DesktopService;
using DesktopServicePtr = SharedPtr<DesktopService>;

class DesktopService {
public:
  ~DesktopService() = default;

  virtual void openUrl(String const& url) = 0;
};

}
