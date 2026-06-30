#pragma once

#include "StarBaseScriptPane.hpp"

namespace Star {

class VoiceSettingsMenu;
using VoiceSettingsMenuPtr = SharedPtr<VoiceSettingsMenu>;

class VoiceSettingsMenu : public BaseScriptPane {
public:
  VoiceSettingsMenu(Json const& config);

  virtual void show() override;
  void displayed() override;
  void dismissed() override;

private:

};

}
