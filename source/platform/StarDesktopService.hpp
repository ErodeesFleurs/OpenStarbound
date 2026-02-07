#pragma once

#include "StarString.hpp"

namespace Star {
class DesktopService {
public:
  ~DesktopService() = default;

  virtual void openUrl(String const& url) = 0;
};

}
