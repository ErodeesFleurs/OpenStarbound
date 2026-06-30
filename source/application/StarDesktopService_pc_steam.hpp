#pragma once

#include "StarPlatformServices_pc.hpp"

namespace Star {

class SteamDesktopService final : public DesktopService {
public:
  SteamDesktopService(PcPlatformServicesStatePtr state);

  void openUrl(String const& url) override;
};

}
