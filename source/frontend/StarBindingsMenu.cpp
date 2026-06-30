#include "StarBindingsMenu.hpp"
#include "StarInputLuaBindings.hpp"

namespace Star {

BindingsMenu::BindingsMenu(Json const& config, BaseScriptPaneServices services, Input& input)
    : BaseScriptPane(config, true, std::move(services)) {
  m_script.setLuaRoot(make_shared<LuaRoot>(m_luaRootServices));
  m_script.addCallbacks("input", LuaBindings::makeInputCallbacks(input));
}

}// namespace Star
