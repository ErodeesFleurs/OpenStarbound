#pragma once

#include "StarBaseScriptPane.hpp"
#include "StarUniverseClient.hpp"

namespace Star {

class ShadersMenu;
using ShadersMenuPtr = SharedPtr<ShadersMenu>;

class ShadersMenu : public BaseScriptPane {
public:
  ShadersMenu(Json const& config, UniverseClientPtr client, BaseScriptPaneServices services);

  void displayed() override;

private:
  UniverseClientPtr m_client;
};

}// namespace Star
