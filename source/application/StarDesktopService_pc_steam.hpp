#pragma once

#include "StarPlatformServices_pc.hpp"

namespace Star {

class SteamDesktopService;
using SteamDesktopServicePtr = SharedPtr<SteamDesktopService>;
using SteamDesktopServiceConstPtr = SharedPtr<SteamDesktopService const>;
using SteamDesktopServiceWeakPtr = WeakPtr<SteamDesktopService>;
using SteamDesktopServiceConstWeakPtr = WeakPtr<SteamDesktopService const>;
using SteamDesktopServiceUPtr = UniquePtr<SteamDesktopService>;
using SteamDesktopServiceConstUPtr = UniquePtr<SteamDesktopService const>;

class SteamDesktopService final : public DesktopService {
public:
  SteamDesktopService(PcPlatformServicesStatePtr state);

  void openUrl(String const& url) override;
};

}
