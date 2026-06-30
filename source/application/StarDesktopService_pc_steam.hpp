#pragma once

#include "StarPlatformServices_pc.hpp"

namespace Star {

class SteamDesktopService;
using SteamDesktopServicePtr = shared_ptr<SteamDesktopService>;
using SteamDesktopServiceConstPtr = shared_ptr<SteamDesktopService const>;
using SteamDesktopServiceWeakPtr = weak_ptr<SteamDesktopService>;
using SteamDesktopServiceConstWeakPtr = weak_ptr<SteamDesktopService const>;
using SteamDesktopServiceUPtr = unique_ptr<SteamDesktopService>;
using SteamDesktopServiceConstUPtr = unique_ptr<SteamDesktopService const>;

class SteamDesktopService final : public DesktopService {
public:
  SteamDesktopService(PcPlatformServicesStatePtr state);

  void openUrl(String const& url) override;
};

}
