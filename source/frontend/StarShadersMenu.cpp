#include "StarShadersMenu.hpp"
#include "StarAlgorithm.hpp"

namespace Star {

ShadersMenu::ShadersMenu(Json const& config, UniverseClientPtr client, BaseScriptPaneServices services)
    : BaseScriptPane(config, true, std::move(services)), m_client(requireServiceValueAs<StarException>(std::move(client), "ShadersMenu", "universe client")) {}

void ShadersMenu::displayed() {
  m_script.setLuaRoot(m_client->luaRoot());
  BaseScriptPane::displayed();
}

}// namespace Star
