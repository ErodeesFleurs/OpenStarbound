#pragma once

#include "StarBaseScriptPane.hpp"

namespace Star {

class Input;
class BindingsMenu;
using BindingsMenuPtr = SharedPtr<BindingsMenu>;

class BindingsMenu : public BaseScriptPane {
public:
  BindingsMenu(Json const& config, BaseScriptPaneServices services, Input& input);

  void show() override;
  void displayed() override;
  void dismissed() override;

private:

};

}
