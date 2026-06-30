#include "StarShadersMenu.hpp"

namespace Star {

ShadersMenu::ShadersMenu(Json const& config, UniverseClientPtr client, BaseScriptPaneServices services)
  : BaseScriptPane(config, true, std::move(services)) {
  m_client = std::move(client);
}

void ShadersMenu::show() {
  BaseScriptPane::show();
}

void ShadersMenu::displayed() {
  m_script.setLuaRoot(m_client->luaRoot());
  BaseScriptPane::displayed();
}

void ShadersMenu::dismissed() {
  BaseScriptPane::dismissed();
}

}
