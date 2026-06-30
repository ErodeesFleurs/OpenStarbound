#include "StarVoiceSettingsMenu.hpp"
#include "StarVoiceLuaBindings.hpp"

namespace Star {

VoiceSettingsMenu::VoiceSettingsMenu(Json const& config, BaseScriptPaneServices services, Voice& voice)
  : BaseScriptPane(config, true, std::move(services)) {
  m_script.setLuaRoot(make_shared<LuaRoot>(m_luaRootServices));
  m_script.addCallbacks("voice", LuaBindings::makeVoiceCallbacks(voice));
}

void VoiceSettingsMenu::show() {
  BaseScriptPane::show();
}

void VoiceSettingsMenu::displayed() {
  BaseScriptPane::displayed();
}

void VoiceSettingsMenu::dismissed() {
  BaseScriptPane::dismissed();
}

}
