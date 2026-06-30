#pragma once

#include "StarBaseScriptPane.hpp"

namespace Star {

class VoiceSettingsMenu;
using VoiceSettingsMenuPtr = SharedPtr<VoiceSettingsMenu>;
class Voice;

class VoiceSettingsMenu : public BaseScriptPane {
public:
  VoiceSettingsMenu(Json const& config, BaseScriptPaneServices services, Voice& voice);

  void show() override;
  void displayed() override;
  void dismissed() override;

private:

};

}
