#pragma once

#include "StarBaseScriptPane.hpp"

namespace Star {

class BindingsMenu;
using BindingsMenuPtr = SharedPtr<BindingsMenu>;

class BindingsMenu : public BaseScriptPane {
public:
  BindingsMenu(Json const& config);

  virtual void show() override;
  void displayed() override;
  void dismissed() override;

private:

};

}
